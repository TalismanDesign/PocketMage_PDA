#include <pocketmage_app/exit_callbacks.h>

#include <cassert>
#include <vector>

static ExitCallbacks callbacks;
static std::vector<int> order;
static void first() { order.push_back(1); }
static void last() {
  order.push_back(3);
  assert(callbacks.add(first) == 0);
}
static void destroy(void *value) { order.push_back(*static_cast<int *>(value)); }

int main() {
  int value = 2;
  assert(callbacks.add(first) != 0);
  assert(callbacks.begin());
  assert(!callbacks.begin());
  assert(callbacks.add(nullptr) != 0);
  assert(callbacks.add(first) == 0);
  assert(callbacks.addDestructor(destroy, &value, &value) == 0);
  callbacks.finalize(&value);
  callbacks.finalize(&value);
  assert(order == std::vector<int>{2});
  assert(callbacks.add(last) == 0);
  callbacks.finish();
  assert((order == std::vector<int>{2, 3, 1, 1}));
  callbacks.finish();
  assert(order.size() == 4);
  assert(callbacks.add(first) != 0);
  assert(callbacks.begin());
  for (size_t i = 0; i < ExitCallbacks::Capacity; ++i)
    assert(callbacks.add(first) == 0);
  assert(callbacks.add(first) != 0);
  callbacks.finish();
  assert(order.size() == 4 + ExitCallbacks::Capacity);
}
