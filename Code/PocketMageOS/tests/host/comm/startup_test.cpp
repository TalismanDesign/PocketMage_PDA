#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using String = std::string;
using ulong = unsigned long;
using esp_err_t = int;
constexpr int ESP_OK = 0, ESP_FAIL = -1, ESP_ERR_NO_MEM = 0x101;
constexpr int WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP_STA = 3;
constexpr int WIFI_PS_NONE = 0, WIFI_SECOND_CHAN_NONE = 0, ESP_MAC_WIFI_STA = 0;
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define pdMS_TO_TICKS(value) (value)
#define TR(value) #value

static std::string failure;
static std::vector<std::string> calls;
static unsigned apStarts = 0, meshStarts = 0, notifications = 0, queued = 0;
static bool callbackInstalled = false;
enum AppState { HOME, COMM };
static AppState CurrentAppState = HOME;
static bool newState = false;

static void require(bool value, const char *message) {
  if (!value) { std::cerr << message << '\n'; std::exit(1); }
}

static int operation(const char *name) {
  calls.emplace_back(name);
  return failure == name ? ESP_FAIL : ESP_OK;
}

static bool setCpuFrequencyMhz(int mhz) { assert(mhz == 240); return true; }
static void vTaskDelay(unsigned) {}
static const char *esp_err_to_name(int) { return "test-error"; }
static void HOME_INIT() { CurrentAppState = HOME; }
static struct { void println(const char *) {} } Serial;
static struct {
  bool mode(int mode) {
    if (mode == WIFI_AP_STA) ++apStarts;
    return operation(mode == WIFI_OFF ? "wifi-off" : "wifi-start") == ESP_OK;
  }
} WiFi;
static struct {
  void sysMessage(const char *, unsigned) { ++notifications; }
} oled;
static auto &OLED() { return oled; }

static int esp_wifi_set_ps(int) { return operation("power-save"); }
static int esp_wifi_set_channel(int channel, int secondary) {
  assert(channel == 1 && secondary == WIFI_SECOND_CHAN_NONE);
  return operation("channel");
}
static int esp_read_mac(uint8_t *mac, int) {
  const uint8_t station[] = {0x02, 0x16, 0x28, 0x3a, 0x4c, 0x5e};
  std::memcpy(mac, station, sizeof(station));
  return operation("mac");
}

struct message_t {};
struct mesh_message_t {};
static int message_queue_init() { return operation("queue"); }
static int message_queue_receive(message_t *, int) {
  if (!queued) return ESP_FAIL;
  --queued;
  return ESP_OK;
}
static int mesh_now_deinit() { return operation("mesh-stop"); }
static int mesh_now_init() { ++meshStarts; return operation("mesh-start"); }
static void meshRecvCb(const mesh_message_t *) {}
static void mesh_now_set_receive_callback(void (*callback)(const mesh_message_t *)) {
  callbackInstalled = callback == meshRecvCb;
}
static void *allocateMessages(size_t count, size_t size) {
  return failure == "allocation" ? nullptr : std::calloc(count, size);
}

#define calloc allocateMessages
#include "comm_startup.inc"
#undef calloc

int main(int argc, char **argv) {
  require(argc == 2, "scenario required");
  const std::string scenario = argv[1];
  if (scenario != "station" && scenario != "reopen") failure = scenario;
  COMM_INIT();
  if (failure.empty()) {
    require(apStarts == 0, "ESP-NOW chat must not start the crashing SoftAP path");
    require(meshReady && callbackInstalled && CurrentAppState == COMM && msgs,
            "station startup must enable chat and its receive callback");
    require(std::strcmp(myMacStr, "02:16:28:3A:4C:5E") == 0,
            "chat identity must use the station MAC");
    if (scenario == "reopen") {
      calls.clear();
      queued = 3;
      COMM_INIT();
      require(calls.empty() && meshStarts == 1 && queued == 0,
              "reopening chat must drain old messages without restarting its radio");
    }
  } else {
    require(!meshReady && !callbackInstalled && CurrentAppState == HOME,
            "startup failure must return home without exposing uninitialized mesh state");
    require(notifications == 1, "startup failure must be visible to the user");
    require(msgs == nullptr, "failed chat startup must release its message buffer");
    if (failure != "mesh-start")
      require(meshStarts == 0, "failed prerequisites must prevent mesh startup");
  }
  std::free(msgs);
}
