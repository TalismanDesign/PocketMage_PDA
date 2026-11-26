#include <pocketmage_audio/wav_stream.h>

#include <fixture_file.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

using Bytes = std::vector<uint8_t>;

void le16(Bytes &out, uint16_t value) {
  out.push_back(uint8_t(value));
  out.push_back(uint8_t(value >> 8));
}

void le32(Bytes &out, uint32_t value) {
  for (unsigned i = 0; i < 4; ++i)
    out.push_back(uint8_t(value >> (8 * i)));
}

void text(Bytes &out, const char *value) {
  while (*value)
    out.push_back(uint8_t(*value++));
}

Bytes extensibleWav(uint16_t subformat, uint16_t chunkLength = 40) {
  Bytes fmt;
  le16(fmt, 0xFFFE);
  le16(fmt, 2);
  le32(fmt, 48000);
  le32(fmt, 192000);
  le16(fmt, 4);
  le16(fmt, 16);
  le16(fmt, 22);
  le16(fmt, 16);
  le32(fmt, 0);
  le16(fmt, subformat);
  static const uint8_t kGuidTail[14] = {0, 0, 0, 0, 0x10, 0, 0x80, 0,
                                        0, 0xAA, 0, 0x38, 0x9B, 0x71};
  fmt.insert(fmt.end(), kGuidTail, kGuidTail + sizeof(kGuidTail));

  Bytes body;
  text(body, "WAVE");
  text(body, "fmt ");
  le32(body, chunkLength);
  const size_t fmtBytes = std::min<size_t>(chunkLength, fmt.size());
  body.insert(body.end(), fmt.begin(), fmt.begin() + fmtBytes);
  if (fmtBytes & 1)
    body.push_back(0);
  text(body, "data");
  le32(body, 8);
  for (uint8_t sample : {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88})
    body.push_back(sample);

  Bytes file;
  text(file, "RIFF");
  le32(file, static_cast<uint32_t>(body.size()));
  file.insert(file.end(), body.begin(), body.end());
  return file;
}

void normalizer_rewrites_extensible_pcm_in_place() {
  Bytes file = extensibleWav(1);
  const uint32_t oldDataOffset = pm_audio::wavLe32(file.data() + 4);
  assert(pm_audio::normalizeWavHeader(file.data(), file.size()) == 24);
  file.resize(file.size() - 24);
  assert(pm_audio::wavLe32(file.data() + 4) == oldDataOffset - 24);
  assert(pm_audio::wavLe32(file.data() + 16) == 16);
  assert(pm_audio::wavLe16(file.data() + 20) == 1);
  assert(!std::memcmp(file.data() + 36, "data", 4));
  assert(pm_audio::wavLe32(file.data() + 40) == 8);
  const Bytes samples{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  assert(Bytes(file.begin() + 44, file.end()) == samples);
  assert(pm_audio::normalizeWavHeader(file.data(), file.size()) == 0);
}

void normalizer_rejects_unsupported_and_short_headers() {
  Bytes unknown = extensibleWav(6);
  assert(pm_audio::normalizeWavHeader(unknown.data(), unknown.size()) == -1);

  Bytes shortFmt = extensibleWav(1, 18);
  assert(pm_audio::normalizeWavHeader(shortFmt.data(), shortFmt.size()) == -1);

  Bytes classic(64, 0);
  std::memcpy(classic.data(), "RIFF", 4);
  std::memcpy(classic.data() + 8, "WAVE", 4);
  const Bytes original = classic;
  assert(pm_audio::normalizeWavHeader(classic.data(), classic.size()) == 0);
  assert(classic == original);
  assert(pm_audio::normalizeWavHeader(classic.data(), 4) == 0);
}

void float_samples_convert_to_pcm16_with_clamping() {
  const float values[] = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f, 2.0f};
  Bytes src;
  for (float value : values) {
    uint32_t raw = 0;
    std::memcpy(&raw, &value, sizeof(raw));
    for (unsigned b = 0; b < sizeof(raw); ++b)
      src.push_back(uint8_t(raw >> (8 * b)));
  }
  const uint32_t nanRaw = 0x7FC00000;
  for (unsigned b = 0; b < sizeof(nanRaw); ++b)
    src.push_back(uint8_t(nanRaw >> (8 * b)));

  int16_t pcm[7] = {};
  assert(pm_audio::wavFloatToPcm16(pcm, src.data(), 7, 32) == sizeof(pcm));
  assert(pcm[0] == -32768);
  assert(pcm[1] == -16384);
  assert(pcm[2] == 0);
  assert(pcm[3] == 16384);
  assert(pcm[4] == 32767);
  assert(pcm[5] == 32767);
  assert(pcm[6] == 0);
}

void float64_samples_convert_to_pcm16() {
  const double values[] = {-0.25, 0.25};
  Bytes src;
  for (double value : values) {
    uint64_t raw = 0;
    std::memcpy(&raw, &value, sizeof(raw));
    for (unsigned b = 0; b < sizeof(raw); ++b)
      src.push_back(uint8_t(raw >> (8 * b)));
  }

  int16_t pcm[2] = {};
  assert(pm_audio::wavFloatToPcm16(pcm, src.data(), 2, 64) == sizeof(pcm));
  assert(pcm[0] == -8192);
  assert(pcm[1] == 8192);
  assert(pm_audio::wavFloatToPcm16(pcm, src.data(), 2, 16) == 0);
}

void real_spec_wav_is_left_unchanged(const std::string &directory) {
  Bytes file = loadFixture(directory, "spec.wav");
  assert(file.size() > 44);
  const Bytes original = file;
  assert(pm_audio::normalizeWavHeader(file.data(), file.size()) == 0);
  assert(file == original);
}

int main(int argc, char **argv) {
  assert(argc == 2);
  normalizer_rewrites_extensible_pcm_in_place();
  normalizer_rejects_unsupported_and_short_headers();
  float_samples_convert_to_pcm16_with_clamping();
  float64_samples_convert_to_pcm16();
  real_spec_wav_is_left_unchanged(argv[1]);
}
