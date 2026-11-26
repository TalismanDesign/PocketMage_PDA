#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

using Bytes = std::vector<uint8_t>;

class RegionSD {
public:
  std::map<std::string, Bytes> files;
  Bytes opened;
  size_t position = 0;
  int opens = 0;
  int closes = 0;

  int streamOpen(const char *path, char mode) {
    if (mode != 'r' || !files.count(path))
      return -1;
    opened = files.at(path);
    position = 0;
    ++opens;
    return 0;
  }

  size_t streamSize(int) const { return opened.size(); }

  bool streamSeek(int, long offset, int whence) {
    if (whence != 0 || offset < 0 || static_cast<size_t>(offset) > opened.size())
      return false;
    position = static_cast<size_t>(offset);
    return true;
  }

  size_t streamRead(int, void *buffer, size_t length) {
    length = std::min(length, opened.size() - position);
    std::memcpy(buffer, opened.data() + position, length);
    position += length;
    return length;
  }

  bool streamClose(int) {
    opened.clear();
    position = 0;
    ++closes;
    return true;
  }
};

inline RegionSD &PM_SD() {
  static RegionSD sd;
  return sd;
}
