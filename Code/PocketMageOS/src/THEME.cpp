#include <globals.h>

// 3rd party themes: 1-bit bitmaps in /assets/theme/ replace the built-in images.
// A file is used only when its size is exactly the bitmap size; anything else
// (missing, wrong size, unreadable) draws the built-in image. See Docs/docs/features/index.md.
static constexpr const char* THEME_DIR = "/assets/theme/";
constexpr int THEME_BG_W = 320;  // app background size
constexpr int THEME_BG_H = 218;

void drawThemeBitmap(const String& name, int x, int y, const unsigned char* fallback, int w, int h) {
  const size_t len = (size_t)((w + 7) / 8) * h;
  uint8_t* buf = nullptr;

  if (global_fs) {
    String path = String(THEME_DIR) + name + ".bin";
    if (global_fs->exists(path)) {
      File f = global_fs->open(path, "r");
      if (f) {
        if (!f.isDirectory() && f.size() == len) {
          buf = (uint8_t*)malloc(len);
          if (buf && f.read(buf, len) != len) {
            free(buf);
            buf = nullptr;
          }
        }
        f.close();
      }
    }
  }

  display.drawBitmap(x, y, buf ? buf : fallback, w, h, GxEPD_BLACK);
  free(buf);
}

void drawThemeBackground(const String& name, const unsigned char* fallback) {
  drawThemeBitmap(name, 0, 0, fallback, THEME_BG_W, THEME_BG_H);
}
