#include <pocketmage_app/pocketmage_app.h>

#include <cassert>
#include <cstdint>
#include <vector>

static int appTask, otherTask;
TaskHandle_t task = &appTask;
static std::vector<int> order;
static bool resident = true;
static int stops = 0;
struct esp_elf_t {
  bool init_done = false;
  void (**fini_array)() = nullptr;
  uint32_t fini_count = 0;
  bool fini_reverse = false;
};
static void cleanup() { assert(resident); order.push_back(4); }
static void finiFirst() { assert(resident); order.push_back(2); }
static void finiLast() {
  assert(resident);
  order.push_back(3);
  assert(P_APP.atExit(cleanup) == 0);
}
static int esp_elf_request(esp_elf_t *elf, int, int, char **) {
  elf->init_done = true;
  assert(P_APP.atExit(cleanup) == 0);
  task = &otherTask;
  assert(P_APP.atExit(cleanup) == -1);
  assert(!P_APP.beginSession());
  P_APP.finishSession();
  task = &appTask;
  order.push_back(1);
  return 0;
}
struct Audio {
  bool stopChecked() { assert(resident); return ++stops != 1; }
} P_AUDIO;
struct SerialLog { void println(const char *) {} } Serial;
static void delay(int) { assert(resident); }
namespace pm_sd_gateway {
bool closeCallerResources() { assert(resident); order.push_back(5); return true; }
}
#include "lifecycle.inc"

int main() {
  void (*finalizers[])() = {finiFirst, nullptr, finiLast};
  esp_elf_t elf{false, finalizers, 3, false};
  assert(executeLoadedElf(&elf, 0, nullptr));
  assert((order == std::vector<int>{1, 2, 3, 4, 4, 5}));
  assert(stops == 3 && !elf.init_done);
  resident = false;
  P_APP.finishSession();
  assert(P_APP.atExit(cleanup) == -1);
  resident = true;
  order.clear();
  elf.fini_reverse = true;
  assert(executeLoadedElf(&elf, 0, nullptr));
  assert((order == std::vector<int>{1, 3, 2, 4, 4, 5}));
}
