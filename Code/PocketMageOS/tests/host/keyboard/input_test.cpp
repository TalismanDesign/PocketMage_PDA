#include <cassert>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <deque>

#include <hid.h>
#include <hid_usage_keyboard.h>
#include <hid_usage_mouse.h>
#include <pocketmage_kb/key_codes.h>

constexpr int pdTRUE = 1, pdPASS = 1;
using QueueHandle_t = std::deque<char> *;
struct hid_host_dev_params_t { int sub_class; int proto; };
static std::deque<char> input;
static bool allocationFails = false;
static QueueHandle_t xQueueCreate(size_t, size_t itemSize) {
  assert(itemSize == sizeof(char));
  return allocationFails ? nullptr : &input;
}
static int xQueueSend(QueueHandle_t queue, const char *value, int) {
  queue->push_back(*value);
  return pdTRUE;
}
static int xQueueReceive(QueueHandle_t queue, char *value, int) {
  if (queue->empty()) return 0;
  *value = queue->front();
  queue->pop_front();
  return pdTRUE;
}
static int xQueueReset(QueueHandle_t queue) { queue->clear(); return pdPASS; }
#define ESP_LOGW(tag, message) fprintf(stderr, "%s: %s\n", tag, message)
#define ESP_LOGE(tag, message) ESP_LOGW(tag, message)

namespace pm_keyboard {
bool prepareInput();
void resetInput();
void handleReport(const hid_host_dev_params_t &, const uint8_t *, size_t);
}
#include "keyboard_input.inc"

int main() {
  allocationFails = true;
  assert(!pm_keyboard::prepareInput());
  allocationFails = false;
  assert(pm_keyboard::prepareInput());
  const hid_host_dev_params_t params{HID_SUBCLASS_BOOT_INTERFACE, HID_PROTOCOL_KEYBOARD};
  const uint8_t pressed[] = {0, 0, HID_KEY_A, 0, 0, 0, 0, 0};
  const uint8_t shifted[] = {HID_LEFT_SHIFT, 0, HID_KEY_B, 0, 0, 0, 0, 0};
  const uint8_t released[8] = {};
  pm_keyboard::handleReport(params, pressed, sizeof(pressed) - 1);
  assert(pop_USB_char() == 0);
  pm_keyboard::handleReport(params, pressed, sizeof(pressed));
  assert(pop_USB_char() == 'a');
  pm_keyboard::handleReport(params, pressed, sizeof(pressed));
  assert(pop_USB_char() == 0);
  pm_keyboard::handleReport(params, released, sizeof(released));
  pm_keyboard::handleReport(params, shifted, sizeof(shifted));
  assert(pop_USB_char() == 'B');
  pm_keyboard::handleReport(params, pressed, sizeof(pressed));
  pm_keyboard::resetInput();
  assert(pop_USB_char() == 0);
  pm_keyboard::handleReport(params, pressed, sizeof(pressed));
  assert(pop_USB_char() == 'a');
}
