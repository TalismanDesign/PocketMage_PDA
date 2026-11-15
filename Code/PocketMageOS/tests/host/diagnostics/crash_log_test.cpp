#include "diagnostics/crash_log.h"

#include <cstdlib>
#include <iostream>

#include "crash_hardware.h"
#include "diagnostics/crash_record.h"

static void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

static std::string savedPath() {
  for (const auto& file : sd.files) {
    if (file.first.size() >= 4 && file.first.substr(file.first.size() - 4) == ".txt")
      return file.first;
  }
  return {};
}

static void drainSerial(const std::string& report) {
  // Even one-byte writes must eventually deliver exactly one complete report.
  for (size_t i = 0; i < report.size() && Serial.output.size() < report.size(); ++i)
    pm_crash::poll();
  require(Serial.output == report, "serial replay must deliver a complete, unduplicated report");
}

static void recordChecks() {
  pm_crash::Record record{};
  require(!pm_crash::valid(record), "uninitialized retained RAM must not be a report");
  record.core = 1;
  record.pc = 0x42001234;
  record.backtraceCount = 2;
  record.backtrace[0] = 0x42001234;
  record.backtrace[1] = 0x40380100;
  std::strcpy(record.reason, "LoadProhibited\nInjected: text");
  pm_crash::seal(record);
  require(pm_crash::valid(record), "a committed panic snapshot must validate");
  char output[pm_crash::kReportCapacity];
  const size_t length = pm_crash::format(record, output, sizeof(output));
  require(length > 0 && length < sizeof(output), "report must fit the documented capacity");
  const std::string text(output, length);
  require(text.find("0x42001234 0x40380100") != std::string::npos,
          "report must preserve backtrace addresses for symbolication");
  require(text.find("\nInjected:") == std::string::npos,
          "multiline panic text must not inject report fields");
  char small[8]{};
  require(pm_crash::format(record, small, sizeof(small)) == 0,
          "report truncation must fail instead of publishing incomplete evidence");
  record.pc ^= 1;
  require(!pm_crash::valid(record), "RTC corruption must be detected");
  pm_crash::seal(record);
  record.backtraceCount = pm_crash::kBacktraceCapacity + 1;
  pm_crash::seal(record);
  require(!pm_crash::valid(record), "oversized backtrace must not allow an out-of-bounds read");
  record.backtraceCount = 0;
  std::memset(record.reason, 'x', sizeof(record.reason));
  pm_crash::seal(record);
  require(!pm_crash::valid(record), "unterminated retained strings must be rejected");
  std::memset(record.reason, 'r', sizeof(record.reason) - 1);
  record.reason[sizeof(record.reason) - 1] = '\0';
  std::memset(record.abortDetails, 'a', sizeof(record.abortDetails) - 1);
  std::memset(record.task, 't', sizeof(record.task) - 1);
  record.flags = pm_crash::RegistersPresent | pm_crash::SnapshotPresent | pm_crash::Aborted |
                 pm_crash::BacktraceCorrupt | pm_crash::BacktraceContinues;
  record.backtraceCount = pm_crash::kBacktraceCapacity;
  for (uint32_t& pc : record.backtrace)
    pc = UINT32_MAX;
  pm_crash::seal(record);
  require(pm_crash::format(record, output, sizeof(output)) > 0,
          "maximum backtrace and text fields must fit the report capacity");
  for (size_t byte = 0; byte < sizeof(record); ++byte) {
    // SAFETY: the record is plain data, so its bytes can be read and changed directly.
    auto* data = reinterpret_cast<unsigned char*>(&record);
    data[byte] ^= 1;
    require(!pm_crash::valid(record), "every retained byte must be integrity protected");
    data[byte] ^= 1;
  }
}

static void capture() {
  pm_crash::begin();
  pm_crash::poll();
  require(panicHandler != nullptr, "boot must install panic capture");
  XtExcFrame frame{};
  frame.pc = 0x42001234;
  frame.a0 = -1;
  frame.a1 = 0x3fca1000;
  frame.exccause = 28;
  frame.excvaddr = 0x10;
  g_exc_frames[1] = &frame;
  arduino_panic_info_t info{};
  info.core = 1;
  // SAFETY: the fake PC is serialized as an address and is never dereferenced.
  info.pc = reinterpret_cast<const void*>(static_cast<uintptr_t>(frame.pc));
  info.reason = "LoadProhibited";
  info.backtrace_len = 2;
  info.backtrace[0] = frame.pc;
  info.backtrace[1] = 0x40380100;
  const unsigned previousIo = sd.ioCalls;
  const unsigned previousQueries = runtimeQueries;
  panicHandler(&info, panicArgument);
  require(sd.ioCalls == previousIo && Serial.output.empty(),
          "panic capture must not perform SD or serial I/O");
  require(runtimeQueries == previousQueries, "panic must use cached service state without locks");
  g_exc_frames[1] = nullptr;
  // begin() simulates the next boot while RTC memory survives.
  pm_crash::begin();
}

int main(int argc, char** argv) {
  require(argc == 2, "scenario required");
  const std::string scenario = argv[1];
  if (scenario == "record") {
    recordChecks();
    return 0;
  }
  if (scenario == "early-abort") {
    pm_crash::begin();
    arduino_panic_info_t info{};
    info.backtrace_len = 61;
    info.backtrace_corrupt = true;
    g_panic_abort = true;
    char details[] = "assert failed: endpoint\nusb_stream.c:42";
    g_panic_abort_details = details;
    panicHandler(&info, panicArgument);
    pm_crash::begin();
    pm_crash::poll();
    const auto& text = sd.files.at(savedPath());
    for (const char* field : {"Runtime sample present: no", "Registers present: no", "Abort: yes",
                              "assert failed: endpoint usb_stream.c:42", "Backtrace corrupt: yes",
                              "Backtrace continues: yes"})
      require(text.find(field) != std::string::npos,
              "early abort must preserve bounded partial evidence");
    return 0;
  }
  pm_crash::begin();
  pm_crash::poll();
  require(sd.ioCalls == 0, "cold boot without a panic must not access the card");
  capture();
  if (scenario == "storage") {
    pm_usb::currentOwner = pm_usb::Owner::Storage;
    pm_crash::poll();
    require(sd.ioCalls == 0, "export must defer while mass storage owns the card");
    pm_usb::currentOwner = pm_usb::Owner::DebugSerial;
  }
  if (scenario == "no-card")
    sd.noCard = true;
  if (scenario == "mkdir")
    sd.failMkdir = true;
  if (scenario == "listing")
    sd.failList = true;
  if (scenario == "write")
    sd.failWrite = true;
  if (scenario == "open")
    sd.failOpen = true;
  if (scenario == "readback")
    sd.corruptRead = true;
  if (scenario == "close")
    sd.failClose = true;
  if (scenario == "rename")
    sd.failRename = true;
  if (scenario == "post-rename")
    sd.corruptFinalRead = true;
  pm_crash::poll();
  if (scenario == "post-rename") {
    require(sd.renames == 1 && sd.opened.empty(),
            "final readback failure must still close the stream");
    const unsigned previousWrites = sd.writes;
    sd.corruptFinalRead = false;
    pm_crash::begin();
    pm_crash::poll();
    require(sd.writes == previousWrites,
            "completed file must be verified and acknowledged without rewriting");
  } else if (scenario != "capture" && scenario != "serial" && scenario != "serial-ownership" &&
             scenario != "collision" && scenario != "storage") {
    require(savedPath().empty(), "failed export must not publish a final report");
    require(sd.opened.empty(), "failed export must close every read stream");
    const unsigned previousIo = sd.ioCalls;
    pm_crash::poll();
    require(sd.ioCalls == previousIo, "failed export must not retry on every poll");
    sd.noCard = sd.failMkdir = sd.failList = sd.failWrite = false;
    sd.failOpen = sd.corruptRead = sd.failClose = sd.failRename = false;
    if (scenario == "write")
      pm_crash::begin();
    clockMs += 6000;
    pm_crash::poll();
  }
  const std::string path = savedPath();
  require(!path.empty(), "reboot must save a report to SD");
  const std::string report = sd.files.at(path);
  require(report.find("LoadProhibited") != std::string::npos, "panic reason must survive reboot");
  require(report.find("usb_stream_proc") != std::string::npos, "crashed task must be identified");
  require(report.find("EXCVADDR: 0x00000010") != std::string::npos, "fault address must survive");
  require(report.find("A0: 0xffffffff") != std::string::npos,
          "signed Xtensa register fields must preserve all 32 bits");
  require(report.find("abcd0000") != std::string::npos,
          "report must identify the exact firmware ELF");
  require(report.find("Audio state: Playing") != std::string::npos,
          "report must include sampled audio state");
  require(report.find("USB owner: Audio") != std::string::npos,
          "report must include sampled USB ownership");
  const unsigned previousWrites = sd.writes;
  if (scenario == "serial-ownership") {
    Serial.connected = true;
    pm_usb::currentOwner = pm_usb::Owner::DebugSerial;
    onAudioSample = [] { pm_usb::currentOwner = pm_usb::Owner::Audio; };
    pm_crash::poll();
    require(Serial.output.empty(),
            "serial replay must not use an owner sampled before USB handoff");
    return 0;
  }
  if (scenario == "collision") {
    // Reproduce the same retained record ID, with another file occupying its path.
    sd.files[path] = "previous unrelated report";
    capture();
    pm_crash::poll();
    require(sd.files.at(path) == "previous unrelated report",
            "export must never overwrite a final file");
    require(sd.writes == previousWrites, "collision must be detected before opening a write");
    return 0;
  }
  if (scenario == "serial") {
    Serial.connected = true;
    pm_crash::poll();
    require(Serial.output.empty(), "audio must release the USB PHY before serial replay");
    pm_usb::currentOwner = pm_usb::Owner::DebugSerial;
    Serial.maxWrite = 0;
    pm_crash::poll();
    require(Serial.output.empty(), "serial backpressure must retain the replay offset");
    Serial.maxWrite = 17;
    drainSerial(report);
    require(Serial.unlockedWrites == 0, "serial writes must exclude concurrent USB role changes");
    Serial.output.clear();
    pm_crash::poll();
    require(Serial.output.empty(), "serial report must not flood on every loop");
    Serial.connected = false;
    pm_crash::poll();
    Serial.connected = true;
    drainSerial(report);
  }
  pm_crash::begin();
  pm_crash::poll();
  require(sd.writes == previousWrites, "successful export must stay acknowledged across reboots");
}
