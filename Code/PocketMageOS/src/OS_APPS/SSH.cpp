// SSH client for the Terminal app.
// Local shell commands (wifi*) and a full-screen SSH session backed by
// LibSSH-ESP32 (libssh port). The session runs on a dedicated FreeRTOS task
// because libssh needs a large stack for key exchange; the e-ink screen
// buffer is shared with the eink handler under a mutex.
#include <globals.h>

#if PM_TARGET_HOST // PocketMage OS Only

#include <libssh_esp32.h>
#include <libssh/libssh.h>
#include <libssh/channels.h>

#include <pocketmage_wifi/pocketmage_wifi.h>

// GEOMETRY
static const int SSH_ROWS = TERM_SMALL_LINES;        // 17 rows fit in small font
static const int SSH_COLS_MAX = 64;                  // physical buffer width
static int sshCols = 0;                              // logical width, from font metrics

// Minimum interval between e-ink refreshes.  Keystroke echo arrives through
// the remote shell, so repainting on every dirty flag would flash the e-ink
static const uint32_t SSH_EINK_MIN_REFRESH_MS = 700;

static const char* kSshTag = "SSH";

// SCREEN CELL
struct SshCell {
  char ch;
  uint8_t attr;   // bit 0: reverse video
};

// THREADING
static TaskHandle_t sshTaskHandle = nullptr;
static QueueHandle_t sshTxQueue = nullptr;       // main -> worker: 1 byte per item
static QueueHandle_t sshPromptQueue = nullptr;   // worker -> main: prompt requests
static QueueHandle_t sshAnswerQueue = nullptr;   // main -> worker: prompt answers
static SemaphoreHandle_t sshScreenMutex = nullptr;

static volatile bool sshBusyFlag = false;        // worker connecting or active
static volatile bool sshDisconnectReq = false;   // main requests disconnect
static volatile bool sshScreenDirty = false;     // worker changed the screen
static uint32_t sshLastEinkRefresh = 0;          // millis() of last e-ink refresh
static String sshExitMsg = "";                   // main-loop owned exit message
static String sshStatus = "";                    // OLED session status line
static String sshBaseStatus = "";                // "ssh user@host", restored after prompts
static String sshOledInput = "";                 // OLED local input line

// PROMPT PROTOCOL
enum SshPromptKind {
  SSH_PROMPT_HOSTKEY,
  SSH_PROMPT_USERNAME,
  SSH_PROMPT_PASSWORD
};

struct SshPrompt {
  SshPromptKind kind;
  char text[96];
};

struct SshAnswer {
  char text[64];
};

// VT100 STATE
static SshCell sshScreen[SSH_ROWS][SSH_COLS_MAX];
static SshCell sshAltScreen[SSH_ROWS][SSH_COLS_MAX];
static SshCell sshSnap[SSH_ROWS][SSH_COLS_MAX];
static SshCell sshLastPainted[SSH_ROWS][SSH_COLS_MAX];
static int sshAltSaveR = 0;   // cursor saved by DECSET 1049h; restored on 1049l.
static int sshAltSaveC = 0;   // Separate from vt->saveR/saveC so the app's own
                              // DECSC/DECRC during the alt-screen session is
                              // not confused with the main-screen cursor.

struct Vt {
  SshCell (*screen)[SSH_COLS_MAX];
  int rows, cols;
  int r, c;
  int saveR, saveC;
  bool wrap, insert;
  int top, bottom;          // scroll region, inclusive rows
  bool reverse;             // current SGR reverse state
  char lastChar;
  // parser
  int state;                // 0 normal, 1 esc, 2 csi, 3 osc, 4 charset
  char paramBuf[24];
  int paramLen;
  bool priv;
  int params[4];
  int paramCount;
  char oscBuf[64];
  int oscLen;
  bool dirty;
};

static Vt sshVt;   // worker-owned terminal state

static void vtClearRow(Vt* vt, int row, int col, int mode) {
  // EL/ED with no parameter means mode 0 (erase from the cursor).  zsh's
  // prompt redraw emits "\x1b[J" and "\x1b[K" without parameters; treating
  // those as a full erase wiped command output on every prompt.
  if (mode < 0) mode = 0;
  if (mode == 0) {
    for (int x = col; x < vt->cols; x++) { vt->screen[row][x].ch = ' '; vt->screen[row][x].attr = 0; }
  } else if (mode == 1) {
    for (int x = 0; x <= col; x++) { vt->screen[row][x].ch = ' '; vt->screen[row][x].attr = 0; }
  } else {
    for (int x = 0; x < vt->cols; x++) { vt->screen[row][x].ch = ' '; vt->screen[row][x].attr = 0; }
  }
  vt->dirty = true;
}

static void vtScrollUp(Vt* vt, int n) {
  n = constrain(n, 0, vt->bottom - vt->top + 1);
  if (n <= 0) return;
  for (int row = vt->top; row <= vt->bottom - n; row++)
    memcpy(vt->screen[row], vt->screen[row + n], sizeof(SshCell) * vt->cols);
  for (int row = vt->bottom - n + 1; row <= vt->bottom; row++)
    memset(vt->screen[row], 0, sizeof(SshCell) * vt->cols);
  vt->dirty = true;
}

static void vtScrollDown(Vt* vt, int n) {
  n = constrain(n, 0, vt->bottom - vt->top + 1);
  if (n <= 0) return;
  for (int row = vt->bottom; row >= vt->top + n; row--)
    memcpy(vt->screen[row], vt->screen[row - n], sizeof(SshCell) * vt->cols);
  for (int row = vt->top; row < vt->top + n; row++)
    memset(vt->screen[row], 0, sizeof(SshCell) * vt->cols);
  vt->dirty = true;
}

static void vtNewline(Vt* vt) {
  if (vt->r == vt->bottom) {
    vtScrollUp(vt, 1);
  } else if (vt->r < vt->rows - 1) {
    vt->r++;
  }
}

static void vtWriteChar(Vt* vt, char ch) {
  if (vt->insert) {
    for (int x = vt->cols - 1; x > vt->c; x--) vt->screen[vt->r][x] = vt->screen[vt->r][x - 1];
    vt->screen[vt->r][vt->c].ch = ' ';
    vt->screen[vt->r][vt->c].attr = 0;
    vt->dirty = true;
  }
  if (vt->c >= vt->cols) vt->c = vt->cols - 1;
  vt->screen[vt->r][vt->c].ch = ch;
  vt->screen[vt->r][vt->c].attr = vt->reverse ? 1 : 0;
  vt->lastChar = ch;
  vt->dirty = true;
  vt->c++;
  if (vt->c >= vt->cols) {
    if (vt->wrap) {
      vt->c = 0;
      vtNewline(vt);
    } else {
      vt->c = vt->cols - 1;
    }
  }
}

static void vtEraseDisplay(Vt* vt, int mode) {
  if (mode < 0) mode = 0;   // bare "\x1b[J" erases below the cursor only
  if (mode == 0) {
    vtClearRow(vt, vt->r, vt->c, 0);
    for (int row = vt->r + 1; row < vt->rows; row++)
      memset(vt->screen[row], 0, sizeof(SshCell) * vt->cols);
  } else if (mode == 1) {
    vtClearRow(vt, vt->r, vt->c, 1);
    for (int row = 0; row < vt->r; row++)
      memset(vt->screen[row], 0, sizeof(SshCell) * vt->cols);
  } else {
    for (int row = 0; row < vt->rows; row++)
      memset(vt->screen[row], 0, sizeof(SshCell) * vt->cols);
  }
  vt->dirty = true;
}

static void vtSaveCursor(Vt* vt) {
  vt->saveR = vt->r;
  vt->saveC = vt->c;
}

static void vtRestoreCursor(Vt* vt) {
  vt->r = constrain(vt->saveR, 0, vt->rows - 1);
  vt->c = constrain(vt->saveC, 0, vt->cols - 1);
}

static void vtReset(Vt* vt) {
  vt->r = 0;
  vt->c = 0;
  vt->saveR = 0;
  vt->saveC = 0;
  vt->wrap = true;
  vt->insert = false;
  vt->reverse = false;
  vt->top = 0;
  vt->bottom = vt->rows - 1;
  vt->state = 0;
  vt->paramLen = 0;
  vt->oscLen = 0;
  for (int row = 0; row < vt->rows; row++)
    memset(vt->screen[row], 0, sizeof(SshCell) * vt->cols);
  vt->dirty = true;
}

static int vtParam(Vt* vt, int idx, int def) {
  if (idx < vt->paramCount) return vt->params[idx];
  return def;
}

// Session globals (worker context). Declared here so the VT100 parser's write
// callback can reach the channel.
static String sshHost, sshUser;
static int sshPort;
static ssh_session sshSession;
static ssh_channel sshChannel;

// Writes session bytes over the channel (worker context).
static int sshWrite(const char* data, size_t len) {
  if (!sshChannel) return -1;
  size_t sent = 0;
  while (sent < len) {
    int n = ssh_channel_write(sshChannel, data + sent, len - sent);
    if (n <= 0) return -1;
    sent += n;
  }
  return (int)sent;
}

// Parses one byte of remote output; responses are written back over the channel.
static void vtFeed(Vt* vt, char ch, int (*writeFn)(const char*, size_t)) {
  switch (vt->state) {
    case 1: // ESC
      if (ch == '[') {
        vt->state = 2;
        vt->paramLen = 0;
        vt->paramCount = 0;
        vt->priv = false;
        return;
      }
      if (ch == ']') {
        vt->state = 3;
        vt->oscLen = 0;
        return;
      }
      if (ch == '(' || ch == ')' || ch == '*' || ch == '+') {
        vt->state = 4;   // charset select: consume next byte
        return;
      }
      vt->state = 0;
      switch (ch) {
        case '7': vtSaveCursor(vt); return;
        case '8': vtRestoreCursor(vt); return;
        case 'c': vtReset(vt); return;
        case 'D':
          if (vt->r == vt->bottom) vtScrollUp(vt, 1); else if (vt->r < vt->rows - 1) vt->r++;
          return;
        case 'M':
          if (vt->r == vt->top) vtScrollDown(vt, 1); else if (vt->r > 0) vt->r--;
          return;
        case 'E':
          vt->c = 0;
          vtNewline(vt);
          return;
        case 'H': case '=': case '>': case 'Z': return;  // tab set / keypad modes: ignore
        default: return;
      }
    case 2: // CSI
      if (ch >= 0x40 && ch <= 0x7e) {
        vt->state = 0;
        vt->paramCount = 0;
        int value = 0;
        bool hasDigit = false;
        for (int i = 0; i < vt->paramLen; i++) {
          char p = vt->paramBuf[i];
          if (p >= '0' && p <= '9') {
            value = value * 10 + (p - '0');
            hasDigit = true;
          } else if (p == ';') {
            vt->params[vt->paramCount++] = hasDigit ? value : -1;
            value = 0;
            hasDigit = false;
            if (vt->paramCount >= 4) break;
          } else if (p == '?') {
            vt->priv = true;
          }
        }
        if (vt->paramCount < 4)
          vt->params[vt->paramCount++] = hasDigit ? value : -1;

        if (ch == 'm') { // SGR: reverse is the only relevant attribute
          if (vt->paramCount == 1 && vt->params[0] == -1) vt->params[0] = 0;
          for (int i = 0; i < vt->paramCount; i++) {
            int p = vt->params[i];
            if (p == 0 || p == 27) vt->reverse = false;
            else if (p == 7) vt->reverse = true;
          }
          return;
        }
        if (ch == 'J') {
          ESP_LOGI(kSshTag, "seq eraseDisplay param='%.*s' cursor r=%d c=%d",
                   vt->paramLen, vt->paramBuf, vt->r, vt->c);
          vtEraseDisplay(vt, vtParam(vt, 0, 0));
          return;
        }
        if (ch == 'K') { vtClearRow(vt, vt->r, vt->c, vtParam(vt, 0, 0)); return; }
        if (ch == 'H' || ch == 'f') {
          int row = vtParam(vt, 0, 1) - 1;
          int col = vtParam(vt, 1, 1) - 1;
          if (row < 0) row = 0;
          if (col < 0) col = 0;
          vt->r = constrain(vt->top + row, vt->top, vt->bottom);
          vt->c = constrain(col, 0, vt->cols - 1);
          return;
        }
        if (ch == 'A') { vt->r = constrain(vt->r - vtParam(vt, 0, 1), vt->top, vt->bottom); return; }
        if (ch == 'B') { vt->r = constrain(vt->r + vtParam(vt, 0, 1), vt->top, vt->bottom); return; }
        if (ch == 'C') { vt->c = constrain(vt->c + vtParam(vt, 0, 1), 0, vt->cols - 1); return; }
        if (ch == 'D') { vt->c = constrain(vt->c - vtParam(vt, 0, 1), 0, vt->cols - 1); return; }
        if (ch == 'E') { vt->c = 0; for (int i = 0; i < vtParam(vt, 0, 1); i++) vtNewline(vt); return; }
        if (ch == 'F') { vt->c = 0; vt->r = constrain(vt->r - vtParam(vt, 0, 1), vt->top, vt->bottom); return; }
        if (ch == 'G') { vt->c = constrain(vtParam(vt, 0, 1) - 1, 0, vt->cols - 1); return; }
        if (ch == 'd') { vt->r = constrain(vtParam(vt, 0, 1) - 1, vt->top, vt->bottom); return; }
        if (ch == '@') { // insert chars
          int n = vtParam(vt, 0, 1);
          for (int x = vt->cols - 1; x >= vt->c + n; x--) vt->screen[vt->r][x] = vt->screen[vt->r][x - n];
          for (int x = vt->c; x < vt->cols && x < vt->c + n; x++) { vt->screen[vt->r][x].ch = ' '; vt->screen[vt->r][x].attr = 0; }
          vt->dirty = true;
          return;
        }
        if (ch == 'P') { // delete chars
          int n = vtParam(vt, 0, 1);
          for (int x = vt->c; x + n < vt->cols; x++) vt->screen[vt->r][x] = vt->screen[vt->r][x + n];
          for (int x = vt->cols - n; x < vt->cols; x++) { vt->screen[vt->r][x].ch = ' '; vt->screen[vt->r][x].attr = 0; }
          vt->dirty = true;
          return;
        }
        if (ch == 'X') { // erase chars
          int n = vtParam(vt, 0, 1);
          for (int x = vt->c; x < vt->cols && x < vt->c + n; x++) { vt->screen[vt->r][x].ch = ' '; vt->screen[vt->r][x].attr = 0; }
          vt->dirty = true;
          return;
        }
        if (ch == 'L') { vtScrollDown(vt, vtParam(vt, 0, 1)); return; }
        if (ch == 'M') { vtScrollUp(vt, vtParam(vt, 0, 1)); return; }
        if (ch == 'S') { vtScrollUp(vt, vtParam(vt, 0, 1)); return; }
        if (ch == 'T') { vtScrollDown(vt, vtParam(vt, 0, 1)); return; }
        if (ch == 'r') {
          int top = vtParam(vt, 0, 1);
          int bottom = vtParam(vt, 1, 1);
          if (top == -1 && vt->paramCount == 1) {
            vt->top = 0;
            vt->bottom = vt->rows - 1;
          } else {
            vt->top = constrain(top - 1, 0, vt->rows - 1);
            vt->bottom = constrain(bottom - 1, vt->top, vt->rows - 1);
          }
          vt->r = vt->top;
          vt->c = 0;
          return;
        }
        if (ch == 's') { vtSaveCursor(vt); return; }
        if (ch == 'u') { vtRestoreCursor(vt); return; }
        if (ch == 'b') { // repeat last char
          int n = vtParam(vt, 0, 1);
          for (int i = 0; i < n; i++) vtWriteChar(vt, vt->lastChar);
          return;
        }
        if (ch == 'h' || ch == 'l') { // modes
          int mode = vtParam(vt, 0, 1);
          bool on = (ch == 'h');
          if (vt->priv) {
            if (mode == 7) {
              vt->wrap = on;
            } else if (mode == 1047 || mode == 1049) {
              ESP_LOGI(kSshTag, "seq altScreen %s mode=%d cursor r=%d c=%d", on ? "on" : "off", mode, vt->r, vt->c);
              if (on && vt->screen != sshAltScreen) {
                memcpy(sshAltScreen, vt->screen, sizeof(SshCell) * vt->rows * vt->cols);
                if (mode == 1049) {
                  sshAltSaveR = vt->r;
                  sshAltSaveC = vt->c;
                }
                vt->screen = sshAltScreen;
                vtReset(vt);
              } else if (!on && vt->screen == sshAltScreen) {
                vt->screen = sshScreen;
                // Restore the saved main screen instead of clearing it; a
                // reset here blanks the display and leaves the shell prompt
                // redrawn at the stale cursor row after the app exits.
                memcpy(vt->screen, sshAltScreen, sizeof(SshCell) * vt->rows * vt->cols);
                if (mode == 1049) {
                  vt->r = constrain(sshAltSaveR, 0, vt->rows - 1);
                  vt->c = constrain(sshAltSaveC, 0, vt->cols - 1);
                }
              }
            } else if (mode == 1048) {
              if (on) vtSaveCursor(vt); else vtRestoreCursor(vt);
            }
          } else {
            if (mode == 4) vt->insert = on;
          }
          return;
        }
        if (ch == 'n') {
          if (vtParam(vt, 0, 1) == 6 && writeFn) {
            char buf[24];
            int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dR", vt->r + 1, vt->c + 1);
            writeFn(buf, len);
          }
          return;
        }
        if (ch == 'c') {
          if (writeFn) writeFn("\x1b[?1;2c", 8);
          return;
        }
        return; // unhandled CSI: ignore
      }
      if (ch >= 0x20 && vt->paramLen < (int)sizeof(vt->paramBuf) - 1)
        vt->paramBuf[vt->paramLen++] = ch;
      return;
    case 3: // OSC: discard until BEL or ESC backslash
      if (ch == 0x07) {
        vt->state = 0;
      } else if (ch == 0x1b) {
        vt->state = 1;
      } else if (vt->oscLen < (int)sizeof(vt->oscBuf) - 1) {
        vt->oscBuf[vt->oscLen++] = ch;
      }
      return;
    case 4: // charset select: ignore the selected charset byte
      vt->state = 0;
      return;
    default:
      vt->state = 0;
      break;
  }

  // NORMAL state
  if (ch == 0x1b) {
    vt->state = 1;
    return;
  }
  if (ch == 0x0d) { vt->c = 0; return; }
  if (ch == 0x0a || ch == 0x0b) { vtNewline(vt); return; }
  if (ch == 0x0c) { vtEraseDisplay(vt, 2); vt->r = 0; vt->c = 0; return; }
  if (ch == 0x08) { if (vt->c > 0) vt->c--; return; }
  if (ch == 0x09) {
    int next = (vt->c / 8 + 1) * 8;
    vt->c = next < vt->cols ? next : vt->cols - 1;
    return;
  }
  if (ch == 0x07) return;  // bell: no buzzer on e-ink
  if (ch < 0x20 || ch == 0x7f) return;
  vtWriteChar(vt, ch);
}

// Writes a progress/diagnostic line into the terminal screen (worker context).
static void sshFeedLine(const char* text) {
  Vt* vt = &sshVt;
  vtFeed(vt, '\r', nullptr);
  for (const char* p = text; *p; p++) vtFeed(vt, *p, nullptr);
  vtFeed(vt, '\n', nullptr);
  sshScreenDirty = true;
}

// KNOWN HOSTS (TOFU)
enum KnownHostState { KH_OK, KH_NEW, KH_CHANGED, KH_ERROR };

static void hashToHex(const unsigned char* hash, size_t hlen, char* out, size_t outLen) {
  const char hex[] = "0123456789abcdef";
  size_t pos = 0;
  for (size_t i = 0; i < hlen && pos + 2 < outLen; i++) {
    out[pos++] = hex[hash[i] >> 4];
    out[pos++] = hex[hash[i] & 0x0f];
  }
  out[pos] = 0;
}

static int knownHostCheck(const String& hostPort, const unsigned char* hash, size_t hlen) {
  if (!global_fs || !global_fs->exists("/sys/known_hosts")) return KH_NEW;

  File f = global_fs->open("/sys/known_hosts", FILE_READ);
  if (!f) return KH_NEW;

  char expect[80];
  hashToHex(hash, hlen, expect, sizeof(expect));

  int result = KH_NEW;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;
    int sp = line.indexOf(' ');
    if (sp < 0) continue;
    if (line.substring(0, sp) != hostPort) continue;
    result = (line.substring(sp + 1) == expect) ? KH_OK : KH_CHANGED;
    break;
  }
  f.close();
  return result;
}

static bool knownHostSave(const String& hostPort, const unsigned char* hash, size_t hlen) {
  if (!global_fs) return false;
  if (!global_fs->exists("/sys")) global_fs->mkdir("/sys");

  char hex[80];
  hashToHex(hash, hlen, hex, sizeof(hex));

  String newContent = "";
  if (global_fs->exists("/sys/known_hosts")) {
    File f = global_fs->open("/sys/known_hosts", FILE_READ);
    if (f) {
      while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        int sp = line.indexOf(' ');
        if (sp < 0 || line.substring(0, sp) != hostPort) newContent += line + "\n";
      }
      f.close();
    }
  }
  newContent += hostPort + " " + hex + "\n";

  File w = global_fs->open("/sys/known_hosts", FILE_WRITE);
  if (!w) return false;
  w.print(newContent);
  w.close();
  return true;
}

// SSH SESSION
// Blocking prompt from the worker: requests UI input from the main loop and
// waits for the answer. Returns false if the user cancels.
static bool sshAskPrompt(SshPromptKind kind, const char* text, char* answer, size_t answerLen) {
  SshPrompt p = {};
  p.kind = kind;
  strncpy(p.text, text, sizeof(p.text) - 1);
  if (xQueueSend(sshPromptQueue, &p, pdMS_TO_TICKS(100)) != pdTRUE) return false;
  SshAnswer a = {};
  if (xQueueReceive(sshAnswerQueue, &a, portMAX_DELAY) != pdTRUE) return false;
  if (strcmp(a.text, "_CANCEL_") == 0) return false;
  strncpy(answer, a.text, answerLen - 1);
  answer[answerLen - 1] = 0;
  return true;
}

static bool sshHostKeyAccept(const unsigned char* hash, size_t hlen) {
  char hex[80];
  hashToHex(hash, hlen, hex, sizeof(hex));
  int half = strlen(hex) / 2;
  sshFeedLine(TR(STR_SSH_FINGERPRINT));
  sshFeedLine(String(hex).substring(0, half).c_str());
  sshFeedLine(String(hex).substring(half).c_str());
  char answer[8] = {0};
  if (!sshAskPrompt(SSH_PROMPT_HOSTKEY, TR(STR_SSH_TRUST_HOST_KEY), answer, sizeof(answer))) return false;
  return answer[0] == 'y' || answer[0] == 'Y';
}

static int sshVerifyKnownHost(const String& hostPort, ssh_key serverKey) {
  unsigned char* hash = NULL;
  size_t hlen = 0;
  if (ssh_get_publickey_hash(serverKey, SSH_PUBLICKEY_HASH_SHA256, &hash, &hlen) < 0) return KH_ERROR;

  int state = knownHostCheck(hostPort, hash, hlen);
  if (state == KH_NEW) {
    sshFeedLine(TR(STR_SSH_FIRST_CONNECTION));
    if (sshHostKeyAccept(hash, hlen)) {
      if (!knownHostSave(hostPort, hash, hlen)) state = KH_ERROR;
    } else {
      sshFeedLine(TR(STR_SSH_HOST_KEY_NOT_TRUSTED));
      state = KH_ERROR;
    }
  } else if (state == KH_CHANGED) {
    sshFeedLine(TR(STR_SSH_HOST_KEY_CHANGED));
    sshFeedLine(TR(STR_SSH_HOST_KEY_MITM));
  }
  ssh_clean_pubkey_hash(&hash);
  return state;
}

static bool sshAuthenticate(const char* password, const char* username) {
  int rc = ssh_userauth_none(sshSession, NULL);
  if (rc == SSH_AUTH_ERROR) return false;
  int method = ssh_userauth_list(sshSession, NULL);

  if (method & SSH_AUTH_METHOD_PASSWORD) {
    rc = ssh_userauth_password(sshSession, username, password);
    if (rc == SSH_AUTH_SUCCESS) return true;
    if (rc == SSH_AUTH_ERROR) return false;
  }
  if (method & SSH_AUTH_METHOD_INTERACTIVE) {
    rc = ssh_userauth_kbdint(sshSession, username, NULL);
    while (rc == SSH_AUTH_INFO) {
      int n = ssh_userauth_kbdint_getnprompts(sshSession);
      for (int i = 0; i < n; i++) {
        char echo = 0;
        const char* prompt = ssh_userauth_kbdint_getprompt(sshSession, i, &echo);
        const char* answer = (prompt && !echo && password[0]) ? password : "";
        ssh_userauth_kbdint_setanswer(sshSession, i, answer);
      }
      rc = ssh_userauth_kbdint(sshSession, username, NULL);
      if (rc == SSH_AUTH_ERROR) return false;
    }
    if (rc == SSH_AUTH_SUCCESS) return true;
  }
  return false;
}

static void sshTask(void* p) {
  (void)p;

  sshBusyFlag = true;
  sshDisconnectReq = false;

  Vt* vt = &sshVt;
  memset(vt, 0, sizeof(*vt));
  vt->screen = sshScreen;
  vt->rows = SSH_ROWS;
  vt->cols = sshCols ? sshCols : 40;
  vt->top = 0;
  vt->bottom = vt->rows - 1;
  vt->wrap = true;
  vtReset(vt);

  static bool libsshInited = false;
  if (!libsshInited) {
    libssh_begin();
    libsshInited = true;
  }

  sshSession = ssh_new();
  if (!sshSession) {
    sshExitMsg = TR(STR_SSH_NO_MEM);
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }

  long port = sshPort;
  int verbosity = 0;
  long timeout = 15000;
  ssh_options_set(sshSession, SSH_OPTIONS_HOST, sshHost.c_str());
  if (sshUser.length() > 0) ssh_options_set(sshSession, SSH_OPTIONS_USER, sshUser.c_str());
  ssh_options_set(sshSession, SSH_OPTIONS_PORT, &port);
  ssh_options_set(sshSession, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
  ssh_options_set(sshSession, SSH_OPTIONS_TIMEOUT, &timeout);

  sshFeedLine((String(TR(STR_SSH_CONNECTING_PREFIX)) + sshHost + ":" + String(sshPort) + "...").c_str());

  if (ssh_connect(sshSession) != SSH_OK) {
    sshExitMsg = String(TR(STR_SSH_CONNECT_FAILED_PREFIX)) + String(ssh_get_error(sshSession));
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }
  ESP_LOGI(kSshTag, "connected to %s:%d", sshHost.c_str(), sshPort);

  if (sshDisconnectReq) {
    sshExitMsg = TR(STR_SSH_DISCONNECTED);
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }

  // Host key verification (TOFU).
  ssh_key serverKey = NULL;
  if (ssh_get_server_publickey(sshSession, &serverKey) < 0) {
    sshExitMsg = TR(STR_SSH_HOST_KEY_FAIL);
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }
  String hostPort = sshHost + ":" + String(sshPort);
  int kh = sshVerifyKnownHost(hostPort, serverKey);
  ssh_key_free(serverKey);
  if (kh != KH_OK) {
    if (sshExitMsg.length() == 0) sshExitMsg = TR(STR_SSH_HOST_KEY_VERIFY_FAIL);
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }

  if (sshDisconnectReq) {
    sshExitMsg = TR(STR_SSH_DISCONNECTED);
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }

  // Username prompt if not given on the command line.
  if (sshUser.length() == 0) {
    char uname[64] = {0};
    sshFeedLine((String(TR(STR_SSH_LOGIN_AS)) + " ").c_str());
    if (!sshAskPrompt(SSH_PROMPT_USERNAME, TR(STR_SSH_LOGIN_AS), uname, sizeof(uname)) || strlen(uname) == 0) {
      sshExitMsg = TR(STR_SSH_LOGIN_CANCELLED);
      ssh_disconnect(sshSession);
      ssh_free(sshSession);
      sshSession = nullptr;
      sshBusyFlag = false;
      vTaskDelete(NULL);
      return;
    }
    sshUser = uname;
    ssh_options_set(sshSession, SSH_OPTIONS_USER, sshUser.c_str());
  }

  // Password.
  char password[65] = {0};
  if (!sshAskPrompt(SSH_PROMPT_PASSWORD, (String(TR(STR_SSH_PASSWORD_FOR_PREFIX)) + sshUser + "@" + sshHost).c_str(), password, sizeof(password))) {
    sshExitMsg = TR(STR_SSH_LOGIN_CANCELLED);
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }

  sshFeedLine(TR(STR_SSH_AUTHENTICATING));
  if (!sshAuthenticate(password, sshUser.c_str())) {
    sshExitMsg = String(TR(STR_SSH_AUTH_FAILED_PREFIX)) + String(ssh_get_error(sshSession));
    memset(password, 0, sizeof(password));
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }
  memset(password, 0, sizeof(password));
  ESP_LOGI(kSshTag, "authenticated as %s", sshUser.c_str());

  sshChannel = ssh_channel_new(sshSession);
  if (!sshChannel) {
    sshExitMsg = TR(STR_SSH_CHANNEL_FAIL);
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }
  if (ssh_channel_open_session(sshChannel) != SSH_OK ||
      ssh_channel_request_pty_size(sshChannel, "xterm", vt->cols, vt->rows) != SSH_OK ||
      ssh_channel_request_shell(sshChannel) != SSH_OK) {
    sshExitMsg = String(TR(STR_SSH_SHELL_FAIL_PREFIX)) + String(ssh_get_error(sshSession));
    ssh_channel_close(sshChannel);
    ssh_channel_free(sshChannel);
    sshChannel = nullptr;
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
    sshBusyFlag = false;
    vTaskDelete(NULL);
    return;
  }
  ESP_LOGI(kSshTag, "shell started pty=%dx%d", vt->cols, vt->rows);

  vtReset(vt);
  sshScreenDirty = true;

  // Session loop
  char rxBuf[256];
  int lastFedBatch = 0;
  while (!sshDisconnectReq) {
    // Drain the keyboard queue into the channel.  Peek before committing so a
    // write only happens when the channel can take it right away.  A burst of
    // keys must never block here: libssh's ssh_channel_write waits up to its
    // 30s default timeout when the remote window is exhausted, and the worker
    // owns screen updates, so a stuck write freezes the terminal.
    char b;
    while (xQueuePeek(sshTxQueue, &b, 0) == pdTRUE) {
      if (sshDisconnectReq) break;
      if (!sshChannel || !ssh_channel_is_open(sshChannel)) { sshDisconnectReq = true; break; }
      if (sshChannel->remote_window == 0) break;   // backpressure: leave queued, retry next tick
      xQueueReceive(sshTxQueue, &b, 0);
      if (ssh_channel_write(sshChannel, &b, 1) <= 0) { sshDisconnectReq = true; break; }
    }
    if (sshDisconnectReq) break;

    int avail = ssh_channel_poll(sshChannel, 0);
    int errAvail = ssh_channel_poll(sshChannel, 1);
    if (avail < 0 || errAvail < 0) break;

    bool any = false;
    int fed = 0;
    for (int pass = 0; pass < 2; pass++) {
      int n = (pass == 0) ? avail : errAvail;
      while (n > 0) {
        int count = n > (int)sizeof(rxBuf) ? (int)sizeof(rxBuf) : n;
        int r = ssh_channel_read_nonblocking(sshChannel, rxBuf, count, pass);
        if (r <= 0) break;
        for (int i = 0; i < r; i++) {
          vtFeed(vt, rxBuf[i], sshWrite);
          if (vt->dirty) { vt->dirty = false; any = true; }
        }
        n -= r;
        fed += r;
      }
    }
    if (any) sshScreenDirty = true;
    if (fed > 0 && fed != lastFedBatch) {
      lastFedBatch = fed;
      ESP_LOGI(kSshTag, "fed %d bytes this tick", fed);
    }

    if (ssh_channel_is_eof(sshChannel)) break;
    vTaskDelay(pdMS_TO_TICKS(5));
  }

  // Teardown
  sshFeedLine(TR(STR_SSH_DISCONNECTED_TAG));
  sshScreenDirty = true;
  sshExitMsg = TR(STR_SSH_SESSION_CLOSED);

  if (sshChannel) {
    ssh_channel_send_eof(sshChannel);
    ssh_channel_close(sshChannel);
    ssh_channel_free(sshChannel);
    sshChannel = nullptr;
  }
  if (sshSession) {
    ssh_disconnect(sshSession);
    ssh_free(sshSession);
    sshSession = nullptr;
  }

  sshBusyFlag = false;
  vTaskDelete(NULL);
}

// PUBLIC API
void sshStartSession(const String& host, const String& user, int port) {
  sshHost = host;
  sshUser = user;
  sshPort = port;
  sshExitMsg = "";
  // Measure the real per-cell advance as the drawUTF8 delta between two
  // glyphs.  charWidth('M') can disagree with the advance actually used when
  // a whole row is rendered, which makes columns run past the right edge.
  int advance = FontEngine::textWidth(DisplayTarget::EINK, "MM", FontStyle::Terminal)
              - FontEngine::textWidth(DisplayTarget::EINK, "M", FontStyle::Terminal);
  if (advance <= 0) advance = FontEngine::charWidth(DisplayTarget::EINK, 'M', FontStyle::Terminal);
  sshCols = (kEinkWidth - TERM_X) / advance;
  if (sshCols <= 0 || sshCols > SSH_COLS_MAX) sshCols = 40;
  sshTxQueue = xQueueCreate(64, sizeof(char));
  sshPromptQueue = xQueueCreate(4, sizeof(SshPrompt));
  sshAnswerQueue = xQueueCreate(4, sizeof(SshAnswer));
  sshScreenMutex = xSemaphoreCreateMutex();
  sshScreenDirty = false;
  sshLastEinkRefresh = 0;
  sshOledInput = "";
  sshBaseStatus = user.length() > 0 ? ("ssh " + user + "@" + host) : ("ssh " + host);
  sshStatus = sshBaseStatus;
  sshBusyFlag = true;   // set before the task starts so the UI never sees a
                        // finished session between launch and first task tick
  sshSession = nullptr;
  sshChannel = nullptr;
  ESP_LOGI(kSshTag, "session start %s:%d user='%s' advance=%d cols=%d", host.c_str(), port, user.c_str(), advance, sshCols);
  xTaskCreatePinnedToCore(sshTask, "pmssh", 51200, nullptr, 2, &sshTaskHandle, 0);
}

bool sshBusy() {
  return sshBusyFlag;
}

const char* sshLastMessage() {
  return sshExitMsg.c_str();
}

void sshRequestDisconnect() {
  sshDisconnectReq = true;
}

static void sshSendByte(char b) {
  if (sshTxQueue) xQueueSend(sshTxQueue, &b, 0);
}

void sshProcessKB() {
  // When the worker finishes (disconnect, EOF, failed login) the session is
  // over: print the reason and fall back to the terminal prompt.
  if (!sshBusy()) {
    const char* msg = sshLastMessage();
    if (msg && msg[0]) {
      termPrint(msg);
      sshExitMsg = "";
    }
    termReturnToPrompt();
    return;
  }

  // Drain worker prompts into the UI.
  SshPrompt p;
  while (xQueueReceive(sshPromptQueue, &p, 0) == pdTRUE) {
    if (p.kind == SSH_PROMPT_HOSTKEY) {
      int ok = boolPrompt(p.text);
      SshAnswer a = {};
      a.text[0] = (ok == 1) ? 'y' : 'n';
      a.text[1] = 0;
      xQueueSend(sshAnswerQueue, &a, pdMS_TO_TICKS(100));
    } else {
      String answer = textPrompt(p.text, "", p.kind == SSH_PROMPT_PASSWORD);
      SshAnswer a = {};
      if (answer == "_EXIT_" || answer == "_CENTER_" || answer == "_RETURN_") {
        strncpy(a.text, "_CANCEL_", sizeof(a.text) - 1);
      } else {
        strncpy(a.text, answer.c_str(), sizeof(a.text) - 1);
      }
      a.text[sizeof(a.text) - 1] = 0;
      xQueueSend(sshAnswerQueue, &a, pdMS_TO_TICKS(100));
    }
    // A prompt may have been the last thing on screen; restore the session
    // label so the OLED does not keep showing the prompt text afterwards.
    sshStatus = sshBaseStatus;
  }

  // OLED session line: status label plus a local echo of what the user is
  // typing.  Keystrokes go to the channel immediately; the e-ink only
  // repaints when the remote output changes, so typing feedback comes from
  // the OLED instead of a full e-ink refresh per key.
  if (millis() - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
    OLEDFPSMillis = millis();
    OLED().oledLine(sshOledInput, sshOledInput.length(), false, sshStatus);
  }

  // Keyboard -> session bytes.
  char inchar = KB().updateKeypress();
  if (inchar == 0) return;

  switch (inchar) {
    case 13: sshOledInput = ""; sshSendByte('\r'); break;
    case 8:  if (sshOledInput.length() > 0) sshOledInput.remove(sshOledInput.length() - 1); sshSendByte(0x7f); break;
    case 9:  sshSendByte('\t'); break;
    case 14: sshSendByte('\x1b'); sshSendByte('['); sshSendByte('Z'); break;  // Shift+Tab
    case 17: KB().toggleShift(); break;
    case 18: KB().toggleFn(); break;
    case 12: sshOledInput = ""; sshSendByte('\x1b'); break;                   // Esc (FN+Left)
    case 7:  sshOledInput = ""; sshSendByte('\x03'); break;                   // Ctrl+C (FN+Select)
    case 6:  sshOledInput = ""; sshSendByte('\x04'); break;                   // Ctrl+D / logout (FN+Right)
    case 19: sshSendByte('\x1b'); sshSendByte('['); sshSendByte('D'); break;  // Left
    case 21: sshSendByte('\x1b'); sshSendByte('['); sshSendByte('C'); break;  // Right
    case 20: sshOledInput = ""; sshSendByte('\r'); break;                     // Select
    case 24: sshRequestDisconnect(); break;                                   // FN+Shift+Left: hang up
    default:
      if (inchar >= 32 && inchar <= 126) {
        if (sshOledInput.length() < 30) sshOledInput += (char)inchar;
        sshSendByte(inchar);
      }
      break;
  }

  if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
}

void sshEinkHandler() {
  // newState forces the first paint when entering SSH mode; after that the
  // worker's dirty flag drives redraws, throttled so a burst of remote echo
  // does not flash the e-ink on every keystroke.
  bool force = newState;
  newState = false;
  if (!force && !sshScreenDirty) return;

  if (sshScreenDirty && !force) {
    uint32_t now = millis();
    if (sshLastEinkRefresh != 0 && now - sshLastEinkRefresh < SSH_EINK_MIN_REFRESH_MS) return;
  }
  sshScreenDirty = false;

  SshCell (*active)[SSH_COLS_MAX] = sshScreen;
  if (sshScreenMutex && xSemaphoreTake(sshScreenMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    active = sshVt.screen ? sshVt.screen : sshScreen;
    memcpy(sshSnap, active, sizeof(SshCell) * SSH_ROWS * SSH_COLS_MAX);
    xSemaphoreGive(sshScreenMutex);
  }

  // Skip a refresh when the snapshot is byte-identical to the last painted
  // frame (e.g. the shell redraws the same prompt).  Avoids burning an e-ink
  // update for no visible change.
  if (!force && memcmp(sshSnap, sshLastPainted, sizeof(SshCell) * SSH_ROWS * SSH_COLS_MAX) == 0) {
    return;
  }
  memcpy(sshLastPainted, sshSnap, sizeof(SshCell) * SSH_ROWS * SSH_COLS_MAX);

  int cols = sshCols ? sshCols : 40;
  bool blank = true;
  int firstRow = -1;
  for (int row = 0; row < SSH_ROWS; row++) {
    for (int x = 0; x < cols; x++) {
      if (sshSnap[row][x].ch && sshSnap[row][x].ch != ' ') {
        blank = false;
        if (firstRow < 0) firstRow = row;
      }
    }
    if (firstRow >= 0 && !blank) break;
  }
  ESP_LOGI(kSshTag, "eink paint force=%d blank=%d cols=%d firstRow=%d", force, blank, cols, firstRow);

  uint16_t bgColor = GxEPD_WHITE;
  uint16_t fgColor = GxEPD_BLACK;
  display.fillRect(0, 0, display.width(), display.height(), bgColor);

  const FontStyle style = FontStyle::Terminal;
  // Same advance the column count was derived from, so cells line up exactly
  // with the layout math and no glyph can step past the right edge.
  int advance = FontEngine::textWidth(DisplayTarget::EINK, "MM", style)
              - FontEngine::textWidth(DisplayTarget::EINK, "M", style);
  if (advance <= 0) advance = FontEngine::charWidth(DisplayTarget::EINK, 'M', style);
  int ascent = FontEngine::fontAscent(DisplayTarget::EINK, style);
  int height = FontEngine::fontHeight(DisplayTarget::EINK, style);

  // First paint before any worker output: show the session label instead of
  // pushing a blank white page.
  if (blank && force) {
    u8g2f.setForegroundColor(fgColor);
    FontEngine::drawText(DisplayTarget::EINK, TERM_X, TERM_SMALL_Y0, sshStatus, style);
  } else {
    // Draw every row cell-by-cell.  Each glyph lands at TERM_X + x*advance,
    // bounding the right edge by construction even if the font's natural
    // advance disagrees with the width measurement.  Reverse-video cells get
    // a filled background with the glyph drawn in the background color.
    for (int row = 0; row < SSH_ROWS; row++) {
      int y = TERM_SMALL_Y0 + row * TERM_SMALL_STEP;
      for (int x = 0; x < cols; x++) {
        char ch = sshSnap[row][x].ch ? sshSnap[row][x].ch : ' ';
        int px = TERM_X + x * advance;
        if (sshSnap[row][x].attr & 1) {
          display.fillRect(px, y - ascent, advance, height, fgColor);
          u8g2f.setForegroundColor(bgColor);
        } else {
          u8g2f.setForegroundColor(fgColor);
        }
        FontEngine::drawGlyph(DisplayTarget::EINK, px, y, (uint16_t)ch, style);
      }
    }
  }

  EINK().refresh();
  sshLastEinkRefresh = millis();
  u8g2f.setForegroundColor(GxEPD_BLACK);
}

// WIFI + SSH COMMANDS
// Runs on the main loop; prints to the terminal scrollback via termPrint.
bool sshCommand(const String& command) {
  String verb = command;
  int verbEnd = verb.indexOf(' ');
  if (verbEnd > 0) verb = verb.substring(0, verbEnd);
  verb.toLowerCase();

  if (verb == "wifi") {
    String args = command.substring(5);
    args.trim();
    if (args.length() == 0) {
      termPrint(TR(STR_SSH_USAGE_WIFI));
      termPrint(TR(STR_SSH_USAGE_WIFI_SUB));
      return false;
    }
    P_WIFI.begin();
    char savedPass[65] = {0};
    if (P_WIFI.hasSavedCredentials(args.c_str())) {
      P_WIFI.loadSavedCredentials(args.c_str(), savedPass, sizeof(savedPass));
      P_WIFI.enable();
      P_WIFI.connect(args.c_str(), savedPass, false);
      termPrint(String(TR(STR_SSH_CONNECTING_SAVED_PREFIX)) + args + "...");
    } else {
      P_WIFI.enable();
      String pass = textPrompt(String(TR(STR_SSH_PASSWORD_FOR_PREFIX)) + args, "", true);
      if (pass == "_EXIT_" || pass == "_CENTER_" || pass == "_RETURN_") {
        termPrint(TR(STR_SSH_WIFI_CONNECT_CANCELLED));
        return false;
      }
      P_WIFI.connect(args.c_str(), pass.c_str(), true);
      termPrint(String(TR(STR_SSH_CONNECTING_PREFIX)) + args + "...");
    }
    return false;
  }
  if (verb == "wifi-scan") {
    P_WIFI.begin();
    P_WIFI.enable();
    P_WIFI.scan();
    termPrint(TR(STR_SSH_SCANNING_NETWORKS));
    termPrint(TR(STR_SSH_SCAN_HINT));
    return false;
  }
  if (verb == "wifi-list") {
    uint16_t count = P_WIFI.getScanResultCount();
    if (count == 0) {
      termPrint(TR(STR_SSH_NO_SCAN_RESULTS));
      return false;
    }
    termPrint(String(count) + TR(STR_SSH_NETWORKS_FOUND_SUFFIX));
    WifiApInfo ap;
    for (uint16_t i = 0; i < count && i < 8; i++) {
      if (P_WIFI.getScanResult(i, ap)) {
        String lock = (ap.authmode == WIFI_AUTH_OPEN) ? "open" : "sec ";
        termPrint(lock + " " + ap.ssid + "  " + String(ap.rssi) + "dBm");
      }
    }
    return false;
  }
  if (verb == "wifi-status") {
    termPrint(String(TR(STR_SSH_WIFI_STATE_PREFIX)) + P_WIFI.getStatusMessage());
    if (P_WIFI.isConnected()) {
      termPrint(String(TR(STR_SSH_SSID_PREFIX)) + P_WIFI.getConnectedSSID());
      termPrint(String(TR(STR_SSH_IP_PREFIX)) + P_WIFI.getIpAddress());
    }
    return false;
  }
  if (verb == "wifi-disconnect") {
    P_WIFI.disconnect();
    termPrint(TR(STR_SSH_WIFI_DISCONNECTED));
    return false;
  }

  if (verb == "ssh") {
    String args = command.substring(3);
    args.trim();
    int port = 22;
    if (args.startsWith("-p ")) {
      int sp = args.indexOf(' ', 3);
      String portStr = (sp < 0) ? args.substring(3) : args.substring(3, sp);
      port = portStr.toInt();
      if (port <= 0 || port > 65535) {
        termPrint(TR(STR_SSH_INVALID_PORT));
        return false;
      }
      args = (sp < 0) ? "" : args.substring(sp + 1);
      args.trim();
    }
    if (args.length() == 0) {
      termPrint(TR(STR_SSH_USAGE_SSH));
      termPrint(TR(STR_SSH_USAGE_SSH_P));
      return false;
    }

    P_WIFI.begin();
    if (P_WIFI.getState() != WifiRadioState::Connected) {
      termPrint(TR(STR_SSH_NO_WIFI));
      termPrint(TR(STR_SSH_JOIN_NETWORK));
      return false;
    }

    String user = "";
    String host = args;
    int at = args.indexOf('@');
    if (at >= 0) {
      user = args.substring(0, at);
      host = args.substring(at + 1);
    }
    int colon = host.indexOf(':');
    if (colon >= 0) {
      int p = host.substring(colon + 1).toInt();
      if (p > 0 && p <= 65535) port = p;
      host = host.substring(0, colon);
    }
    if (host.length() == 0) {
      termPrint(TR(STR_SSH_NO_HOST));
      return false;
    }

    EINK().setFastFullRefresh(true);
    sshStartSession(host, user, port);
    return true;   // caller switches into SSH mode
  }

  return false;
}

#endif // PM_TARGET_HOST
