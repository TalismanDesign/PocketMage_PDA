#include <globals.h>
#include <pm_overlay_api.h>

#include <climits>
#include <cstring>

#if PM_TARGET_HOST

namespace {
constexpr int kOverlayFiles = 4;
struct OverlayFile {
  char path[PocketmageSD::kMaxPathLen]{};
  size_t size = 0;
};
// The overlay loader and its providers run synchronously on the app entry task.
OverlayFile files[kOverlayFiles];

OverlayFile* fileAt(int handle) {
  return handle >= 0 && handle < kOverlayFiles && files[handle].path[0]
             ? &files[handle] : nullptr;
}
}

extern "C" int pm_elfo_open(const char* path) {
  if (!path || !path[0] || strlen(path) >= sizeof(files[0].path))
    return -1;
  size_t size = 0;
  bool directory = false;
  if (!PM_SD().statPath(path, &size, &directory) || directory || size > LONG_MAX)
    return -1;
  for (int handle = 0; handle < kOverlayFiles; ++handle) {
    if (!files[handle].path[0]) {
      strcpy(files[handle].path, path);
      files[handle].size = size;
      return handle;
    }
  }
  return -1;
}

extern "C" int pm_elfo_read(int handle, size_t offset, void* buf, size_t len) {
  const OverlayFile* file = fileAt(handle);
  if (!file || (!buf && len) || offset > file->size || len > file->size - offset)
    return -1;
  // A dormant overlay must not keep SD activity or the idle-sleep inhibit asserted.
  const int stream = PM_SD().streamOpen(file->path, 'r');
  if (stream < 0)
    return -1;
  const bool read = PM_SD().streamSeek(stream, static_cast<long>(offset), SEEK_SET) &&
                    PM_SD().streamRead(stream, buf, len) == len;
  const bool closed = PM_SD().streamClose(stream);
  return read && closed ? 0 : -1;
}

extern "C" size_t pm_elfo_size(int handle) {
  const OverlayFile* file = fileAt(handle);
  return file ? file->size : 0;
}

extern "C" void pm_elfo_close(int handle) {
  OverlayFile* file = fileAt(handle);
  if (file)
    *file = {};
}

#endif
