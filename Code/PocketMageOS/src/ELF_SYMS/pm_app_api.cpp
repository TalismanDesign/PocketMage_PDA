// Host side of pm_app_api.h. The display primitives and the wrappers that must
// validate their arguments live here; gen_app_api.py owns the rest.

#include <globals.h>

#include <pm_app_api.h>

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

void pm_oled_send(void) { u8g2.sendBuffer(); }

int pm_eink_width(void) { return display.width(); }

int pm_eink_height(void) { return display.height(); }

void pm_eink_clear(void) { display.fillScreen(GxEPD_WHITE); }

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
  return FontEngine::fontHeight(static_cast<DisplayTarget>(target),
                                static_cast<FontStyle>(style));
}

void pm_text_color(int target, uint16_t color) {
  if (!target_valid(target)) {
    return;
  }
  // 1 renders glyphs, 0 leaves them blank.
  FontEngine::setTextColor(static_cast<DisplayTarget>(target), color);
}

void pm_oled_sysmsg(const char* msg, int show_ms) {
  OLED().sysMessage(String(msg), show_ms);
}

void pm_oled_set_power_save(bool on) { OLED().setPowerSave(on); }

bool pm_oled_power_save(void) { return OLED().getPowerSave(); }

void pm_eink_set_fast_full_refresh(bool fast) {
  EINK().setFastFullRefresh(fast);
}

void pm_eink_refresh(void) { EINK().refresh(); }

char pm_kb_read(void) { return KB().updateKeypress(); }

int pm_kb_state(void) { return KB().getKeyboardState(); }

void pm_bz_play_jingle(int which) {
  if (which < 0 || which >= PM_JINGLE_COUNT) return;
  BZ().playJingle(*pm_jingle_table[which]);
}

bool pm_clock_valid(void) { return CLOCK().isValid(); }

int64_t pm_clock_epoch(void) {
  return static_cast<int64_t>(CLOCK().nowDT().unixtime());
}

const char* pm_clock_timestamp(void) {
  // Static rather than pooled
  static char buf[32];
  const DateTime dt = CLOCK().nowDT();
  snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
           static_cast<unsigned>(dt.year()), static_cast<unsigned>(dt.month()),
           static_cast<unsigned>(dt.day()), static_cast<unsigned>(dt.hour()),
           static_cast<unsigned>(dt.minute()), static_cast<unsigned>(dt.second()));
  return buf;
}
