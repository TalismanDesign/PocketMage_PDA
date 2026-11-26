#include <pocketmage_sd/sd_completion.h>

#include <cassert>
#include <cstdint>

namespace {
struct FakeNotify {
  uint32_t slots[2] = {};

  void clear(unsigned slot) { slots[slot] = 0; }
  void take(unsigned slot) { slots[slot] = 0; }
  void give(unsigned slot) { ++slots[slot]; }
};
} 

int main() {
  FakeNotify notify;
  pm_sd_gateway::CompletionWait<FakeNotify> completion(notify);

  completion.reset();
  notify.give(0);
  completion.signal();
  assert(notify.slots[0] == 1);
  assert(notify.slots[pm_sd_gateway::kCompletionSlot] == 1);

  completion.wait();
  assert(notify.slots[0] == 1);
  assert(notify.slots[pm_sd_gateway::kCompletionSlot] == 0);

  notify.give(pm_sd_gateway::kCompletionSlot);
  notify.give(pm_sd_gateway::kCompletionSlot);
  completion.reset();
  assert(notify.slots[pm_sd_gateway::kCompletionSlot] == 0);
  assert(notify.slots[0] == 1);
}
