#include <pocketmage_audio/audio_pcm.h>

#include <array>
#include <cassert>
#include <cstring>

int main() {
  std::array<int16_t, 5> samples{-32768, -1000, 0, 1000, 32767};
  auto original = samples;
  assert(pm_audio::scalePcm16(samples.data(), sizeof(samples), -16));
  for (auto sample : samples) assert(sample == 0);
  samples = original;
  assert(pm_audio::scalePcm16(samples.data(), sizeof(samples), 0));
  assert(samples == original);
  assert(pm_audio::scalePcm16(samples.data(), sizeof(samples), 16));
  assert((samples == std::array<int16_t, 5>{-32768, -2000, 0, 2000, 32767}));
  assert(!pm_audio::scalePcm16(samples.data(), 3, 0));
  assert(!pm_audio::scalePcm16(nullptr, 2, 0));

  pm_audio::PlaybackClock clock;
  // One second of stereo 44.1 kHz, regardless of encoded bitrate or tags.
  for (int frame = 0; frame < 441; ++frame)
    assert(clock.advance(400, 2, 44100));
  assert(clock.milliseconds() == 1000);
  assert(clock.advance(96000, 1, 48000));
  assert(clock.milliseconds() == 2000);
  assert(!clock.advance(3, 2, 44100));
  assert(!clock.advance(4, 0, 44100));
  assert(!clock.advance(4, 2, 0));
  assert(clock.milliseconds() == 2000);
  clock.reset();
  assert(clock.milliseconds() == 0);
  pm_audio::PlaybackTimeline timeline;
  timeline.request(1);
  assert(timeline.advance(1, 176400, 2, 44100));
  timeline.request(2);
  assert(timeline.advance(1, 176400, 2, 44100));
  assert(timeline.milliseconds() == 0);
  assert(timeline.advance(2, 176400, 2, 44100));
  assert(timeline.milliseconds() == 1000);
}
