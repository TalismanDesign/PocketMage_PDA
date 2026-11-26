#include "region_hardware.h"
#include <pocketmage_image/region_stream.h>

#include <cassert>
#include <string>

namespace {

std::string encode(const Bytes &in) {
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

Bytes decodeAt(const char *path, uint32_t off, uint32_t len) {
  regionSet(off, len);
  RegionStream *rs = regionOpen(path);
  regionClear();
  assert(rs);
  Bytes out;
  uint8_t buffer[7];
  int32_t n;
  while ((n = regionRead(rs, buffer, sizeof(buffer))) > 0)
    out.insert(out.end(), buffer, buffer + n);
  regionClose(rs);
  return out;
}

void base64_region_streams_and_seeks_without_caching() {
  Bytes jpeg{0xFF, 0xD8, 0xFF, 0xE0, 0, 16, 'J', 'F', 'I', 'F', 0, 'a', 0xFF, 0xD9};
  const std::string encoded = encode(jpeg);
  Bytes file(11, 0);
  file.insert(file.end(), encoded.begin(), encoded.end());
  file.insert(file.end(), 4, 0);
  PM_SD().files["/art/b64.jpg"] = file;

  assert(decodeAt("/art/b64.jpg", 11, encoded.size()) == jpeg);

  regionSet(11, encoded.size());
  RegionStream *rs = regionOpen("/art/b64.jpg");
  regionClear();
  assert(rs);
  Bytes forward;
  uint8_t buffer[32] = {};
  int32_t n;
  while ((n = regionRead(rs, buffer, 5)) > 0)
    forward.insert(forward.end(), buffer, buffer + n);
  assert(forward == jpeg);

  for (uint32_t at = 0; at < jpeg.size(); ++at) {
    assert(regionSeek(rs, static_cast<int32_t>(at)));
    assert(regionRead(rs, buffer, 1) == 1 && buffer[0] == jpeg[at]);
  }
  assert(regionSeek(rs, 0));
  assert(regionRead(rs, buffer, static_cast<int32_t>(jpeg.size())) ==
         static_cast<int32_t>(jpeg.size()));
  assert(Bytes(buffer, buffer + jpeg.size()) == jpeg);
  regionClose(rs);
}

void base64_region_skips_header_before_image_magic() {
  const Bytes jpeg{0xFF, 0xD8, 0xFF, 0xD9};
  Bytes payload{0x11, 0x22};
  payload.insert(payload.end(), jpeg.begin(), jpeg.end());
  const std::string encoded = encode(payload);
  Bytes file(5, 0);
  file.insert(file.end(), encoded.begin(), encoded.end());
  PM_SD().files["/art/b64_skip.jpg"] = file;

  assert(decodeAt("/art/b64_skip.jpg", 5, encoded.size()) == jpeg);
  regionSet(5, encoded.size());
  RegionStream *rs = regionOpen("/art/b64_skip.jpg");
  regionClear();
  assert(rs);
  uint8_t buffer[8] = {};
  assert(regionSeek(rs, 1));
  assert(regionRead(rs, buffer, 1) == 1 && buffer[0] == 0xD8);
  assert(regionSeek(rs, 0));
  assert(regionRead(rs, buffer, sizeof(buffer)) == 4);
  assert(Bytes(buffer, buffer + 4) == jpeg);
  regionClose(rs);
}

void raw_region_is_unchanged() {
  Bytes png{0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
  Bytes file(3, 0);
  file.insert(file.end(), png.begin(), png.end());
  file.insert(file.end(), 2, 0);
  PM_SD().files["/art/raw.png"] = file;
  assert(decodeAt("/art/raw.png", 3, png.size()) == png);
}

} 

int main() {
  base64_region_streams_and_seeks_without_caching();
  base64_region_skips_header_before_image_magic();
  raw_region_is_unchanged();
}
