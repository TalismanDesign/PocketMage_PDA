#pragma once

#include <cstddef>
#include <cstdint>

/** Retained panic record and its text report, independent of hardware. */
namespace pm_crash {

/** Maximum number of PCs provided by Arduino's panic callback. */
constexpr size_t kBacktraceCapacity = 60;
/** Buffer size, including NUL, sufficient for a fully populated text report. */
constexpr size_t kReportCapacity = 4096;

/** Availability and backtrace quality flags stored in Record::flags. */
enum RecordFlag : uint32_t {
  RegistersPresent = 1U << 0,
  BacktraceCorrupt = 1U << 1,
  BacktraceContinues = 1U << 2,
  SnapshotPresent = 1U << 3,
  Aborted = 1U << 4,
};

/** Last completed normal-task sample; uptime is milliseconds since boot, modulo 2^32. */
struct Snapshot {
  uint32_t uptimeMs;
  int32_t appState;
  uint32_t elfRunning;
  uint32_t freeInternalHeap;
  uint32_t largestInternalBlock;
  uint32_t usbConnected;
  char audioState[16];
  char audioOutput[8];
  char usbOwner[16];
};

/** Plain data kept in RTC RAM across reboots. magic is written last, and checksum
 * covers version through the end.
 * See https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/memory-types.html
 */
struct Record {
  uint32_t magic;
  uint32_t checksum;
  uint32_t version;
  uint32_t size;
  uint32_t bootNonce;
  uint32_t uptimeMs;
  int32_t core;
  uint32_t pc;
  uint32_t flags;
  uint32_t backtraceCount;
  uint32_t backtrace[kBacktraceCapacity];
  uint32_t ps;
  uint32_t a[16];
  uint32_t sar;
  uint32_t exccause;
  uint32_t excvaddr;
  uint32_t lbeg;
  uint32_t lend;
  uint32_t lcount;
  Snapshot snapshot;
  uint8_t elfSha256[32];
  char task[32];
  char reason[192];
  char abortDetails[192];
};

/** Commit record's schema and CRC, writing magic last. Does not validate payload. */
void seal(Record& record);
/** Return false for incomplete, incompatible, corrupt, or unbounded records. */
bool valid(const Record& record);
/** Format a valid record into output, whose capacity includes NUL. Return the
 * length excluding NUL, or zero if the record is invalid or the text would not fit.
 */
size_t format(const Record& record, char* output, size_t capacity);

}  // namespace pm_crash
