#include <pocketmage_audio/audio_settings_format.h>

#include <cassert>
#include <cstdint>

namespace pas = pm_audio_settings;

namespace {

constexpr uint8_t kMaxOutput = 2;
constexpr uint8_t kUsbOutput = 1;
constexpr uint8_t kSentinel = 7;

pas::Settings sentinel() {
  pas::Settings settings;
  settings.piezoLevel = kSentinel;
  settings.usbLevel = kSentinel;
  settings.output = kSentinel;
  return settings;
}

void current_record_roundtrips() {
  pas::Settings in;
  in.piezoLevel = 200;
  in.usbLevel = 30;
  in.output = kUsbOutput;
  uint8_t data[pas::kBytes];
  pas::encode(in, data);
  assert(data[0] == pas::kVersion);
  pas::Settings out = sentinel();
  assert(pas::decode(data, sizeof(data), kMaxOutput, out));
  assert(out.piezoLevel == in.piezoLevel && out.usbLevel == in.usbLevel &&
         out.output == in.output);
}

void defaults_keep_usb_quieter_than_piezo() {
  const pas::Settings defaults;
  assert(defaults.usbLevel == pas::kDefaultUsbLevel);
  assert(defaults.piezoLevel == pas::kDefaultPiezoLevel);
  assert(unsigned(pas::kDefaultUsbLevel) * 100 / pas::kLevelMaximum == 20);
}

void shared_level_record_migrates_to_piezo() {
  const uint8_t data[] = {pas::kSharedLevelVersion, 210, kUsbOutput};
  pas::Settings out = sentinel();
  assert(pas::decode(data, sizeof(data), kMaxOutput, out));
  assert(out.piezoLevel == 210);
  assert(out.usbLevel == pas::kDefaultUsbLevel);
  assert(out.output == kUsbOutput);
}

void gain_step_record_normalizes_and_caps() {
  const uint8_t half[] = {pas::kGainStepVersion, pas::kGainSteps / 4, 0};
  pas::Settings out = sentinel();
  assert(pas::decode(half, sizeof(half), kMaxOutput, out));
  assert(out.piezoLevel == (pas::kGainSteps / 4 * pas::kLevelMaximum + pas::kGainSteps / 2) /
                               pas::kGainSteps);
  assert(out.usbLevel == pas::kDefaultUsbLevel);
  const uint8_t loud[] = {pas::kGainStepVersion, pas::kGainSteps, 0};
  assert(pas::decode(loud, sizeof(loud), kMaxOutput, out));
  assert(out.piezoLevel == pas::kDefaultPiezoLevel);
}

void assert_rejected(const uint8_t *data, size_t length) {
  pas::Settings out = sentinel();
  assert(!pas::decode(data, length, kMaxOutput, out));
  assert(out.piezoLevel == kSentinel && out.usbLevel == kSentinel && out.output == kSentinel);
}

void malformed_records_leave_output_unchanged() {
  assert_rejected(nullptr, pas::kBytes);
  const uint8_t empty[] = {pas::kVersion};
  assert_rejected(empty, 0);
  const uint8_t unknown[] = {9, 1, 2, 0};
  assert_rejected(unknown, sizeof(unknown));
  const uint8_t shortCurrent[] = {pas::kVersion, 1, 2};
  assert_rejected(shortCurrent, sizeof(shortCurrent));
  const uint8_t longLegacy[] = {pas::kSharedLevelVersion, 1, 0, 0};
  assert_rejected(longLegacy, sizeof(longLegacy));
  const uint8_t badOutput[] = {pas::kVersion, 1, 2, kMaxOutput + 1};
  assert_rejected(badOutput, sizeof(badOutput));
  const uint8_t badLegacyOutput[] = {pas::kSharedLevelVersion, 1, kMaxOutput + 1};
  assert_rejected(badLegacyOutput, sizeof(badLegacyOutput));
  const uint8_t badGainStep[] = {pas::kGainStepVersion, pas::kGainSteps + 1, 0};
  assert_rejected(badGainStep, sizeof(badGainStep));
}

} 

int main() {
  current_record_roundtrips();
  defaults_keep_usb_quieter_than_piezo();
  shared_level_record_migrates_to_piezo();
  gain_step_record_normalizes_and_caps();
  malformed_records_leave_output_unchanged();
  return 0;
}
