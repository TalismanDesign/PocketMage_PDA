#include <pocketmage_media/media_reader.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

class MemoryReader : public pm_media::Reader {
public:
  std::vector<uint8_t> data;
  bool fail = false;
  size_t bytesRead = 0;
  uint32_t size() const override { return static_cast<uint32_t>(data.size()); }
  bool readAt(uint32_t offset, void *out, size_t count) override {
    if (fail || offset > data.size() || count > data.size() - offset) return false;
    std::memcpy(out, data.data() + offset, count);
    bytesRead += count;
    return true;
  }
};

int main() {
  MemoryReader reader;
  reader.data = {'I','D','3',3,0,0,0,0,0,44,
                 'A','P','I','C',0,0,0,18,0,0,
                 0,'i','m','a','g','e','/','j','p','e','g',0,3,0,255,216,255,217,
                 'T','I','T','2',0,0,0,6,0,0,3,'H','e','l','l','o'};
  MediaInfo info{};
  assert(pm_media::readId3(reader, info));
  assert(std::strcmp(info.title, "Hello") == 0);
  assert(info.audioOffset == 54 && info.coverOffset == 34 && info.coverLength == 4);
  assert(info.coverFormat == ImageFormat::JPEG);

  // A large image must not hide following text tags behind a prefix-read limit.
  reader.data.insert(reader.data.begin() + 38, 8192, 0);
  reader.data[16] = 32; // APIC payload 8210, big-endian.
  reader.data[8] = 64; // Tag body 8236, syncsafe.
  info = {};
  assert(pm_media::readId3(reader, info));
  assert(std::strcmp(info.title, "Hello") == 0 && info.coverLength == 8196);
  reader.bytesRead = 0;
  uint32_t audioOffset = 0;
  assert(pm_media::audioOffset(reader, audioOffset) && audioOffset == 8246);
  assert(reader.bytesRead == 10);
  reader.data[6] = 1;
  assert(!pm_media::readId3(reader, info));
  reader.data = {'I','D','3',2,0,0,0,0,0,12,'T','T','2',0,0,6,0,'C','a','f',233,0};
  info = {};
  assert(pm_media::readId3(reader, info));
  assert(std::strcmp(info.title, "Caf\xc3\xa9") == 0);
  reader.data = {'I','D','3',4,0,0,0,0,0,17,'T','L','E','N',0,0,0,7,0,0,
                 3,'1','2','3','4','5','6'};
  info = {};
  assert(pm_media::readId3(reader, info) && info.durationMs == 123456);
  reader.fail = true;
  assert(!pm_media::readId3(reader, info));

  reader.fail = false;
  reader.data.assign(417 * 2, 0);
  for (size_t offset : {0u, 417u}) {
    reader.data[offset] = 0xff; reader.data[offset + 1] = 0xfb;
    reader.data[offset + 2] = 0x90;
  }
  uint64_t duration = 0;
  assert(pm_media::mp3Duration(reader, 0, duration));
  assert(duration == 52); // 2304 samples at 44100 Hz, rounded down once.
}
