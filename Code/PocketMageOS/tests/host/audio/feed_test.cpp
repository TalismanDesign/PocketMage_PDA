#include <pocketmage_audio/audio_feed.h>

#include <cassert>

int main() {
  constexpr uint32_t capacity = 4096;

  auto full = pm_audio::updateFeed(capacity, 0, false, (int)capacity);
  assert(full.avail == capacity);
  assert(!full.eos);
  assert(!full.storageEof);

  auto last = pm_audio::updateFeed(capacity, 0, false, (int)capacity / 2);
  assert(last.avail == capacity / 2);
  assert(last.eos);
  assert(last.storageEof);

  auto buffered = pm_audio::updateFeed(capacity, 512, true, (int)(capacity - 512));
  assert(buffered.avail == capacity);
  assert(!buffered.eos);
  assert(buffered.storageEof);

  auto drained = pm_audio::updateFeed(capacity, capacity, true, 0);
  assert(drained.avail == capacity);
  assert(drained.eos);

  assert(pm_audio::feedRequest(capacity, 0) == capacity);
  assert(pm_audio::feedRequest(capacity, capacity) == 0);

  assert(!pm_audio::parserStalled(capacity, capacity, false, 0, 0));
  assert(!pm_audio::parserStalled(capacity, capacity, true, 1, 0));
  assert(pm_audio::parserStalled(capacity, capacity, true, 0, 0));
  assert(!pm_audio::parserStalled(capacity, capacity - 1, true, 0, 0));
  assert(!pm_audio::parserStalled(capacity, capacity, true, 0, 512));

  constexpr uint32_t idleLimit = 3;
  constexpr uint32_t drainLimit = 3;

  pm_audio::ParserRetryState idle;
  assert(idle.shouldRetry(0, 0, idleLimit, drainLimit));
  assert(idle.shouldRetry(0, 0, idleLimit, drainLimit));
  assert(idle.shouldRetry(0, 128, idleLimit, drainLimit));
  assert(idle.shouldRetry(0, 0, idleLimit, drainLimit));
  assert(idle.shouldRetry(0, 0, idleLimit, drainLimit));
  assert(!idle.shouldRetry(0, 0, idleLimit, drainLimit));

  pm_audio::ParserRetryState drain;
  assert(drain.shouldRetry(0, 128, idleLimit, drainLimit));
  assert(drain.shouldRetry(0, 128, idleLimit, drainLimit));
  assert(!drain.shouldRetry(0, 128, idleLimit, drainLimit));

  pm_audio::ParserRetryState progress;
  assert(progress.shouldRetry(0, 0, idleLimit, drainLimit));
  assert(progress.shouldRetry(0, 128, idleLimit, drainLimit));
  assert(progress.shouldRetry(64, 0, idleLimit, drainLimit));
  assert(progress.shouldRetry(0, 0, idleLimit, drainLimit));
  assert(progress.shouldRetry(0, 0, idleLimit, drainLimit));
  assert(!progress.shouldRetry(0, 0, idleLimit, drainLimit));
  return 0;
}
