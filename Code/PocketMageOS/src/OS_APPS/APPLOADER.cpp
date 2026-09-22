// AUDIT 1
#include <globals.h>
#include <elf_runner.h>
#include <ESP32-targz.h>


#define APP_DIRECTORY   "/apps"
#define TEMP_DIR        "/apps/temp"
#define PREFS_NAMESPACE "AppLoader"
#if PM_TARGET_HOST // POCKETMAGE_OS
static String currentLine = "";

enum AppLoaderState {MENU, SWAP_OR_EDIT, INSTALLING, SWAP};
AppLoaderState CurrentAppLoaderState = MENU;

uint8_t selectedSlot = 0; //1:A, 2:B, etc.

// ---------- Globals ----------
volatile uint8_t g_installProgress = 0; // 0-100 (0-50: extract, 50-100: intall)
volatile bool g_installDone = false;
volatile bool g_installFailed = false;

// ---------- Utilities ----------
static bool ensureDir(fs::FS &fs, const char *path) {
    if (fs.exists(path)) return true;
    return fs.mkdir(path);
}

static bool rmRF(fs::FS &fs, const char *path) {
    File entry = fs.open(path);
    if (!entry) return true; // nothing to delete
    if (!entry.isDirectory()) {
        entry.close();
        return fs.remove(path);
    }

    File child;
    while ((child = entry.openNextFile())) {
        String name = child.name();
        int sep = name.lastIndexOf('/');
        if (sep >= 0) name = name.substring(sep + 1);
        String childPath = String(path) + "/" + name;
        child.close();
        if (!rmRF(fs, childPath.c_str())) { entry.close(); return false; }
    }
    entry.close();
    return fs.rmdir(path);
}

bool copyFile(fs::FS &fs, const char *src, const char *dst) {
  File in = fs.open(src, "r");
  if (!in) return false;

  File out = fs.open(dst, "w");
  if (!out) { in.close(); return false; }

  uint8_t buf[1024];
  while (in.available()) {
    size_t r = in.read(buf, sizeof(buf));
    if (out.write(buf, r) != r) {
      in.close(); out.close();
      return false;
    }
    vTaskDelay(1); // feed watchdog
  }

  in.close();
  out.close();
  return true;
}

// ---------- Place this at the top of the .cpp file, before installTask ----------
void copyDirRecursive(File src, const String &assetsSrc, const String &assetsDst) {
    while (true) {
        File entry = src.openNextFile();
        if (!entry) break;

        String name = entry.name();
        
        // Skip macOS metadata files
        if (name.startsWith("._")) {
            entry.close();
            continue;
        }

        String relative = name.substring(assetsSrc.length());
        String dstFile = assetsDst + relative;

        if (entry.isDirectory()) {
            ensureDir(*global_fs, dstFile.c_str());
            copyDirRecursive(entry, assetsSrc, assetsDst);
        } else {
            File dst = global_fs->open(dstFile.c_str(), FILE_WRITE);
            if (dst) {
                uint8_t buf[512];
                size_t len;
                while ((len = entry.read(buf, sizeof(buf))) > 0) {
                    dst.write(buf, len);
                }
                dst.close();
                Serial.printf("Copied %s -> %s\n", name.c_str(), dstFile.c_str());
            } else {
                Serial.printf("Failed to open destination file %s\n", dstFile.c_str());
            }
        }
        entry.close();
        
        vTaskDelay(1); 
    }
}

bool copyAssetsFlat(fs::FS &fs, const char *srcDir, const char *dstDir) {
  ensureDir(fs, dstDir);

  File dir = fs.open(srcDir);
  if (!dir || !dir.isDirectory()) return false;

  File f;
  while ((f = dir.openNextFile())) {
    String name = String(f.name());
    
    // Skip macOS metadata files
    if (name.startsWith("._")) {
        f.close();
        continue;
    }

    String srcPath = String(srcDir) + "/" + name;
    String dstPath = String(dstDir) + "/" + name;

    if (f.isDirectory()) {
      ensureDir(fs, dstPath.c_str());
      copyAssetsFlat(fs, srcPath.c_str(), dstPath.c_str());
    } else {
      if (!copyFile(fs, srcPath.c_str(), dstPath.c_str())) {
        f.close();
        dir.close();
        return false;
      }
    }
    f.close();
    vTaskDelay(1);
  }
  dir.close();
  return true;
}


static String basenameNoExt(const String &path, const char *ext = ".tar") {
  int slash = path.lastIndexOf('/');
  String name = (slash >= 0) ? path.substring(slash + 1) : path;
  if (name.endsWith(ext)) return name.substring(0, name.length() - (int)strlen(ext));
  return name;
}

// Join two paths safely (ensures exactly one slash between them)
static String pathJoin(const String &a, const String &b) {
  if (a.length() == 0) return b;
  if (b.length() == 0) return a;
  if (a.endsWith("/")) {
    if (b.startsWith("/")) return a + b.substring(1); // avoid double slash
    return a + b;
  } else {
    if (b.startsWith("/")) return a + b;
    return a + "/" + b;
  }
}

// ---------- Saving/Loading appInfo ----------

// Layout constants, app icon geometry
constexpr int APPLOADER_ICON_S   = kIconCellSize;  // app icon cell size
constexpr int APPLOADER_NAME_GAP = kIconNameGap;   // icon-to-name baseline gap

void loadAndDrawAppIcon(int x, int y, int slot, bool showName, int maxNameWidth) {
  pocketmage::setCpuSpeed(240);

  ElfAppInfo app;
  if (!loadElfAppInfo(slot, app)) {
    // Ensure CPU speed is reset if we abort early
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    return;
  }

  bool iconLoaded = false;
  uint8_t buf[APPLOADER_ICON_S * 5]; // 40x40 1-bit = 200 bytes

  if (global_fs->exists(app.iconPath)) {
    File f = global_fs->open(app.iconPath, "r");
    if (f) {
      if (f.read(buf, sizeof(buf)) == sizeof(buf)) {
        iconLoaded = true;
      }
      f.close();
    }
  }

  display.fillRect(x, y, APPLOADER_ICON_S, APPLOADER_ICON_S, GxEPD_WHITE);

  if (iconLoaded) {
    display.drawBitmap(x, y, buf, APPLOADER_ICON_S, APPLOADER_ICON_S, GxEPD_BLACK);
  } else {
    display.drawBitmap(x, y, noIcon, APPLOADER_ICON_S, APPLOADER_ICON_S, GxEPD_BLACK);
  }

  if (showName) {
    FontStyle nameStyle = FontEngine::fitStyle(DisplayTarget::EINK, app.name,
                                               maxNameWidth, kLabelCascade,
                                               kLabelCascadeCount);
    String appNameStr = truncateWithEllipsis(app.name, maxNameWidth, nameStyle);

    u8g2f.setForegroundColor(GxEPD_BLACK);

    int w = FontEngine::textWidth(DisplayTarget::EINK, appNameStr, nameStyle);

    int tx = x + (APPLOADER_ICON_S - w) / 2;
    int ty = y + APPLOADER_ICON_S + APPLOADER_NAME_GAP;

    FontEngine::drawText(DisplayTarget::EINK, tx, ty, appNameStr, nameStyle);
  }

  if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
}

void cleanupAppsTemp(String binPath) {
  // --- Cleanup TEMP_DIR, keep *_ICON.bin only ---
  File root = global_fs->open(TEMP_DIR);
  if (root && root.isDirectory()) {
    File entry;
    while ((entry = root.openNextFile())) {
      String name = entry.name();
      int sep = name.lastIndexOf('/');
      if (sep >= 0) name = name.substring(sep + 1);
      String fullPath = pathJoin(TEMP_DIR, name);
      entry.close();
      if (!name.endsWith("_ICON.bin")) {
        global_fs->remove(fullPath);
      }
    }
    root.close();
  }
}
bool cleanupAppsTempRecursive(fs::FS &fs, const String &dirPath) {
    File dir = fs.open(dirPath);
    if (!dir || !dir.isDirectory()) return false;

    File entry;
    while ((entry = dir.openNextFile())) {
        String name = entry.name();
        int sep = name.lastIndexOf('/');
        if (sep >= 0) name = name.substring(sep + 1);
        String fullPath = pathJoin(dirPath, name);

        if (entry.isDirectory()) {
            entry.close();
            cleanupAppsTempRecursive(fs, fullPath); // recurse
            // try to remove directory if empty
            fs.rmdir(fullPath.c_str());
        } else {
            entry.close();
            if (!name.endsWith("_ICON.bin")) {
                fs.remove(fullPath.c_str());
            }
        }
    }
    dir.close();
    return true;
}


// ---------- Install Task ----------

struct InstallTaskParams {
    char tarRelName[128];
    int slot; // 1..4
};

static void installTask(void *param) {
  pocketmage::setCpuSpeed(240);

  InstallTaskParams *p = (InstallTaskParams *)param;
  g_installProgress = 0;
  g_installDone = false;
  g_installFailed = false;

  String tarPath = pathJoin(APP_DIRECTORY, p->tarRelName);

  // --- Check TAR exists ---
  if (!global_fs->exists(tarPath.c_str())) {
    Serial.printf("Tar not found: %s\n", tarPath.c_str());
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  // --- Ensure directories ---
  if (!ensureDir(*global_fs, APP_DIRECTORY) ||
    !ensureDir(*global_fs, TEMP_DIR)) {
    Serial.println("Failed to prepare TEMP_DIR");
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  // --- TAR extraction ---
  TarUnpacker unpacker;
  unpacker.haltOnError(true);
  unpacker.setTarProgressCallback([](uint8_t progress) {
    uint8_t mappedProgress = progress / 2; // 0-50% for extraction
    
    // Never let the bar go backwards.
    if (mappedProgress > g_installProgress) {
        g_installProgress = mappedProgress;
    }
  });

  if (!unpacker.tarExpander(*global_fs, tarPath.c_str(), *global_fs, TEMP_DIR)) {
    Serial.printf("Extraction failed (err=%d)\n", unpacker.tarGzGetError());

    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);

    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  g_installProgress = 50; // halfway

// --- Determine main .bin and base name ---
String elfPath = "";
String base = "";
String iconPath = "";

File tempRoot = global_fs->open(TEMP_DIR);
if (tempRoot && tempRoot.isDirectory()) {
    File entry;
    while ((entry = tempRoot.openNextFile())) {
        String name = entry.name();
        int sep = name.lastIndexOf('/');
        if (sep >= 0) name = name.substring(sep + 1);

        // Skip macOS metadata files
        if (name.startsWith("._")) {
            entry.close();
            continue;
        }

        // --- Main .app.elf ---
        if (elfPath.length() == 0 && name.endsWith(".app.elf")) {
            elfPath = pathJoin(TEMP_DIR, name);

            // Derive base by stripping ".app.elf" (icon is <base>_ICON.bin)
            if (name.endsWith(".app.elf") && name.length() > 8) {
                base = name.substring(0, name.length() - 8);
            } else {
                int dot = name.lastIndexOf('.');
                base = (dot > 0) ? name.substring(0, dot) : name;
            }

            entry.close();
            continue;
        }
        entry.close();
    }
    tempRoot.close();
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
    File scan = global_fs->open(TEMP_DIR);
    if (scan && scan.isDirectory()) {
        File entry;
        while ((entry = scan.openNextFile())) {
            String name = entry.name();
            int sep = name.lastIndexOf('/');
            if (sep >= 0) name = name.substring(sep + 1);
            if (!name.startsWith("._") && name.equalsIgnoreCase(want)) {
                iconPath = pathJoin(TEMP_DIR, name);
                entry.close();
                break;
            }
            entry.close();
        }
        scan.close();
    }
    if (iconPath.length() == 0) {
        Serial.printf("Icon not found for app '%s' (non-fatal)\n", base.c_str());
    } else {
        Serial.printf("Icon found: %s\n", iconPath.c_str());
    }
}


// Wait up to ~200 ms for SD_MMC to see the files
int waitMs = 0;
while (waitMs < 200) {
    File tempRoot = global_fs->open(TEMP_DIR);
    bool found = false;
    if (tempRoot && tempRoot.isDirectory()) {
        File entry;
        while ((entry = tempRoot.openNextFile())) {
            String name = String(entry.name());

            // Skip macOS metadata files
            if (name.startsWith("._")) {
                entry.close();
                continue;
            }

            if (name.endsWith(".app.elf")) {
                elfPath = pathJoin(TEMP_DIR, name);
                found = true;
            }
            entry.close();
            if (found) break;
        }
        tempRoot.close();
    }
    if (found && global_fs->exists(elfPath.c_str())) break;

    vTaskDelay(10 / portTICK_PERIOD_MS);
    waitMs += 10;
}

Serial.println("Listing /apps/temp:");
tempRoot = global_fs->open(TEMP_DIR);
if (tempRoot && tempRoot.isDirectory()) {
    File entry;
    while ((entry = tempRoot.openNextFile())) {
        Serial.printf("  %s%s\n", entry.name(), entry.isDirectory() ? "/" : "");
        entry.close();
    }
    tempRoot.close();
}

if (elfPath.length() == 0 || !global_fs->exists(elfPath.c_str())) {
    Serial.printf("ELF not found after extraction: %s\n", elfPath.c_str());
    cleanupAppsTempRecursive(*global_fs, TEMP_DIR);
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
}

vTaskDelay(pdMS_TO_TICKS(100)); // Safe wait

// --- Copy assets folder ---
String assetsSrc = pathJoin(TEMP_DIR, "assets");
String assetsDst = pathJoin("/assets", base);   // <-- correct target path

if (global_fs->exists(assetsSrc.c_str())) {
    rmRF(*global_fs, assetsDst.c_str()); // clean old assets
    Serial.printf("Copying assets: %s -> %s\n", assetsSrc.c_str(), assetsDst.c_str());
    if (!copyAssetsFlat(*global_fs, assetsSrc.c_str(), assetsDst.c_str())) {
        Serial.println("Failed to copy assets!");
        g_installFailed = true;
        g_installDone = true;
        cleanupAppsTempRecursive(*global_fs, TEMP_DIR);
            if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
        delete p;
        vTaskDelete(NULL);
    }
}


  // --- Install into the slot directory (no flashing: .elf runs in-process) ---
  char slotDir[32];
  elfAppSlotDir(p->slot, slotDir, sizeof(slotDir));
  if (!ensureDir(*global_fs, slotDir)) {
    Serial.printf("Failed to create slot dir %s\n", slotDir);
    cleanupAppsTempRecursive(*global_fs, TEMP_DIR);
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }

  String elfDst = pathJoin(slotDir, base + ".app.elf");
  String iconDst = iconPath.length() > 0 ? pathJoin(slotDir, base + "_ICON.bin") : String();

  File f = global_fs->open(elfPath.c_str(), "r");
  if (!f) {
    Serial.printf("Failed to open: %s\n", elfPath.c_str());
    cleanupAppsTempRecursive(*global_fs, TEMP_DIR);
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }
  uint32_t sz = f.size();
  f.close();
  Serial.printf("Installing %s (%u bytes) -> %s\n", elfPath.c_str(), sz, elfDst.c_str());

  if (!copyFile(*global_fs, elfPath.c_str(), elfDst.c_str())) {
    Serial.println("ELF copy failed");
    cleanupAppsTempRecursive(*global_fs, TEMP_DIR);
    if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    g_installFailed = true;
    g_installDone = true;
    delete p;
    vTaskDelete(NULL);
  }
  g_installProgress = 75;

  if (iconDst.length() > 0) {
    if (!copyFile(*global_fs, iconPath.c_str(), iconDst.c_str())) {
      Serial.println("Icon copy failed (non-fatal)");
      iconDst = "";
    }
  }
  g_installProgress = 90;

  // --- Save ElfAppInfo (also clears legacy bin-era keys for this slot) ---
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

  cleanupAppsTempRecursive(*global_fs, TEMP_DIR);
  if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);

  g_installProgress = 100;
  g_installDone = true;

  delete p;
  vTaskDelete(NULL);
}

// ---------- Async API ----------
bool installAppTarToSlotAsync(const char *tarRelName, int slot) {
    auto *params = new InstallTaskParams;
    strncpy(params->tarRelName, tarRelName, sizeof(params->tarRelName) - 1);
    params->tarRelName[sizeof(params->tarRelName) - 1] = '\0';
    params->slot = slot;

    BaseType_t res = xTaskCreate(
        installTask,
        "installTask",
        12288, // stack size (increase if extraction is large)
        params,
        1,
        NULL
    );

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
  if (progressPx > 10) u8g2.drawRBox(20, 3, progressPx, 16, 5);

  // Show text
  String progressText = "";
  if (progress < 52) progressText = TR(STR_APPLOADER_EXTRACTING);
  else               progressText = TR(STR_APPLOADER_INSTALLING);
  int pw = FontEngine::textWidth(DisplayTarget::OLED, progressText, FontStyle::Body);
  FontEngine::drawText(DisplayTarget::OLED,
               (u8g2.getDisplayWidth() - pw)/2,
               u8g2.getDisplayHeight()-3, progressText, FontStyle::Body);

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
          if (inchar == 12 || inchar == 23) { // Home or App Switcher Kill Signal
            HOME_INIT();
          }
          else if (inchar == 'a' || inchar == 'A' || inchar == '1') {
            selectedSlot = 1;
            CurrentAppLoaderState = SWAP_OR_EDIT;
            KB().setKeyboardState(NORMAL);
          }
          else if (inchar == 'b' || inchar == 'B' || inchar == '2') {
            selectedSlot = 2;
            CurrentAppLoaderState = SWAP_OR_EDIT;
            KB().setKeyboardState(NORMAL);
          }
          else if (inchar == 'c' || inchar == 'C' || inchar == '3') {
            selectedSlot = 3;
            CurrentAppLoaderState = SWAP_OR_EDIT;
            KB().setKeyboardState(NORMAL);
          }
          else if (inchar == 'd' || inchar == 'D' || inchar == '4') {
            selectedSlot = 4;
            CurrentAppLoaderState = SWAP_OR_EDIT;
            KB().setKeyboardState(NORMAL);
          }
          // All other keys are ignored in the MENU state
        }
      }

      // 3. Update OLED at true OLED_MAX_FPS, completely independent of keyboard bounce
      currentMillis = millis();
      if (currentMillis - OLEDFPSMillis >= (1000/OLED_MAX_FPS)) {
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

          if (inchar == 12 || inchar == 23) { // Home or App Switcher Kill Signal
            selectedSlot = 0;
            CurrentAppLoaderState = MENU;
            newState = true; // Trigger e-ink redraw of the menu
          }
          else if (inchar == 'S' || inchar == 's' || inchar == '!') {
            CurrentAppLoaderState = SWAP;
          }
          else if (inchar == 'D' || inchar == 'd' || inchar == '$') {
            // Clear the slot: drop metadata and remove the slot directory
            clearElfAppInfo(selectedSlot);

            char slotDir[32];
            elfAppSlotDir(selectedSlot, slotDir, sizeof(slotDir));
            if (rmRF(*global_fs, slotDir)) {
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
      if (currentMillis - OLEDFPSMillis >= (1000/OLED_MAX_FPS)) {
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
      }
      else if (outPath != "") {
        // Reject MacOS metadata files disguised as tar files
        if (outPath.indexOf("/._") != -1) {
            OLED().sysMessage(TR(STR_APPLOADER_MACOS_META), 2000);
            CurrentAppLoaderState = MENU;
            newState = true; // Force background redraw just in case
            break;
        }

        // Ensure file is a .tar
        if (outPath.endsWith(".tar") || outPath.endsWith(".TAR")) {
          // Strip leading APP_DIRECTORY + '/' so installer gets relative path
          String relName = outPath;
          if (relName.startsWith(APP_DIRECTORY "/")) {
            relName = relName.substring(strlen(APP_DIRECTORY) + 1);
          }

          Serial.printf("Installing app from %s (rel=%s) into slot %d\n",
                        outPath.c_str(), relName.c_str(), selectedSlot);

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
      // Update OLED progress
      if (!g_installDone) {
        drawProgressBar(g_installProgress);
      } else {
        vTaskDelay(pdMS_TO_TICKS(500));
        if (g_installFailed) {
          OLED().sysMessage(TR(STR_APPLOADER_INSTALL_FAILED), 2000);
        } 
        else {
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

        loadAndDrawAppIcon(42 , 146, 1, true, kGridLabelMaxW);  // Slot 1
        loadAndDrawAppIcon(106, 146, 2, true, kGridLabelMaxW);  // Slot 2
        loadAndDrawAppIcon(174, 146, 3, true, kGridLabelMaxW);  // Slot 3
        loadAndDrawAppIcon(238, 146, 4, true, kGridLabelMaxW);  // Slot 4

        endEinkScreen(TR(STR_APPLOADER_TYPE_LETTER), EinkRefresh::Normal);
      }
      break;
  }
}
#endif