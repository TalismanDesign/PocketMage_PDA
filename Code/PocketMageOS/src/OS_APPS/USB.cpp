#include <USB.h>
#include <USBMSC.h>
#include <atomic>
#include <climits>
#include <cstdlib>
#include <elf_runner.h>
#include <freertos/semphr.h>
#include <globals.h>
#include <pocketmage_kb/usb_keyboard.h>
#include <pocketmage_sd/sd_gateway.h>
#include <pocketmage_sys/usb_role.h>
#include <sdmmc_cmd.h>

#if PM_TARGET_HOST  // POCKETMAGE_OS
static constexpr const char* TAG = "USB";
static constexpr uint32_t kSectorBytes = 512;
static constexpr uint32_t kOledFrameMs = 100;
static USBMSC msc;
static sdmmc_card_t* card = nullptr;
static SemaphoreHandle_t ioMutex = nullptr;
static bool initAttempted = false;
static String startupError;
enum class UsbStatus { Waiting, Connected, Suspended, Ejected };
static std::atomic<UsbStatus> usbStatus{UsbStatus::Waiting};
static std::atomic<bool> usb_is_shutting_down{false};

class UsbIoLock {
 public:
  UsbIoLock() : locked_(ioMutex && xSemaphoreTake(ioMutex, portMAX_DELAY) == pdTRUE) {
    if (ioMutex && !locked_) {
      ESP_LOGE(TAG, "USB I/O mutex acquisition failed");
      std::abort();
    }
  }
  ~UsbIoLock() {
    if (locked_ && xSemaphoreGive(ioMutex) != pdTRUE) {
      ESP_LOGE(TAG, "USB I/O mutex release failed");
      std::abort();
    }
  }
  bool ready() const { return locked_ && !usb_is_shutting_down && card; }
  UsbIoLock(const UsbIoLock&) = delete;
  UsbIoLock& operator=(const UsbIoLock&) = delete;

 private:
  bool locked_;
};

static bool validTransfer(uint32_t lba, uint32_t offset, const void* buffer, uint32_t bytes) {
  if (!card || !buffer || bytes > INT32_MAX || offset >= kSectorBytes ||
      card->csd.sector_size != kSectorBytes || lba >= card->csd.capacity)
    return false;
  const uint64_t remaining = (uint64_t(card->csd.capacity) - lba) * kSectorBytes - offset;
  return bytes <= remaining;
}

static void drawUsbStatus() {
  if (startupError.length()) {
    OLED().oledWord(startupError);
    return;
  }
  switch (usbStatus.load()) {
    case UsbStatus::Waiting:
      OLED().oledWord(TR(STR_USB_WAITING));
      break;
    case UsbStatus::Connected:
      OLED().oledWord(TR(STR_USB_CONNECTED));
      break;
    case UsbStatus::Suspended:
      OLED().oledWord(TR(STR_USB_SUSPENDED));
      break;
    case UsbStatus::Ejected:
      OLED().oledWord(TR(STR_USB_EJECTED));
      break;
  }
}

static void failStartup(const String& message) {
  startupError = message;
  msc.mediaPresent(false);
  mscEnabled = false;
  ESP_LOGE(TAG, "USB startup failed: %s", message.c_str());
  drawUsbStatus();
}

/** Revoke MSC media and drain in-flight I/O before the caller reboots. */
void USBAppShutdown() {
  usb_is_shutting_down = true;
  UsbIoLock lock;
  ESP_LOGI(TAG, "Shutting down USB MSC...");
  msc.mediaPresent(false);
  msc.end();
  // The SD service owns this allocation and host. Its FAT volume stays
  // unmounted until reboot; no filesystem caches may survive host writes.
  card = nullptr;
  mscEnabled = false;
  PowerSystem.setUSBControlESP();
}

/** Close USB transfer and reboot to Home so FAT and the USB PHY start cleanly. */
void USB_EXIT() {
  USBAppShutdown();
  if (prefs.begin("PocketMage", false)) {
    if (prefs.putInt("CurrentAppState", static_cast<int>(HOME)) != sizeof(int) ||
        prefs.putBool("Seamless_Reboot", true) != sizeof(bool))
      ESP_LOGE(TAG, "Could not persist USB exit state");
    prefs.end();
  } else {
    ESP_LOGE(TAG, "Could not open preferences for USB exit");
  }
  pm_usb::restoreSerialForRestart();
  esp_restart();
}

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  UsbIoLock lock;
  if (!lock.ready() || !validTransfer(lba, offset, buffer, bufsize))
    return -1;

  PM_SDAUTO().beginIO();
  constexpr uint32_t secSize = kSectorBytes;

  uint32_t written = 0;

  if (offset > 0) {
    uint32_t chunk = min(secSize - offset, bufsize);
    uint8_t sector[secSize];
    if (sdmmc_read_sectors(card, sector, lba, 1) != ESP_OK) {
      PM_SDAUTO().endIO();
      return -1;
    }
    memcpy(sector + offset, buffer, chunk);
    if (sdmmc_write_sectors(card, sector, lba, 1) != ESP_OK) {
      PM_SDAUTO().endIO();
      return -1;
    }
    lba++;
    buffer += chunk;
    written += chunk;
  }

  uint32_t fullSectors = (bufsize - written) / secSize;
  if (fullSectors > 0) {
    if (sdmmc_write_sectors(card, buffer, lba, fullSectors) != ESP_OK) {
      PM_SDAUTO().endIO();
      return -1;
    }
    lba += fullSectors;
    buffer += fullSectors * secSize;
    written += fullSectors * secSize;
  }

  uint32_t remainder = (bufsize - written);
  if (remainder > 0) {
    uint8_t sector[secSize];
    if (sdmmc_read_sectors(card, sector, lba, 1) != ESP_OK) {
      PM_SDAUTO().endIO();
      return -1;
    }
    memcpy(sector, buffer, remainder);
    if (sdmmc_write_sectors(card, sector, lba, 1) != ESP_OK) {
      PM_SDAUTO().endIO();
      return -1;
    }
  }

  PM_SDAUTO().endIO();
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  UsbIoLock lock;
  if (!lock.ready() || !validTransfer(lba, offset, buffer, bufsize))
    return -1;

  PM_SDAUTO().beginIO();
  constexpr uint32_t secSize = kSectorBytes;

  uint8_t* buf = (uint8_t*)buffer;
  uint32_t read = 0;

  if (offset > 0) {
    uint32_t chunk = min(secSize - offset, bufsize);
    uint8_t sector[secSize];
    if (sdmmc_read_sectors(card, sector, lba, 1) != ESP_OK) {
      PM_SDAUTO().endIO();
      return -1;
    }
    memcpy(buf, sector + offset, chunk);
    lba++;
    buf += chunk;
    read += chunk;
  }

  uint32_t fullSectors = (bufsize - read) / secSize;
  if (fullSectors > 0) {
    if (sdmmc_read_sectors(card, buf, lba, fullSectors) != ESP_OK) {
      PM_SDAUTO().endIO();
      return -1;
    }
    lba += fullSectors;
    buf += fullSectors * secSize;
    read += fullSectors * secSize;
  }

  uint32_t remainder = (bufsize - read);
  if (remainder > 0) {
    uint8_t sector[secSize];
    if (sdmmc_read_sectors(card, sector, lba, 1) != ESP_OK) {
      PM_SDAUTO().endIO();
      return -1;
    }
    memcpy(buf, sector, remainder);
  }

  PM_SDAUTO().endIO();
  return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool eject) {
  ESP_LOGI(TAG, "MSC Start/Stop: power=%u, start=%d, eject=%d\n", power_condition, start, eject);
  if (eject && !start)
    usbStatus = UsbStatus::Ejected;
  else if (start)
    usbStatus = UsbStatus::Connected;
  return true;
}

static void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id,
                             void* event_data) {
  if (event_base == ARDUINO_USB_EVENTS) {
    switch (event_id) {
      case ARDUINO_USB_STARTED_EVENT:
        usbStatus = UsbStatus::Connected;
        ESP_LOGI(TAG, "USB Connected");
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        usbStatus = UsbStatus::Waiting;
        ESP_LOGI(TAG, "USB Disconnected");
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        usbStatus = UsbStatus::Suspended;
        ESP_LOGI(TAG, "USB Suspended");
        break;
      case ARDUINO_USB_RESUME_EVENT:
        usbStatus = UsbStatus::Connected;
        ESP_LOGI(TAG, "USB Resumed");
        break;
    }
  }
}

/** Enter USB transfer; failures remain visible until the user exits and reboots. */
void USB_INIT() {
  if (initAttempted)
    return;
  // Boot selects USB before the lock prompt runs. Defer the hardware handoff
  // until the OS input dispatcher reaches this app after unlocking.
  if (deviceLocked || elfAppRunning())
    return;
  initAttempted = true;
  CurrentAppState = USB_APP;
  newState = true;
  disableTimeout = true;
  usb_is_shutting_down = false;
  OLED().setPowerSave(false);
  OLED().oledWord(TR(STR_USB_INIT));
  pocketmage::setCpuSpeed(240);

  // Both stacks use the S3 internal PHY. usb_streaming_stop() must release it
  // before TinyUSB resets the device controller and acquires that PHY.
  // https://docs.espressif.com/projects/esp-iot-solution/en/latest/usb/usb_host/usb_stream.html
  if (!P_AUDIO.stopChecked()) {
    failStartup(P_AUDIO.getLastError());
    return;
  }
  if (!pm_keyboard::stopUsb()) {
    failStartup(TR(STR_USB_DEVICE_FAIL));
    return;
  }
  pm_usb::Lock usbLock;
  if (!pm_usb::claim(pm_usb::Owner::Storage)) {
    failStartup(TR(STR_USB_DEVICE_FAIL));
    return;
  }
  if (!pm_sd_gateway::beginUsbAccess()) {
    failStartup(TR(STR_USB_CARD_BUSY));
    return;
  }
  card = getSdCard();
  if (!card || card->csd.sector_size != kSectorBytes || card->csd.capacity == 0) {
    failStartup(TR(STR_USB_CARD_FAIL));
    return;
  }
  ioMutex = xSemaphoreCreateMutex();
  if (!ioMutex) {
    failStartup(TR(STR_USB_CARD_STRUCT_FAIL));
    return;
  }

  PowerSystem.setUSBControlESP();
  ESP_LOGI(TAG, "Initializing USB MSC...");

  msc.vendorID("ESP32");
  msc.productID("PocketMage");
  msc.productRevision("1.0");
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(false);
  if (!msc.begin(card->csd.capacity, card->csd.sector_size)) {
    failStartup(TR(STR_USB_MSC_FAIL));
    return;
  }

  USB.onEvent(usbEventCallback);
  if (!USB.begin()) {
    msc.end();
    failStartup(TR(STR_USB_DEVICE_FAIL));
    return;
  }

  msc.mediaPresent(true);
  mscEnabled = true;
  keypad.flush();
  KB().setKeyboardState(NORMAL);
  KBBounceMillis = millis();
  drawUsbStatus();
}

/** Process exit input and refresh the OLED independently of USB event delivery. */
void processKB_USB() {
  if (!initAttempted) {
    USB_INIT();
    return;
  }
  uint32_t currentMillis = millis();
  char inchar = 0;

  // 1. Drain the hardware buffer continuously at loop speed
  inchar = KB().updateKeypress();

  // 2. Only process the actual input if the cooldown has expired
  if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
    if (inchar != 0) {
      KBBounceMillis = currentMillis;  // Reset the debounce timer

      // HANDLE INPUTS
      // Home / Exit / App Switcher recieved
      if (inchar == 12 || inchar == 8 || inchar == 19 || inchar == 28 || inchar == 23) {
        USB_EXIT();
        return;
      }
    }
  }

  // 3. Update OLED at 10FPS, completely independent of keyboard bounce
  currentMillis = millis();
  if (currentMillis - OLEDFPSMillis >= kOledFrameMs) {
    OLEDFPSMillis = currentMillis;
    drawUsbStatus();
  }
}

/** Render the USB transfer instructions on entry. */
void einkHandler_USB() {
  if (initAttempted && newState) {
    newState = false;
    beginEinkScreen();
    display.drawBitmap(0, 0, _usb, 320, 218, GxEPD_BLACK);
    endEinkScreen(TR(STR_USB_CONNECT));
  }
}
#endif
