#include "hardware.h"
#include <pocketmage_sys/usb_role.h>

namespace pm_usb {
static Owner current = Owner::DebugSerial;
Lock::Lock() { hardware::roleMutex.lock(); }
Lock::~Lock() { hardware::roleMutex.unlock(); }
Owner owner() { return current; }
bool claim(Owner requested) {
  if (current != Owner::DebugSerial) return false;
  current = requested;
  return true;
}
bool release(Owner requested) {
  assert(current == requested && !hardware::host && !hardware::driver);
  if (hardware::retainPhy) return false;
  current = Owner::DebugSerial;
  return true;
}
}

namespace pm_keyboard {
bool beginUsb();
bool stopUsb();
bool stopping();
bool prepareInput() { return !hardware::fails("input"); }
void resetInput() {
  assert(!hardware::driver && hardware::connectionsJoined);
  ++hardware::resets;
}
void handleReport(const hid_host_dev_params_t &, const uint8_t *data, size_t length) {
  assert(length == 1 && data[0] == 4);
  ++hardware::reports;
}
}
#include "usb_keyboard.inc"

static void joinThreads() {
  for (auto &task : tasks) task->thread.join();
  tasks.clear();
}

static void connectKeyboard() {
  hardware::devices = 1;
  hardware::connection(hardware::device, HID_HOST_DRIVER_EVENT_CONNECTED, nullptr);
  std::unique_lock<std::mutex> lock(hardware::eventMutex);
  hardware::eventReady.wait(lock, [] { return hardware::started.load(); });
}

int main(int argc, char **argv) {
  assert(argc == 2);
  const std::string scenario = argv[1];
  if (scenario.rfind("start:", 0) == 0) {
    hardware::failure = scenario.substr(6);
    if (hardware::failure.rfind("allocation", 0) == 0)
      hardware::failAllocation = static_cast<unsigned>(hardware::failure.back() - '0');
    assert(!pm_keyboard::beginUsb());
    assert(pm_usb::owner() == pm_usb::Owner::DebugSerial && !pm_keyboard::stopping());
  } else {
    assert(pm_keyboard::beginUsb());
    assert(pm_keyboard::beginUsb());
    connectKeyboard();
    hardware::report(hardware::device, HID_HOST_INTERFACE_EVENT_INPUT_REPORT, nullptr);
    assert(hardware::reports == 1);
    const unsigned resets = hardware::resets;
    if (scenario != "normal") {
      if (scenario == "devices") hardware::retainDevices = true;
      else if (scenario == "phy") hardware::retainPhy = true;
      else hardware::failure = scenario;
      assert(!pm_keyboard::stopUsb());
      assert(pm_usb::owner() == pm_usb::Owner::Keyboard && pm_keyboard::stopping());
      assert(!pm_keyboard::beginUsb());
      if (scenario != "phy") {
        assert(hardware::resources == 3 && hardware::resets == resets);
      }
      hardware::failure.clear();
      hardware::retainPhy = false;
      if (hardware::retainDevices) hardware::disconnect();
    }
    assert(pm_keyboard::stopUsb());
    assert(pm_keyboard::stopUsb());
  }
  joinThreads();
  assert(!hardware::host && !hardware::driver && hardware::resources == 0);
  assert(pm_usb::owner() == pm_usb::Owner::DebugSerial);
  // A new session must not consume stale worker acknowledgements or held input.
  hardware::failure.clear();
  hardware::failAllocation = 0;
  hardware::retainDevices = false;
  hardware::wakeLibrary = false;
  semaphoreIndex = 0;
  assert(pm_keyboard::beginUsb());
  assert(pm_keyboard::stopUsb());
  joinThreads();
  assert(hardware::resources == 0);
}
