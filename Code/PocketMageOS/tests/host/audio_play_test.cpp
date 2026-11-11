#include <pocketmage_audio/pocketmage_audio.h>
#include <pocketmage_sys/usb_role.h>

#include <iostream>

static constexpr uint32_t kShutdownTimeoutMs = 5000;
static constexpr uint32_t kUsbProbeTimeoutMs = 5000;
static constexpr int ESP_AUDIO_ERR_OK = 0;
using esp_audio_err_t = int;
static bool s_decodersRegistered = false;
static int esp_audio_dec_register_default() { return ESP_AUDIO_ERR_OK; }
static int esp_audio_simple_dec_register_default() { return ESP_AUDIO_ERR_OK; }
#include "usb_role.inc"
#include "audio_usb.inc"
#include "audio_play.inc"

static unsigned decodedClips = 0;
static std::function<void()> duringDecode;
static constexpr uint32_t kSampleRate = 48000;
static constexpr size_t kOneSecondMonoPcmBytes = kSampleRate * sizeof(int16_t);

// The scheduler and codec are hardware boundaries. Request validation, decoder
// admission, completion tracking, and lifecycle methods are production code.
void PocketMageAudio::audioTaskFunc(void *context) {
  static_cast<PocketMageAudio *>(context)->doPlay();
}
void PocketMageAudio::releasePin() { std::abort(); }
void PocketMageAudio::decodeLoop(uint32_t playbackId) {
  ++decodedClips;
  assert(_clock.advance(playbackId, kOneSecondMonoPcmBytes, 1, kSampleRate));
  if (duringDecode) duringDecode();
  assert(_clock.advance(playbackId, kOneSecondMonoPcmBytes, 1, kSampleRate));
  _playback.finish(true);
}

static void require(bool value, const char *message) {
  if (!value) { std::cerr << message << '\n'; std::exit(1); }
}

static void runDecoder() {
  require(scheduledTask != nullptr, "play must schedule the decoder");
  scheduledTask(scheduledContext);
}

int main(int argc, char **argv) {
  require(argc == 2, "scenario required");
  const std::string scenario = argv[1];
  auto &audio = PocketMageAudio::getInstance();
  audio.play("/music/first.mp3");
  const uint32_t previous = audio.getPlaybackId();
  require(previous != 0 && audio.getPlaybackResult(previous) == PlaybackSession::Pending,
          "accepted play must publish a pending request");
  if (scenario != "queued") {
    runDecoder();
    require(audio.getPlaybackResult(previous) == PlaybackSession::Ended,
            "the first clip must finish naturally");
    require(audio.getPositionMs() == 2000, "completed clip must retain its PCM duration");
  }

  const char *path = "/music/next.mp3";
  const std::string longPath(256, 'x');
  if (scenario == "null" || scenario == "queued") path = nullptr;
  if (scenario == "empty") path = "";
  if (scenario == "long") path = longPath.c_str();
  if (scenario == "task" || scenario == "init" || scenario == "transfer") {
    require(audio.stopChecked(), "completed session must stop");
    if (scenario == "task") taskCreationSucceeds = false;
    if (scenario == "init") queueCreationSucceeds = false;
    if (scenario == "transfer") CurrentAppState = USB_APP;
  }
  if (scenario == "shutdown") {
    shutdownAcknowledged = false;
    require(!audio.stopChecked(), "missing worker acknowledgement must retain shutdown state");
  }
  if (scenario == "queue") queueSendSucceeds = false;
  const unsigned previousCommands = commandsSent;
  if (scenario == "stale") {
    audio.play(path);
    duringDecode = [&audio] { audio.play(nullptr); };
    runDecoder();
    duringDecode = nullptr;
  } else {
    audio.play(path);
  }

  const uint32_t latest = audio.getPlaybackId();
  require(commandsSent == previousCommands + (scenario == "accepted" || scenario == "stale"),
          "rejected requests must not enqueue decoder work");
  require(latest != 0 && latest != previous, "every play attempt must receive a fresh ID");
  require(audio.getPlaybackResult(previous) == PlaybackSession::Cancelled,
          "superseded completion must not remain visible as ENDED");
  require(audio.getPositionMs() == 0, "a new request must not inherit old PCM duration");
  if (scenario == "accepted") {
    require(audio.getPlaybackResult(latest) == PlaybackSession::Pending,
            "accepted replacement must remain pending until decoded");
    runDecoder();
    require(audio.getPlaybackResult(latest) == PlaybackSession::Ended,
            "accepted replacement must still finish naturally");
  } else {
    require(audio.getPlaybackResult(latest) == PlaybackSession::Failed,
            "rejected play must report FAILED, including after stale decoder completion");
    require(!audio.getLastError().empty(), "rejection must explain its failure");
    if (scenario == "queued") {
      runDecoder();
      require(decodedClips == 0, "queued work must not decode a rejected request's old path");
    }
  }
  shutdownAcknowledged = true;
  require(audio.stopChecked(), "failed playback must remain cleanly stoppable");
  if (scenario != "accepted")
    require(audio.getPlaybackResult(latest) == PlaybackSession::Failed,
            "service cleanup must preserve the terminal failure");
}
