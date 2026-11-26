#include <pocketmage_media/media_reader.h>

#include <fixture_file.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

using Bytes = std::vector<uint8_t>;

class MemoryReader : public pm_media::Reader {
public:
  Bytes data;
  uint32_t size() const override { return static_cast<uint32_t>(data.size()); }
  bool readAt(uint32_t offset, void *out, size_t count) override {
    if (offset > data.size() || count > data.size() - offset) return false;
    std::memcpy(out, data.data() + offset, count);
    return true;
  }
};

void le16(Bytes &out, uint16_t value) {
  out.push_back(uint8_t(value));
  out.push_back(uint8_t(value >> 8));
}

void le32(Bytes &out, uint32_t value) {
  for (unsigned i = 0; i < 4; ++i)
    out.push_back(uint8_t(value >> (8 * i)));
}

void le64(Bytes &out, uint64_t value) {
  for (unsigned i = 0; i < 8; ++i)
    out.push_back(uint8_t(value >> (8 * i)));
}

void be32(Bytes &out, uint32_t value) {
  for (unsigned i = 0; i < 4; ++i)
    out.push_back(uint8_t(value >> (8 * (3 - i))));
}

void text(Bytes &out, const std::string &value) {
  out.insert(out.end(), value.begin(), value.end());
}

void chunk(Bytes &out, const char *id, const Bytes &body) {
  text(out, id);
  le32(out, static_cast<uint32_t>(body.size()));
  out.insert(out.end(), body.begin(), body.end());
  if (body.size() & 1)
    out.push_back(0);
}

Bytes wavFmt(uint16_t codec, uint16_t channels, uint32_t rate, uint16_t bits) {
  Bytes fmt;
  le16(fmt, codec);
  le16(fmt, channels);
  le32(fmt, rate);
  le32(fmt, rate * channels * bits / 8);
  le16(fmt, channels * bits / 8);
  le16(fmt, bits);
  return fmt;
}

Bytes wavExtensibleFmt(uint16_t subformat, uint16_t channels, uint32_t rate, uint16_t bits) {
  Bytes fmt = wavFmt(0xFFFE, channels, rate, bits);
  le16(fmt, 22); 
  le16(fmt, bits);
  le32(fmt, 0); 
  le16(fmt, subformat); 
  static const uint8_t kGuidTail[14] = {0, 0, 0, 0, 0x10, 0, 0x80, 0,
                                        0, 0xAA, 0, 0x38, 0x9B, 0x71};
  fmt.insert(fmt.end(), kGuidTail, kGuidTail + sizeof(kGuidTail));
  assert(fmt.size() == 40);
  return fmt;
}

Bytes wavWithFmt(const Bytes &fmt, uint32_t dataBytes = 176400) {
  Bytes body;
  text(body, "WAVE");
  chunk(body, "fmt ", fmt);
  chunk(body, "data", Bytes(dataBytes, 0x80));
  Bytes file;
  text(file, "RIFF");
  le32(file, static_cast<uint32_t>(body.size()));
  file.insert(file.end(), body.begin(), body.end());
  return file;
}

Bytes wavFile() {
  Bytes fmt = wavFmt(1, 2, 44100, 16);
  Bytes info;
  Bytes title, artist, album;
  text(title, "Wave Song");
  title.push_back(0);
  text(artist, "Wave Artist");
  artist.push_back(0);
  text(album, "Wave Album");
  album.push_back(0);
  chunk(info, "INAM", title);
  chunk(info, "IART", artist);
  chunk(info, "IPRD", album);
  Bytes list;
  text(list, "INFO");
  list.insert(list.end(), info.begin(), info.end());
  Bytes data(176400, 0x80);
  Bytes body;
  text(body, "WAVE");
  chunk(body, "fmt ", fmt);
  chunk(body, "LIST", list);
  chunk(body, "data", data);
  Bytes file;
  text(file, "RIFF");
  le32(file, static_cast<uint32_t>(body.size()));
  file.insert(file.end(), body.begin(), body.end());
  return file;
}

Bytes id3PictureTag() {
  Bytes apic{0, 'i', 'm', 'a', 'g', 'e', '/', 'p', 'n', 'g', 0, 3, 0, 0x89, 'P', 'N', 'G'};
  Bytes frame;
  text(frame, "APIC");
  be32(frame, static_cast<uint32_t>(apic.size()));
  frame.push_back(0); 
  frame.push_back(0);
  frame.insert(frame.end(), apic.begin(), apic.end());
  if (frame.size() & 1)
    frame.push_back(0);
  Bytes tag{'I', 'D', '3', 3, 0, 0, 0, 0,
            static_cast<uint8_t>(frame.size() >> 7),
            static_cast<uint8_t>(frame.size() & 0x7f)};
  assert(tag.size() == 10);
  tag.insert(tag.end(), frame.begin(), frame.end());
  return tag;
}

Bytes wavWithId3() {
  Bytes file = wavFile();
  Bytes tag = id3PictureTag();
  Bytes body(file.begin() + 12, file.end());
  chunk(body, "id3 ", tag);
  Bytes out(file.begin(), file.begin() + 4);
  le32(out, static_cast<uint32_t>(body.size()));
  text(out, "WAVE");
  out.insert(out.end(), body.begin(), body.end());
  return out;
}

void page(Bytes &out, uint8_t type, uint64_t granule, uint32_t sequence,
          const Bytes &payload) {
  text(out, "OggS");
  out.push_back(0);
  out.push_back(type);
  le64(out, granule);
  le32(out, 1); 
  le32(out, sequence);
  le32(out, 0); 
  size_t left = payload.size();
  Bytes segments;
  while (left >= 255) {
    segments.push_back(255);
    left -= 255;
  }
  if (left)
    segments.push_back(static_cast<uint8_t>(left));
  out.push_back(static_cast<uint8_t>(segments.size()));
  out.insert(out.end(), segments.begin(), segments.end());
  out.insert(out.end(), payload.begin(), payload.end());
}

Bytes vorbisId() {
  Bytes packet;
  packet.push_back(1);
  text(packet, "vorbis");
  le32(packet, 0);
  packet.push_back(2);
  le32(packet, 44100);
  le32(packet, 0);
  le32(packet, 128000);
  le32(packet, 0);
  packet.push_back(0xB8);
  packet.push_back(1);
  assert(packet.size() == 30);
  return packet;
}

Bytes comment(const std::vector<std::string> &entries, bool opus = false) {
  Bytes packet;
  if (opus)
    text(packet, "OpusTags");
  else {
    packet.push_back(3);
    text(packet, "vorbis");
  }
  le32(packet, 4);
  text(packet, "test");
  le32(packet, static_cast<uint32_t>(entries.size()));
  for (const auto &entry : entries) {
    le32(packet, static_cast<uint32_t>(entry.size()));
    text(packet, entry);
  }
  return packet;
}

std::string base64(const Bytes &in) {
  static const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  for (size_t i = 0; i < in.size(); i += 3) {
    const uint32_t word = uint32_t(in[i]) << 16 |
                          (i + 1 < in.size() ? uint32_t(in[i + 1]) << 8 : 0) |
                          (i + 2 < in.size() ? uint32_t(in[i + 2]) : 0);
    out.push_back(alphabet[(word >> 18) & 63]);
    out.push_back(alphabet[(word >> 12) & 63]);
    out.push_back(i + 1 < in.size() ? alphabet[(word >> 6) & 63] : '=');
    out.push_back(i + 2 < in.size() ? alphabet[word & 63] : '=');
  }
  return out;
}

Bytes decodeBase64(const uint8_t *in, size_t len) {
  Bytes out;
  auto value = [](char c) -> uint8_t {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    return c == '+' ? 62 : c == '/' ? 63 : 0;
  };
  for (size_t i = 0; i + 3 < len; i += 4) {
    const uint32_t word = uint32_t(value(char(in[i]))) << 18 |
                          uint32_t(value(char(in[i + 1]))) << 12 |
                          (in[i + 2] == '=' ? 0 : uint32_t(value(char(in[i + 2]))) << 6) |
                          (in[i + 3] == '=' ? 0 : value(char(in[i + 3])));
    out.push_back(uint8_t(word >> 16));
    if (in[i + 2] != '=') out.push_back(uint8_t(word >> 8));
    if (in[i + 3] != '=') out.push_back(uint8_t(word));
  }
  return out;
}

std::string pictureComment(const Bytes &image) {
  Bytes block;
  be32(block, 3); 
  be32(block, 10);
  text(block, "image/jpeg");
  be32(block, 0);
  be32(block, 32);
  be32(block, 32);
  be32(block, 24);
  be32(block, 0);
  be32(block, static_cast<uint32_t>(image.size()));
  block.insert(block.end(), image.begin(), image.end());
  return "METADATA_BLOCK_PICTURE=" + base64(block);
}

std::string pictureComment() {
  return pictureComment(Bytes{0xFF, 0xD8, 0xFF, 0xD9});
}

Bytes oggVorbis() {
  const std::string art = pictureComment();
  const std::vector<std::string> entries{"TITLE=Ogg Song", "ARTIST=Ogg Artist",
                                         "ALBUM=Ogg Album", art,
                                         "IGNORE=" + std::string(400, 'x')};
  Bytes body = comment(entries);
  Bytes file;
  Bytes first = vorbisId();
  first.insert(first.end(), body.begin(), body.end());
  page(file, 0, 0, 0, Bytes(first.begin(), first.begin() + 255));
  page(file, 0, 0, 1, Bytes(first.begin() + 255, first.end()));
  page(file, 4, 44100, 2, Bytes{'x'});
  return file;
}

Bytes oggVorbisLargeArt() {
  Bytes image{0xFF, 0xD8};
  image.insert(image.end(), 4096, 0x11);
  image.push_back(0xFF);
  image.push_back(0xD9);
  Bytes body = comment({"TITLE=Ogg Song", pictureComment(image)});
  Bytes first = vorbisId();
  first.insert(first.end(), body.begin(), body.end());
  Bytes file;
  page(file, 0, 0, 0, first);
  page(file, 4, 44100, 1, Bytes{'x'});
  return file;
}

Bytes opusHead() {
  Bytes packet;
  text(packet, "OpusHead");
  packet.push_back(1);
  packet.push_back(2);
  le16(packet, 312);
  le32(packet, 48000);
  le16(packet, 0);
  packet.push_back(0);
  return packet;
}

Bytes oggOpus() {
  Bytes file;
  Bytes first = opusHead();
  Bytes tags = comment({"TITLE=Opus Song"}, true);
  first.insert(first.end(), tags.begin(), tags.end());
  page(file, 0, 0, 0, first);
  page(file, 4, 120000, 1, Bytes{'x'});
  return file;
}

Bytes oggSpecOpus() {
  Bytes file;
  page(file, 2, 0, 0, opusHead());
  const std::vector<std::string> entries{"title=Spec Opus", "artist=Spec Artist",
                                         "album=Spec Album", pictureComment()};
  page(file, 0, 0, 1, comment(entries, true));
  page(file, 4, 144000, 2, Bytes{'x'});
  return file;
}

void wav_format_resolves_classic_pcm() {
  MemoryReader reader;
  reader.data = wavWithFmt(wavFmt(1, 2, 44100, 16));
  pm_media::WavFormat format{};
  assert(pm_media::wavFormat(reader, format));
  assert(format.codec == 1 && format.channels == 2 && format.bitsPerSample == 16);
  assert(format.sampleRate == 44100 && format.blockAlign == 4);
  assert(format.dataOffset == 44 && format.dataBytes == 176400);
}

void wav_format_resolves_extensible_pcm_and_float() {
  MemoryReader reader;
  reader.data = wavWithFmt(wavExtensibleFmt(1, 2, 48000, 16));
  pm_media::WavFormat format{};
  assert(pm_media::wavFormat(reader, format));
  assert(format.codec == 1 && format.channels == 2 && format.bitsPerSample == 16);
  assert(format.sampleRate == 48000 && format.blockAlign == 4);
  assert(format.dataOffset == 68);

  reader.data = wavWithFmt(wavExtensibleFmt(3, 2, 48000, 32), 384000);
  assert(pm_media::wavFormat(reader, format));
  assert(format.codec == 3 && format.bitsPerSample == 32 && format.blockAlign == 8);
  assert(format.dataBytes == 384000);

  MediaInfo info{};
  assert(pm_media::wavInfo(reader, info));
  assert(info.durationMs == 1000);
}

void wav_format_rejects_unknown_extensible_and_short_fmt() {
  MemoryReader reader;
  pm_media::WavFormat format{};
  reader.data = wavWithFmt(wavExtensibleFmt(6, 2, 48000, 16));
  assert(!pm_media::wavFormat(reader, format));

  Bytes shortFmt = wavFmt(0xFFFE, 2, 48000, 16);
  le16(shortFmt, 22);
  reader.data = wavWithFmt(shortFmt);
  assert(!pm_media::wavFormat(reader, format));
}

void wav_probe_reads_tags_duration_and_rebased_cover() {
  MemoryReader reader;
  reader.data = wavFile();
  MediaInfo info{};
  assert(pm_media::wavInfo(reader, info));
  assert(std::strcmp(info.title, "Wave Song") == 0);
  assert(std::strcmp(info.artist, "Wave Artist") == 0);
  assert(std::strcmp(info.album, "Wave Album") == 0);
  assert(info.durationMs == 1000 && info.coverLength == 0);

  reader.data = wavWithId3();
  info = {};
  assert(pm_media::wavInfo(reader, info));
  assert(std::strcmp(info.title, "Wave Song") == 0);
  assert(info.coverOffset && info.coverLength == 4);
  assert(info.coverFormat == ImageFormat::PNG);
  const Bytes expected{0x89, 'P', 'N', 'G'};
  assert(Bytes(reader.data.begin() + info.coverOffset,
               reader.data.begin() + info.coverOffset + info.coverLength) == expected);
}

void ogg_vorbis_probe_streams_tags_duration_and_art() {
  MemoryReader reader;
  reader.data = oggVorbis();
  MediaInfo info{};
  assert(pm_media::oggInfo(reader, info));
  assert(std::strcmp(info.title, "Ogg Song") == 0);
  assert(std::strcmp(info.artist, "Ogg Artist") == 0);
  assert(std::strcmp(info.album, "Ogg Album") == 0);
  assert(info.durationMs == 1000);
  assert(info.coverFormat == ImageFormat::JPEG && info.coverLength == 8);
  const Bytes image = decodeBase64(reader.data.data() + info.coverOffset, info.coverLength);
  assert(image == Bytes({0xFF, 0xD8, 0xFF, 0xD9}));
}

void ogg_vorbis_probe_streams_large_art() {
  MemoryReader reader;
  reader.data = oggVorbisLargeArt();
  MediaInfo info{};
  assert(pm_media::oggInfo(reader, info));
  assert(info.coverFormat == ImageFormat::JPEG);
  assert(info.coverOffset && info.coverLength > 4096);
  const Bytes image = decodeBase64(reader.data.data() + info.coverOffset, info.coverLength);
  assert(image.size() == 4100 && image.front() == 0xFF && image.back() == 0xD9);
}

void ogg_opus_probe_uses_48k_granule() {
  MemoryReader reader;
  reader.data = oggOpus();
  MediaInfo info{};
  assert(pm_media::oggInfo(reader, info));
  assert(std::strcmp(info.title, "Opus Song") == 0);
  assert(info.durationMs == 2500);
}

void ogg_opus_probe_reads_spec_comments_and_picture() {
  MemoryReader reader;
  reader.data = oggSpecOpus();
  MediaInfo info{};
  assert(pm_media::oggInfo(reader, info));
  assert(std::strcmp(info.title, "Spec Opus") == 0);
  assert(std::strcmp(info.artist, "Spec Artist") == 0);
  assert(std::strcmp(info.album, "Spec Album") == 0);
  assert(info.durationMs == 3000);
  assert(info.coverFormat == ImageFormat::JPEG && info.coverLength == 8);
  const Bytes image = decodeBase64(reader.data.data() + info.coverOffset, info.coverLength);
  assert(image == Bytes({0xFF, 0xD8, 0xFF, 0xD9}));
}

struct SpecFile {
  const char *name;
  const char *title;
  const char *artist;
  const char *album;
  uint64_t durationMs;
  bool wav;
};

void jpeg_cover_is_streamable(const Bytes &image) {
  size_t start = 0;
  while (start + 1 < image.size() && !(image[start] == 0xFF && image[start + 1] == 0xD8))
    ++start;
  assert(start + 1 < image.size());
  assert(image.size() - start >= 4);
  assert(image[image.size() - 2] == 0xFF && image.back() == 0xD9);
}

void spec_audio_files_probe_full_metadata(const std::string &directory) {
  const SpecFile files[] = {
      {"spec.wav", "Wave Title", "Wave Artist", "Wave Album", 3000, true},
      {"spec.ogg", "Vorbis Title", "Vorbis Artist", "Vorbis Album", 3001, false},
      {"spec.opus", "Opus Title", "Opus Artist", "Opus Album", 3006, false},
  };
  for (const SpecFile &file : files) {
    MemoryReader reader;
    reader.data = loadFixture(directory, file.name);
    MediaInfo info{};
    assert(file.wav ? pm_media::wavInfo(reader, info) : pm_media::oggInfo(reader, info));
    assert(std::strcmp(info.title, file.title) == 0);
    assert(std::strcmp(info.artist, file.artist) == 0);
    assert(std::strcmp(info.album, file.album) == 0);
    assert(info.durationMs == file.durationMs);
    assert(info.coverFormat == ImageFormat::JPEG && info.coverLength);
    assert(info.coverOffset < reader.data.size());
    assert(info.coverLength <= reader.data.size() - info.coverOffset);
    const uint8_t *region = reader.data.data() + info.coverOffset;
    const Bytes image = file.wav ? Bytes(region, region + info.coverLength)
                                 : decodeBase64(region, info.coverLength);
    jpeg_cover_is_streamable(image);
  }
}

void malformed_formats_are_rejected() {
  MemoryReader reader;
  MediaInfo info{};
  reader.data = Bytes{'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E'};
  assert(!pm_media::wavInfo(reader, info) && info.durationMs == 0);
  reader.data = Bytes{'O', 'g', 'g', 'S', 0};
  assert(!pm_media::oggInfo(reader, info) && info.durationMs == 0);
  reader.data = Bytes(64, 0);
  assert(!pm_media::wavInfo(reader, info) && !pm_media::oggInfo(reader, info));
}

int main(int argc, char **argv) {
  assert(argc == 2);
  wav_format_resolves_classic_pcm();
  wav_format_resolves_extensible_pcm_and_float();
  wav_format_rejects_unknown_extensible_and_short_fmt();
  wav_probe_reads_tags_duration_and_rebased_cover();
  ogg_vorbis_probe_streams_tags_duration_and_art();
  ogg_vorbis_probe_streams_large_art();
  ogg_opus_probe_uses_48k_granule();
  ogg_opus_probe_reads_spec_comments_and_picture();
  spec_audio_files_probe_full_metadata(argv[1]);
  malformed_formats_are_rejected();
}
