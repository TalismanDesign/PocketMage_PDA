// Host side of pm_app_api.h. The display primitives and the wrappers that must
// validate their arguments live here; gen_app_api.py owns the rest.

#include <globals.h>
#include <pm_app_api.h>
#include <pocketmage_audio/pocketmage_audio.h>
#include <pocketmage_image/pocketmage_image.h>
#include <pocketmage_sd/pocketmage_sd.h>

#include <cstdio>

// Jingle pointers in SDK declaration order, generated into pm_sdk_app.cpp.
// C++ typed, so it stays out of the C-facing header.
extern const Jingle* const pm_jingle_table[];

namespace {

// FontEngine indexes its per-target font tables with these unchecked.
bool target_valid(int target) {
  return target >= PM_TARGET_OLED && target <= PM_TARGET_EINK;
}

bool style_valid(int style) {
  return style >= PM_STYLE_TINY && style < PM_STYLE_COUNT;
}

}  // namespace

void pm_oled_send(void) {
  u8g2.sendBuffer();
}

void pm_oled_clear(void) {
  u8g2.clearBuffer();
}

int pm_eink_width(void) {
  return display.width();
}

int pm_eink_height(void) {
  return display.height();
}

void pm_eink_clear(void) {
  display.fillScreen(GxEPD_WHITE);
}

void pm_eink_pixel(int x, int y, bool ink) {
  display.drawPixel(x, y, ink ? GxEPD_BLACK : GxEPD_WHITE);
}

void pm_eink_rect(int x0, int y0, int x1, int y1, bool fill, bool ink) {
  if (x0 > x1) {
    const int t = x0;
    x0 = x1;
    x1 = t;
  }
  if (y0 > y1) {
    const int t = y0;
    y0 = y1;
    y1 = t;
  }
  const int w = x1 - x0 + 1;
  const int h = y1 - y0 + 1;
  if (w <= 0 || h <= 0) {
    return;
  }
  const uint16_t color = ink ? GxEPD_BLACK : GxEPD_WHITE;
  if (fill) {
    display.fillRect(x0, y0, w, h, color);
  } else {
    display.drawRect(x0, y0, w, h, color);
  }
}

void pm_text(int target, int x, int y, const char* text, int style) {
  if (!target_valid(target) || !style_valid(style)) {
    return;
  }
  FontEngine::drawText(static_cast<DisplayTarget>(target), x, y, text,
                       static_cast<FontStyle>(style));
}

int pm_text_width(int target, const char* text, int style) {
  if (!target_valid(target) || !style_valid(style)) {
    return 0;
  }
  return FontEngine::textWidth(static_cast<DisplayTarget>(target), text,
                               static_cast<FontStyle>(style));
}

int pm_font_height(int target, int style) {
  if (!target_valid(target) || !style_valid(style)) {
    return 0;
  }
  return FontEngine::fontHeight(static_cast<DisplayTarget>(target), static_cast<FontStyle>(style));
}

void pm_text_color(int target, uint16_t color) {
  if (!target_valid(target)) {
    return;
  }
  // color is boolean: 1 renders glyphs in black ink, 0 leaves them blank (white).
  FontEngine::setTextColor(static_cast<DisplayTarget>(target), color ? GxEPD_BLACK : GxEPD_WHITE);
}

void pm_oled_sysmsg(const char* msg, int show_ms) {
  OLED().sysMessage(String(msg), show_ms);
}

void pm_oled_set_power_save(bool on) {
  OLED().setPowerSave(on);
}

bool pm_oled_power_save(void) {
  return OLED().getPowerSave();
}

void pm_eink_set_fast_full_refresh(bool fast) {
  EINK().setFastFullRefresh(fast);
}

void pm_eink_refresh(void) {
  EINK().refresh();
}

char pm_kb_read(void) {
  return KB().updateKeypress();
}

int pm_kb_state(void) {
  return KB().getKeyboardState();
}

void pm_bz_play_jingle(int which) {
  if (which < 0 || which >= PM_JINGLE_COUNT)
    return;
  BZ().playJingle(*pm_jingle_table[which]);
}

bool pm_clock_valid(void) {
  return CLOCK().isValid();
}

int64_t pm_clock_epoch(void) {
  return static_cast<int64_t>(CLOCK().nowDT().unixtime());
}

const char* pm_clock_timestamp(void) {
  // Static rather than pooled
  static char buf[32];
  const DateTime dt = CLOCK().nowDT();
  snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u", static_cast<unsigned>(dt.year()),
           static_cast<unsigned>(dt.month()), static_cast<unsigned>(dt.day()),
           static_cast<unsigned>(dt.hour()), static_cast<unsigned>(dt.minute()),
           static_cast<unsigned>(dt.second()));
  return buf;
}

// SD primitives
bool pm_sd_write_binary_file(const char* path, const uint8_t* buf, size_t len) {
  if (!path || (!buf && len > 0)) {
    return false;
  }
  return PM_SD().writeBinaryFile(path, buf, len);
}

bool pm_sd_append_binary_file(const char* path, const uint8_t* buf, size_t len) {
  if (!path || (!buf && len > 0)) {
    return false;
  }
  return PM_SD().appendBinaryFile(path, buf, len);
}

bool pm_sd_mkdir(const char* path) {
  if (!path || !path[0]) {
    return false;
  }
  return PM_SD().mkdirPath(path);
}

// One enumeration lives at a time; the gateway owns the snapshot and hands
// back leaf names.
int pm_sd_dir_open(const char* path) {
  if (!path || !path[0]) {
    return -1;
  }
  return PM_SD().dirEnumOpen(path);
}

bool pm_sd_dir_entry(int i, char* name_out, size_t name_size, bool* is_dir) {
  return PM_SD().dirEnumEntry(i, name_out, name_size, is_dir);
}

void pm_sd_dir_close(void) {
  PM_SD().dirEnumClose();
}

// Image decode primitives (JPEG via esp_new_jpeg, PNG via PNGdec). Writes a
// Floyd-Steinberg-dithered, nearest-neighbour-scaled 1-bit image into the
// caller's framebuffer at (out_x, out_y) bounded by max_size.
bool pm_image_decode_to_1bit(const char* path, uint8_t* buf, int buf_w, int buf_h, int out_x,
                             int out_y, int max_size) {
  if (!buf || buf_w <= 0 || buf_h <= 0 || max_size <= 0) {
    return false;
  }
  return P_IMAGE.decodeTo1Bit(path, buf, buf_w, buf_h, out_x, out_y, max_size);
}

// Probe an image file. Fills width/height/format (0=none, 1=jpeg, 2=png).
// Returns true on recognized format.
bool pm_image_probe(const char* path, int* width, int* height, int* format) {
  if (!path || !width || !height || !format) {
    return false;
  }
  ImageInfo info;
  if (!P_IMAGE.probe(path, info)) {
    return false;
  }
  *width = info.width;
  *height = info.height;
  *format = static_cast<int>(info.format);
  return true;
}

// Select the audio output sink. output=0 is piezo (PWM on BZ_PIN, default),
// output=1 is USB UAC host speaker. If usb is selected but no device is
// connected, playback falls back to piezo.
void pm_audio_set_output(int output) {
  // 0 = Piezo, 1 = Usb (matches AudioOutput enum order).
  P_AUDIO.setOutput(static_cast<AudioOutput>(output));
}
