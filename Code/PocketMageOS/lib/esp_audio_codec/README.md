# esp_audio_codec

Espressif `esp_audio_codec` (ESP Audio Codec / Simple Decoder) consumed by PocketMageOS as a PlatformIO library.

## Enabled decoders

Default: MP3 + WAV (with the raw PCM decoder for WAV payloads), selected by the `CONFIG_AUDIO_*_SUPPORT` flags in `library.json`. AAC/FLAC are opt-in: add the matching `-DCONFIG_AUDIO_DECODER_AAC_SUPPORT=1` / `-DCONFIG_AUDIO_DECODER_FLAC_SUPPORT=1` to the env `build_flags`. On the no-PSRAM PM_BETA build, registering AAC+FLAC together is tight on internal heap.
