#pragma once

#include <cstdint>

struct JPEGFILE {
  void *fHandle = nullptr;
  int32_t iSize = 0;
};

typedef void *(JPEG_OPEN_CALLBACK)(const char *szFilename, int32_t *pFileSize);
typedef void(JPEG_CLOSE_CALLBACK)(void *pHandle);
typedef int32_t(JPEG_READ_CALLBACK)(JPEGFILE *pFile, uint8_t *pBuf, int32_t iLen);
typedef int32_t(JPEG_SEEK_CALLBACK)(JPEGFILE *pFile, int32_t iPosition);

struct JPEGDRAW;
typedef int(JPEG_DRAW_CALLBACK)(JPEGDRAW *pDraw);

struct JPEGDRAW {
  void *pUser = nullptr;
  int x = 0, y = 0, iWidth = 0, iWidthUsed = 0, iHeight = 0;
  uint8_t *pPixels = nullptr;
};

static constexpr int EIGHT_BIT_GRAYSCALE = 1;
static constexpr int JPEG_SCALE_HALF = 2;
static constexpr int JPEG_SCALE_QUARTER = 3;
static constexpr int JPEG_SCALE_EIGHTH = 4;
static constexpr int JPEG_LUMA_ONLY = 8;
static constexpr int JPEG_MODE_PROGRESSIVE = 2;

inline bool gJpegOpenFails = false;

class JPEGDEC {
public:
  int open(const char *name, JPEG_OPEN_CALLBACK *pfnOpen, JPEG_CLOSE_CALLBACK *pfnClose,
           JPEG_READ_CALLBACK *, JPEG_SEEK_CALLBACK *, JPEG_DRAW_CALLBACK *) {
    int32_t size = 0;
    handle_ = pfnOpen(name, &size);
    if (!handle_)
      return 0;
    closeCb_ = pfnClose;
    return gJpegOpenFails ? 0 : 1;
  }

  void close() {
    if (handle_ && closeCb_)
      closeCb_(handle_);
    handle_ = nullptr;
  }

  int getWidth() const { return 8; }
  int getHeight() const { return 8; }
  int getJPEGType() const { return 0; }
  void setPixelType(int) {}
  void setUserPointer(void *) {}
  int decode(int, int, int) { return 0; }

private:
  void *handle_ = nullptr;
  JPEG_CLOSE_CALLBACK *closeCb_ = nullptr;
};
