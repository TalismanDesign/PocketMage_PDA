#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>
#include <pocketmage_sys/usb_role.h>

using String = std::string;
using esp_err_t = int;
using TaskHandle_t = void *;
using QueueHandle_t = void *;
using SemaphoreHandle_t = std::recursive_mutex *;
using usb_stream_state_t = int;
constexpr int ESP_OK = 0, ESP_FAIL = -1;
#include "usb_role_hardware.h"
constexpr int pdTRUE = 1, pdPASS = 1, portMAX_DELAY = -1;
constexpr int STREAM_CONNECTED = 1, STREAM_UAC_SPK = 1;
constexpr int CTRL_RESUME = 0, CTRL_SUSPEND = 1;
constexpr int UAC_CH_ANY = 0, UAC_FREQUENCY_ANY = 0;
constexpr int FLAG_UAC_SPK_SUSPEND_AFTER_START = 1;
#define PM_TARGET_HOST 1
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
#define pdMS_TO_TICKS(value) (value)

enum AppState { HOME, USB_APP };
inline AppState CurrentAppState = HOME;
inline bool hostRunning = false;
inline bool startSucceeds = true, stopSucceeds = true;
inline unsigned hostStarts = 0;
inline uint32_t nowMillis = 0;
inline unsigned long millis() { return nowMillis; }
inline bool sinkEnabled = false;
inline void (*stateCallback)(usb_stream_state_t, void *) = nullptr;
inline void *stateContext = nullptr;
inline size_t speakerFrames = 0;
inline int pwm_audio_set_volume(int8_t) { return ESP_OK; }
inline std::vector<uint8_t> savedAudioSettings;
inline bool settingsWritable = true;
class Preferences {
public:
  bool begin(const char *name, bool) { return std::strcmp(name, "PMAudio") == 0; }
  bool isKey(const char *) { return !savedAudioSettings.empty(); }
  size_t getBytesLength(const char *) { return savedAudioSettings.size(); }
  size_t getBytes(const char *, void *data, size_t size) {
    if (size != savedAudioSettings.size()) return 0;
    std::memcpy(data, savedAudioSettings.data(), size);
    return size;
  }
  size_t putBytes(const char *, const void *data, size_t size) {
    if (!settingsWritable) return 0;
    const auto *bytes = static_cast<const uint8_t *>(data);
    savedAudioSettings.assign(bytes, bytes + size);
    return size;
  }
  void end() {}
};
constexpr int PWR_BTN = 0, FALLING = 0;
inline int digitalPinToInterrupt(int pin) { return pin; }
inline void detachInterrupt(int) {}
inline void attachInterrupt(int, void (*)(), int) {}
namespace pocketmage { inline void PWR_BTN_irq() {} }
struct FakePower {
  bool boost = false, needBoost = false, esp = false, works = true;
  void setUSBControlESP() { esp = true; }
  bool getBoostState(bool &value) { value = boost; return works; }
  bool getOTGNeed(bool &value) { value = needBoost; return works; }
  bool setBoost(bool value) { if (!works) return false; boost = value; return true; }
};
inline FakePower PowerSystem;

namespace pm_keyboard {
inline bool stopSucceeds = false;
inline unsigned stopCalls = 0;
inline bool stopUsb() {
  ++stopCalls;
  if (!stopSucceeds)
    return false;
  usbPhyBusy = false;
  return pm_usb::release(pm_usb::Owner::Keyboard);
}
}

inline SemaphoreHandle_t xSemaphoreCreateRecursiveMutex() { return new std::recursive_mutex; }
inline SemaphoreHandle_t xSemaphoreCreateBinary() { return new std::recursive_mutex; }
inline int xSemaphoreTakeRecursive(SemaphoreHandle_t mutex, int) { mutex->lock(); return pdTRUE; }
inline int xSemaphoreGiveRecursive(SemaphoreHandle_t mutex) { mutex->unlock(); return pdTRUE; }
inline bool shutdownAcknowledged = true;
inline int xSemaphoreTake(SemaphoreHandle_t, int) { return shutdownAcknowledged ? pdTRUE : 0; }
inline void vSemaphoreDelete(SemaphoreHandle_t mutex) { delete mutex; }
inline bool queueCreationSucceeds = true, queueSendSucceeds = true, taskCreationSucceeds = true;
inline unsigned commandsSent = 0;
inline void (*scheduledTask)(void *) = nullptr;
inline void *scheduledContext = nullptr;
inline QueueHandle_t xQueueCreate(int, size_t) { return queueCreationSucceeds ? malloc(1) : nullptr; }
inline int xQueueSend(QueueHandle_t queue, const void *, int) {
  assert(queue);
  if (!queueSendSucceeds) return 0;
  ++commandsSent;
  return pdTRUE;
}
inline int xTaskCreatePinnedToCore(void (*task)(void *), const char *, uint32_t, void *context,
                                 int, TaskHandle_t *handle, int) {
  if (!taskCreationSucceeds) return 0;
  scheduledTask = task;
  scheduledContext = context;
  *handle = context;
  return pdPASS;
}
inline void vQueueDelete(QueueHandle_t queue) { free(queue); }
inline const char *esp_err_to_name(int) { return "test-error"; }

struct uac_config_t {
  int spk_ch_num, spk_bit_resolution, spk_samples_frequence;
  size_t spk_buf_size, mic_buf_size;
  void (*mic_cb)(void *);
  void *mic_cb_arg;
  int spk_interface, spk_ep_addr, spk_ep_mps, ac_interface, spk_fu_id, flags;
};
inline int uac_streaming_config(const uac_config_t *) { return ESP_OK; }
inline int usb_streaming_state_register(void (*callback)(usb_stream_state_t, void *), void *context) {
  stateCallback = callback;
  stateContext = context;
  return ESP_OK;
}
inline int uac_frame_size_list_get(int, void *, size_t *frames, void *) {
  *frames = speakerFrames;
  return ESP_OK;
}
inline int usb_streaming_start() {
  ++hostStarts;
  if (!startSucceeds) return ESP_FAIL;
  assert(!hostRunning);
  hostRunning = true;
  usbPhyBusy = true;
  serialPhy = false;
  return ESP_OK;
}
inline int usb_streaming_stop() {
  if (!stopSucceeds) return ESP_FAIL;
  hostRunning = false;
  usbPhyBusy = false;
  // ESP-IDF usb_del_phy() disables the pads, without restoring serial routing.
  return ESP_OK;
}
inline int uac_frame_size_reset(int, int, int, int) { return ESP_OK; }
inline int usb_streaming_control(int, int, void *) { return ESP_OK; }
inline int uac_spk_streaming_write(void *, size_t, size_t) { return ESP_OK; }
