
#include <globals.h>
#include <pocketmage_sd/pocketmage_sd.h>

#include <cstring>

#if PM_TARGET_HOST  // POCKETMAGE_OS
enum LexState { MENU, DEF };
LexState CurrentLexState = MENU;

// -----------------------------
// Nth-definition helpers
// -----------------------------

struct LexQuery {
  String word;
  int requestedIndex;  // -1 if not specified, 0-based otherwise
};

LexQuery parseLexQuery(const String& input) {
  LexQuery q;
  q.word = input;
  q.requestedIndex = -1;

  String trimmed = input;
  trimmed.trim();
  if (trimmed.length() == 0)
    return q;

  int lastSpace = trimmed.lastIndexOf(' ');
  if (lastSpace == -1) {
    q.word = trimmed;
    return q;
  }

  String maybeNumber = trimmed.substring(lastSpace + 1);
  bool isNumber = true;
  for (size_t i = 0; i < maybeNumber.length(); i++) {
    if (!isDigit(maybeNumber[i])) {
      isNumber = false;
      break;
    }
  }

  if (isNumber) {
    q.word = trimmed.substring(0, lastSpace);
    q.word.trim();
    int n = maybeNumber.toInt();
    if (n > 0)
      q.requestedIndex = n - 1;  // convert to 0-based
  } else {
    q.word = trimmed;
  }

  return q;
}

int clampDefinitionIndex(int idx, int total) {
  if (total <= 0)
    return 0;
  if (idx < 0)
    return 0;
  if (idx >= total)
    return total - 1;
  return idx;
}

static String currentLine = "";
static int cursor_pos = 0;

// Vector to hold the definitions
std::vector<std::pair<String, String>> defList;
int definitionIndex = 0;

void LEXICON_INIT() {
  currentLine = "";
  CurrentAppState = LEXICON;
  CurrentLexState = MENU;
  KB().setKeyboardState(NORMAL);
  newState = true;
  definitionIndex = 0;

  // Verify that dict is installed
  pocketmage::setCpuSpeed(240);
  delay(50);
  if (!PM_SD().exists("/dict/A.txt")) {
    OLED().sysMessage(TR(STR_LEX_INSTALL_DICT), 5000);
    pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    HOME_INIT();
    return;
  }
  if (SAVE_POWER)
    pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
}

void loadDefinitions(String input) {
  OLED().oledWord(TR(STR_LEX_LOADING));
  PM_SDAUTO().beginIO();
  pocketmage::setCpuSpeed(240);
  delay(50);

  CurrentLexState = MENU;
  defList.clear();  // Clear previous results

  // Parse query (word + optional index)
  LexQuery query = parseLexQuery(input);
  String word = query.word;

  if (word.length() == 0 || PM_SDAUTO().getNoSD()) {
    PM_SDAUTO().endIO();
    if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    return;
  }

  char firstChar = tolower(word[0]);
  if (firstChar < 'a' || firstChar > 'z') {
    PM_SDAUTO().endIO();
    if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    return;
  }

  String filePath = "/dict/" + String((char)toupper(firstChar)) + ".txt";
  int slot = PM_SD().streamOpen(filePath.c_str(), 'r');
  if (slot < 0) {
    OLED().sysMessage(TR(STR_LEX_MISSING), 2000);
    PM_SDAUTO().endIO();
    if (SAVE_POWER)
      pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    return;
  }

  word.toLowerCase();

  // Stream the letter file in bounded chunks.  The old whole-file String failed
  // once the heap fragmented; one small buffer works regardless of file size.
  // Entries are alphabetically sorted, so scanning stops once we pass the word.
  constexpr size_t kChunkSize = 1024;
  char chunk[kChunkSize];
  String line;
  bool done = false;
  int lineScanCount = 0;

  while (!done) {
    const size_t n = PM_SD().streamRead(slot, chunk, sizeof(chunk));
    if (n == 0)
      break;

    size_t pos = 0;
    while (pos < n && !done) {
      const char *start = chunk + pos;
      const char *nl = static_cast<const char *>(memchr(start, '\n', n - pos));
      const size_t segLen = nl ? static_cast<size_t>(nl - start) : (n - pos);

      line += String(start, segLen);
      pos += segLen + (nl ? 1 : 0);

      // Without a newline the line continues in the next chunk.
      if (!nl)
        break;

      line.trim();
      if (++lineScanCount % 100 == 0)
        yield();

      if (line.length() > 0) {
        const int defSplit = line.indexOf(')');
        if (defSplit != -1) {
          String key = line.substring(0, defSplit + 1);
          String keyLower = key;
          keyLower.toLowerCase();

          if (keyLower.startsWith(word)) {
            String def = line.substring(defSplit + 1);
            def.trim();
            defList.push_back({key, def});
          } else if (!defList.empty()) {
            done = true;  // past the query; entries are sorted
          }
        }
      }
      line.remove(0);
    }
  }

  // Handle a final line without a trailing newline.
  if (!done && line.length() > 0) {
    line.trim();
    const int defSplit = line.indexOf(')');
    if (defSplit != -1) {
      String key = line.substring(0, defSplit + 1);
      String keyLower = key;
      keyLower.toLowerCase();
      if (keyLower.startsWith(word)) {
        String def = line.substring(defSplit + 1);
        def.trim();
        defList.push_back({key, def});
      }
    }
  }

  PM_SD().streamClose(slot);

  if (!defList.empty())
    EINK().setFullRefreshAfter(FULL_REFRESH_AFTER);

  if (defList.empty()) {
    OLED().sysMessage(TR(STR_LEX_NONE), 2000);

    CurrentLexState = MENU;
    newState = true;

  } else {
    CurrentLexState = DEF;
    KB().setKeyboardState(NORMAL);

    if (query.requestedIndex >= 0) {
      definitionIndex = clampDefinitionIndex(query.requestedIndex, defList.size());

    } else {
      definitionIndex = 0;
    }

    newState = true;
  }

  if (SAVE_POWER)
    pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  PM_SDAUTO().endIO();
}

void processKB_LEXICON() {
  int currentMillis = millis();
  String left = "";
  String right = "";
  String command = "";

  switch (CurrentLexState) {
    case MENU:
      KB().setKeyboardState(NORMAL);
      command = textPrompt();
      if (command == "_RETURN_")
        return;
      else if (command != "_EXIT_")
        loadDefinitions(command);
      else
        HOME_INIT();
      break;

    case DEF:
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
        char inchar = KB().updateKeypress();
        // HANDLE INPUTS
        // No char recieved
        if (inchar == 0)
          ;
        // CR Recieved
        else if (inchar == 13) {
          loadDefinitions(currentLine);
          currentLine = "";
          cursor_pos = 0;
        }
        // SHIFT Recieved
        else if (inchar == 17) {
          KB().toggleShift();
        }
        // FN Recieved
        else if (inchar == 18) {
          KB().toggleFn();
        }
        // BKSP Recieved
        else if (inchar == 8) {
          if (currentLine.length() > 0 && cursor_pos != 0) {
            if (cursor_pos == currentLine.length()) {
              currentLine.remove(currentLine.length() - 1, 1);
            } else {
              currentLine.remove(cursor_pos - 1, 1);
            }
            cursor_pos--;
          }
        }
        // LEFT
        else if (inchar == 19) {
          if (currentLine.length() == 0) {
            definitionIndex--;
            if (definitionIndex < 0)
              definitionIndex = 0;
            newState = true;
          } else {
            if (cursor_pos > 0) {
              cursor_pos--;
            }
          }
        }
        // RIGHT
        else if (inchar == 21) {
          if (currentLine.length() == 0) {
            definitionIndex++;
            if (definitionIndex >= defList.size())
              definitionIndex = defList.size() - 1;
            newState = true;
          } else {
            if (cursor_pos < currentLine.length()) {
              cursor_pos++;
            }
          }
        }
        // CENTER
        else if (inchar == 20) {
        }
        // SHIFT+LEFT
        else if (inchar == 28) {
          cursor_pos = 0;
          KB().setKeyboardState(NORMAL);
        }
        // SHIFT+RIGHT
        else if (inchar == 30) {
          cursor_pos = currentLine.length();
          KB().setKeyboardState(NORMAL);
        }
        // SHIFT+CENTER
        else if (inchar == 29) {
          KB().setKeyboardState(NORMAL);
        }
        // FN+LEFT
        else if (inchar == 12) {
          HOME_INIT();
        }
        // FN+RIGHT
        else if (inchar == 6) {
          KB().setKeyboardState(NORMAL);
        }
        // FN+CENTER
        else if (inchar == 7) {
          currentLine = "";
          cursor_pos = 0;
          KB().setKeyboardState(NORMAL);
        }
        // TAB, SHIFT+TAB / FN+TAB, FN+SHIFT+TAB
        else if (inchar == 9 || inchar == 14) {
          KB().setKeyboardState(NORMAL);
        } else {
          // split line at cursor_pos
          if (cursor_pos == 0) {
            currentLine = inchar + currentLine;
          } else if (cursor_pos == currentLine.length()) {
            currentLine += inchar;
          } else {
            left = currentLine.substring(0, cursor_pos);
            right = currentLine.substring(cursor_pos);
            currentLine = left + inchar + right;
          }
          cursor_pos++;
          if (inchar >= 48 && inchar <= 57) {
          }  // Only leave FN on if typing numbers
          else if (KB().getKeyboardState() != NORMAL) {
            KB().setKeyboardState(NORMAL);
          }
        }

        currentMillis = millis();
        // Make sure oled only updates at OLED_MAX_FPS
        if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentLine, cursor_pos, false);
        }
      }
      break;
  }
}

// Layout constants
constexpr int LEX_MARGIN = 8;        // left/right margin
constexpr int LEX_WORD_X = 12;       // word x
constexpr int LEX_WORD_Y = 50;       // word baseline
constexpr int LEX_DEF_Y0 = 87;       // first definition-line baseline
constexpr int LEX_LINE_HEIGHT = 20;  // definition line pitch

void einkHandler_LEXICON() {
  switch (CurrentLexState) {
    case MENU:
      if (newState) {
        newState = false;
        beginEinkScreen(true);
        display.drawBitmap(0, 0, _lex0, 320, 218, GxEPD_BLACK);

        endEinkScreen(TR(STR_LEX_TYPE_WORD));
      }
      break;

    case DEF:
      if (newState) {
        newState = false;

        beginEinkScreen();

        display.drawBitmap(0, 0, _lex1, 320, 218, GxEPD_BLACK);
        u8g2f.setForegroundColor(GxEPD_BLACK);

        // Draw Word
        FontEngine::drawText(DisplayTarget::EINK, LEX_WORD_X, LEX_WORD_Y,
                             defList[definitionIndex].first, FontStyle::Body);

        // Draw Definition with Word Wrap

        String defText = defList[definitionIndex].second;
        int maxW = kEinkWidth - (2 * LEX_MARGIN);
        int cursorY = LEX_DEF_Y0;

        auto wrappedLines = wordWrap(defText, maxW, FontStyle::Body);
        for (const auto& line : wrappedLines) {
          if (line.length() > 0) {
            FontEngine::drawText(DisplayTarget::EINK, LEX_MARGIN, cursorY, line, FontStyle::Body);
          }
          cursorY += LEX_LINE_HEIGHT;
        }

        endEinkScreen(TR(STR_LEX_TYPE_NEW_WORD), EinkRefresh::Normal);
      }
      break;
  }
}
#endif