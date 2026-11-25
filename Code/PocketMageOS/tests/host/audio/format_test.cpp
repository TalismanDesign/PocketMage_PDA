#include <pocketmage_audio/audio_format.h>

#include <cassert>
#include <string>

using pm_audio::AudioFormat;

int main() {
  assert(AudioFormat::None == pm_audio::formatFromPath(nullptr));
  assert(AudioFormat::None == pm_audio::formatFromPath(""));
  assert(AudioFormat::None == pm_audio::formatFromPath("/music/track"));
  assert(AudioFormat::None == pm_audio::formatFromPath("/music/track.aac"));
  assert(AudioFormat::None == pm_audio::formatFromPath("/music/track.m4a"));
  assert(AudioFormat::None == pm_audio::formatFromPath("/music/track.flac"));
  assert(AudioFormat::None == pm_audio::formatFromPath("/music/track.ts"));
  assert(AudioFormat::None == pm_audio::formatFromPath("/music/track.txt"));
  assert(AudioFormat::Mp3 == pm_audio::formatFromPath("/music/track.mp3"));
  assert(AudioFormat::Mp3 == pm_audio::formatFromPath("/music/TRACK.MP3"));
  assert(AudioFormat::Wav == pm_audio::formatFromPath("/music/track.wav"));
  assert(AudioFormat::Wav == pm_audio::formatFromPath("/music/track.WaV"));
  assert(AudioFormat::Ogg == pm_audio::formatFromPath("/music/track.ogg"));
  assert(AudioFormat::Ogg == pm_audio::formatFromPath("/music/track.OGG"));
  assert(AudioFormat::Ogg == pm_audio::formatFromPath("/music/track.opus"));
  assert(AudioFormat::Ogg == pm_audio::formatFromPath("/music/track.OPUS"));

  assert(pm_audio::decoderStackBytes(AudioFormat::None) == 0);
  assert(pm_audio::decoderStackBytes(AudioFormat::Mp3) == 8192);
  assert(pm_audio::decoderStackBytes(AudioFormat::Wav) == 6144);
  assert(pm_audio::decoderStackBytes(AudioFormat::Ogg) >= 20 * 1024);
  assert(pm_audio::readChunkBytes(AudioFormat::None) == 0);
  assert(pm_audio::readChunkBytes(AudioFormat::Mp3) == 4096);
  assert(pm_audio::readChunkBytes(AudioFormat::Wav) == 4096);
  assert(pm_audio::readChunkBytes(AudioFormat::Ogg) == 512);
  assert(std::string(pm_audio::formatName(AudioFormat::Mp3)) == "mp3");
  assert(std::string(pm_audio::formatName(AudioFormat::Wav)) == "wav");
  assert(std::string(pm_audio::formatName(AudioFormat::Ogg)) == "ogg");
}
