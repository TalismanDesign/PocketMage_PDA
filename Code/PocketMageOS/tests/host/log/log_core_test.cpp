#include <pocketmage_log/log_core.h>

#include <cstdlib>
#include <iostream>
#include <string>

using pm_log::Level;

static void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

template <size_t N> static std::string queued(const pm_log::LineQueue<N>& queue) {
  char out[N];
  const size_t length = queue.peek(out, sizeof(out));
  return std::string(out, length);
}

template <size_t N> static std::string tailText(const pm_log::TailBuffer<N>& tail,
                                                size_t capacity = N + 1) {
  std::string out(capacity, '\0');
  const size_t length = tail.copy(out.data(), capacity);
  require(out[length] == '\0', "tail copy must be NUL-terminated");
  out.resize(length);
  return out;
}

static void pushLine(pm_log::LineQueue<64>& queue, const std::string& header,
                     const std::string& message) {
  queue.push(header.data(), header.size(), message.data(), message.size());
}

static void levelChecks() {
  for (Level level : {Level::Error, Level::Warn, Level::Info, Level::Debug, Level::Verbose}) {
    Level parsed = Level::None;
    require(pm_log::levelFromLetter(pm_log::levelLetter(level), &parsed) && parsed == level,
            "every message level must round-trip through its letter");
    require(pm_log::messageLevel(static_cast<int>(level)), "Error to Verbose are message levels");
  }
  require(pm_log::levelLetter(Level::None) == '?', "None has no letter");
  require(!pm_log::messageLevel(0) && !pm_log::messageLevel(6) && !pm_log::messageLevel(-1),
          "values outside Error to Verbose must be rejected before narrowing");
  Level untouched = Level::Warn;
  require(!pm_log::levelFromLetter('X', &untouched) && untouched == Level::Warn,
          "an unknown letter must leave the output untouched");
}

static void formatChecks() {
  char header[pm_log::kHeaderCapacity];
  size_t length = pm_log::formatHeader(header, 1234, Level::Warn, "AUDIO");
  require(std::string(header, length) == "1234 W [AUDIO] ", "header must follow the line format");

  length = pm_log::formatHeader(header, UINT32_MAX, Level::Error, "A_TAG_LONGER_THAN_FIFTEEN");
  require(std::string(header, length) == "4294967295 E [A_TAG_LONGER_TH] ",
          "the widest header must fit with its tag cut to kTagCapacity - 1");

  length = pm_log::formatHeader(header, 5, Level::Info, "T\nX");
  require(std::string(header, length) == "5 I [T X] ", "tags must not break the line");

  length = pm_log::formatHeader(header, 0, Level::Info, nullptr);
  require(std::string(header, length) == "0 I [] ", "a null tag must give an empty tag");

  char message[] = "first\tsecond\x7f\r\n\n";
  length = pm_log::cleanMessage(message, sizeof(message) - 1);
  require(std::string(message, length) == "first second ",
          "trailing breaks are dropped and control bytes become spaces");
  require(pm_log::cleanMessage(nullptr, 4) == 0, "null message cleans to nothing");

  require(pm_log::storedLength(-1, 16) == 0, "an encoding error stores nothing");
  require(pm_log::storedLength(10, 16) == 10, "a short message stores its full length");
  require(pm_log::storedLength(300, 256) == 255, "a cut message stores capacity - 1");
  require(pm_log::storedLength(5, 0) == 0, "an empty buffer stores nothing");
}

static void idfParseChecks() {
  char line[] = "E (1234) wifi: connect failed: timeout\n";
  Level level = Level::None;
  const char* tag = nullptr;
  const char* message = nullptr;
  require(pm_log::parseIdfLine(line, &level, &tag, &message), "a v1 IDF line must parse");
  require(level == Level::Error && std::string(tag) == "wifi" &&
              std::string(message) == "connect failed: timeout\n",
          "only the first separator ends the tag");

  for (const char* bad : {"", "X (1) t: m", "E 1) t: m", "E () t: m", "E (12 t: m", "E (12) : m",
                          "E (12) tag only", "plain text"}) {
    std::string copy = bad;
    Level unchanged = Level::Debug;
    const char* unchangedTag = "keep";
    require(!pm_log::parseIdfLine(copy.data(), &unchanged, &unchangedTag, &message) &&
                unchanged == Level::Debug && std::string(unchangedTag) == "keep" && copy == bad,
            "a malformed line must be rejected without changes");
  }
  require(!pm_log::parseIdfLine(nullptr, &level, &tag, &message), "null input is rejected");
}

static void levelTableChecks() {
  pm_log::TagLevels<2> levels{Level::Info};
  require(levels.allows(Level::Info, "any") && !levels.allows(Level::Debug, "any"),
          "the fallback level applies to unknown tags");
  require(!levels.allows(Level::None, "any"), "None is never written");
  require(levels.set("USB", Level::Verbose) && levels.allows(Level::Verbose, "USB"),
          "a tag entry overrides the fallback");
  require(levels.set("USB", Level::Error) && levels.levelFor("USB") == Level::Error,
          "setting a tag again updates its entry");
  require(levels.set("SD", Level::Warn), "the second slot fills");
  require(!levels.set("WIFI", Level::Debug), "a full table refuses new tags");
  require(levels.set("SD", Level::Debug), "a full table still updates existing tags");
  require(!levels.set("", Level::Debug), "an empty tag is refused");
  require(!levels.set("SIXTEEN_CHARS_XX", Level::Debug), "a tag that cannot fit is refused");
  require(levels.set("*", Level::Warn) && levels.levelFor("other") == Level::Warn,
          "\"*\" sets the fallback");
  require(levels.set(nullptr, Level::Error) && levels.levelFor(nullptr) == Level::Error,
          "null sets and reads the fallback");
  require(levels.levelFor("USB") == Level::Error, "the fallback does not touch tag entries");
}

static void queueChecks() {
  pm_log::LineQueue<64> queue;
  pushLine(queue, "1 I [A] ", "one");
  pushLine(queue, "2 I [A] ", "two");
  require(queued(queue) == "1 I [A] one\n2 I [A] two\n", "lines queue in order with newlines");

  char partial[5];
  require(queue.peek(partial, sizeof(partial)) == 5, "peek copies at most its capacity");
  require(queued(queue).size() == 24, "peek must not remove bytes");
  queue.consume(12);
  require(queued(queue) == "2 I [A] two\n", "consume removes from the front");
  queue.consume(1000);
  require(queue.used == 0, "consume past the end empties the queue");

  queue.clear();
  const std::string big(40, 'x');
  pushLine(queue, "", big);
  pushLine(queue, "", big);
  pushLine(queue, "", big);
  require(queue.dropped == 2, "lines that do not fit are counted");
  queue.consume(41);
  pushLine(queue, "3 E [B] ", "back");
  require(queued(queue) == "-- 2 log lines dropped --\n3 E [B] back\n",
          "the next line that fits is preceded by the dropped count");
  require(queue.dropped == 0, "the count resets once reported");

  queue.clear();
  pushLine(queue, "", std::string(30, 'a'));
  queue.consume(31);
  pushLine(queue, "", std::string(50, 'b'));
  require(queued(queue) == std::string(50, 'b') + "\n", "a line wraps around the ring intact");

  queue.clear();
  pushLine(queue, "", std::string(64, 'c'));
  require(queue.used == 0 && queue.dropped == 1, "a line over capacity is dropped, not cut");
  queue.clear();
  require(queue.dropped == 0 && queue.used == 0, "clear resets the drop count");

  pm_log::LineQueue<40> tight;
  const std::string fill(30, 'f');
  tight.push("", 0, fill.data(), fill.size());
  tight.push("", 0, fill.data(), fill.size());
  const size_t before = tight.used;
  tight.push("h ", 2, "m", 1);
  require(before == fill.size() + 1 && tight.used == before && tight.dropped == 2,
          "a line that fits only without its dropped note is dropped too");
}

static void rotationChecks() {
  require(!pm_log::needsRotation(0, 1000, 100), "an empty file never rotates");
  require(!pm_log::needsRotation(50, 50, 100), "a file reaching the limit exactly stays");
  require(pm_log::needsRotation(51, 50, 100), "a file passing the limit rotates first");
}

static void tailChecks() {
  pm_log::TailBuffer<16> tail;
  require(tailText(tail).empty(), "an empty tail copies nothing");
  tail.push("a ", 2, "b", 1);
  require(tailText(tail) == "a b\n", "a short tail copies whole");

  tail.push("ccc ", 4, "dddd", 4);
  tail.push("ee ", 3, "ff", 2);
  require(tail.wrapped, "the tail must have wrapped");
  require(tailText(tail) == "ccc dddd\nee ff\n",
          "after wrapping, the copy starts after the first newline");

  tail.push("g", 1, "", 0);
  require(tailText(tail) == "ee ff\ng\n", "the cut line is skipped, not shown in part");

  pm_log::TailBuffer<64> roomy;
  roomy.push("1 ", 2, "first", 5);
  roomy.push("2 ", 2, "second", 6);
  require(tailText(roomy, 11) == "2 second\n",
          "a small output keeps the newest whole lines only");

  char one[1];
  require(roomy.copy(one, sizeof(one)) == 0 && one[0] == '\0', "a 1-byte output gets just NUL");
  require(roomy.copy(nullptr, 8) == 0, "null output is ignored");

  pm_log::TailBuffer<8> noBreak;
  noBreak.append("abcdefghijk", 11);
  require(tailText(noBreak).empty(), "a wrapped tail without a newline shows nothing partial");
}

static void replayChecks() {
  pm_log::TagLevels<3> levels{Level::Info};
  require(levels.set("USB_STREAM", Level::Error), "a tag entry is stored before the replay");
  require(levels.set("ELF", Level::Debug), "a second entry is stored before the replay");

  // IDF clears every per-tag level when the default changes, so the replay has to follow it.
  std::string order;
  const size_t applied = pm_log::replayLevels(levels, [&order](const char* tag,
                                                               pm_log_level level) {
    order += std::string(tag) + "=" + std::to_string(static_cast<int>(level)) + " ";
  });
  require(applied == 2, "the replay applies every stored entry");
  require(order == "USB_STREAM=1 ELF=4 ", "the replay keeps the order the tags were set in");

  require(levels.set("*", Level::Warn), "\"*\" still sets the fallback");
  require(levels.levelFor("other") == Level::Warn, "\"*\" changes unknown tags only");
  require(levels.levelFor("USB_STREAM") == Level::Error, "\"*\" keeps the stored entries");
  require(pm_log::replayLevels(levels, [](const char*, pm_log_level) {}) == 2,
          "the entries are still there to put back after a \"*\" change");

  pm_log::TagLevels<1> full{Level::Info};
  require(full.set("ONE", Level::Warn), "the single slot fills");
  require(full.copyEntries(nullptr, 1) == 1, "a null output still reports the entry count");
  pm_log::TagLevels<1>::Entry copied[2];
  require(full.copyEntries(copied, 2) == 1 && std::string(copied[0].tag) == "ONE" &&
              copied[0].level == Level::Warn,
          "the snapshot copies the stored entry");
}

int main() {
  levelChecks();
  formatChecks();
  idfParseChecks();
  levelTableChecks();
  replayChecks();
  queueChecks();
  rotationChecks();
  tailChecks();
  return 0;
}
