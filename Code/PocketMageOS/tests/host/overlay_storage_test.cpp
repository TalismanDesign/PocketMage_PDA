#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstring>

#define PM_TARGET_HOST 1
static bool failSeek = false, failRead = false, failClose = false;
static int openStreams = 0, opens = 0;
class PocketmageSD {
public:
  static constexpr size_t kMaxPathLen = 256;
  bool statPath(const char *, size_t *size, bool *dir) { *size = 32; *dir = false; return true; }
  int streamOpen(const char *, char) { ++opens; ++openStreams; return 0; }
  bool streamSeek(int, long, int) { return !failSeek; }
  size_t streamRead(int, void *, size_t len) { return failRead ? 0 : len; }
  bool streamClose(int) { --openStreams; return !failClose; }
} sd;
static PocketmageSD &PM_SD() { return sd; }
#include "storage.inc"

int main() {
  assert(pm_elfo_open(nullptr) == -1);
  int handle = pm_elfo_open("/apps/music.elfo");
  assert(handle >= 0 && openStreams == 0 && opens == 0);
  assert(pm_elfo_size(handle) == 32);
  char buffer[8];
  assert(pm_elfo_read(handle, 0, buffer, sizeof(buffer)) == 0 && openStreams == 0);
  assert(pm_elfo_read(handle, 30, buffer, sizeof(buffer)) == -1 && opens == 1);
  assert(pm_elfo_read(handle, 0, nullptr, sizeof(buffer)) == -1);
  failSeek = true;
  assert(pm_elfo_read(handle, 0, buffer, sizeof(buffer)) == -1 && openStreams == 0);
  failSeek = false;
  failRead = true;
  assert(pm_elfo_read(handle, 0, buffer, sizeof(buffer)) == -1 && openStreams == 0);
  failRead = false;
  failClose = true;
  assert(pm_elfo_read(handle, 0, buffer, sizeof(buffer)) == -1 && openStreams == 0);
  pm_elfo_close(handle);
  assert(pm_elfo_size(handle) == 0 && pm_elfo_read(handle, 0, buffer, 1) == -1);
}
