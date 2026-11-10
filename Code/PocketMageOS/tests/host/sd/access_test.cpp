#include <atomic>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <mutex>
#include <string>
#include <sys/stat.h>

using TaskHandle_t = void *;
using QueueHandle_t = void *;
class String : public std::string {
public:
  using std::string::string;
};
struct PocketmageSD {
  static constexpr const char *kSdMountPoint = "/";
  static constexpr size_t kMaxPathLen = 512;
};
constexpr int pdTRUE = 1, pdPASS = 1, portMAX_DELAY = -1;
static int appTask, foreignTask, gatewayTask;
static TaskHandle_t currentTask = &appTask;
static TaskHandle_t xTaskGetCurrentTaskHandle() { return currentTask; }
static unsigned ulTaskNotifyTake(int, int) { return 1; }
static void delay(unsigned) {}
#define ESP_LOGE(tag, ...) do { (void)(tag); } while (false)
#include "gateway_types.inc"

namespace pm_sd_gateway {
static void exec(Request *request);
static Request *queuedBeforeRead = nullptr;
static int xQueueSend(QueueHandle_t, Request **request, int) {
  TaskHandle_t previous = currentTask;
  currentTask = &gatewayTask;
  if (queuedBeforeRead) {
    exec(queuedBeforeRead);
    queuedBeforeRead = nullptr;
  }
  exec(*request);
  currentTask = previous;
  return pdPASS;
}
// Only hardware/remount and unrelated filesystem operations are substituted.
// The tested stream/snapshot handlers, submission, and POSIX I/O remain real.
static bool unmountForUsb() { std::abort(); }
static void remount() { std::abort(); }
static bool removeTreeInner(const char *) { std::abort(); }
static bool copyFileInner(const char *, const char *) { std::abort(); }
static void listDirInner(Request *) { std::abort(); }
static void dirListInner(Request *) { std::abort(); }
static void extractTarInner(Request *) { std::abort(); }
static bool isMacMeta(const char *) { return false; }
#include "gateway_operations.inc"
}

using namespace pm_sd_gateway;

static Request requestFor(Op op, int slot = -1) {
  Request request{};
  request.op = op;
  request.slot = slot;
  return request;
}

static int openStream(const std::string &path, char mode) {
  Request request = requestFor(Op::StreamOpen);
  request.path = path.c_str();
  request.whence = mode;
  assert(submit(&request));
  return static_cast<int>(request.retVal);
}

static void openSnapshot(const std::string &path) {
  Request request = requestFor(Op::DirEnum);
  request.path = path.c_str();
  assert(submit(&request));
}

static void foreignStreamAccess(const std::string &operation, const std::string &path) {
  const int slot = openStream(path, operation == "write" ? 'a' : 'r');
  Request initialPosition = requestFor(Op::StreamTell, slot);
  assert(submit(&initialPosition));
  const Op ops[] = {Op::StreamRead, Op::StreamWrite, Op::StreamSeek,
                    Op::StreamTell, Op::StreamSize, Op::StreamClose};
  const char *names[] = {"read", "write", "seek", "tell", "size", "close"};
  for (size_t i = 0; i < sizeof(ops) / sizeof(ops[0]); ++i) {
    if (operation != names[i]) continue;
    const TaskHandle_t callers[] = {&foreignTask, nullptr};
    for (TaskHandle_t caller : callers) {
      currentTask = caller;
      uint8_t output = 0x5a, input = 'x';
      Request request = requestFor(ops[i], slot);
      request.rbuf = &output;
      request.wbuf = &input;
      request.len = 1;
      request.off = 2;
      request.whence = SEEK_SET;
      assert(!submit(&request));
      assert(output == 0x5a);
    }
  }
  currentTask = &appTask;
  Request size = requestFor(Op::StreamSize, slot);
  assert(submit(&size) && size.retVal == 5);
  Request position = requestFor(Op::StreamTell, slot);
  assert(submit(&position) && position.retVal == initialPosition.retVal);
  Request close = requestFor(Op::StreamClose, slot);
  assert(submit(&close));
}

static void foreignSnapshotAccess(const std::string &scenario, const std::string &path) {
  openSnapshot(path);
  const int count = snapshotCount();
  assert(count > 0);
  char original[128]{};
  bool directory = false;
  assert(snapshotEntry(0, original, sizeof(original), &directory));
  currentTask = &foreignTask;
  if (scenario == "snapshot-read") {
    char output[] = "unchanged";
    bool isDirectory = !directory;
    assert(!snapshotEntry(0, output, sizeof(output), &isDirectory));
    assert(std::strcmp(output, "unchanged") == 0 && isDirectory == !directory);
  } else if (scenario == "snapshot-count") {
    assert(snapshotCount() == 0);
  } else {
    const std::string empty = path + "/empty";
    Request request = requestFor(scenario == "snapshot-close" ? Op::DirEnumClose : Op::DirEnum);
    request.path = empty.c_str();
    assert(!submit(&request));
  }
  currentTask = &appTask;
  char remaining[128]{};
  assert(snapshotCount() == count);
  assert(snapshotEntry(0, remaining, sizeof(remaining), nullptr));
  assert(std::strcmp(original, remaining) == 0);
  assert(closeCallerResources());
}

static void snapshotReadFollowsQueuedReplacement(const std::string &path) {
  openSnapshot(path);
  const std::string empty = path + "/empty";
  Request replace = requestFor(Op::DirEnum);
  replace.path = empty.c_str();
  replace.caller = &appTask;
  queuedBeforeRead = &replace;
  char output[] = "unchanged";
  assert(!snapshotEntry(0, output, sizeof(output), nullptr));
  assert(replace.ok && snapshotCount() == 0);
  assert(std::strcmp(output, "unchanged") == 0);
  assert(closeCallerResources());
}

static void ownerOperations(const std::string &path) {
  const int slot = openStream(path, 'r');
  for (int invalid : {-1, 4}) {
    Request close = requestFor(Op::StreamClose, invalid);
    assert(!submit(&close));
  }
  Request seek = requestFor(Op::StreamSeek, slot);
  seek.off = 1;
  seek.whence = SEEK_SET;
  assert(submit(&seek));
  uint8_t output[4]{};
  Request read = requestFor(Op::StreamRead, slot);
  read.rbuf = output;
  read.len = sizeof(output);
  assert(submit(&read) && read.retVal == 4);
  assert(std::memcmp(output, "usic", 4) == 0);
  assert(closeCallerResources());
  const int writer = openStream(path, 'a');
  Request write = requestFor(Op::StreamWrite, writer);
  const uint8_t appended = '!';
  write.wbuf = &appended;
  write.len = 1;
  assert(submit(&write) && write.retVal == 1);
  assert(closeCallerResources());
  const int reader = openStream(path, 'r');
  Request size = requestFor(Op::StreamSize, reader);
  assert(submit(&size) && size.retVal == 6);
  assert(closeCallerResources());
}

int main(int argc, char **argv) {
  assert(argc == 3);
  const std::string path = argv[1], scenario = argv[2];
  s_queue = &gatewayTask;
  s_task = &gatewayTask;
  if (scenario.rfind("stream-", 0) == 0) {
    foreignStreamAccess(scenario.substr(7), path + "/track.mp3");
  } else if (scenario == "snapshot-queue") {
    snapshotReadFollowsQueuedReplacement(path);
  } else if (scenario == "owner") {
    ownerOperations(path + "/owner.mp3");
  } else if (scenario == "empty") {
    openSnapshot(path + "/empty");
    currentTask = &foreignTask;
    Request request = requestFor(Op::DirEnum);
    request.path = argv[1];
    assert(!submit(&request));
    currentTask = &appTask;
    openSnapshot(path);
    assert(snapshotCount() > 0);
    assert(closeCallerResources());
  } else {
    foreignSnapshotAccess(scenario, path);
  }
  assert(s_openStreams == 0 && s_snapshotOwner == nullptr);
}
