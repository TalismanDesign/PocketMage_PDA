#include <pocketmage_audio/pocketmage_audio.h>
#include <pocketmage_sys/usb_role.h>

#include <iostream>

static constexpr uint32_t kShutdownTimeoutMs = 5000;
static constexpr uint32_t kUsbProbeTimeoutMs = 5000;
#include "usb_role.inc"
#include "audio_usb.inc"

// USB-only scenarios never schedule a decoder; the fake scheduler must not run it.
void PocketMageAudio::audioTaskFunc(void *) { std::abort(); }

static void require(bool value, const char *message) {
  if (!value) { std::cerr << message << '\n'; std::exit(1); }
}

int main(int argc, char **argv) {
  require(argc == 2, "scenario required");
  const std::string scenario = argv[1];
  auto &audio = PocketMageAudio::getInstance();
  if (scenario == "serial-default" || scenario == "piezo") {
    if (scenario == "piezo") {
      audio.setOutput(AudioOutput::Piezo);
      PowerSystem.needBoost = true;
    }
    audio.begin();
    require(!audio.pollUsb(), "serial-only audio must allow keyboard polling");
    require(serialPhy && !hostRunning && hostStarts == 0,
            "opening Piezo or unattached Auto audio must retain USB serial");
    require(audio.stopChecked(), "serial-only audio session must stop cleanly");
    return 0;
  }
  PowerSystem.needBoost = true;
  if (scenario == "keyboard-handoff" || scenario == "keyboard-handoff-retry" ||
      scenario == "piezo-keyboard-usb") {
    if (scenario == "piezo-keyboard-usb") {
      audio.setOutput(AudioOutput::Piezo);
      audio.begin();
    }
    require(pm_usb::claim(pm_usb::Owner::Keyboard), "HID must claim a pre-attached accessory");
    usbPhyBusy = true;
    serialPhy = false;
    pm_keyboard::stopSucceeds = scenario != "keyboard-handoff-retry";
    if (scenario == "piezo-keyboard-usb") {
      require(!audio.pollUsb() && pm_keyboard::stopCalls == 0,
              "Piezo must preserve the keyboard host");
      audio.setOutput(AudioOutput::Usb);
      audio.pollUsb();
    } else {
      audio.begin();
    }
    if (scenario == "keyboard-handoff-retry") {
      require(pm_keyboard::stopCalls == 1 && hostStarts == 0 && !serialPhy &&
                  pm_usb::owner() == pm_usb::Owner::Keyboard,
              "failed HID teardown must retain its live controller");
      pm_keyboard::stopSucceeds = true;
      require(audio.pollUsb(), "incomplete HID handoff must be retried without replugging");
    }
    require(hostRunning && pm_usb::owner() == pm_usb::Owner::Audio,
            "audio must enumerate an accessory already held by the keyboard host");
    require(pm_keyboard::stopCalls > 0, "audio must use checked HID teardown before claiming");
    speakerFrames = 1;
    stateCallback(STREAM_CONNECTED, stateContext);
    require(audio.stopChecked() && serialPhy, "handoff session must restore serial on exit");
    return 0;
  }
  if (scenario == "settings" || scenario == "settings-failure") {
    require(audio.loadSettings(), "an empty store must load audio defaults");
    audio.setVolumeLevel(0);
    require(audio.getVolumeLevel() == 0 && audio.getVolume() == -16,
            "zero volume must mute the shared sink gain");
    audio.setOutput(AudioOutput::Usb);
    require(audio.saveSettings(), "audio preferences must persist");
    audio.setVolumeLevel(255);
    require(audio.getVolume() == 16, "maximum level must preserve the legacy gain range");
    audio.setOutput(AudioOutput::Piezo);
    require(audio.loadSettings(), "saved audio settings must load");
    require(audio.getVolumeLevel() == 0 && audio.getOutput() == AudioOutput::Usb,
            "volume and output must be shared across sessions");
    if (scenario == "settings-failure") {
      settingsWritable = false;
      require(!audio.saveSettings(), "NVS failures must reach the caller");
      savedAudioSettings = {1, 33, 1};
      require(!audio.loadSettings(), "invalid saved gain must be rejected");
      savedAudioSettings = {1, 16, 99};
      require(!audio.loadSettings(), "invalid saved output must be rejected");
      require(audio.getVolumeLevel() == 0 && audio.getOutput() == AudioOutput::Usb,
              "invalid settings must not partially change live audio");
    }
    require(serialPhy && hostStarts == 0, "settings must not claim USB");
    return 0;
  }
  if (scenario == "auto-output") {
    // ABI 2 reserves Piezo=0 and Usb=1; Auto is the additive third policy.
    const auto automatic = static_cast<AudioOutput>(2);
    audio.setOutput(automatic);
    require(audio.getOutput() == automatic, "native audio must own Auto routing");
    audio.setOutput(static_cast<AudioOutput>(99));
    require(audio.getOutput() == automatic, "invalid output must preserve the policy");
    require(serialPhy && hostStarts == 0, "policy selection alone must retain serial");
    return 0;
  }
  if (scenario == "keyboard-busy" || scenario == "storage-busy") {
    const auto owner = scenario == "keyboard-busy" ? pm_usb::Owner::Keyboard : pm_usb::Owner::Storage;
    require(pm_usb::claim(owner), "other stack must reserve the connector");
    usbPhyBusy = true;
    serialPhy = false;
    audio.begin();
    require(hostStarts == 0, "audio must not start over another stack");
    require(audio.stopChecked() && !serialPhy && pm_usb::owner() == owner,
            "audio stop must preserve another stack's PHY");
    usbPhyBusy = false;
    require(pm_usb::release(owner), "stopped stack must release its reservation");
    return 0;
  }
  if (scenario == "live-phy") {
    require(pm_usb::claim(pm_usb::Owner::Audio), "audio must reserve the connector");
    usbPhyBusy = true;
    serialPhy = false;
    require(!pm_usb::release(pm_usb::Owner::Audio) && !serialPhy,
            "release must reject a live PHY even if its caller requests serial");
    usbPhyBusy = false;
    require(pm_usb::release(pm_usb::Owner::Audio) && serialPhy, "release must be retryable");
    return 0;
  }
  if (scenario == "start-failure") startSucceeds = false;
  audio.begin();
  if (scenario == "start-failure") {
    require(!hostRunning && serialPhy, "failed audio startup must retain serial");
    require(!audio.pollUsb() && hostStarts == 1,
            "failed startup must not repeatedly take serial on each poll");
  } else {
    require(hostRunning && !serialPhy, "an explicit audio request must start its host");
  }
  if (scenario == "stop-failure") {
    stopSucceeds = false;
    require(!audio.stopChecked(), "failed host teardown must be reported");
    require(hostRunning && !serialPhy, "failed stop must not reset a live host");
    stopSucceeds = true;
  }
  if (scenario == "power-failure") {
    PowerSystem.works = false;
    require(!audio.stopChecked(), "failed VBUS handoff must be reported");
    require(!hostRunning && !serialPhy && pm_usb::owner() == pm_usb::Owner::Audio,
            "failed serial handoff must retain its reservation for retry");
    PowerSystem.works = true;
  }
  if (scenario == "connected") {
    speakerFrames = 1;
    stateCallback(STREAM_CONNECTED, stateContext);
    nowMillis = kUsbProbeTimeoutMs;
    require(audio.pollUsb() && hostRunning, "a detected speaker must survive the probe deadline");
  }
  if (scenario == "piezo-stop-retry" || scenario == "piezo-power-failure") {
    audio.setOutput(AudioOutput::Piezo);
    if (scenario == "piezo-stop-retry") stopSucceeds = false;
    else PowerSystem.works = false;
    require(audio.pollUsb() && !serialPhy,
            "failed teardown must retain ownership without resetting the live host");
    if (scenario == "piezo-power-failure")
      require(!hostRunning, "Piezo must stop its host even when Type-C status cannot be read");
    stopSucceeds = true;
    PowerSystem.works = true;
    require(!audio.pollUsb() && serialPhy, "successful Piezo teardown must allow keyboard polling");
  }
  if (scenario == "auto-unplug" || scenario == "auto-reconnect" || scenario == "piezo-switch" ||
      scenario == "probe-timeout") {
    if (scenario == "piezo-switch") audio.setOutput(AudioOutput::Piezo);
    else if (scenario == "probe-timeout") nowMillis = kUsbProbeTimeoutMs;
    else PowerSystem.needBoost = false;
    require(!audio.pollUsb(), "a released audio host must allow keyboard polling");
    require(serialPhy && !hostRunning, "unused audio host must restore serial before app exit");
    if (scenario == "auto-reconnect") {
      PowerSystem.needBoost = true;
      require(audio.pollUsb() && hostRunning && hostStarts == 2,
              "Auto must detect a new accessory after restoring serial");
    }
    if (scenario == "probe-timeout") {
      require(!audio.pollUsb() && !hostRunning && hostStarts == 1,
               "an unsupported attached device must not cause repeated serial disconnects");
      require(pm_usb::claim(pm_usb::Owner::Keyboard), "keyboard must claim after the audio probe");
      require(!audio.pollUsb(), "a suppressed audio probe must not block the keyboard owner");
      require(pm_usb::release(pm_usb::Owner::Keyboard), "keyboard must release its reservation");
    }
  }
  require(audio.stopChecked(), "audio stop must succeed");
  require(!hostRunning && serialPhy, "successful audio stop must restore USB serial");
  if (scenario == "restart") {
    audio.begin();
    require(hostRunning && hostStarts == 2, "audio must support another session after serial");
    require(audio.stopChecked() && serialPhy, "second stop must restore serial");
  }
}
