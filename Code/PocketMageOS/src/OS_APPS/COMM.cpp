#include <globals.h>

#if PM_TARGET_HOST // POCKETMAGE_OS
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <vector>

extern "C" {
  #include "mesh_now.h"
  #include "message_queue.h"
}

static constexpr const char* TAG = "COMM";

// CONFIG
#define MAX_CHAT_MSGS 50
#define MAX_VISIBLE_LINES 10

// Layout constants
constexpr int COMM_BAR_H         = 20;    // top bar height
constexpr int COMM_BAR_SEP_Y     = 21;    // top-bar separator line y
constexpr int COMM_BAR_TEXT_Y    = 16;    // top bar text baseline
constexpr int COMM_BAR_LEFT_X    = 4;     // top bar left column x
constexpr int COMM_BAR_MID_X     = 164;   // top bar middle column x
constexpr int COMM_BAR_RIGHT_X   = 290;   // top bar right column x
constexpr int COMM_CURSOR_X      = 4;     // selection arrow x
constexpr int COMM_LIST_X        = 20;    // peer list label x
constexpr int COMM_LIST_Y0       = 36;    // first peer row baseline
constexpr int COMM_LIST_PITCH    = 20;    // peer row pitch
constexpr int COMM_SB_Y          = 26;    // list scrollbar track y
constexpr int COMM_SB_W          = 4;     // list scrollbar thumb width
constexpr int COMM_SB_MARGIN     = 7;     // list scrollbar right margin
constexpr int COMM_CHAT_Y        = 26;    // message area y origin
constexpr int COMM_CHAT_H        = kEinkContentH;  // 214: message area height
constexpr int COMM_BARW          = 3;     // chat scrollbar width
constexpr int COMM_EDGE_MARGIN   = 6;     // chat right margin
constexpr int COMM_WRAP_RESERVE  = 55;    // bubble text wrap inset (bubble padding + scrollbar + margins)
constexpr int COMM_BUBBLE_PAD_X  = 8;     // bubble text x inset
constexpr int COMM_BUBBLE_PAD    = 16;    // bubble horizontal padding
constexpr int COMM_BUBBLE_R      = 10;    // bubble corner radius
constexpr int COMM_BUBBLE_GAP    = 4;     // vertical gap between bubbles
constexpr int COMM_BUBBLE_HEAD   = 21;    // bubble height above the message block
constexpr int COMM_LINE_SPACE    = 2;     // extra per-line spacing
constexpr int COMM_META_GAP      = 10;    // name-to-time gap
constexpr int COMM_NAME_PAD      = 8;     // name baseline offset from bubble top
constexpr int COMM_MSG_OFFSET    = 7;     // message baseline offset from name baseline
constexpr int COMM_BUBBLE_AVG_H  = 64;    // average bubble height (scrollbar ratio)

// Message-bubble font: the largest mono ladder face whose full content still
// fits the bubble text width; anything longer falls back to the Caption face.
// Shared with wrapTextPx() so font choice and wrapping agree.
static FontStyle msgFont(const char* content) {
  const FontStyle cascade[] = { FontStyle::MonoBold, FontStyle::Mono, FontStyle::Caption };
  return FontEngine::fitStyle(DisplayTarget::EINK, content,
                              kEinkWidth - COMM_WRAP_RESERVE,
                              cascade, sizeof(cascade) / sizeof(cascade[0]));
}

// TYPES
enum CommState { PEER_LIST, CHAT_VIEW };
enum ChatMode { LOCAL_CHAT, DIRECT_CHAT };

struct ChatMsg {
  uint32_t timestamp;
  uint8_t hr;
  uint8_t mn;
  char sender[18];
  char content[128];
  bool sentByLocal;
};

// STATE
static bool meshReady = false;
static CommState currentState = PEER_LIST;
static ChatMode chatMode = LOCAL_CHAT;

static ChatMsg msgs[MAX_CHAT_MSGS];
static int msgCount = 0;
static bool autoScroll = true;

static uint8_t myMAC[6];
static char myMacStr[18] = "00:00:00:00:00:00";
static uint8_t peerMAC[6];
static char peerMacStr[18] = "00:00:00:00:00:00";
static int selPeer = 0;
static int prevSelPeer = 0;
static bool comm_first_draw = true;
static int last_peer_count = -1;

// UI FLAGS
static bool cursor_moved = false;

// CHAT INPUT STATE
static String chatInputBuffer = "";
static int chatCursorPos = 0;
static ulong chatScrollIndex = 0;

// MAC HELPERS
static void macToStr(const uint8_t* m, char* out) {
  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X",
          m[0], m[1], m[2], m[3], m[4], m[5]);
}

static String displayName(const char* mac) {
  if (strcmp(mac, myMacStr) == 0) return "Me";
  if (strcmp(mac, "00:1A:2B:3C:4D:5E") == 0) return "Chris";
  return String(mac);
}

static std::vector<String> wrapTextPx(const String& text, FontStyle style) {
  return wordWrap(text, kEinkWidth - COMM_WRAP_RESERVE, style);
}

// SD LOGGING
static void logToSD(const ChatMsg* m) {
  if (PM_SDAUTO().getNoSD() || !global_fs) return;
  PM_SDAUTO().beginIO();
  global_fs->mkdir("/chat");
  char path[64];
  if (chatMode == LOCAL_CHAT) {
    snprintf(path, sizeof(path), "/chat/_broadcast.log");
  } else {
    char fm[18];
    macToStr(peerMAC, fm);
    for (char* p = fm; *p; p++) if (*p == ':') *p = '-';
    snprintf(path, sizeof(path), "/chat/dm_%s.log", fm);
  }
  File f = global_fs->open(path, FILE_APPEND);
  if (f) {
    f.printf("%u|%s|%s\n", m->timestamp, m->sender, m->content);
    f.close();
  }
  PM_SDAUTO().endIO();
}

// MESSAGE BUFFER
static void addMsg(const char* sender, const char* content, bool local) {
  if (msgCount >= MAX_CHAT_MSGS) {
    int mv = MAX_CHAT_MSGS - 1;
    memmove(&msgs[0], &msgs[1], mv * sizeof(ChatMsg));
    msgCount = mv;
  }
  ChatMsg* m = &msgs[msgCount++];
  strncpy(m->sender, sender, sizeof(m->sender));
  m->sender[sizeof(m->sender) - 1] = '\0';
  strncpy(m->content, content, sizeof(m->content));
  m->content[sizeof(m->content) - 1] = '\0';
  m->sentByLocal = local;
  
  DateTime now = CLOCK().nowDT();
  m->timestamp = now.unixtime();
  m->hr = now.hour();
  m->mn = now.minute();

  logToSD(m);
  newState = true; // Incoming message forces an E-ink refresh
}

// MESH-NOW RECEIVE CALLBACK
static void meshRecvCb(const mesh_message_t* msg) {
  if (msg->type != MSG_TYPE_CHAT && msg->type != MSG_TYPE_DIRECT) {
    return;
  }
  if (msg->type == MSG_TYPE_DIRECT) {
    if (memcmp(msg->target_mac, myMAC, 6) != 0) {
      return;
    }
  }
  message_t q{};
  strncpy(q.message, msg->message, sizeof(q.message));
  q.message[sizeof(q.message) - 1] = '\0';
  memcpy(q.sender_mac, msg->sender_mac, 6);
  memcpy(q.target_mac, msg->target_mac, 6);
  q.type = msg->type;
  q.timestamp = msg->timestamp;
  message_queue_send(&q);
}

// INIT
void COMM_INIT() {
  setCpuFrequencyMhz(240);
  CurrentAppState = COMM;
  currentState = PEER_LIST;
  chatMode = LOCAL_CHAT;
  msgCount = 0;
  autoScroll = true;
  selPeer = 0;
  prevSelPeer = 0;
  cursor_moved = false;
  chatInputBuffer = "";
  chatCursorPos = 0;
  chatScrollIndex = 0;
  last_peer_count = -1;
  newState = true;

  comm_first_draw = true;
  if (meshReady) {
    message_t discard{};
    while (message_queue_receive(&discard, 0) == ESP_OK) {}
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  esp_read_mac(myMAC, ESP_MAC_WIFI_STA);
  macToStr(myMAC, myMacStr);

  message_queue_init();
  mesh_now_deinit();
  if (mesh_now_init() == ESP_OK) {
    mesh_now_set_receive_callback(meshRecvCb);
    meshReady = true;
  }
}

// DRAIN INCOMING QUEUE
static void drainQueue() {
  if (!meshReady) return;
  message_t q{};
  while (message_queue_receive(&q, 0) == ESP_OK) {
    char s[18];
    macToStr(q.sender_mac, s);
    bool fromMe = (memcmp(q.sender_mac, myMAC, 6) == 0);
    if (q.type == MSG_TYPE_DIRECT) {
      if (!fromMe && memcmp(q.sender_mac, peerMAC, 6) == 0 && memcmp(q.target_mac, myMAC, 6) == 0) {
        addMsg(s, q.message, false);
      }
    } else {
      if (!fromMe) addMsg(s, q.message, false);
    }
  }
}

void chatScrollPreview() {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  int startLine = 0;
  if (chatScrollIndex >= 1) startLine = chatScrollIndex - 1;
  int y = kOledPrevY0;
  for (int i = startLine; i < startLine + kOledPrevRows; i++) {
    if (i >= msgCount) break;
    if (i == (int)chatScrollIndex) u8g2.drawTriangle(0, y - 2 * kOledPrevTriH, 0, y, kOledPrevTriW, y - kOledPrevTriH);
    String dispStr = String(msgs[i].sender) + ": " + String(msgs[i].content);
    if (dispStr.length() > 38) dispStr = dispStr.substring(0, 38) + "..";
    FontEngine::drawText(DisplayTarget::OLED, kOledPrevX, y, dispStr, FontStyle::Tiny);
    y += kOledPrevPitch;
  }
  u8g2.sendBuffer();
}

// KEYBOARD / LOOP
void processKB_COMM() {
  drainQueue();

  if (!meshReady) return;
  int current_pc = mesh_now_get_peer_count();
  if (current_pc != last_peer_count) {
    last_peer_count = current_pc;
    if (currentState == PEER_LIST) {
      newState = true;
    }
  }

  int nowMs = millis();

  // Handle TOUCH scrolling
  if (currentState == CHAT_VIEW) {
      int maxScrollIndex = 0;
      if (msgCount > 0) {
          int totalH = 0;
          int top = msgCount - 1;
          while (top >= 0) {
              FontStyle mf = msgFont(msgs[top].content);
              int lineH = FontEngine::fontHeight(DisplayTarget::EINK, mf);
              int lineSpacing = lineH + 2;
              std::vector<String> lines = wrapTextPx(msgs[top].content, mf);
              int bH = (lines.size() * lineSpacing) + lineH + 21;
              if (totalH + bH > 214) { top++; break; }
              totalH += bH;
              if (top == 0) break;
              top--;
          }
          maxScrollIndex = top;
      }
      if (TOUCH().updateScroll(maxScrollIndex, chatScrollIndex)) {
          newState = true;
          autoScroll = (chatScrollIndex >= maxScrollIndex);
      }
  }

  if (nowMs - KBBounceMillis >= KB_COOLDOWN) {
    char ch = KB().updateKeypress();
    if (ch != 0) {
      KBBounceMillis = nowMs;

      if (currentState == PEER_LIST) {
        int totalRooms = 1 + mesh_now_get_peer_count();
        prevSelPeer = selPeer;

        if (ch == 19 || ch == 7 || ch == 29) {
          if (selPeer > 0) { selPeer--; cursor_moved = true; }
        } 
        else if (ch == 21 || ch == 6 || ch == 25 || ch == 30) {
          if (selPeer < totalRooms - 1) { selPeer++; cursor_moved = true; }
        } 
        else if (ch == 13 || ch == ' ' || ch == 20) {
          chatInputBuffer = "";
          chatCursorPos = 0;
          autoScroll = true;

          if (selPeer == 0) {
            chatMode = LOCAL_CHAT;
            currentState = CHAT_VIEW;
            newState = true;
            OLED().oledWord(TR(STR_COMM_LOCAL_CHAT));
          } else {
            mesh_peer_t* peers = mesh_now_get_peers();
            int pc = mesh_now_get_peer_count();
            int peerIdx = selPeer - 1;
            if (pc > 0 && peerIdx < pc) {
              chatMode = DIRECT_CHAT;
              memcpy(peerMAC, peers[peerIdx].peer_addr, 6);
              macToStr(peerMAC, peerMacStr);
              currentState = CHAT_VIEW;
              newState = true;
              OLED().oledWord(displayName(peerMacStr).c_str());
            }
          }
        } 
        else if (ch == 12 || ch == 8 || ch == 127) {
          HOME_INIT();
        }
      } 
      else if (currentState == CHAT_VIEW) {
        if (ch == 13) { 
            if (chatInputBuffer.length() > 0) {
                esp_err_t sendRet;
                if (chatMode == LOCAL_CHAT) sendRet = mesh_now_send_broadcast(chatInputBuffer.c_str());
                else sendRet = mesh_now_send_direct(peerMAC, chatInputBuffer.c_str());
                
                if (sendRet == ESP_OK) {
                    addMsg(myMacStr, chatInputBuffer.c_str(), true);
                }
                chatInputBuffer = "";
                chatCursorPos = 0;
                autoScroll = true;
                newState = true;
            }
        }
        else if (ch == 12) { 
            currentState = PEER_LIST;
            chatInputBuffer = "";
            chatCursorPos = 0;
            newState = true;
            OLED().oledWord(TR(STR_COMM_CHAT));
        }
        else if (ch == 17) {
            KB().toggleShift();
        }
        else if (ch == 18) {
            KB().toggleFn();
        }
        else if (ch == 8) { 
            if (chatInputBuffer.length() > 0 && chatCursorPos != 0) {
                int old_cursor = chatCursorPos;
                do { chatCursorPos--; } while (chatCursorPos > 0 && (chatInputBuffer[chatCursorPos] & 0xC0) == 0x80);
                int bytesToDelete = old_cursor - chatCursorPos;
                chatInputBuffer.remove(chatCursorPos, bytesToDelete);
            }
        }
        else if (ch == 19) { 
            if (chatCursorPos > 0) {
                do { chatCursorPos--; } while (chatCursorPos > 0 && (chatInputBuffer[chatCursorPos] & 0xC0) == 0x80);
            }
        }
        else if (ch == 21) { 
            if (chatCursorPos < chatInputBuffer.length()) {
                do { chatCursorPos++; } while (chatCursorPos < chatInputBuffer.length() && (chatInputBuffer[chatCursorPos] & 0xC0) == 0x80);
            }
        }
        else if (ch == 28) { chatCursorPos = 0; KB().setKeyboardState(NORMAL); }
        else if (ch == 30) { chatCursorPos = chatInputBuffer.length(); KB().setKeyboardState(NORMAL); }
        else if (ch == 29) { KB().setKeyboardState(NORMAL); }
        else if (ch == 6) { KB().setKeyboardState(NORMAL); }
        else if (ch == 7) { chatInputBuffer = ""; chatCursorPos = 0; KB().setKeyboardState(NORMAL); }
        else if (ch == 24 || ch == 25 || ch == 26) { KB().setKeyboardState(NORMAL); }
        else if (ch == 9 || ch == 14) { KB().setKeyboardState(NORMAL); } 
        else if (ch == 20 || ch == 23) {} 
        else { 
            String chStr = String(ch);
            if (chatCursorPos == 0) {
                chatInputBuffer = chStr + chatInputBuffer;
            } else if (chatCursorPos == chatInputBuffer.length()) {
                chatInputBuffer += chStr;
            } else {
                String left = chatInputBuffer.substring(0, chatCursorPos);
                String right = chatInputBuffer.substring(chatCursorPos);
                chatInputBuffer = left + chStr + right;
            }
            chatCursorPos++;
            if (ch >= 48 && ch <= 57) {} 
            else if (KB().getKeyboardState() != NORMAL) KB().setKeyboardState(NORMAL);
        }
      }
    }
  }

  nowMs = millis();
  if (nowMs - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
    OLEDFPSMillis = nowMs;
    if (currentState == PEER_LIST) {
      if (TOUCH().getLastTouch() == -1) OLED().oledWord(TR(STR_COMM_CHAT));
    } else {
      if (TOUCH().getLastTouch() == -1) {
        OLED().oledLine(chatInputBuffer, chatCursorPos, false, TR(STR_COMM_MSG_PREFIX));
      } else {
        chatScrollPreview();
      }
    }
  }
}

// E-INK DRAW
void einkHandler_COMM() {
  if (cursor_moved) {
    cursor_moved = false;
    
    int totalRooms = 1 + mesh_now_get_peer_count();
    int vis = min(totalRooms, MAX_VISIBLE_LINES);
    
    int scrollTop = max(selPeer - vis / 2, 0);
    if (scrollTop + vis > totalRooms) scrollTop = max(totalRooms - vis, 0);
    
    int prevScrollTop = max(prevSelPeer - vis / 2, 0);
    if (prevScrollTop + vis > totalRooms) prevScrollTop = max(totalRooms - vis, 0);
    
    if (scrollTop != prevScrollTop || newState) {
      newState = true; 
    } else {
      // Safely perform native partial window update without blanking the rest of the screen
      display.fillRect(0, 28, 16, kEinkContentH, GxEPD_WHITE);
      FontEngine::setTextColor(DisplayTarget::EINK, GxEPD_BLACK);
      
      for (int i = 0; i < vis; i++) {
        int idx = scrollTop + i;
        if (idx == selPeer) {
          int yPos = COMM_LIST_Y0 + i * COMM_LIST_PITCH;
          FontEngine::drawText(DisplayTarget::EINK, COMM_CURSOR_X, yPos, ">", FontStyle::Body);
        }
      }
      
      // Push only this rectangle to the display
      display.displayWindow(0, 28, 16, kEinkContentH); 
      return;
    }
  }

  if (!newState && !comm_first_draw) return;
  comm_first_draw = false;
  newState = false;

  display.fillScreen(GxEPD_WHITE);

  // Top bar
  display.fillRect(0, 0, kEinkWidth, COMM_BAR_H, GxEPD_BLACK);
  FontEngine::setTextColor(DisplayTarget::EINK, GxEPD_WHITE);

  if (currentState == PEER_LIST) {
    FontEngine::drawText(DisplayTarget::EINK, COMM_BAR_LEFT_X, COMM_BAR_TEXT_Y, TR(STR_COMM_SELECT_ROOM), FontStyle::Body);
    FontEngine::drawText(DisplayTarget::EINK, COMM_BAR_MID_X, COMM_BAR_TEXT_Y, "Me " + String(myMacStr), FontStyle::Caption);
    FontEngine::drawText(DisplayTarget::EINK, COMM_BAR_RIGHT_X, COMM_BAR_TEXT_Y, "P: " + String(mesh_now_get_peer_count()), FontStyle::Caption);

    int totalRooms = 1 + mesh_now_get_peer_count();
    mesh_peer_t* allPeers = mesh_now_get_peers();
    
    int listY = COMM_LIST_Y0;
    int vis = min(totalRooms, MAX_VISIBLE_LINES);
    int scrollTop = max(selPeer - vis / 2, 0);
    if (scrollTop + vis > totalRooms) scrollTop = max(totalRooms - vis, 0);
    
    FontEngine::setTextColor(DisplayTarget::EINK, GxEPD_BLACK);
    
    for (int i = 0; i < vis; i++) {
      int idx = scrollTop + i;
      if (idx >= totalRooms) break;
      int yPos = listY + i * COMM_LIST_PITCH;
      if (yPos > kEinkHeight - 2) break; 
      
      bool selected = (idx == selPeer);
      String label;
      
      if (idx == 0) {
        label = TR(STR_COMM_LOCAL_CHAT);
      } else {
        int peerIdx = idx - 1;
        if (peerIdx < mesh_now_get_peer_count()) {
          char macStr[18];
          macToStr(allPeers[peerIdx].peer_addr, macStr);
          label = displayName(macStr);
        } else {
          label = "---";
        }
      }
      
      if (selected) {
        FontEngine::drawText(DisplayTarget::EINK, COMM_CURSOR_X, yPos, ">", FontStyle::Body);
      }
      FontEngine::drawText(DisplayTarget::EINK, COMM_LIST_X, yPos, label, FontStyle::Body);
    }
    
    // Scrollbar (Extended to bottom)
    if (totalRooms > vis) {
      int sbY = COMM_SB_Y;
      int sbH = kEinkHeight - COMM_SB_Y;
      float step = (float)sbH / totalRooms;
      int thumbY = sbY + (int)(selPeer * step);
      int thumbH = max((int)(vis * step), 8);
      display.fillRect(kEinkWidth - COMM_SB_MARGIN, thumbY, COMM_SB_W, thumbH, GxEPD_BLACK);
    }
  } else {
    if (chatMode == LOCAL_CHAT) {
      FontEngine::drawText(DisplayTarget::EINK, COMM_BAR_LEFT_X, COMM_BAR_TEXT_Y, TR(STR_COMM_LOCAL_CHAT), FontStyle::Body);
    } else {
      String name = displayName(peerMacStr);
      FontEngine::drawText(DisplayTarget::EINK, COMM_BAR_LEFT_X, COMM_BAR_TEXT_Y, "> " + name, FontStyle::Body);
    }
    FontEngine::drawText(DisplayTarget::EINK, COMM_BAR_MID_X, COMM_BAR_TEXT_Y, chatMode == LOCAL_CHAT ? "ESP-NOW" : TR(STR_COMM_DIRECT), FontStyle::Caption);
    FontEngine::drawText(DisplayTarget::EINK, COMM_BAR_RIGHT_X, COMM_BAR_TEXT_Y, "P: " + String(mesh_now_get_peer_count()), FontStyle::Caption);
  }

  // Separator line
  display.drawFastHLine(0, COMM_BAR_SEP_Y, kEinkWidth, GxEPD_BLACK);

  // Message area (CHAT_VIEW only)
  if (currentState == CHAT_VIEW) {
    int y = COMM_CHAT_Y;

    int maxScrollIndex = 0;
    if (msgCount > 0) {
        int totalH = 0;
        int top = msgCount - 1;
        while (top >= 0) {
            FontStyle mf = msgFont(msgs[top].content);
            int lineH = FontEngine::fontHeight(DisplayTarget::EINK, mf);
            int lineSpacing = lineH + COMM_LINE_SPACE;
            std::vector<String> lines = wrapTextPx(msgs[top].content, mf);
            int bH = (lines.size() * lineSpacing) + lineH + COMM_BUBBLE_HEAD;
            if (totalH + bH > COMM_CHAT_H) { top++; break; }
            totalH += bH;
            if (top == 0) break;
            top--;
        }
        maxScrollIndex = top;
    }

    if (autoScroll) chatScrollIndex = maxScrollIndex;
    if (chatScrollIndex > (ulong)maxScrollIndex) chatScrollIndex = maxScrollIndex;

    for (int i = chatScrollIndex; i < msgCount && y < kEinkHeight; i++) {
      ChatMsg* m = &msgs[i];
      FontStyle mf = msgFont(m->content);
      int ascent = FontEngine::fontAscent(DisplayTarget::EINK, mf);
      int lineH = FontEngine::fontHeight(DisplayTarget::EINK, mf);
      int lineSpacing = lineH + COMM_LINE_SPACE;

      std::vector<String> lines = wrapTextPx(m->content, mf);

      String nameText = displayName(m->sender);
      String timeText = String(m->hr) + ":" + (m->mn < 10 ? "0" : "") + String(m->mn);

      int nameW = FontEngine::textWidth(DisplayTarget::EINK, nameText, mf);
      int timeW = FontEngine::textWidth(DisplayTarget::EINK, timeText, FontStyle::Caption);
      int metaW = nameW + timeW + COMM_META_GAP;

      int textW = 0;
      for (const String& l : lines) {
        int lw = FontEngine::textWidth(DisplayTarget::EINK, l, mf);
        if (lw > textW) textW = lw;
      }

      int bubbleW = max(textW, metaW) + COMM_BUBBLE_PAD;
      int bubbleH = (lines.size() * lineSpacing) + lineH + COMM_BUBBLE_HEAD;

      int x = m->sentByLocal ? (kEinkWidth - bubbleW - COMM_EDGE_MARGIN - (COMM_BARW + 2)) : COMM_EDGE_MARGIN;

      if (m->sentByLocal) {
          display.fillRoundRect(x, y, bubbleW, bubbleH, COMM_BUBBLE_R, GxEPD_BLACK);
          FontEngine::setTextColor(DisplayTarget::EINK, GxEPD_WHITE);
      } else {
          display.drawRoundRect(x, y, bubbleW, bubbleH, COMM_BUBBLE_R, GxEPD_BLACK);
          FontEngine::setTextColor(DisplayTarget::EINK, GxEPD_BLACK);
      }

      int nameY = y + COMM_NAME_PAD + ascent;
      FontEngine::drawText(DisplayTarget::EINK, x + COMM_BUBBLE_PAD_X, nameY, nameText, mf);
      FontEngine::drawText(DisplayTarget::EINK, x + bubbleW - COMM_BUBBLE_PAD_X - timeW, nameY, timeText, FontStyle::Caption);

      display.drawFastHLine(x + COMM_BUBBLE_PAD_X, nameY + 2, bubbleW - COMM_BUBBLE_PAD, m->sentByLocal ? GxEPD_WHITE : GxEPD_BLACK);

      int msgY = nameY + COMM_MSG_OFFSET + ascent;
      for (size_t l = 0; l < lines.size(); l++) {
          FontEngine::drawText(DisplayTarget::EINK, x + COMM_BUBBLE_PAD_X, msgY + l * lineSpacing, lines[l], mf);
      }

      y += bubbleH + COMM_BUBBLE_GAP;
    }

    if (maxScrollIndex > 0) {
      float avgBubbleH = COMM_BUBBLE_AVG_H;
      float visibleRatio = (float)COMM_CHAT_H / ((maxScrollIndex + 1) * avgBubbleH);
      if (visibleRatio > 1.0) visibleRatio = 1.0;
      int handleHeight = max((int)(COMM_CHAT_H * visibleRatio), 15);
      float scrollFraction = (float)chatScrollIndex / maxScrollIndex;
      int handleY = COMM_CHAT_Y + scrollFraction * (COMM_CHAT_H - handleHeight);

      display.fillRect(kEinkWidth - COMM_BARW - 1, handleY, COMM_BARW, handleHeight, GxEPD_BLACK);
    }
  }

  EINK().refresh();
}

#endif
