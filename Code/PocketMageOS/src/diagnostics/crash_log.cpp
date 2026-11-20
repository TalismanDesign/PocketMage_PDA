#include "crash_log.h"

#include <Arduino.h>
#include <elf_runner.h>
#include <esp_app_desc.h>
#include <esp_attr.h>
#include <esp_heap_caps.h>
#include <esp_memory_utils.h>
#include <esp_private/panic_internal.h>
#include <esp_random.h>
#include <pocketmage.h>
#include <pocketmage_globals.h>
#include <pocketmage_log/pocketmage_log.h>
#include <pocketmage_sys/usb_role.h>
#include <xtensa_context.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <vector>

#include "crash_record.h"

#if !CONFIG_IDF_TARGET_ESP32S3
#error "Panic register capture requires the ESP32-S3 Xtensa exception frame"
#endif

namespace pm_crash {
namespace {
constexpr const char* kTag = "CRASH";
constexpr const char* kLogDirectory = "/logs";
constexpr const char* kDirectory = "/logs/crash";
constexpr uint32_t kRetryMs = 5000;
constexpr size_t kPathCapacity = 64;
constexpr size_t kIoChunk = 256;
constexpr uint32_t kAckMagic = 0x504d4143;

struct Acknowledgment {
  uint32_t magic;
  uint32_t checksum;
  uint32_t complement;
};

RTC_NOINIT_ATTR Record retained;
RTC_NOINIT_ATTR Acknowledgment acknowledgment;
Snapshot samples[2]{};
std::atomic<unsigned> sampleIndex{0};
std::atomic<bool> samplePresent{false};
// Xtensa GCC reports RMW atomics as not always lock-free. These load/store-only
// operations emit l32i/s32i/l8ui/s8i with memw, without libatomic or scheduler calls.
static_assert(sizeof(sampleIndex) == sizeof(uint32_t) && alignof(decltype(sampleIndex)) >= 4,
              "The published sample index must use an aligned 32-bit atomic");
static_assert(std::extent<decltype(arduino_panic_info_t::backtrace)>::value == kBacktraceCapacity,
              "Retained backtrace capacity must match the pinned Arduino callback ABI");
#if defined(__XTENSA__)
static_assert(sizeof(uintptr_t) == sizeof(uint32_t),
              "Retained PCs must fit the target address space");
#endif
uint32_t bootNonce;
uint8_t firmwareSha[32];
char report[kReportCapacity];
size_t reportLength;
char reportPath[kPathCapacity];
char temporaryPath[kPathCapacity];
bool exportPending;
bool attemptedExport;
uint32_t lastAttemptMs;
bool serialWasConnected;
size_t serialOffset;

template <size_t N> void copyText(char (&destination)[N], const char* source) {
  size_t i = 0;
  // Panic strings may be null or damaged. Check each address before dereferencing.
  while (source && i + 1 < N && esp_ptr_byte_accessible(source + i) && source[i]) {
    destination[i] = source[i];
    ++i;
  }
  destination[i] = '\0';
}

const char* usbOwnerName(pm_usb::Owner owner) {
  switch (owner) {
    case pm_usb::Owner::DebugSerial:
      return "DebugSerial";
    case pm_usb::Owner::Audio:
      return "Audio";
    case pm_usb::Owner::Keyboard:
      return "Keyboard";
    case pm_usb::Owner::Storage:
      return "Storage";
  }
  return "Unknown";
}

void sampleRuntime(pm_usb::Owner owner) {
  const unsigned next = sampleIndex.load(std::memory_order_relaxed) ^ 1U;
  Snapshot& sample = samples[next];
  sample.uptimeMs = millis();
  sample.appState = static_cast<int32_t>(CurrentAppState);
  sample.elfRunning = elfAppRunning();
  sample.freeInternalHeap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  sample.largestInternalBlock =
      heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  sample.minimumInternalHeap =
      heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  sample.taskCount = uxTaskGetNumberOfTasks();
  copyText(sample.usbOwner, usbOwnerName(owner));
  sampleIndex.store(next, std::memory_order_release);
  samplePresent.store(true, std::memory_order_release);
}

void captureRegisters(int core) {
  const auto* frame = static_cast<const XtExcFrame*>(g_exc_frames[core]);
  // SAFETY: convert without dereferencing; validate alignment and the full frame range below.
  const uintptr_t address = reinterpret_cast<uintptr_t>(frame);
  if (!frame || address % alignof(XtExcFrame) != 0 || address > UINTPTR_MAX - sizeof(*frame) + 1 ||
      !esp_ptr_byte_accessible(frame) ||
      !esp_ptr_byte_accessible(reinterpret_cast<const void*>(address + sizeof(*frame) - 1)))
    return;
  retained.ps = frame->ps;
  // Xtensa exposes signed long fields; preserve their representation rather than narrowing.
  const decltype(frame->a0) registers[] = {
      frame->a0, frame->a1, frame->a2,  frame->a3,  frame->a4,  frame->a5,  frame->a6,  frame->a7,
      frame->a8, frame->a9, frame->a10, frame->a11, frame->a12, frame->a13, frame->a14, frame->a15};
  static_assert(sizeof(registers) == sizeof(retained.a), "Xtensa A registers must each be 32 bits");
  std::memcpy(retained.a, registers, sizeof(registers));
  retained.sar = frame->sar;
  retained.exccause = frame->exccause;
  retained.excvaddr = frame->excvaddr;
  retained.lbeg = frame->lbeg;
  retained.lend = frame->lend;
  retained.lcount = frame->lcount;
  retained.flags |= RegistersPresent;
}

void capture(arduino_panic_info_t* info, void*) {
  if (!info || info->core < 0 || info->core >= SOC_CPU_CORES_NUM)
    return;
  // The pinned IDF Xtensa panic entry restores cache and stalls other CPUs before
  // Arduino invokes this hook. No locks, allocation, SDK services or I/O here.
  // https://github.com/espressif/esp-idf/blob/v5.5/components/esp_system/port/panic_handler.c
  retained.magic = 0;
  acknowledgment.magic = 0;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  std::memset(&retained, 0, sizeof(retained));
  retained.bootNonce = bootNonce;
  retained.uptimeMs = millis();
  retained.core = info->core;
  // SAFETY: ESP32-S3 PCs are 32-bit code addresses.
  retained.pc = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(info->pc));
  copyText(retained.reason, info->reason);
  // pcTaskGetName(NULL) reads the current TCB without taking a scheduler lock.
  copyText(retained.task, pcTaskGetName(nullptr));
  if (g_panic_abort) {
    retained.flags |= Aborted;
    copyText(retained.abortDetails, g_panic_abort_details);
  }
  retained.backtraceCount = std::min<size_t>(info->backtrace_len, kBacktraceCapacity);
  for (size_t i = 0; i < retained.backtraceCount; ++i)
    retained.backtrace[i] = info->backtrace[i];
  if (info->backtrace_corrupt)
    retained.flags |= BacktraceCorrupt;
  if (info->backtrace_continues || info->backtrace_len > kBacktraceCapacity)
    retained.flags |= BacktraceContinues;
  captureRegisters(info->core);
  std::memcpy(retained.elfSha256, firmwareSha, sizeof(firmwareSha));
  if (samplePresent.load(std::memory_order_acquire)) {
    // Only poll() writes samples. The stalled writer cannot reuse this buffer.
    retained.snapshot = samples[sampleIndex.load(std::memory_order_acquire)];
    retained.flags |= SnapshotPresent;
  }
  // The log tail is lock-free and bounds every index, so a half-written line costs at most a line.
  pm_log::copyTail(retained.logTail, sizeof(retained.logTail));
  seal(retained);
}

bool acknowledged() {
  return acknowledgment.magic == kAckMagic && acknowledgment.checksum == retained.checksum &&
         acknowledgment.complement == ~retained.checksum;
}

void acknowledge() {
  acknowledgment.magic = 0;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  acknowledgment.checksum = retained.checksum;
  acknowledgment.complement = ~retained.checksum;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  acknowledgment.magic = kAckMagic;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  exportPending = false;
}

bool matchesReport(const char* path) {
  const int slot = PM_SD().streamOpen(path, 'r');
  if (slot < 0)
    return false;
  bool matches = PM_SD().streamSize(slot) == reportLength;
  char chunk[kIoChunk];
  size_t offset = 0;
  while (matches && offset < reportLength) {
    const size_t count =
        PM_SD().streamRead(slot, chunk, std::min(sizeof(chunk), reportLength - offset));
    if (!count || count > reportLength - offset || std::memcmp(chunk, report + offset, count) != 0)
      matches = false;
    else
      offset += count;
  }
  const bool closed = PM_SD().streamClose(slot);
  return matches && closed;
}

bool exportReport() {
  if (PM_SD().getNoSD() || !PM_SD().mkdirPath(kLogDirectory) || !PM_SD().mkdirPath(kDirectory))
    return false;
  std::vector<PocketmageSD::DirItem> entries;
  // A failed stat cannot distinguish absence from an I/O error. Require a full
  // successful listing before creating the private temporary path.
  if (!PM_SD().listDirItems(kDirectory, entries))
    return false;
  const char* separator = std::strrchr(reportPath, '/');
  if (!separator)
    return false;
  const char* basename = separator + 1;
  for (const auto& entry : entries) {
    if (entry.name == basename)
      return !entry.isDir && matchesReport(reportPath);
  }
  // SAFETY: reportLength excludes NUL and is within report.
  if (!PM_SD().writeBinaryFile(temporaryPath, reinterpret_cast<const uint8_t*>(report),
                               reportLength) ||
      !matchesReport(temporaryPath))
    return false;
  // FatFs f_rename refuses an existing destination. The gateway serializes card
  // operations, so even a competing creation cannot replace a completed report.
  // https://elm-chan.org/fsw/ff/doc/rename.html
  return PM_SD().renamePath(temporaryPath, reportPath) && matchesReport(reportPath);
}

void replaySerial() {
  if (!reportLength)
    return;
  // Hold the USB role through the write so an app cannot take the PHY mid-replay.
  pm_usb::Lock lock;
  const bool connected = pm_usb::owner() == pm_usb::Owner::DebugSerial && bool(Serial);
  if (!connected) {
    serialWasConnected = false;
    serialOffset = 0;
    return;
  }
  if (!serialWasConnected) {
    serialWasConnected = true;
    serialOffset = 0;
  }
  if (serialOffset < reportLength) {
    const size_t length = std::min(kIoChunk, reportLength - serialOffset);
    // SAFETY: offset and length are bounded by reportLength.
    const size_t written =
        Serial.write(reinterpret_cast<const uint8_t*>(report + serialOffset), length);
    if (written <= length)
      serialOffset += written;
  }
}
}  // namespace

void begin() {
  reportLength = 0;
  exportPending = false;
  attemptedExport = false;
  serialWasConnected = false;
  serialOffset = 0;
  samplePresent.store(false, std::memory_order_relaxed);
  sampleIndex.store(0, std::memory_order_relaxed);
  bootNonce = esp_random();
  const esp_app_desc_t* description = esp_app_get_description();
  std::memcpy(firmwareSha, description->app_elf_sha256, sizeof(firmwareSha));
  set_arduino_panic_handler(capture, nullptr);
  if (!valid(retained))
    return;
  reportLength = format(retained, report, sizeof(report));
  if (!reportLength)
    return;
  const int finalLength =
      std::snprintf(reportPath, sizeof(reportPath), "%s/crash-%08x-%08x.txt", kDirectory,
                    unsigned(retained.bootNonce), unsigned(retained.checksum));
  const int temporaryLength =
      std::snprintf(temporaryPath, sizeof(temporaryPath), "%s.tmp", reportPath);
  if (finalLength < 0 || static_cast<size_t>(finalLength) >= sizeof(reportPath) ||
      temporaryLength < 0 || static_cast<size_t>(temporaryLength) >= sizeof(temporaryPath))
    return;
  exportPending = !acknowledged();
}

void poll() {
  const pm_usb::Owner owner = pm_usb::owner();
  sampleRuntime(owner);
  const uint32_t now = millis();
  if (exportPending && owner != pm_usb::Owner::Storage &&
      (!attemptedExport || now - lastAttemptMs >= kRetryMs)) {
    attemptedExport = true;
    lastAttemptMs = now;
    if (exportReport()) {
      acknowledge();
      PM_LOGI(kTag, "Saved %s", reportPath);
    } else {
      PM_LOGE(kTag, "Report pending: %s; retry in %u ms", reportPath, unsigned(kRetryMs));
    }
  }
  replaySerial();
}
}  // namespace pm_crash
