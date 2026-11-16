#include "crash_record.h"

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <type_traits>

namespace pm_crash {
namespace {
constexpr uint32_t kMagic = 0x504d4352;
constexpr uint32_t kVersion = 2;
constexpr uint32_t kCrcPolynomial = 0xedb88320;
constexpr size_t kChecksumOffset = offsetof(Record, version);

static_assert(std::is_trivial<Record>::value, "RTC records must not run constructors");
static_assert(sizeof(Snapshot) == 44 && sizeof(Record) == 1888,
              "The retained schema must have no padding or platform-sized fields");

uint32_t checksum(const Record& record) {
  // SAFETY: uint8_t is unsigned char on ESP32-S3; inspect only this POD's object bytes.
  const auto* bytes = reinterpret_cast<const uint8_t*>(&record);
  uint32_t crc = UINT32_MAX;
  for (size_t i = kChecksumOffset; i < sizeof(record); ++i) {
    crc ^= bytes[i];
    for (unsigned bit = 0; bit < 8; ++bit)
      crc = (crc >> 1) ^ ((crc & 1) ? kCrcPolynomial : 0);
  }
  return ~crc;
}

template <size_t N> bool terminated(const char (&text)[N]) {
  return std::memchr(text, '\0', N) != nullptr;
}

class Writer {
 public:
  Writer(char* output, size_t capacity) : output_(output), capacity_(capacity) {}

  void append(const char* pattern, ...) {
    if (failed_)
      return;
    va_list args;
    va_start(args, pattern);
    const int count = std::vsnprintf(output_ + length_, capacity_ - length_, pattern, args);
    va_end(args);
    if (count < 0 || static_cast<size_t>(count) >= capacity_ - length_) {
      failed_ = true;
      return;
    }
    length_ += static_cast<size_t>(count);
  }

  void field(const char* label, const char* value) {
    append("%s: ", label);
    for (; *value; ++value) {
      const unsigned char c = static_cast<unsigned char>(*value);
      append("%c", c >= ' ' && c <= '~' ? c : ' ');
    }
    append("\n");
  }

  /** Write text under a heading line, keeping its line breaks and blanking other control bytes. */
  void block(const char* heading, const char* text) {
    append("%s:\n", heading);
    for (; *text; ++text) {
      const unsigned char c = static_cast<unsigned char>(*text);
      append("%c", c == '\n' || (c >= ' ' && c <= '~') ? c : ' ');
    }
    if (text[-1] != '\n')
      append("\n");
  }

  size_t finish() {
    if (failed_) {
      output_[0] = '\0';
      return 0;
    }
    return length_;
  }

 private:
  char* output_;
  size_t capacity_;
  size_t length_ = 0;
  bool failed_ = false;
};
}  // namespace

void seal(Record& record) {
  record.magic = 0;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  record.version = kVersion;
  record.size = sizeof(record);
  record.checksum = checksum(record);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  record.magic = kMagic;
  std::atomic_thread_fence(std::memory_order_seq_cst);
}

bool valid(const Record& record) {
  return record.magic == kMagic && record.version == kVersion && record.size == sizeof(record) &&
         record.backtraceCount <= kBacktraceCapacity && record.core >= 0 && record.core < 2 &&
         terminated(record.task) && terminated(record.reason) && terminated(record.abortDetails) &&
         terminated(record.snapshot.usbOwner) && terminated(record.logTail) &&
         record.checksum == checksum(record);
}

size_t format(const Record& record, char* output, size_t capacity) {
  if (!output || !capacity)
    return 0;
  output[0] = '\0';
  if (!valid(record))
    return 0;
  Writer writer(output, capacity);
  writer.append("PocketMage crash report v%u\nRecord: %08x-%08x\nFirmware ELF SHA256: ",
                unsigned(record.version), unsigned(record.bootNonce), unsigned(record.checksum));
  for (uint8_t byte : record.elfSha256)
    writer.append("%02x", unsigned(byte));
  writer.append("\nUptime ms (mod 2^32): %u\nCore: %d\nPC: 0x%08x\n", unsigned(record.uptimeMs),
                int(record.core), unsigned(record.pc));
  writer.field("Task", record.task);
  writer.field("Reason", record.reason);
  writer.append("Abort: %s\n", record.flags & Aborted ? "yes" : "no");
  writer.field("Abort details", record.abortDetails);
  writer.append("Registers present: %s\n", record.flags & RegistersPresent ? "yes" : "no");
  if (record.flags & RegistersPresent) {
    writer.append("PS: 0x%08x\n", unsigned(record.ps));
    for (size_t i = 0; i < 16; ++i)
      writer.append("A%u: 0x%08x%s", unsigned(i), unsigned(record.a[i]), i % 4 == 3 ? "\n" : "  ");
    writer.append("SAR: 0x%08x\nEXCCAUSE: 0x%08x\nEXCVADDR: 0x%08x\n"
                  "LBEG: 0x%08x\nLEND: 0x%08x\nLCOUNT: 0x%08x\n",
                  unsigned(record.sar), unsigned(record.exccause), unsigned(record.excvaddr),
                  unsigned(record.lbeg), unsigned(record.lend), unsigned(record.lcount));
  }
  writer.append("Backtrace corrupt: %s\nBacktrace continues: %s\nBacktrace PCs:",
                record.flags & BacktraceCorrupt ? "yes" : "no",
                record.flags & BacktraceContinues ? "yes" : "no");
  for (size_t i = 0; i < record.backtraceCount; ++i)
    writer.append(" 0x%08x", unsigned(record.backtrace[i]));
  writer.append("\nRuntime sample present: %s\n", record.flags & SnapshotPresent ? "yes" : "no");
  if (record.flags & SnapshotPresent) {
    const Snapshot& sample = record.snapshot;
    writer.append("Sample uptime ms (mod 2^32): %u\nSample age ms (mod 2^32): %u\n"
                  "OS app state: %d\nELF app running: %s\n"
                  "Free internal heap: %u\nLargest internal block: %u\n"
                  "Lowest internal heap: %u\nTasks: %u\n",
                  unsigned(sample.uptimeMs), unsigned(record.uptimeMs - sample.uptimeMs),
                  int(sample.appState), sample.elfRunning ? "yes" : "no",
                  unsigned(sample.freeInternalHeap), unsigned(sample.largestInternalBlock),
                  unsigned(sample.minimumInternalHeap), unsigned(sample.taskCount));
    writer.field("USB owner", sample.usbOwner);
  }
  if (record.logTail[0])
    writer.block("Recent log", record.logTail);
  writer.append("End PocketMage crash report\n");
  return writer.finish();
}
}  // namespace pm_crash
