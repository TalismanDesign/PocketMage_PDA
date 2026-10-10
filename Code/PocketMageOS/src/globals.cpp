#include "globals.h"
#include "sdmmc_cmd.h"

// Shared state (vspi/hspi/global_fs/prefs, settings, CurrentAppState) lives in
// the SDK

// ===================== SYSTEM STATE =====================
int OLEDFPSMillis = 0;                   // Last OLED FPS update time
int KBBounceMillis = 0;                  // Last keyboard debounce time
volatile bool newState = false;          // App state changed
// SDK config.h defines exactly one of PM_TARGET_HOST / PM_TARGET_APP, so
// normalize both to 0/1 for value contexts below.
#ifndef PM_TARGET_APP
#define PM_TARGET_APP 0
#endif
#ifndef PM_TARGET_HOST
#define PM_TARGET_HOST 0
#endif
volatile bool disableTimeout = PM_TARGET_APP ? true: false;    // Disable timeout globally, PM_TARGET_APP: disable timeout by default
bool fileLoaded = false;    
unsigned int flashMillis = 0;            // Flash timing

// ===================== APP STATES =====================
#if PM_TARGET_HOST // POCKETMAGE_OS
const unsigned char *appIcons[11] = { _homeIcons2, _homeIcons3, _homeIcons4, _homeIcons5, _homeIcons6, taskIconTasks0, _homeIcons7, _homeIcons8, _homeIcons9, _homeIcons11, _homeIcons10}; // App icons
#endif

// ===================== TASKS APP =====================
std::vector<std::vector<String>> tasks;  // Task list

// ===================== HOME APP =====================
HOMEState CurrentHOMEState = HOME_HOME;  // Current home state
