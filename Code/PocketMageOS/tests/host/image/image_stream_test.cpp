#include "region_hardware.h"

#include <pocketmage_image/jpeg.h>
#include <pocketmage_image/png.h>

#include <cassert>
#include <cstdarg>
#include <vector>

extern "C" void pm_log_write(int, const char *, const char *, ...) {}

int main() {
  PM_SD().files["art"] = Bytes{'x', 'x', 'x', 'x'};
  uint8_t framebuffer[16 * 16] = {};

  gJpegOpenFails = true;
  int opens = PM_SD().opens;
  int closes = PM_SD().closes;
  assert(!JpegDecoder().decodeTo1Bit("art", 0, 4, framebuffer, 16, 16, 0, 0, 16, false));
  assert(PM_SD().opens - opens == 1);
  assert(PM_SD().closes - closes == 1);

  gPngOpenFails = true;
  opens = PM_SD().opens;
  closes = PM_SD().closes;
  assert(!PngDecoder().decodeTo1Bit("art", 0, 4, framebuffer, 16, 16, 0, 0, 16, false));
  assert(PM_SD().opens - opens == 1);
  assert(PM_SD().closes - closes == 1);
  return 0;
}
