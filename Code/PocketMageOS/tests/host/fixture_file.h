#pragma once

#include <cassert>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

inline std::vector<uint8_t> loadFixture(const std::string &directory, const char *name) {
  std::ifstream file(directory + "/" + name, std::ios::binary);
  assert(file.is_open());
  return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                              std::istreambuf_iterator<char>());
}
