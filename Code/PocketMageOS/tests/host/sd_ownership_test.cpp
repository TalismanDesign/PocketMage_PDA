#include <cassert>
#include <cstdio>

using TaskHandle_t = void *;
static constexpr int kStreamSlots = 4;
static FILE *s_streams[kStreamSlots]{};
static TaskHandle_t s_streamOwners[kStreamSlots]{};
static TaskHandle_t s_snapshotOwner = nullptr;
static int s_openStreams = 0, closes = 0, snapshots = 0;
static int closeResult = 0;
static int closeFile(FILE *) { ++closes; return closeResult; }
static void snapshotFree() { ++snapshots; s_snapshotOwner = nullptr; }
#define fclose closeFile
#include "cleanup.inc"
#undef fclose

int main() {
  int app, audio;
  // Opaque handles are only compared by the fake close boundary, never dereferenced.
  FILE *handle = reinterpret_cast<FILE *>(&app);
  s_streams[0] = s_streams[1] = s_streams[2] = handle;
  s_streamOwners[0] = s_streamOwners[2] = &app;
  s_streamOwners[1] = &audio;
  s_openStreams = 3;
  s_snapshotOwner = &audio;
  assert(!closeOwnedResources(nullptr));
  closeResult = -1;
  assert(!closeOwnedResources(&app));
  assert(closes == 2 && s_openStreams == 1 && snapshots == 0);
  assert(!s_streams[0] && !s_streamOwners[0] && !s_streams[2]);
  assert(s_streams[1] == handle && s_streamOwners[1] == &audio);
  assert(closeOwnedResources(&app) && closes == 2);
  closeResult = 0;
  assert(closeOwnedResources(&audio));
  assert(closes == 3 && s_openStreams == 0 && snapshots == 1);
  assert(!closeStream(-1) && !closeStream(kStreamSlots) && !closeStream(0));
}
