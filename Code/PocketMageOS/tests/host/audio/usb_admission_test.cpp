#include <pocketmage_audio/usb_audio_admission.h>

#include <cassert>
#include <cstddef>
#include <initializer_list>

using pm_audio::DmaSnapshot;
using pm_audio::SpeakerFootprint;

namespace {
constexpr uint32_t kRingBytes = 6 * 1024;
constexpr uint32_t kReadBytes = 4 * 1024;
constexpr uint32_t kPcmBytes = 4608;
constexpr DmaSnapshot kFragmented{40 * 1024, 7156};
constexpr DmaSnapshot kAbortedResume{14024, 9204};
constexpr uint32_t kFirmwareRingBytes = 8 * 1024;
constexpr uint32_t kTaskStackBytes = 8192;

struct FakeHeap {
  static constexpr size_t kMaxBlocks = 8;
  static size_t blocks[kMaxBlocks];
  static size_t blockCount;
  static size_t live;
  static size_t calls;

  static void reset(std::initializer_list<size_t> sizes) {
    blockCount = 0;
    for (size_t size : sizes)
      blocks[blockCount++] = size;
    live = 0;
    calls = 0;
  }
  static void *calloc(size_t count, size_t size) {
    ++calls;
    const size_t need = count * size + pm_audio::kHeapBlockOverhead;
    for (size_t i = 0; i < blockCount; ++i) {
      if (blocks[i] >= need) {
        blocks[i] -= need;
        ++live;
        return &blocks[i];
      }
    }
    return nullptr;
  }
  static void free(void *block) {
    if (block)
      --live;
  }
  static pm_audio::DmaAllocator allocator() { return {calloc, free}; }
};
size_t FakeHeap::blocks[FakeHeap::kMaxBlocks] = {};
size_t FakeHeap::blockCount = 0;
size_t FakeHeap::live = 0;
size_t FakeHeap::calls = 0;
} 

int main() {
  SpeakerFootprint stereo{};
  assert(pm_audio::speakerFootprint(48000, 16, 2, stereo));
  assert(stereo.packetBytes == 192);
  assert(stereo.urbBytes > 16 * 192 && stereo.urbBytes < 4 * 1024);
  assert(stereo.totalBytes > CONFIG_NUM_ISOC_SPK_URBS * 16 * 192 &&
         stereo.totalBytes < 16 * 1024);

  SpeakerFootprint mono{};
  assert(pm_audio::speakerFootprint(44100, 16, 1, mono));
  assert(mono.packetBytes == 88 && mono.totalBytes < stereo.totalBytes);

  SpeakerFootprint rejected{};
  assert(!pm_audio::speakerFootprint(96000, 16, 2, rejected));
  assert(!pm_audio::speakerFootprint(48000, 24, 2, rejected));
  assert(!pm_audio::speakerFootprint(48000, 16, 6, rejected));
  assert(!pm_audio::speakerFootprint(999, 16, 1, rejected));

  assert(pm_audio::canHostPlay(kFragmented, kRingBytes, 0));
  assert(pm_audio::canResumeSpeaker(kFragmented, stereo));
  assert(!pm_audio::canHostPlay({20 * 1024, 7156}, kRingBytes, 0));

  assert(!pm_audio::canHostPlay({128 * 1024, kRingBytes - 1}, kRingBytes, 0));
  assert(!pm_audio::canHostPlay({pm_audio::hostStartBytes(kRingBytes), 32 * 1024}, kRingBytes, 0));

  assert(!pm_audio::canResumeSpeaker({64 * 1024, stereo.urbBytes - 1}, stereo));
  assert(!pm_audio::canResumeSpeaker({stereo.totalBytes, 32 * 1024}, stereo));
  assert(pm_audio::canResumeSpeaker(
      {stereo.totalBytes + pm_audio::kAdmissionHeadroom, stereo.urbBytes}, stereo));

  const uint32_t hostAndSpeaker =
      pm_audio::hostConnectedBytes(kRingBytes) + stereo.totalBytes + pm_audio::kAdmissionHeadroom;
  assert(pm_audio::canHostPlay({hostAndSpeaker, 32 * 1024}, kRingBytes, 0));
  assert(!pm_audio::canHostPlay({hostAndSpeaker - 1, 32 * 1024}, kRingBytes, 0));
  assert(!pm_audio::canHostPlay({pm_audio::hostStartBytes(kRingBytes) + stereo.totalBytes +
                                     pm_audio::kAdmissionHeadroom,
                                 32 * 1024},
                                kRingBytes, 0));

  const uint32_t reserve =
      pm_audio::decodeBufferBytes(kReadBytes, kPcmBytes) + pm_audio::kMp3DecoderBytes;
  assert(reserve > kReadBytes + kPcmBytes + pm_audio::kMp3DecoderBytes);
  assert(pm_audio::canHostPlay({hostAndSpeaker + reserve, 32 * 1024}, kRingBytes, reserve));
  assert(!pm_audio::canHostPlay({hostAndSpeaker + reserve - 1, 32 * 1024}, kRingBytes, reserve));

  constexpr uint32_t kHidReturnedBytes = 24716;
  constexpr DmaSnapshot kSecondLaunchHandoff{63220 + kHidReturnedBytes, 31732};
  const uint32_t fullReserve =
      pm_audio::decodeTaskBytes(kTaskStackBytes) + reserve;
  SpeakerFootprint cd{};
  assert(pm_audio::speakerFootprint(44100, 16, 2, cd));
  assert(pm_audio::canResumeSpeaker(kAbortedResume, cd));
  assert(pm_audio::canHostPlay(kSecondLaunchHandoff, kFirmwareRingBytes, fullReserve));
  assert(!pm_audio::canHostPlay(
      {kSecondLaunchHandoff.freeBytes - pm_audio::kAdmissionHeadroom, 31732}, kFirmwareRingBytes,
      fullReserve));
  assert(!pm_audio::canHostPlay({63220, 31732}, kFirmwareRingBytes, fullReserve));

  FakeHeap::reset({kAbortedResume.largestBlock, 1000, 1000, 1000, 1000, 1000});
  assert(!pm_audio::trialSpeakerUrbs(cd, FakeHeap::allocator()));
  assert(FakeHeap::live == 0);
  FakeHeap::reset({16 * 1024});
  assert(pm_audio::trialSpeakerUrbs(stereo, FakeHeap::allocator()));
  assert(FakeHeap::live == 0);
  assert(FakeHeap::calls == 1 + 2 * CONFIG_NUM_ISOC_SPK_URBS);
}
