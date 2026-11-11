#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using esp_err_t = int;
using TickType_t = uint32_t;
using hid_host_device_handle_t = void *;
using hid_host_interface_event_t = int;
using hid_host_driver_event_t = int;
constexpr int ESP_OK = 0, ESP_FAIL = -1, ESP_ERR_TIMEOUT = 1;
constexpr int ESP_ERR_NOT_FINISHED = 2, ESP_ERR_NO_MEM = 3;
constexpr int pdTRUE = 1, pdPASS = 1, ESP_INTR_FLAG_LEVEL1 = 1;
constexpr int HID_HOST_INTERFACE_EVENT_DISCONNECTED = 0;
constexpr int HID_HOST_INTERFACE_EVENT_INPUT_REPORT = 1;
constexpr int HID_HOST_DRIVER_EVENT_CONNECTED = 0;
constexpr int HID_SUBCLASS_BOOT_INTERFACE = 1, HID_PROTOCOL_KEYBOARD = 1;
constexpr int HID_REPORT_PROTOCOL_BOOT = 0, USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS = 1;
#define pdMS_TO_TICKS(value) (value)

inline void testLog(const char *, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
}
#define ESP_LOGE(...) testLog(__VA_ARGS__)
inline const char *esp_err_to_name(int) { return "injected failure"; }

struct hid_host_dev_params_t { int sub_class = 1, proto = 1; };
using InterfaceCallback = void (*)(void *, int, void *);
struct hid_host_device_config_t { InterfaceCallback callback; void *callback_arg; };
struct hid_host_driver_config_t {
  bool create_background_task;
  unsigned task_priority, stack_size, core_id;
  InterfaceCallback callback;
};
struct usb_host_config_t { int intr_flags; bool root_port_unpowered; };
struct usb_host_lib_info_t { unsigned num_devices; };

namespace hardware {
inline std::string failure;
inline std::atomic<bool> host{false}, driver{false}, powered{false}, opened{false};
inline std::atomic<bool> started{false}, retainDevices{false}, retainPhy{false};
inline std::atomic<unsigned> devices{0}, reports{0}, resets{0}, ticks{0};
inline unsigned allocations = 0, failAllocation = 0, resources = 0;
inline std::recursive_mutex roleMutex;
inline std::mutex eventMutex;
inline std::condition_variable eventReady;
inline bool wakeLibrary = false;
inline std::atomic<bool> libraryJoined{true}, connectionsJoined{true};
inline thread_local std::string currentTask;
inline InterfaceCallback connection = nullptr, report = nullptr;
inline int deviceStorage;
inline void *device = &deviceStorage;
inline bool fails(const char *operation) { return failure == operation; }
inline bool allocate() { return ++allocations != failAllocation; }
inline void disconnect() {
  if (opened && report) report(device, HID_HOST_INTERFACE_EVENT_DISCONNECTED, nullptr);
  devices = 0;
}
}

struct Queue {
  std::mutex mutex;
  std::condition_variable ready;
  std::deque<void *> items;
  size_t capacity;
  bool wake = false;
};
struct Semaphore {
  std::mutex mutex;
  std::condition_variable ready;
  bool given = false;
  unsigned index;
};
struct Task { std::thread thread; };
using QueueHandle_t = Queue *;
using SemaphoreHandle_t = Semaphore *;
using TaskHandle_t = Task *;
inline Queue *eventQueue = nullptr;
inline unsigned semaphoreIndex = 0;
inline std::vector<std::unique_ptr<Task>> tasks;

inline Queue *xQueueCreate(size_t count, size_t itemSize) {
  assert(itemSize == sizeof(void *));
  if (!hardware::allocate()) return nullptr;
  ++hardware::resources;
  eventQueue = new Queue;
  eventQueue->capacity = count;
  return eventQueue;
}
inline int xQueueSend(Queue *queue, const void *item, unsigned) {
  std::lock_guard<std::mutex> lock(queue->mutex);
  if (queue->items.size() == queue->capacity) return 0;
  queue->items.push_back(*static_cast<void *const *>(item));
  queue->ready.notify_all();
  return pdTRUE;
}
inline int xQueueReceive(Queue *queue, void *item, unsigned) {
  std::unique_lock<std::mutex> lock(queue->mutex);
  queue->ready.wait(lock, [&] { return queue->wake || !queue->items.empty(); });
  if (queue->items.empty()) return 0;
  *static_cast<void **>(item) = queue->items.front();
  queue->items.pop_front();
  return pdTRUE;
}
inline void vQueueDelete(Queue *queue) {
  assert(hardware::connectionsJoined && !hardware::driver);
  delete queue;
  eventQueue = nullptr;
  --hardware::resources;
}
inline Semaphore *xSemaphoreCreateBinary() {
  if (!hardware::allocate()) return nullptr;
  ++hardware::resources;
  auto *semaphore = new Semaphore;
  semaphore->index = semaphoreIndex++;
  return semaphore;
}
inline int xSemaphoreGive(Semaphore *semaphore) {
  std::lock_guard<std::mutex> lock(semaphore->mutex);
  assert(!semaphore->given);
  semaphore->given = true;
  semaphore->ready.notify_all();
  return pdTRUE;
}
inline int xSemaphoreTake(Semaphore *semaphore, unsigned) {
  const bool library = semaphore->index % 2 == 1;
  if (hardware::fails(library ? "library-join" : "connections-join")) return 0;
  if (library) {
    std::lock_guard<std::mutex> lock(hardware::eventMutex);
    hardware::wakeLibrary = true;
    hardware::eventReady.notify_all();
  } else if (eventQueue) {
    std::lock_guard<std::mutex> lock(eventQueue->mutex);
    eventQueue->wake = true;
    eventQueue->ready.notify_all();
  }
  std::unique_lock<std::mutex> lock(semaphore->mutex);
  semaphore->ready.wait(lock, [&] { return semaphore->given; });
  semaphore->given = false;
  (library ? hardware::libraryJoined : hardware::connectionsJoined) = true;
  return pdTRUE;
}
inline void vSemaphoreDelete(Semaphore *semaphore) {
  delete semaphore;
  --hardware::resources;
}
inline int xTaskCreate(void (*function)(void *), const char *name, size_t, void *,
                       unsigned, Task **out) {
  const bool library = std::strcmp(name, "usb_events") == 0;
  if (hardware::fails(library ? "library-task" : "connections-task")) return 0;
  (library ? hardware::libraryJoined : hardware::connectionsJoined) = false;
  auto task = std::make_unique<Task>();
  task->thread = std::thread([=] {
    hardware::currentTask = name;
    function(nullptr);
  });
  *out = task.get();
  tasks.push_back(std::move(task));
  return pdPASS;
}
inline int xTaskCreatePinnedToCore(void (*function)(void *), const char *name,
                                   size_t stack, void *arg, unsigned priority, Task **out, int) {
  return xTaskCreate(function, name, stack, arg, priority, out);
}
inline void vTaskDelete(Task *task) { assert(!task); }
inline TickType_t xTaskGetTickCount() { return hardware::ticks; }
inline void vTaskDelay(unsigned ticks) { hardware::ticks += ticks; }

inline esp_err_t usb_host_install(const usb_host_config_t *config) {
  assert(config->root_port_unpowered);
  if (hardware::fails("host-install")) return ESP_FAIL;
  hardware::host = true;
  return ESP_OK;
}
inline esp_err_t usb_host_lib_handle_events(unsigned, uint32_t *flags) {
  assert(hardware::host && flags);
  if (hardware::currentTask == "usb_events") {
    std::unique_lock<std::mutex> lock(hardware::eventMutex);
    hardware::eventReady.wait(lock, [] { return hardware::wakeLibrary; });
  } else {
    assert(hardware::libraryJoined);
    if (!hardware::fails("free-devices")) hardware::devices = 0;
  }
  *flags = 0;
  return hardware::fails("events") ? ESP_FAIL : ESP_OK;
}
inline esp_err_t usb_host_device_free_all() {
  assert(!hardware::driver);
  if (hardware::fails("free-devices")) hardware::devices = 1;
  return hardware::fails("free-devices") ? ESP_ERR_NOT_FINISHED : ESP_OK;
}
inline esp_err_t usb_host_lib_info(usb_host_lib_info_t *info) {
  info->num_devices = hardware::devices;
  return hardware::fails("info") ? ESP_FAIL : ESP_OK;
}
inline esp_err_t usb_host_lib_set_root_port_power(bool enabled) {
  if (hardware::fails(enabled ? "power-on" : "power-off")) return ESP_FAIL;
  if (!enabled) {
    assert(hardware::connectionsJoined);
    if (!hardware::retainDevices) hardware::disconnect();
  }
  hardware::powered = enabled;
  return ESP_OK;
}
inline esp_err_t usb_host_uninstall() {
  assert(!hardware::driver && hardware::libraryJoined && !hardware::devices);
  if (hardware::fails("host-uninstall")) return ESP_FAIL;
  hardware::host = false;
  return ESP_OK;
}
inline esp_err_t hid_host_install(const hid_host_driver_config_t *config) {
  assert(config->create_background_task && !hardware::powered && eventQueue);
  if (hardware::fails("driver-install")) return ESP_FAIL;
  hardware::connection = config->callback;
  hardware::driver = true;
  return ESP_OK;
}
inline esp_err_t hid_host_uninstall() {
  assert(!hardware::devices && !hardware::opened && hardware::connectionsJoined);
  if (hardware::fails("driver-uninstall")) return ESP_FAIL;
  hardware::driver = false;
  return ESP_OK;
}
inline esp_err_t hid_host_device_get_params(void *, hid_host_dev_params_t *params) {
  *params = {};
  return ESP_OK;
}
inline esp_err_t hid_host_device_open(void *, const hid_host_device_config_t *config) {
  hardware::report = config->callback;
  hardware::opened = true;
  return ESP_OK;
}
inline esp_err_t hid_class_request_set_protocol(void *, int) { return ESP_OK; }
inline esp_err_t hid_class_request_set_idle(void *, int, int) { return ESP_OK; }
inline esp_err_t hid_host_device_start(void *) {
  std::lock_guard<std::mutex> lock(hardware::eventMutex);
  hardware::started = true;
  hardware::eventReady.notify_all();
  return ESP_OK;
}
inline esp_err_t hid_host_device_close(void *) {
  hardware::opened = false;
  return ESP_OK;
}
inline esp_err_t hid_host_device_get_raw_input_report_data(void *, uint8_t *data,
                                                          size_t, size_t *length) {
  data[0] = 4;
  *length = 1;
  return ESP_OK;
}
