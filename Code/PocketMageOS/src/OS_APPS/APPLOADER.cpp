#include <elf_runner.h>
#include <globals.h>
#include <pocketmage_sd/pocketmage_sd.h>

#include <cstring>

#define APP_DIRECTORY "/apps"
#define TEMP_DIR "/apps/temp"
#define PREFS_NAMESPACE "AppLoader"
#if PM_TARGET_HOST  // POCKETMAGE_OS
static String currentLine = "";

enum AppLoaderState { MENU, SWAP_OR_EDIT, INSTALLING, SWAP };
AppLoaderState CurrentAppLoaderState = MENU;

uint8_t selectedSlot = 0;  // 1:A, 2:B, etc.

static String basenameNoExt(const String& path, const char* ext = ".tar") {
  int slash = path.lastIndexOf('/');
  String name = (slash >= 0) ? path.substring(slash + 1) : path;
  if (name.endsWith(ext))
    return name.substring(0, name.length() - (int)strlen(ext));
  return name;
}

// Join two paths safely (ensures exactly one slash between them)
static String pathJoin(const String& a, const String& b) {
  if (a.length() == 0)
    return b;
  if (b.length() == 0)
    return a;
  if (a.endsWith("/")) {
    if (b.startsWith("/"))
      return a + b.substring(1);  // avoid double slash
    return a + b;
  } else {
    if (b.startsWith("/"))
      return a + b;
    return a + "/" + b;
  }
}

// ---------- Globals ----------
volatile uint8_t g_installProgress = 0;  // 0-100 (0-50: extract, 50-100: intall)
// Raw 0-100 written by the SD gateway while extractTar blocks the installer;
// the UI halves it so extraction maps onto the first half of the bar.
volatile int g_extractProgress = 0;
volatile bool g_installDone = false;
volatile bool g_installFailed = false;

// ---------- Utilities ----------
// All card access goes through the gateway-backed PM_SD methods: the gateway
// resolves relative paths and ramps the CPU per op, so no CPU-speed juggling
// or sdPath() wrapping is needed here.
static bool ensureDir(const char* path) {
  return PM_SD().mkdirPath(path);
}

static bool rmRF(const char* path) {
  return PM_SD().removeTree(path);
}

bool copyFile(const char* src, const char* dst) {
  return PM_SD().copyFileRaw(src, dst);
}

bool copyAssetsFlat(const char* srcDir, const char* dstDir) {
  ensureDir(dstDir);

  std::vector<PocketmageSD::DirItem> items;
  if (!PM_SD().listDirItems(srcDir, items))
    return false;

  for (const auto& item : items) {
    // Skip macOS metadata files
    if (item.name.startsWith("._"))
      continue;

    String srcPath = pathJoin(srcDir, item.name);
    String dstPath = pathJoin(dstDir, item.name);

    if (item.isDir) {
      if (!copyAssetsFlat(srcPath.c_str(), dstPath.c_str()))
        return false;
    } else {
      if (!copyFile(srcPath.c_str(), dstPath.c_str()))
        return false;
    }
    vTaskDelay(1);  // feed watchdog
  }
  return true;
}

// ---------- Saving/Loading appInfo ----------

// Layout constants, app icon geometry
constexpr int APPLOADER_ICON_S = kIconCellSize;   // app icon cell size
constexpr int APPLOADER_NAME_GAP = kIconNameGap;  // icon-to-name baseline gap

void loadAndDrawAppIcon(int x, int y, int slot, bool showName, int maxNameWidth) {
  ElfAppInfo app;
  if (!loadElfAppInfo(slot, app)) {
    return;
  }

  uint8_t buf[APPLOADER_ICON_S * 5];  // 40x40 1-bit = 200 bytes
  bool iconLoaded = PM_SD().readBinaryFile(app.iconPath, buf, sizeof(buf));

  display.fillRect(x, y, APPLOADER_ICON_S, APPLOADER_ICON_S, GxEPD_WHITE);

  if (iconLoaded) {
    display.drawBitmap(x, y, buf, APPLOADER_ICON_S, APPLOADER_ICON_S, GxEPD_BLACK);
  } else {
    display.drawBitmap(x, y, noIcon, APPLOADER_ICON_S, APPLOADER_ICON_S, GxEPD_BLACK);
  }

  if (showName) {
    FontStyle nameStyle = FontEngine::fitStyle(DisplayTarget::EINK, app.name, maxNameWidth,
                                               kLabelCascade, kLabelCascadeCount);
    String appNameStr = truncateWithEllipsis(app.name, maxNameWidth, nameStyle);

    u8g2f.setForegroundColor(GxEPD_BLACK);

    int w = FontEngine::textWidth(DisplayTarget::EINK, appNameStr, nameStyle);

    int tx = x + (APPLOADER_ICON_S - w) / 2;
    int ty = y + APPLOADER_ICON_S + APPLOADER_NAME_GAP;

    FontEngine::drawText(DisplayTarget::EINK, tx, ty, appNameStr, nameStyle);
  }
}

bool cleanupAppsTempRecursive(const char* dirPath) {
  std::vector<PocketmageSD::DirItem> items;
  if (!PM_SD().listDirItems(dirPath, items))
    return false;

  for (const auto& item : items) {
    String fullPath = pathJoin(dirPath, item.name);

    if (item.isDir) {
      cleanupAppsTempRecursive(fullPath.c_str());
      // removeFile() on a directory only succeeds when it is now empty,
      // matching the old rmdir-if-empty behavior: dirs that still hold
      // an _ICON.bin survive.
      PM_SD().removeFile(fullPath.c_str());
    } else {
      if (!item.name.endsWith("_ICON.bin")) {
        PM_SD().removeFile(fullPath.c_str());
      }
    }
  }
  return true;
}

// ---------- Install Task ----------

struct InstallTaskParams {
  char tarRelName[128];
  int slot;  // 1..4
};

static void installTask(void* param) {
  InstallTaskParams* p = (InstallTaskParams*)param;
  g_installProgress = 0;
  g_extractProgress = 0;
  g_installDone = false;
  g_installFailed = false;

  String tarPath = pathJoin(APP_DIRECTORY, p->tarRelName);

  // --- Check TAR exists ---
  if (!PM_SD().exists(tarPath.c_str())) {
    Serial.printf("Tar not found: %s\n", tarPath.c_str());
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  // --- Ensure directories ---
  if (!ensureDir(APP_DIRECTORY) || !ensureDir(TEMP_DIR)) {
    Serial.println("Failed to prepare TEMP_DIR");
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  // --- TAR extraction (one gateway op: microtar, sanitized entry names,
  // mkdirs for missing parents, 4KB streaming; runs at 240MHz on the
  // gateway task while it blocks here) ---
  if (!PM_SD().extractTar(tarPath.c_str(), TEMP_DIR, &g_extractProgress)) {
    Serial.println("Extraction failed");
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  g_installProgress = 50;  // halfway

  // --- Determine main .bin and base name ---
  String elfPath = "";
  String base = "";
  String iconPath = "";

  {
    std::vector<PocketmageSD::DirItem> items;
    PM_SD().listDirItems(TEMP_DIR, items);
    for (const auto& item : items) {
      String name = item.name;
      int sep = name.lastIndexOf('/');
      if (sep >= 0)
        name = name.substring(sep + 1);

      // Skip macOS metadata files
      if (name.startsWith("._")) {
        continue;
      }

      // --- Main .app.elf ---
      if (elfPath.length() == 0 && name.endsWith(".app.elf")) {
        elfPath = pathJoin(TEMP_DIR, name);

        // Derive base by stripping ".app.elf" (icon is <base>_ICON.bin)
        if (name.length() > 8) {
          base = name.substring(0, name.length() - 8);
        } else {
          int dot = name.lastIndexOf('.');
          base = (dot > 0) ? name.substring(0, dot) : name;
        }
      }
    }
  }

  if (elfPath.length() == 0 || base.length() == 0) {
    Serial.printf("ELF not found after extraction in %s\n", TEMP_DIR);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  Serial.printf("App base name determined: '%s'\n", base.c_str());

  // --- Icon scan (order-independent): <base>_ICON.bin anywhere in TEMP_DIR ---
  {
    String want = base + "_ICON.bin";
    std::vector<PocketmageSD::DirItem> items;
    PM_SD().listDirItems(TEMP_DIR, items);
    for (const auto& item : items) {
      String name = item.name;
      int sep = name.lastIndexOf('/');
      if (sep >= 0)
        name = name.substring(sep + 1);
      if (!name.startsWith("._") && name.equalsIgnoreCase(want)) {
        iconPath = pathJoin(TEMP_DIR, name);
        break;
      }
    }
    if (iconPath.length() == 0) {
      Serial.printf("Icon not found for app '%s' (non-fatal)\n", base.c_str());
    } else {
      Serial.printf("Icon found: %s\n", iconPath.c_str());
    }
  }

  // Extraction is synchronous through the gateway, so the extracted files are
  // already visible; still poll briefly for the ELF as a safety net.
  {
    int waitMs = 0;
    while (waitMs < 200 && !PM_SD().exists(elfPath.c_str())) {
      vTaskDelay(10 / portTICK_PERIOD_MS);
      waitMs += 10;
    }
  }

  Serial.println("Listing /apps/temp:");
  {
    std::vector<PocketmageSD::DirItem> items;
    PM_SD().listDirItems(TEMP_DIR, items);
    for (const auto& item : items) {
      Serial.printf("  %s%s\n", item.name.c_str(), item.isDir ? "/" : "");
    }
  }

  if (elfPath.length() == 0 || !PM_SD().exists(elfPath.c_str())) {
    Serial.printf("ELF not found after extraction: %s\n", elfPath.c_str());
    cleanupAppsTempRecursive(TEMP_DIR);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  vTaskDelay(pdMS_TO_TICKS(100));  // Safe wait

  // --- Copy assets folder ---
  String assetsSrc = pathJoin(TEMP_DIR, "assets");
  String assetsDst = pathJoin("/assets", base);  // <-- correct target path

  if (PM_SD().statIsDir(assetsSrc.c_str())) {
    rmRF(assetsDst.c_str());  // clean old assets
    Serial.printf("Copying assets: %s -> %s\n", assetsSrc.c_str(), assetsDst.c_str());
    if (!copyAssetsFlat(assetsSrc.c_str(), assetsDst.c_str())) {
      Serial.println("Failed to copy assets!");
      g_installFailed = true;
      g_installDone = true;
      cleanupAppsTempRecursive(TEMP_DIR);
      delete p;
      vTaskDelete(NULL);
    }
  }

  // --- Install into the slot directory (no flashing: .elf runs in-process) ---
  char slotDir[32];
  elfAppSlotDir(p->slot, slotDir, sizeof(slotDir));
  if (!ensureDir(slotDir)) {
    Serial.printf("Failed to create slot dir %s\n", slotDir);
    cleanupAppsTempRecursive(TEMP_DIR);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  String elfDst = pathJoin(slotDir, base + ".app.elf");
  String iconDst = iconPath.length() > 0 ? pathJoin(slotDir, base + "_ICON.bin") : String();

  if (!PM_SD().exists(elfPath.c_str())) {
    Serial.printf("Failed to open: %s\n", elfPath.c_str());
    cleanupAppsTempRecursive(TEMP_DIR);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }
  size_t sz = 0;
  PM_SD().statPath(elfPath.c_str(), &sz, nullptr);
  Serial.printf("Installing %s (%u bytes) -> %s\n", elfPath.c_str(), sz, elfDst.c_str());

  if (!copyFile(elfPath.c_str(), elfDst.c_str())) {
    Serial.println("ELF copy failed");
    cleanupAppsTempRecursive(TEMP_DIR);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }
  g_installProgress = 75;

  if (iconDst.length() > 0) {
    if (!copyFile(iconPath.c_str(), iconDst.c_str())) {
      Serial.println("Icon copy failed (non-fatal)");
      iconDst = "";
    }
  }
  g_installProgress = 90;

  // Optional, but a stale manifest must not outlive the app it names
  String propsSrc = pathJoin(TEMP_DIR, "app.properties");
  String propsDst = pathJoin(slotDir, "app.properties");
  if (PM_SD().exists(propsSrc.c_str())) {
    if (!copyFile(propsSrc.c_str(), propsDst.c_str())) {
      Serial.println("app.properties copy failed (non-fatal)");
    }
  } else if (PM_SD().exists(propsDst.c_str())) {
    PM_SD().removeFile(propsDst.c_str());
  }

  // --- Overlay binary (optional) ---
  String overlayDst = pathJoin(slotDir, base + ".elfo");
  String overlaySrc = "";
  {
    String want1 = base + ".elfo";
    String want2 = base + ".app.elfo";
    std::vector<PocketmageSD::DirItem> items;
    PM_SD().listDirItems(TEMP_DIR, items);
    for (const auto& item : items) {
      String name = item.name;
      int sep = name.lastIndexOf('/');
      if (sep >= 0)
        name = name.substring(sep + 1);
      if (name.startsWith("._")) {
        continue;
      }
      if (name.equalsIgnoreCase(want1) || name.equalsIgnoreCase(want2) || name.endsWith(".elfo")) {
        // prefer exact matches for base name
        if (name.equalsIgnoreCase(want1) || name.equalsIgnoreCase(want2)) {
          overlaySrc = pathJoin(TEMP_DIR, name);
          break;
        }
        if (overlaySrc.length() == 0) {
          overlaySrc = pathJoin(TEMP_DIR, name);
        }
      }
    }
  }
  if (overlaySrc.length() > 0) {
    if (PM_SD().exists(overlayDst.c_str()))
      PM_SD().removeFile(overlayDst.c_str());
    if (copyFile(overlaySrc.c_str(), overlayDst.c_str())) {
      Serial.printf("Overlay copied: %s -> %s\n", overlaySrc.c_str(), overlayDst.c_str());
    } else {
      Serial.printf("Overlay copy failed (non-fatal): %s\n", overlaySrc.c_str());
    }
  }

  // --- Save ElfAppInfo ---
  ElfAppInfo info = {};
  strncpy(info.name, base.c_str(), sizeof(info.name) - 1);
  strncpy(info.elfPath, elfDst.c_str(), sizeof(info.elfPath) - 1);
  strncpy(info.iconPath, iconDst.c_str(), sizeof(info.iconPath) - 1);

  clearElfAppInfo(p->slot);
  if (!saveElfAppInfo(p->slot, info)) {
    Serial.printf("Failed to save ElfAppInfo for slot %d\n", p->slot);
    g_installFailed = true;
  } else {
    Serial.println("Install OK");
  }

  cleanupAppsTempRecursive(TEMP_DIR);

  g_installProgress = 100;
  g_installDone = true;

  delete p;
  vTaskDelete(NULL);
}

// ---------- Async API ----------
bool installAppTarToSlotAsync(const char* tarRelName, int slot) {
  auto* params = new InstallTaskParams;
  strncpy(params->tarRelName, tarRelName, sizeof(params->tarRelName) - 1);
  params->tarRelName[sizeof(params->tarRelName) - 1] = '\0';
  params->slot = slot;

  BaseType_t res = xTaskCreate(installTask, "installTask",
                               12288,  // stack size (increase if extraction is large)
                               params, 1, NULL);

  if (res != pdPASS) {
    Serial.println("Failed to create install task");
    delete params;
    return false;
  }
  return true;
}

// ---------- Helpers ----------

void drawProgressBar(uint8_t progress) {
  uint progressPx = map(progress, 0, 100, 1, 216);

  u8g2.clearBuffer();
  // Draw rounded rectangle border
  u8g2.drawRFrame(20, 3, 216, 16, 5);
  // Draw progress bar
  if (progressPx > 10)
    u8g2.drawRBox(20, 3, progressPx, 16, 5);

  // Show text
  String progressText = "";
  if (progress < 52)
    progressText = TR(STR_APPLOADER_EXTRACTING);
  else
    progressText = TR(STR_APPLOADER_INSTALLING);
  int pw = FontEngine::textWidth(DisplayTarget::OLED, progressText, FontStyle::Body);
  FontEngine::drawText(DisplayTarget::OLED, (u8g2.getDisplayWidth() - pw) / 2,
                       u8g2.getDisplayHeight() - 3, progressText, FontStyle::Body);

  u8g2.sendBuffer();
}

// ---------- Operations ----------

void APPLOADER_INIT() {
  currentLine = "";
  CurrentAppState = APPLOADER;
  CurrentAppLoaderState = MENU;
  KB().setKeyboardState(NORMAL);
  newState = true;
}

void processKB_APPLOADER() {
  int currentMillis = millis();
  String outPath = "";
  char inchar = 0;

  switch (CurrentAppLoaderState) {
    case MENU:
      // 1. Drain the hardware buffer continuously at loop speed
      inchar = KB().updateKeypress();

      // 2. Only process the actual input if the cooldown has expired
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
        if (inchar != 0) {
          KBBounceMillis = currentMillis;

          // HANDLE INPUTS
          if (inchar == 12 || inchar == 23) {  // Home or App Switcher Kill Signal
            HOME_INIT();
          } else if (inchar == 'a' || inchar == 'A' || inchar == '1') {
            selectedSlot = 1;
            CurrentAppLoaderState = SWAP_OR_EDIT;
            KB().setKeyboardState(NORMAL);
          } else if (inchar == 'b' || inchar == 'B' || inchar == '2') {
            selectedSlot = 2;
            CurrentAppLoaderState = SWAP_OR_EDIT;
            KB().setKeyboardState(NORMAL);
          } else if (inchar == 'c' || inchar == 'C' || inchar == '3') {
            selectedSlot = 3;
            CurrentAppLoaderState = SWAP_OR_EDIT;
            KB().setKeyboardState(NORMAL);
          } else if (inchar == 'd' || inchar == 'D' || inchar == '4') {
            selectedSlot = 4;
            CurrentAppLoaderState = SWAP_OR_EDIT;
            KB().setKeyboardState(NORMAL);
          }
          // All other keys are ignored in the MENU state
        }
      }

      // 3. Update OLED at true OLED_MAX_FPS, completely independent of keyboard bounce
      currentMillis = millis();
      if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
        OLEDFPSMillis = currentMillis;
        OLED().oledWord(TR(STR_APPLOADER_CHOOSE_SLOT));
      }
      break;

    case SWAP_OR_EDIT:
      // 1. Drain the hardware buffer continuously at loop speed
      inchar = KB().updateKeypress();

      // 2. Only process the actual input if the cooldown has expired
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
        if (inchar != 0) {
          KBBounceMillis = currentMillis;

          if (inchar == 12 || inchar == 23) {  // Home or App Switcher Kill Signal
            selectedSlot = 0;
            CurrentAppLoaderState = MENU;
            newState = true;  // Trigger e-ink redraw of the menu
          } else if (inchar == 'S' || inchar == 's' || inchar == '!') {
            CurrentAppLoaderState = SWAP;
          } else if (inchar == 'D' || inchar == 'd' || inchar == '$') {
            // Clear the slot: drop metadata and remove the slot directory
            clearElfAppInfo(selectedSlot);

            char slotDir[32];
            elfAppSlotDir(selectedSlot, slotDir, sizeof(slotDir));
            if (rmRF(slotDir)) {
              Serial.printf("Slot %d cleared\n", selectedSlot);
            }

            OLED().sysMessage(TR(STR_APPLOADER_APP_REMOVED), 2000);

            newState = true;
            CurrentAppLoaderState = MENU;
          }
        }
      }

      // 3. Update OLED at true OLED_MAX_FPS, completely independent of keyboard bounce
      currentMillis = millis();
      if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
        OLEDFPSMillis = currentMillis;
        OLED().oledWord(TR(STR_APPLOADER_SWAP_DELETE));
      }
      break;

    case SWAP:
      outPath = fileWizardMini(false, APP_DIRECTORY, 0);

      // Catch standard exits as well as the App Switcher 23 kill signal string
      if (outPath == "_EXIT_" || outPath == "_APP_SWITCH_" || outPath == "_RETURN_") {
        // Return to menu
        CurrentAppLoaderState = MENU;
        newState = true;
        break;
      } else if (outPath != "") {
        // Reject MacOS metadata files disguised as tar files
        if (outPath.indexOf("/._") != -1) {
          OLED().sysMessage(TR(STR_APPLOADER_MACOS_META), 2000);
          CurrentAppLoaderState = MENU;
          newState = true;  // Force background redraw just in case
          break;
        }

        // Ensure file is a .tar
        if (outPath.endsWith(".tar") || outPath.endsWith(".TAR")) {
          // Strip leading APP_DIRECTORY + '/' so installer gets relative path
          String relName = outPath;
          if (relName.startsWith(APP_DIRECTORY "/")) {
            relName = relName.substring(strlen(APP_DIRECTORY) + 1);
          }

          Serial.printf("Installing app from %s (rel=%s) into slot %d\n", outPath.c_str(),
                        relName.c_str(), selectedSlot);

          installAppTarToSlotAsync(relName.c_str(), selectedSlot);
          CurrentAppLoaderState = INSTALLING;
        } else {
          OLED().sysMessage(TR(STR_APPLOADER_NOT_TAR), 2000);
          CurrentAppLoaderState = MENU;
          newState = true;
        }
      }
      break;

    case INSTALLING:
      // Update OLED progress. Extraction maps onto the first half of the
      // bar (gateway reports raw 0-100 there), the install phase owns the
      // second half; max() keeps the bar monotonic across the handover.
      if (!g_installDone) {
        uint8_t shown = (uint8_t)(g_extractProgress / 2);
        if (g_installProgress > shown)
          shown = g_installProgress;
        drawProgressBar(shown);
      } else {
        vTaskDelay(pdMS_TO_TICKS(500));
        if (g_installFailed) {
          OLED().sysMessage(TR(STR_APPLOADER_INSTALL_FAILED), 2000);
        } else {
          OLED().sysMessage(TR(STR_APPLOADER_INSTALL_COMPLETE), 2000);
        }
        newState = true;
        CurrentAppLoaderState = MENU;
      }
      break;
  }
}

void einkHandler_APPLOADER() {
  switch (CurrentAppLoaderState) {
    case MENU:
      if (newState) {
        newState = false;
        beginEinkScreen(true);
        display.drawBitmap(0, 0, _appLoader, 320, 218, GxEPD_BLACK);

        loadAndDrawAppIcon(42, 146, 1, true, kGridLabelMaxW);   // Slot 1
        loadAndDrawAppIcon(106, 146, 2, true, kGridLabelMaxW);  // Slot 2
        loadAndDrawAppIcon(174, 146, 3, true, kGridLabelMaxW);  // Slot 3
        loadAndDrawAppIcon(238, 146, 4, true, kGridLabelMaxW);  // Slot 4

        endEinkScreen(TR(STR_APPLOADER_TYPE_LETTER), EinkRefresh::Normal);
      }
      break;
  }
}
#endif
