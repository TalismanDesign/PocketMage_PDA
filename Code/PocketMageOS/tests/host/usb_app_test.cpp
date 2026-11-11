#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

using std::min;
using String = std::string;
using esp_err_t = int;
using gpio_num_t = int;
using esp_event_base_t = int;
constexpr int ESP_OK = 0;
constexpr int ESP_FAIL = -1;
constexpr int ARDUINO_USB_EVENTS = 1;
constexpr int ARDUINO_USB_STARTED_EVENT = 2;
constexpr int ARDUINO_USB_STOPPED_EVENT = 3;
constexpr int ARDUINO_USB_SUSPEND_EVENT = 4;
constexpr int ARDUINO_USB_RESUME_EVENT = 5;
constexpr int NORMAL = 0;
constexpr int POWER_SAVE_FREQ = 40;
constexpr int PWR_BTN = 0;
constexpr int FALLING = 0;
constexpr int SD_CLK = 1, SD_CMD = 2, SD_D0 = 3;
constexpr int SDMMC_FREQ_DEFAULT = 20000, SDMMC_HOST_SLOT_1 = 1;
constexpr int SDMMC_SLOT_FLAG_INTERNAL_PULLUP = 1;
constexpr uint32_t KB_COOLDOWN = 50;
constexpr size_t kTestSectorBytes = 512;
constexpr size_t kTestSectors = 4;
#define PM_TARGET_HOST 1
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGV(...) ((void)0)
#define TR(value) String(#value)

static std::vector<std::string> calls;
static uint32_t clockMs = 1000;
static uint32_t KBBounceMillis = 0, OLEDFPSMillis = 0;
static bool disableTimeout = false, SAVE_POWER = true, mscEnabled = false;
static bool sinkEnabled = false, HIDInitialized = false, deviceLocked = false;
static bool newState = false;
static int TIMEOUT = 120, OLED_BRIGHTNESS = 255, OLED_MAX_FPS = 60;
static bool DEBUG_VERBOSE, SYSTEM_CLOCK, SHOW_YEAR, ALLOW_NO_MICROSD;
static bool HOME_ON_BOOT, FAST_REFRESH, MUTE_BUZZER;
static bool preferHome = false, lockEnabled = false;
#define POCKETMAGE_HW_VERSION 3
enum class Lang { English };
namespace I18n { static void setLanguage(Lang) {} }
struct { void setContrast(int) {} } u8g2;
enum AppState { HOME, USB_APP, ELFAPP, ONBOARDING, TXT, FILEWIZ, SETTINGS, TASKS,
                COMM, CALENDAR, LEXICON, JOURNAL, TERMINAL, APPLOADER };
static AppState CurrentAppState = HOME;
static int cpu = POWER_SAVE_FREQ;
static bool stopSucceeds = true, claimSucceeds = true;
static bool mscSucceeds = true, deviceSucceeds = true;
static bool mediaPresent = false, restarted = false, sdBusy = false;
static bool fakeOpenStreams = false;
static int unmountResult = 0;
static unsigned unmountCalls = 0;
static unsigned readCalls = 0, writeCalls = 0;
static std::array<uint8_t, kTestSectorBytes * kTestSectors> disk;
static uint32_t millis() { return clockMs; }
static void delay(uint32_t ms) { clockMs += ms; }
static const char* esp_err_to_name(int) { return "test-error"; }
static void esp_restart() { restarted = true; }
static bool elfAppRunning() { return CurrentAppState == ELFAPP; }
static void HOME_INIT() { CurrentAppState = HOME; }
static void ONBOARDING_INIT() { CurrentAppState = ONBOARDING; }
static void TXT_INIT() { CurrentAppState = TXT; }
static void FILEWIZ_INIT() { CurrentAppState = FILEWIZ; }
static void SETTINGS_INIT() { CurrentAppState = SETTINGS; }
static void TASKS_INIT() { CurrentAppState = TASKS; }
static void COMM_INIT() { CurrentAppState = COMM; }
static void CALENDAR_INIT() { CurrentAppState = CALENDAR; }
static void LEXICON_INIT() { CurrentAppState = LEXICON; }
static void JOURNAL_INIT() { CurrentAppState = JOURNAL; }
static void TERMINAL_INIT() { CurrentAppState = TERMINAL; }
static void APPLOADER_INIT() { CurrentAppState = APPLOADER; }
static int digitalPinToInterrupt(int pin) { return pin; }
static void detachInterrupt(int) {}
static void attachInterrupt(int, void (*)(), int) {}
static void init_USBHID() { calls.push_back("hid-start"); }

namespace pocketmage {
void setCpuSpeed(int value);
static void PWR_BTN_irq() {}
}
enum class WifiRadioState { Off };
struct { WifiRadioState getState() { return WifiRadioState::Off; } } P_WIFI;
constexpr int WIFI_CPU_FREQ_MHZ = 240;
static int getCpuFrequencyMhz() { return cpu; }
static bool setCpuFrequencyMhz(int value) { cpu = value; return true; }

using SemaphoreHandle_t = std::recursive_mutex*;
constexpr int pdTRUE = 1, portMAX_DELAY = -1;
static std::recursive_mutex fakeIoMutex, fakeRoleMutex;
static SemaphoreHandle_t xSemaphoreCreateMutex() { return &fakeIoMutex; }
static SemaphoreHandle_t xSemaphoreCreateRecursiveMutex() { return &fakeRoleMutex; }
static int xSemaphoreTake(SemaphoreHandle_t mutex, int) { mutex->lock(); return pdTRUE; }
static int xSemaphoreGive(SemaphoreHandle_t mutex) { mutex->unlock(); return pdTRUE; }
static int xSemaphoreTakeRecursive(SemaphoreHandle_t mutex, int timeout) {
  return xSemaphoreTake(mutex, timeout);
}
static int xSemaphoreGiveRecursive(SemaphoreHandle_t mutex) { return xSemaphoreGive(mutex); }

struct sdmmc_card_t {
  struct { uint32_t capacity = kTestSectors; uint32_t sector_size = kTestSectorBytes; } csd;
};
static sdmmc_card_t mountedCard;
static sdmmc_card_t* getSdCard() { return &mountedCard; }
struct sdmmc_host_t { int max_freq_khz = 0; };
struct sdmmc_slot_config_t { int clk, cmd, d0, d1, d2, d3, width, flags = 0; };
#define SDMMC_HOST_DEFAULT() sdmmc_host_t{}
#define SDMMC_SLOT_CONFIG_DEFAULT() sdmmc_slot_config_t{}
struct esp_vfs_fat_sdmmc_mount_config_t {
  bool format_if_mount_failed;
  int max_files, allocation_unit_size;
};
static int esp_vfs_fat_sdmmc_unmount() { calls.push_back("destroy-card"); return ESP_OK; }
static int esp_vfs_fat_sdmmc_mount(const char*, sdmmc_host_t*, sdmmc_slot_config_t*,
                                  esp_vfs_fat_sdmmc_mount_config_t*, sdmmc_card_t**) {
  return ESP_OK;
}
static int sdmmc_host_init() { return ESP_OK; }
static int sdmmc_host_init_slot(int, sdmmc_slot_config_t*) { return ESP_OK; }
static int sdmmc_card_init(sdmmc_host_t*, sdmmc_card_t* value) {
  *value = mountedCard;
  return ESP_OK;
}
static int sdmmc_host_deinit() { calls.push_back("destroy-host"); return ESP_OK; }
static int sdmmc_read_sectors(sdmmc_card_t*, void* buf, size_t lba, size_t count) {
  ++readCalls;
  if (lba + count > kTestSectors) return ESP_FAIL;
  std::memcpy(buf, disk.data() + lba * kTestSectorBytes, count * kTestSectorBytes);
  return ESP_OK;
}
static int sdmmc_write_sectors(sdmmc_card_t*, const void* buf, size_t lba, size_t count) {
  ++writeCalls;
  if (lba + count > kTestSectors) return ESP_FAIL;
  std::memcpy(disk.data() + lba * kTestSectorBytes, buf, count * kTestSectorBytes);
  return ESP_OK;
}

struct FakeSd {
  void setEditingFile(String) {}
  void beginIO() { sdBusy = true; }
  void endIO() { sdBusy = false; }
  bool getNoSD() { return false; }
};
static FakeSd sd;
static FakeSd& PM_SDAUTO() { return sd; }
static FakeSd& PM_SD() { return sd; }
namespace pm_sd_gateway {
using BYTE = uint8_t;
using FRESULT = int;
constexpr int FF_VOLUMES = 2, FR_OK = 0;
static std::atomic<bool> s_usbAccess{false};
static bool s_needRemount = true;
static bool streamsOpen() { return fakeOpenStreams; }
static bool usbAccessActive() { return s_usbAccess; }
static BYTE ff_diskio_get_pdrv_card(const sdmmc_card_t*) { return 0; }
static FRESULT f_mount(void* fs, const char* path, int) {
  assert(fs == nullptr && std::strcmp(path, "0:") == 0);
  ++unmountCalls;
  return unmountResult;
}
static bool unmountForUsb();
static bool beginUsbAccess() {
  calls.push_back("claim-card");
  return claimSucceeds && unmountForUsb();
}
}
struct FakeAudio {
  bool pollUsb() { return false; }
  bool stopChecked() { calls.push_back("stop-audio"); return stopSucceeds; }
  String getLastError() { return "audio shutdown failed"; }
};
static FakeAudio P_AUDIO;
struct FakePower {
  bool esp = false, boost = false, needBoost = false;
  void setUSBControlESP() { esp = true; }
  void setUSBControlBMS() { esp = false; }
  bool getOTGNeed(bool& value) { value = needBoost; return true; }
  bool getBoostState(bool& value) { value = boost; return true; }
  bool setBoost(bool value) { boost = value; return true; }
} PowerSystem;
struct FakeOled {
  String last;
  bool asleep = true;
  void setPowerSave(bool value) { asleep = value; }
  void oledWord(String text, bool = false, bool = true, String = "") { last = text; }
  void oledLine(String text, size_t, bool) { last = text; }
  void sysMessage(String text, int) { last = text; }
};
static FakeOled oled;
static FakeOled& OLED() { return oled; }
struct PocketmageKB {
  char next = 0;
  void setKeyboardState(int) {}
  char updateKeypress() { char value = next; next = 0; return value; }
  void checkUSBKB();
};
static PocketmageKB kb;
static PocketmageKB& KB() { return kb; }
struct FakeKeypad { void flush() { kb.next = 0; } } keypad;
struct FakePrefs {
  bool begin(const char*, bool) { return true; }
  size_t putInt(const char*, int) { return sizeof(int); }
  size_t putBool(const char*, bool) { return sizeof(bool); }
  int getInt(const char*, int fallback) { return fallback; }
  String getString(const char*, const char* fallback) { return fallback; }
  bool getBool(const char* key, bool fallback) {
    if (std::strcmp(key, "HOME_ON_BOOT") == 0) return preferHome;
    if (std::strcmp(key, "LOCK_ENABLED") == 0) return lockEnabled;
    if (std::strcmp(key, "Onboarded") == 0) return true;
    return fallback;
  }
  void end() {}
} prefs;
static void beginEinkScreen() {}
static void endEinkScreen(String) {}
static const uint8_t _usb[] = {};
constexpr int GxEPD_BLACK = 0;
struct { void drawBitmap(int, int, const uint8_t*, int, int, int) {} } display;
struct USBMSC {
  void mediaPresent(bool value) { ::mediaPresent = value; }
  void end() { calls.push_back("msc-stop"); }
  void vendorID(const char*) {}
  void productID(const char*) {}
  void productRevision(const char*) {}
  template <typename T> void onRead(T) {}
  template <typename T> void onWrite(T) {}
  template <typename T> void onStartStop(T) {}
  bool begin(uint32_t, uint16_t) { calls.push_back("msc-start"); return mscSucceeds; }
};
struct FakeUsb {
  template <typename T> void onEvent(T) {}
  bool begin() { calls.push_back("device-start"); return deviceSucceeds; }
} USB;

#include "usb_role_hardware.h"
#include <pocketmage_sys/usb_role.h>
#include "usb_role.inc"
namespace pm_keyboard {
static bool stopSucceeds = true;
static bool stopping() { return false; }
static bool beginUsb() {
  if (!pm_usb::claim(pm_usb::Owner::Keyboard)) return false;
  calls.push_back("hid-start");
  return true;
}
static bool stopUsb() {
  if (pm_usb::owner() != pm_usb::Owner::Keyboard) return true;
  calls.push_back("hid-stop");
  return stopSucceeds && pm_usb::release(pm_usb::Owner::Keyboard);
}
}
#include "usb_app.inc"

static void require(bool value, const char* message) {
  if (!value) { std::cerr << message << '\n'; std::exit(1); }
}

int main(int argc, char** argv) {
  require(argc == 2, "scenario required");
  const std::string scenario = argv[1];
  if (scenario == "keyboard-unplug") {
    require(pm_usb::claim(pm_usb::Owner::Keyboard), "keyboard must reserve the connector");
    HIDInitialized = true;
    KB().checkUSBKB();
    require(pm_usb::owner() == pm_usb::Owner::DebugSerial && serialPhy,
            "keyboard unplug must restore serial without rebooting");
    PowerSystem.needBoost = true;
    clockMs += 500;
    KB().checkUSBKB();
    require(pm_usb::owner() == pm_usb::Owner::Keyboard,
            "keyboard must reconnect after teardown");
    return 0;
  }
  if (scenario == "keyboard-transfer") {
    require(pm_usb::claim(pm_usb::Owner::Keyboard), "keyboard must reserve the connector");
    USB_INIT();
    require(mscEnabled && pm_usb::owner() == pm_usb::Owner::Storage,
            "transfer must stop HID before claiming its PHY");
    require(calls[0] == "stop-audio" && calls[1] == "hid-stop" && calls[2] == "claim-card",
            "transfer must drain both hosts before unmounting FAT");
    return 0;
  }
  if (scenario == "serial-poll") {
    KB().checkUSBKB();
    require(PowerSystem.esp && !PowerSystem.boost,
            "idle polling must leave the connector on ESP serial without sourcing VBUS");
    require(calls.empty(), "idle polling must not start a USB host");
    return 0;
  }
  if (scenario == "audio-poll") {
    PowerSystem.needBoost = true;
    require(pm_usb::claim(pm_usb::Owner::Audio), "audio must reserve the connector");
    KB().checkUSBKB();
    require(PowerSystem.esp && PowerSystem.boost && !HIDInitialized && calls.empty(),
            "audio polling must power the speaker without starting a competing HID host");
    return 0;
  }
  if (scenario == "keyboard-busy") {
    pm_keyboard::stopSucceeds = false;
    require(pm_usb::claim(pm_usb::Owner::Keyboard), "keyboard must reserve the connector");
    USB_INIT();
    require(!mscEnabled && !mediaPresent && unmountCalls == 0,
            "busy keyboard PHY must prevent MSC from exposing or unmounting the card");
    return 0;
  }
  if (scenario.rfind("boot", 0) == 0) {
    preferHome = scenario == "boot-home-preference";
    lockEnabled = scenario == "boot-locked";
    loadState(true, 'u');
    require(CurrentAppState == USB_APP, "U must select USB at boot, including Home-on-boot");
    require(calls.empty(), "boot selection must defer USB hardware until after the PIN gate");
    if (lockEnabled) {
      processKB_USB();
      require(calls.empty(), "locked boot must not expose the disk");
      deviceLocked = false;
    }
    processKB_USB();
    require(mscEnabled, "USB must start once the boot input dispatcher runs unlocked");
    return 0;
  }
  if (scenario == "stop-failure") stopSucceeds = false;
  if (scenario == "card-busy") claimSucceeds = false;
  if (scenario == "msc-failure") mscSucceeds = false;
  if (scenario == "device-failure") deviceSucceeds = false;
  if (scenario == "open-stream") fakeOpenStreams = true;
  if (scenario == "unmount-failure") unmountResult = ESP_FAIL;
  USB_INIT();
  if (scenario == "home") {
    require(calls.size() >= 4 && calls[0] == "stop-audio" && calls[1] == "claim-card",
            "USB must stop the host and claim the mounted card before device startup");
    require(mscEnabled && CurrentAppState == USB_APP && !oled.asleep,
            "Home launch must start MSC and wake the OLED");
  } else if (scenario == "poll") {
    KB().checkUSBKB();
    require(PowerSystem.esp && mscEnabled, "keyboard polling disconnected active MSC");
  } else if (scenario.find("failure") != std::string::npos || scenario == "card-busy" ||
             scenario == "open-stream") {
    require(!mscEnabled && !mediaPresent, "failed startup must not publish a ready disk");
    require(CurrentAppState == USB_APP && !oled.last.empty(),
            "failed startup must retain a visible error in USB mode");
    if (scenario == "stop-failure" || scenario == "card-busy")
      require(std::find(calls.begin(), calls.end(), "device-start") == calls.end(),
              "failed resource handoff must not start the device controller");
    if (scenario == "open-stream")
      require(unmountCalls == 0, "open streams must prevent FAT unmount");
    if (scenario == "unmount-failure")
      require(!pm_sd_gateway::usbAccessActive(), "failed unmount must not grant raw access");
  } else if (scenario == "cpu-floor") {
    pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
    require(cpu >= 80, "power saving must not stall raw SDMMC I/O");
  } else if (scenario == "status") {
    clockMs += 1000;
    processKB_USB();
    const String waiting = oled.last;
    usbEventCallback(nullptr, ARDUINO_USB_EVENTS, ARDUINO_USB_STARTED_EVENT, nullptr);
    clockMs += 1000;
    processKB_USB();
    require(!waiting.empty() && oled.last != waiting, "OLED must show changing USB status");
  } else if (scenario == "bounds") {
    std::array<uint8_t, kTestSectorBytes * 2> buffer{};
    require(onRead(kTestSectors - 1, 0, buffer.data(), buffer.size()) == -1,
            "read past capacity must fail");
    require(readCalls == 0, "invalid requests must not reach the card driver");
    require(onWrite(0, kTestSectorBytes, buffer.data(), 1) == -1,
            "sector offset must be bounded");
    require(writeCalls == 0 && !sdBusy, "invalid writes must not touch the card");
  } else if (scenario == "partial") {
    disk.fill(0x71);
    std::array<uint8_t, 800> buffer;
    buffer.fill(0x29);
    require(onWrite(0, 400, buffer.data(), buffer.size()) == 800, "partial write failed");
    require(disk[399] == 0x71 && disk[1200] == 0x71, "partial write clobbered adjacent bytes");
    buffer.fill(0);
    require(onRead(0, 400, buffer.data(), buffer.size()) == 800, "partial read failed");
    require(std::all_of(buffer.begin(), buffer.end(), [](uint8_t b) { return b == 0x29; }),
            "partial read did not round-trip");
  } else if (scenario == "shutdown") {
    USBAppShutdown();
    uint8_t byte = 0;
    require(onRead(0, 0, &byte, 1) == -1 && !mediaPresent && !mscEnabled,
            "shutdown must revoke media and reject late I/O");
    require(std::find(calls.begin(), calls.end(), "destroy-host") == calls.end(),
            "USB must not destroy the SD service's borrowed card/host");
  }
}
