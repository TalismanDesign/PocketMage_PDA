#include <pocketmage_audio/audio_pcm.h>

#include <array>
#include <cassert>
#include <cmath>
#include <cstring>

int main() {
  const uint8_t unity = 255;
  std::array<int16_t, 5> samples{-32768, -1000, 0, 1000, 32767};
  auto original = samples;
  assert(pm_audio::scalePcm16(samples.data(), sizeof(samples), 0));
  for (auto sample : samples) assert(sample == 0);
  samples = original;
  assert(pm_audio::scalePcm16(samples.data(), sizeof(samples), unity));
  assert(samples == original);
  int previous = 0;
  for (unsigned level = 1; level <= 255; ++level) {
    samples = original;
    assert(pm_audio::scalePcm16(samples.data(), sizeof(samples), static_cast<uint8_t>(level)));
    assert(samples[0] >= original[0] && samples[4] <= original[4]);
    assert(samples[4] >= previous && samples[4] > 0);
    previous = samples[4];
  }
  for (unsigned level : {64U, 128U, 192U, 255U}) {
    std::array<int16_t, 2> step{32767, 32767};
    assert(pm_audio::scalePcm16(&step[0], sizeof(int16_t), static_cast<uint8_t>(level)));
    assert(pm_audio::scalePcm16(&step[1], sizeof(int16_t), static_cast<uint8_t>(level - 16)));
    const double differenceDb = 20.0 * std::log10(double(step[0]) / step[1]);
    assert(differenceDb > 2.8 && differenceDb < 3.3);
  }
  std::array<uint8_t, 5> unaligned{0, 0xff, 0x7f, 0x00, 0x80};
  const auto unalignedOriginal = unaligned;
  assert(pm_audio::scalePcm16(unaligned.data() + 1, 4, unity));
  assert(unaligned == unalignedOriginal);
  assert(!pm_audio::scalePcm16(samples.data(), 3, 0));
  assert(!pm_audio::scalePcm16(nullptr, 2, 0));

  pm_audio::PlaybackClock clock;
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
