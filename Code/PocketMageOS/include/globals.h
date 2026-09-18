#ifndef GLOBALS_H
#define GLOBALS_H

// LIBRARIES
#include <USBMSC.h>
#include <SD_MMC.h>
#include <SD.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <pocketmage.h>
// OTA_APP: remove assets.h + assets.cpp, and OS_APPS/, follow OTA_APP: tag instructions in codebase
#include <assets.h> // OTA_APP: remove
// Shared state (vspi/hspi/global_fs/prefs, settings, AppState/KBState,
// getBatteryVoltage) is owned by the SDK.
#include <pocketmage_globals.h>

// ===================== SYSTEM STATE =====================
extern int OLEDFPSMillis;                       // Last OLED FPS update time
extern int KBBounceMillis;                      // Last keyboard debounce time
extern volatile bool newState;                  // App state changed
extern volatile bool disableTimeout;            // Disable timeout globally
extern bool fileLoaded;     
extern unsigned int flashMillis;                // Flash timing

extern String OTA1_APP;
extern String OTA2_APP;
extern String OTA3_APP;
extern String OTA4_APP;

// ===================== APP STATES =====================
extern const unsigned char *appIcons[11];       // App icons

// ===================== TASKS APP =====================
extern std::vector<std::vector<String>> tasks;  // Task list

// ===================== HOME APP =====================
enum HOMEState { HOME_HOME, NOWLATER };         // Home app states
extern HOMEState CurrentHOMEState;              // Current home state

// ===================== PocketMage APP PROTOTYPES =====================
// <APP.cpp>
void APP_INIT();
void processKB_APP();
void einkHandler_APP();

//utils.cpp
void printDebug();
void checkTimeout();
void wakeFromNowlater(char bootKey = 0);
void updateBattState();
String textPrompt(String promptText = "", String prefix = "", bool mask = false, bool lockGlyph = false);
int boolPrompt(String promptText = "Are you sure?");
void waitForKeypress(String message = "Press any button to continue...");
String datePrompt(String defaultYYYYMMDD = "");
int timePrompt(int defaultTime = -1);
bool applyDateFromPrompt();
bool applyTimeFromPrompt();
void runClockSetupFlow(bool ask);
#if !OTA_APP
void saveEditingFile(); // OTA_APP: Remove saveEditingFile
#endif
// <PocketMage>
void einkHandler(void *parameter);


// OTA_APP: Remove all pocketmage v3 prototypes below this line
#if !OTA_APP // POCKETMAGE_OS

// <UTILS.cpp>
// Maps a boot shortcut letter (pressed while the device is off / on NOWLATER)
// to its app; returns the current app unchanged when there is no match.
AppState bootShortcutApp(char inchar);

// <LOCK.cpp>
extern volatile bool deviceLocked;      // Transient lock requirement; see LOCK.cpp
bool lockIsEnabled();
bool lockHasPin();
bool lockPinValid(const String& pin);
void lockSetPin(const String& pin);
void lockDisable();
bool lockVerifyPin(const String& pin);
void lockEnsureUnlocked();
void einkHandler_LOCK();

// <FILEWIZ.cpp>
void processKB_FILEWIZ();
void einkHandler_FILEWIZ();
String fileWizardMini(bool allowRecentSelect = false, String rootDir = "/", char inchar_ = 0);

// <TXT.cpp>
void TXT_INIT_JournalMode();
void processKB_TXT_NEW();
void einkHandler_TXT_NEW();
void saveMarkdownFile(const String& path);

// <HOME.cpp>
void HOME_INIT();
void einkHandler_HOME();
void processKB_HOME();
String commandSelect(String command);
void mageIdle(bool internalRefresh = true);
void resetIdle();

// <TASKS.cpp>
void sortTasksByDueDate(std::vector<std::vector<String>> &tasks);
void updateTaskArray();
void einkHandler_TASKS();
void processKB_TASKS();

// <settings.cpp>
void processKB_SETTINGS();
void einkHandler_SETTINGS();
String settingCommandSelect(String command);

// <ONBOARDING.cpp>
void ONBOARDING_INIT();
void processKB_ONBOARDING();
void einkHandler_ONBOARDING();

// <USB.cpp>
void processKB_USB();
void einkHandler_USB();

// <CALENDAR.cpp>
void processKB_CALENDAR();
void einkHandler_CALENDAR();

// <LEXICON.cpp>
void processKB_LEXICON();
void einkHandler_LEXICON();

// <JOURNAL.cpp>
void processKB_JOURNAL();
void einkHandler_JOURNAL();
String getCurrentJournal();

// <APPLOADER.cpp>
void processKB_APPLOADER();
void einkHandler_APPLOADER();
void rebootToAppSlot(int otaIndex);
void loadAndDrawAppIcon(int x, int y, int otaIndex, bool showName = true, int maxNameWidth = kGridLabelMaxW);

// <TERMINAL.cpp>
void processKB_TERMINAL();
void einkHandler_TERMINAL();
void termPrint(const String& line);
void termReturnToPrompt();
// Wrench
const char* readCFile(const String& path);
void compileWrench(const char* wrenchCode);

// Terminal/SSH shared layout (small font page). Owned by TERMINAL.cpp and
// SSH.cpp; SSH uses these to size the VT100 screen buffer.
constexpr int TERM_X             = 5;    // output text x
constexpr int TERM_SMALL_Y0      = 10;   // first baseline, small font (Terminal)
constexpr int TERM_SMALL_STEP    = 14;   // row pitch, small font
constexpr int TERM_SMALL_LINES   = 17;   // lines per page, small font

// <SSH.cpp>
void sshStartSession(const String& host, const String& user, int port);
bool sshBusy();
const char* sshLastMessage();
void sshRequestDisconnect();
void sshProcessKB();
void sshEinkHandler();
bool sshCommand(const String& command);

// <COMM.cpp>
void processKB_COMM();
void einkHandler_COMM();

#endif // POCKETMAGE_OS

#endif // GLOBALS_H
