#include <pocketmage_audio/ogg_stream.h>

#include <fixture_file.h>

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

using Bytes = std::vector<uint8_t>;

Bytes oggPage(const Bytes &payload) {
  Bytes out{'O', 'g', 'g', 'S', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0};
  out[26] = 1; 
  out.push_back(static_cast<uint8_t>(payload.size()));
  out.insert(out.end(), payload.begin(), payload.end());
  return out;
}

Bytes textBytes(const char *text) {
  return Bytes(text, text + std::strlen(text));
}

void codec_is_identified_from_the_first_page() {
  Bytes vorbis{1, 'v', 'o', 'r', 'b', 'i', 's'};
  vorbis.insert(vorbis.end(), 32, 0);
  Bytes page = oggPage(vorbis);
  assert(std::string(pm_audio::oggCodec(page.data(), page.size())) == "vorbis");

  Bytes opus = textBytes("OpusHead");
  opus.insert(opus.end(), 12, 0);
  page = oggPage(opus);
  assert(std::string(pm_audio::oggCodec(page.data(), page.size())) == "opus");

  Bytes flac{0x7F, 'F', 'L', 'A', 'C'};
  page = oggPage(flac);
  assert(std::string(pm_audio::oggCodec(page.data(), page.size())) == "flac");
}

void codec_rejects_non_ogg_and_truncated_pages() {
  Bytes noise(64, 0);
  assert(std::string(pm_audio::oggCodec(noise.data(), noise.size())) == "not-ogg");
  assert(std::string(pm_audio::oggCodec(nullptr, 0)) == "not-ogg");

  Bytes page = oggPage(textBytes("unknown-codec"));
  assert(std::string(pm_audio::oggCodec(page.data(), page.size())) == "unknown");

  Bytes shortPage{'O', 'g', 'g', 'S'};
  assert(std::string(pm_audio::oggCodec(shortPage.data(), shortPage.size())) == "not-ogg");

  page[26] = 100;
  assert(std::string(pm_audio::oggCodec(page.data(), page.size())) == "truncated");
}

void spec_files_identify_their_codecs(const std::string &directory) {
  struct Case {
    const char *name;
    const char *codec;
  };
  const Case cases[] = {{"spec.ogg", "vorbis"}, {"spec.opus", "opus"}};
  for (const Case &item : cases) {
    Bytes file = loadFixture(directory, item.name);
    assert(std::string(pm_audio::oggCodec(file.data(), file.size())) == item.codec);
  }
}

int main(int argc, char **argv) {
  assert(argc == 2);
  codec_is_identified_from_the_first_page();
  codec_rejects_non_ogg_and_truncated_pages();
  spec_files_identify_their_codecs(argv[1]);
}
