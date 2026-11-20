#pragma once

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#define RTC_NOINIT_ATTR
#define IRAM_ATTR
#define CONFIG_IDF_TARGET_ESP32S3 1
#define PM_TARGET_HOST 1
#define SOC_CPU_CORES_NUM 2
#define MALLOC_CAP_INTERNAL 1
#define MALLOC_CAP_8BIT 2

inline void testLog(const char*, const char*, ...) {}
#define ESP_LOGE(...) testLog(__VA_ARGS__)
#define ESP_LOGI(...) testLog(__VA_ARGS__)
#define PM_LOGE(...) testLog(__VA_ARGS__)
#define PM_LOGW(...) testLog(__VA_ARGS__)
#define PM_LOGI(...) testLog(__VA_ARGS__)
#define PM_LOGD(...) testLog(__VA_ARGS__)
#define PM_LOGV(...) testLog(__VA_ARGS__)

inline unsigned runtimeQueries = 0;
inline void (*onTaskCount)() = nullptr;

struct arduino_panic_info_t {
  int core;
  const char* reason;
  const void* pc;
  bool backtrace_corrupt;
  bool backtrace_continues;
  unsigned int backtrace_len;
  unsigned int backtrace[60];
};
using arduino_panic_handler_t = void (*)(arduino_panic_info_t*, void*);
inline arduino_panic_handler_t panicHandler = nullptr;
inline void* panicArgument = nullptr;
inline void set_arduino_panic_handler(arduino_panic_handler_t handler, void* argument) {
  panicHandler = handler;
  panicArgument = argument;
}

struct XtExcFrame {
  int32_t pc, ps, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
  int32_t sar, exccause, excvaddr, lbeg, lend, lcount;
};
inline void* g_exc_frames[SOC_CPU_CORES_NUM]{};
inline bool g_panic_abort = false;
inline char* g_panic_abort_details = nullptr;
inline bool esp_ptr_byte_accessible(const void* pointer) {
  return pointer != nullptr;
}
inline bool esp_ptr_in_dram(const void* pointer) {
  return pointer != nullptr;
}
inline const char* pcTaskGetName(void*) {
  return "usb_stream_proc";
}
inline uint32_t clockMs = 100;
inline uint32_t millis() {
  return clockMs;
}
inline int64_t esp_timer_get_time() {
  return static_cast<int64_t>(clockMs) * 1000;
}
inline uint32_t esp_random() {
  return 0x12345678;
}
inline size_t heap_caps_get_free_size(unsigned) {
  ++runtimeQueries;
  return 65536;
}
inline size_t heap_caps_get_largest_free_block(unsigned) {
  ++runtimeQueries;
  return 32768;
}
inline size_t heap_caps_get_minimum_free_size(unsigned) {
  ++runtimeQueries;
  return 16384;
}
inline unsigned uxTaskGetNumberOfTasks() {
  ++runtimeQueries;
  if (onTaskCount)
    onTaskCount();
  return 17;
}

namespace pm_log {
inline std::string tail = "120 I [SYSTEM] setup sd\n130 E [USB] endpoint stall\n";
inline size_t copyTail(char* out, size_t capacity) {
  const size_t length = std::min(tail.size(), capacity - 1);
  std::memcpy(out, tail.data(), length);
  out[length] = '\0';
  return length;
}
}  // namespace pm_log
struct esp_app_desc_t {
  uint8_t app_elf_sha256[32];
};
inline const esp_app_desc_t* esp_app_get_description() {
  static const esp_app_desc_t description{{0xab, 0xcd}};
  return &description;
}

inline int CurrentAppState = 42;
inline bool elfAppRunning() {
  return true;
}
namespace pm_usb {
enum class Owner { DebugSerial, Audio, Keyboard, Storage };
inline Owner currentOwner = Owner::Audio;
inline unsigned lockDepth = 0;
struct Lock {
  Lock() {
    ++lockDepth;
  }
  ~Lock() {
    --lockDepth;
  }
};
inline Owner owner() {
  ++runtimeQueries;
  return currentOwner;
}
}

struct TestSerial {
  bool connected = false;
  size_t maxWrite = 256;
  unsigned unlockedWrites = 0;
  std::string output;
  explicit operator bool() const {
    return connected;
  }
  size_t write(const uint8_t* data, size_t length) {
    if (!pm_usb::lockDepth)
      ++unlockedWrites;
    length = std::min(length, maxWrite);
    // SAFETY: character storage aliases the caller's length-bounded byte buffer.
    output.append(reinterpret_cast<const char*>(data), length);
    return length;
  }
};
inline TestSerial Serial;

// Only SD transport and scheduling are simulated. Capture, checksums, report
// formatting, write verification, rename, retry and replay use production code.
struct TestSD {
  struct DirItem {
    std::string name;
    bool isDir;
  };
  bool noCard = false, failMkdir = false, failList = false, failWrite = false;
  bool corruptRead = false, failClose = false, failRename = false;
  bool failOpen = false, corruptFinalRead = false;
  unsigned ioCalls = 0, writes = 0, renames = 0;
  std::map<std::string, std::string> files;
  std::string opened;
  size_t position = 0;
  bool getNoSD() const {
    return noCard;
  }
  bool mkdirPath(const char*) {
    ++ioCalls;
    return !failMkdir;
  }
  bool listDirItems(const char* directory, std::vector<DirItem>& items) {
    ++ioCalls;
    if (failList)
      return false;
    const std::string prefix = std::string(directory) + "/";
    for (const auto& file : files) {
      if (file.first.compare(0, prefix.size(), prefix) == 0)
        items.push_back({file.first.substr(prefix.size()), false});
    }
    return true;
  }
  bool writeBinaryFile(const char* path, const uint8_t* data, size_t length) {
    ++ioCalls;
    ++writes;
    // SAFETY: character storage aliases the caller's length-bounded byte buffer.
    files[path].assign(reinterpret_cast<const char*>(data), failWrite ? length / 2 : length);
    return !failWrite;
  }
  bool appendBinaryFile(const char* path, const uint8_t* data, size_t length) {
    ++ioCalls;
    ++writes;
    if (failWrite)
      return false;
    // SAFETY: character storage aliases the caller's length-bounded byte buffer.
    files[path].append(reinterpret_cast<const char*>(data), length);
    return true;
  }
  int streamOpen(const char* path, char mode) {
    ++ioCalls;
    if (mode != 'r' || failOpen || !files.count(path))
      return -1;
    opened = path;
    position = 0;
    return 0;
  }
  size_t streamSize(int) {
    ++ioCalls;
    return files.at(opened).size();
  }
  size_t streamRead(int, void* buffer, size_t length) {
    ++ioCalls;
    const auto& data = files.at(opened);
    length = std::min(length, data.size() - position);
    std::memcpy(buffer, data.data() + position, length);
    if (length && (corruptRead || (corruptFinalRead && opened.substr(opened.size() - 4) == ".txt")))
      static_cast<char*>(buffer)[0] ^= 1;
    position += length;
    return length;
  }
  bool streamClose(int) {
    ++ioCalls;
    opened.clear();
    return !failClose;
  }
  bool renamePath(const char* source, const char* destination) {
    ++ioCalls;
    if (failRename || files.count(destination))
      return false;
    files[destination] = files.at(source);
    files.erase(source);
    ++renames;
    return true;
  }
};
inline TestSD sd;
using PocketmageSD = TestSD;
inline TestSD& PM_SD() {
  return sd;
}
