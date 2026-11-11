// C type/semantic adapters. Native SDK services own shared behavior and validation.
#include <globals.h>
#include <pm_app_api.h>
#include <pocketmage_image/pocketmage_image.h>

#include <climits>
#include <cstdio>

namespace {

// Validate before narrowing C integers to the SDK's byte-sized enum types.
bool target_valid(int target) {
  return target >= PM_TARGET_OLED && target <= PM_TARGET_EINK;
}

bool style_valid(int style) {
  return style >= PM_STYLE_TINY && style < PM_STYLE_COUNT;
}

}  // namespace

void pm_text(int target, int x, int y, const char* text, int style) {
  if (!target_valid(target) || !style_valid(style))
    return;
  FontEngine::drawText(static_cast<DisplayTarget>(target), x, y, text,
                       static_cast<FontStyle>(style));
}

int pm_text_width(int target, const char* text, int style) {
  if (!target_valid(target) || !style_valid(style))
    return 0;
  return FontEngine::textWidth(static_cast<DisplayTarget>(target), text,
                               static_cast<FontStyle>(style));
}

int pm_font_height(int target, int style) {
  if (!target_valid(target) || !style_valid(style))
    return 0;
  return FontEngine::fontHeight(static_cast<DisplayTarget>(target), static_cast<FontStyle>(style));
}

void pm_text_color(int target, uint16_t color) {
  if (!target_valid(target))
    return;
  FontEngine::setTextColor(static_cast<DisplayTarget>(target), color ? GxEPD_BLACK : GxEPD_WHITE);
}

bool pm_image_decode_to_1bit(const char* path, uint8_t* buf, int buf_w, int buf_h,
                             int out_x, int out_y, int max_size) {
  return P_IMAGE.decodeTo1Bit(path, buf, buf_w, buf_h, out_x, out_y, max_size, false);
}

bool pm_image_probe(const char* path, int* width, int* height, int* format) {
  if (!width || !height || !format)
    return false;
  ImageInfo info;
  if (!P_IMAGE.probe(path, info))
    return false;
  *width = info.width;
  *height = info.height;
  *format = static_cast<int>(info.format);
  return true;
}

int pm_sd_stream_open(const char* path) {
  return PM_SD().streamOpen(path, 'r');
}

bool pm_sd_stream_seek(int slot, uint32_t offset) {
  if (offset > static_cast<unsigned long>(LONG_MAX))
    return false;
  return PM_SD().streamSeek(slot, static_cast<long>(offset), SEEK_SET);
}

void pm_sd_stream_close(int slot) {
  if (!PM_SD().streamClose(slot))
    ESP_LOGW("ELF", "legacy stream close failed for slot %d", slot);
}
