#pragma once

#include <cstdint>

struct PNGFILE {
  void *fHandle = nullptr;
  int32_t iSize = 0;
};

typedef void *(PNG_OPEN_CALLBACK)(const char *szFilename, int32_t *pFileSize);
typedef void(PNG_CLOSE_CALLBACK)(void *pHandle);
typedef int32_t(PNG_READ_CALLBACK)(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen);
typedef int32_t(PNG_SEEK_CALLBACK)(PNGFILE *pFile, int32_t iPosition);
struct PNGDRAW;
typedef int(PNG_DRAW_CALLBACK)(PNGDRAW *pDraw);

struct PNGDRAW {
  void *pUser = nullptr;
  int iWidth = 0;
};

static constexpr int PNG_SUCCESS = 1;
static constexpr int PNG_TOO_BIG = -2;
static constexpr int PNG_RGB565_LITTLE_ENDIAN = 1;

inline bool gPngOpenFails = false;

class PNG {
public:
  int open(const char *name, PNG_OPEN_CALLBACK *pfnOpen, PNG_CLOSE_CALLBACK *pfnClose,
           PNG_READ_CALLBACK *, PNG_SEEK_CALLBACK *, PNG_DRAW_CALLBACK *) {
    int32_t size = 0;
    handle_ = pfnOpen(name, &size);
    if (!handle_)
      return 0;
    closeCb_ = pfnClose;
    return gPngOpenFails ? PNG_TOO_BIG : PNG_SUCCESS;
  }

  void close() {
    if (handle_ && closeCb_)
      closeCb_(handle_);
    handle_ = nullptr;
  }

  int decode(void *, int) { return 0; }
  int isInterlaced() const { return 0; }
  int getWidth() const { return 8; }
  int getHeight() const { return 8; }
  void getLineAsRGB565(PNGDRAW *, uint16_t *, int, uint32_t) {}

private:
  void *handle_ = nullptr;
  PNG_CLOSE_CALLBACK *closeCb_ = nullptr;
};
