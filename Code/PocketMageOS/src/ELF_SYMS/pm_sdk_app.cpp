// Host wrappers backing pm_sdk_app.h. Regenerate, do not edit.
// Each wrapper is extern "C", so SDK mangling stays local to this file.

#include <globals.h>
#include <pocketmage_image/pocketmage_image.h>
#include <pocketmage_media/pocketmage_media.h>

#include <pm_sdk_app.h>

// Fail the build if an SDK enumerator moves under a C constant.
static_assert(static_cast<int>(DisplayTarget::OLED) == PM_TARGET_OLED,
              "PM_TARGET_OLED parity: DisplayTarget::OLED");
static_assert(static_cast<int>(DisplayTarget::EINK) == PM_TARGET_EINK,
              "PM_TARGET_EINK parity: DisplayTarget::EINK");
static_assert(static_cast<int>(FontStyle::Tiny) == PM_STYLE_TINY,
              "PM_STYLE_TINY parity: FontStyle::Tiny");
static_assert(static_cast<int>(FontStyle::Body) == PM_STYLE_BODY,
              "PM_STYLE_BODY parity: FontStyle::Body");
static_assert(static_cast<int>(FontStyle::BodyBold) == PM_STYLE_BODY_BOLD,
              "PM_STYLE_BODY_BOLD parity: FontStyle::BodyBold");
static_assert(static_cast<int>(FontStyle::BodyItalic) == PM_STYLE_BODY_ITALIC,
              "PM_STYLE_BODY_ITALIC parity: FontStyle::BodyItalic");
static_assert(static_cast<int>(FontStyle::BodyBoldItalic) == PM_STYLE_BODY_BOLD_ITALIC,
              "PM_STYLE_BODY_BOLD_ITALIC parity: FontStyle::BodyBoldItalic");
static_assert(static_cast<int>(FontStyle::Medium) == PM_STYLE_MEDIUM,
              "PM_STYLE_MEDIUM parity: FontStyle::Medium");
static_assert(static_cast<int>(FontStyle::Small) == PM_STYLE_SMALL,
              "PM_STYLE_SMALL parity: FontStyle::Small");
static_assert(static_cast<int>(FontStyle::BodyNarrow) == PM_STYLE_BODY_NARROW,
              "PM_STYLE_BODY_NARROW parity: FontStyle::BodyNarrow");
static_assert(static_cast<int>(FontStyle::Mono) == PM_STYLE_MONO,
              "PM_STYLE_MONO parity: FontStyle::Mono");
static_assert(static_cast<int>(FontStyle::MonoBold) == PM_STYLE_MONO_BOLD,
              "PM_STYLE_MONO_BOLD parity: FontStyle::MonoBold");
static_assert(static_cast<int>(FontStyle::MonoItalic) == PM_STYLE_MONO_ITALIC,
              "PM_STYLE_MONO_ITALIC parity: FontStyle::MonoItalic");
static_assert(static_cast<int>(FontStyle::MonoBoldItalic) == PM_STYLE_MONO_BOLD_ITALIC,
              "PM_STYLE_MONO_BOLD_ITALIC parity: FontStyle::MonoBoldItalic");
static_assert(static_cast<int>(FontStyle::Sans) == PM_STYLE_SANS,
              "PM_STYLE_SANS parity: FontStyle::Sans");
static_assert(static_cast<int>(FontStyle::SansBold) == PM_STYLE_SANS_BOLD,
              "PM_STYLE_SANS_BOLD parity: FontStyle::SansBold");
static_assert(static_cast<int>(FontStyle::SansItalic) == PM_STYLE_SANS_ITALIC,
              "PM_STYLE_SANS_ITALIC parity: FontStyle::SansItalic");
static_assert(static_cast<int>(FontStyle::SansBoldItalic) == PM_STYLE_SANS_BOLD_ITALIC,
              "PM_STYLE_SANS_BOLD_ITALIC parity: FontStyle::SansBoldItalic");
static_assert(static_cast<int>(FontStyle::Caption) == PM_STYLE_CAPTION,
              "PM_STYLE_CAPTION parity: FontStyle::Caption");
static_assert(static_cast<int>(FontStyle::Heading3) == PM_STYLE_HEADING3,
              "PM_STYLE_HEADING3 parity: FontStyle::Heading3");
static_assert(static_cast<int>(FontStyle::Heading2) == PM_STYLE_HEADING2,
              "PM_STYLE_HEADING2 parity: FontStyle::Heading2");
static_assert(static_cast<int>(FontStyle::Heading1) == PM_STYLE_HEADING1,
              "PM_STYLE_HEADING1 parity: FontStyle::Heading1");
static_assert(static_cast<int>(FontStyle::Large) == PM_STYLE_LARGE,
              "PM_STYLE_LARGE parity: FontStyle::Large");
static_assert(static_cast<int>(FontStyle::OledWord) == PM_STYLE_OLED_WORD,
              "PM_STYLE_OLED_WORD parity: FontStyle::OledWord");
static_assert(static_cast<int>(FontStyle::Terminal) == PM_STYLE_TERMINAL,
              "PM_STYLE_TERMINAL parity: FontStyle::Terminal");
static_assert(static_cast<int>(FontStyle::TerminalBig) == PM_STYLE_TERMINAL_BIG,
              "PM_STYLE_TERMINAL_BIG parity: FontStyle::TerminalBig");
static_assert(static_cast<int>(FontStyle::ClockDigit) == PM_STYLE_CLOCK_DIGIT,
              "PM_STYLE_CLOCK_DIGIT parity: FontStyle::ClockDigit");
static_assert(static_cast<int>(FontStyle::_StyleCount) == PM_STYLE_COUNT,
              "PM_STYLE_COUNT parity: FontStyle::_StyleCount");
static_assert(static_cast<int>(Lang::English) == PM_LANG_ENGLISH,
              "PM_LANG_ENGLISH parity: Lang::English");
static_assert(static_cast<int>(Lang::French) == PM_LANG_FRENCH,
              "PM_LANG_FRENCH parity: Lang::French");
static_assert(static_cast<int>(Lang::Spanish) == PM_LANG_SPANISH,
              "PM_LANG_SPANISH parity: Lang::Spanish");
static_assert(static_cast<int>(Lang::German) == PM_LANG_GERMAN,
              "PM_LANG_GERMAN parity: Lang::German");
static_assert(static_cast<int>(Lang::_LANG_COUNT) == PM_LANG_COUNT,
              "PM_LANG_COUNT parity: Lang::_LANG_COUNT");
static_assert(static_cast<int>(EinkRefresh::Normal) == PM_REFRESH_NORMAL,
              "PM_REFRESH_NORMAL parity: EinkRefresh::Normal");
static_assert(static_cast<int>(EinkRefresh::ForceFull) == PM_REFRESH_FORCE_FULL,
              "PM_REFRESH_FORCE_FULL parity: EinkRefresh::ForceFull");
static_assert(static_cast<int>(AudioState::Idle) == PM_AUDIO_STATE_IDLE,
              "PM_AUDIO_STATE_IDLE parity: AudioState::Idle");
static_assert(static_cast<int>(AudioState::Opening) == PM_AUDIO_STATE_OPENING,
              "PM_AUDIO_STATE_OPENING parity: AudioState::Opening");
static_assert(static_cast<int>(AudioState::Playing) == PM_AUDIO_STATE_PLAYING,
              "PM_AUDIO_STATE_PLAYING parity: AudioState::Playing");
static_assert(static_cast<int>(AudioState::Paused) == PM_AUDIO_STATE_PAUSED,
              "PM_AUDIO_STATE_PAUSED parity: AudioState::Paused");
static_assert(static_cast<int>(AudioState::Stopping) == PM_AUDIO_STATE_STOPPING,
              "PM_AUDIO_STATE_STOPPING parity: AudioState::Stopping");
static_assert(static_cast<int>(AudioOutput::Piezo) == PM_AUDIO_OUTPUT_PIEZO,
              "PM_AUDIO_OUTPUT_PIEZO parity: AudioOutput::Piezo");
static_assert(static_cast<int>(AudioOutput::Usb) == PM_AUDIO_OUTPUT_USB,
              "PM_AUDIO_OUTPUT_USB parity: AudioOutput::Usb");
static_assert(static_cast<int>(AudioOutput::Auto) == PM_AUDIO_OUTPUT_AUTO,
              "PM_AUDIO_OUTPUT_AUTO parity: AudioOutput::Auto");
static_assert(static_cast<int>(PlaybackSession::Result::Pending) == PM_AUDIO_RESULT_PENDING,
              "PM_AUDIO_RESULT_PENDING parity: PlaybackSession::Result::Pending");
static_assert(static_cast<int>(PlaybackSession::Result::Ended) == PM_AUDIO_RESULT_ENDED,
              "PM_AUDIO_RESULT_ENDED parity: PlaybackSession::Result::Ended");
static_assert(static_cast<int>(PlaybackSession::Result::Failed) == PM_AUDIO_RESULT_FAILED,
              "PM_AUDIO_RESULT_FAILED parity: PlaybackSession::Result::Failed");
static_assert(static_cast<int>(PlaybackSession::Result::Cancelled) == PM_AUDIO_RESULT_CANCELLED,
              "PM_AUDIO_RESULT_CANCELLED parity: PlaybackSession::Result::Cancelled");
static_assert(static_cast<int>(ImageFormat::None) == PM_IMAGE_FORMAT_NONE,
              "PM_IMAGE_FORMAT_NONE parity: ImageFormat::None");
static_assert(static_cast<int>(ImageFormat::JPEG) == PM_IMAGE_FORMAT_JPEG,
              "PM_IMAGE_FORMAT_JPEG parity: ImageFormat::JPEG");
static_assert(static_cast<int>(ImageFormat::PNG) == PM_IMAGE_FORMAT_PNG,
              "PM_IMAGE_FORMAT_PNG parity: ImageFormat::PNG");
static_assert(static_cast<int>(KeyCode::KeyNone) == PM_KEY_NONE,
              "PM_KEY_NONE parity: KeyCode::KeyNone");
static_assert(static_cast<int>(KeyCode::KeyFnRight) == PM_KEY_FN_RIGHT,
              "PM_KEY_FN_RIGHT parity: KeyCode::KeyFnRight");
static_assert(static_cast<int>(KeyCode::KeyFnSelect) == PM_KEY_FN_SELECT,
              "PM_KEY_FN_SELECT parity: KeyCode::KeyFnSelect");
static_assert(static_cast<int>(KeyCode::KeyBackspace) == PM_KEY_BACKSPACE,
              "PM_KEY_BACKSPACE parity: KeyCode::KeyBackspace");
static_assert(static_cast<int>(KeyCode::KeyTab) == PM_KEY_TAB,
              "PM_KEY_TAB parity: KeyCode::KeyTab");
static_assert(static_cast<int>(KeyCode::KeyEscape) == PM_KEY_ESCAPE,
              "PM_KEY_ESCAPE parity: KeyCode::KeyEscape");
static_assert(static_cast<int>(KeyCode::KeyEnter) == PM_KEY_ENTER,
              "PM_KEY_ENTER parity: KeyCode::KeyEnter");
static_assert(static_cast<int>(KeyCode::KeyBackTab) == PM_KEY_BACK_TAB,
              "PM_KEY_BACK_TAB parity: KeyCode::KeyBackTab");
static_assert(static_cast<int>(KeyCode::KeyShift) == PM_KEY_SHIFT,
              "PM_KEY_SHIFT parity: KeyCode::KeyShift");
static_assert(static_cast<int>(KeyCode::KeyFn) == PM_KEY_FN,
              "PM_KEY_FN parity: KeyCode::KeyFn");
static_assert(static_cast<int>(KeyCode::KeyLeft) == PM_KEY_LEFT,
              "PM_KEY_LEFT parity: KeyCode::KeyLeft");
static_assert(static_cast<int>(KeyCode::KeySelect) == PM_KEY_SELECT,
              "PM_KEY_SELECT parity: KeyCode::KeySelect");
static_assert(static_cast<int>(KeyCode::KeyRight) == PM_KEY_RIGHT,
              "PM_KEY_RIGHT parity: KeyCode::KeyRight");
static_assert(static_cast<int>(KeyCode::KeyUp) == PM_KEY_UP,
              "PM_KEY_UP parity: KeyCode::KeyUp");
static_assert(static_cast<int>(KeyCode::KeyDown) == PM_KEY_DOWN,
              "PM_KEY_DOWN parity: KeyCode::KeyDown");
static_assert(static_cast<int>(KeyCode::KeyFnShiftRight) == PM_KEY_FN_SHIFT_RIGHT,
              "PM_KEY_FN_SHIFT_RIGHT parity: KeyCode::KeyFnShiftRight");
static_assert(static_cast<int>(KeyCode::KeyAsciiEscape) == PM_KEY_ASCII_ESCAPE,
              "PM_KEY_ASCII_ESCAPE parity: KeyCode::KeyAsciiEscape");
static_assert(static_cast<int>(KeyCode::KeyShiftLeft) == PM_KEY_SHIFT_LEFT,
              "PM_KEY_SHIFT_LEFT parity: KeyCode::KeyShiftLeft");
static_assert(static_cast<int>(KeyCode::KeyShiftSelect) == PM_KEY_SHIFT_SELECT,
              "PM_KEY_SHIFT_SELECT parity: KeyCode::KeyShiftSelect");
static_assert(static_cast<int>(KeyCode::KeyShiftRight) == PM_KEY_SHIFT_RIGHT,
              "PM_KEY_SHIFT_RIGHT parity: KeyCode::KeyShiftRight");
static_assert(static_cast<int>(KeyCode::KeySpace) == PM_KEY_SPACE,
              "PM_KEY_SPACE parity: KeyCode::KeySpace");
static_assert(static_cast<int>(TransportIcon::Play) == PM_TRANSPORT_ICON_PLAY,
              "PM_TRANSPORT_ICON_PLAY parity: TransportIcon::Play");
static_assert(static_cast<int>(TransportIcon::Pause) == PM_TRANSPORT_ICON_PAUSE,
              "PM_TRANSPORT_ICON_PAUSE parity: TransportIcon::Pause");
static_assert(static_cast<int>(TransportIcon::Stop) == PM_TRANSPORT_ICON_STOP,
              "PM_TRANSPORT_ICON_STOP parity: TransportIcon::Stop");
extern const Jingle* const pm_jingle_table[] = {
  &Jingles::Startup,
  &Jingles::Shutdown,
};
static_assert(sizeof(pm_jingle_table) / sizeof(pm_jingle_table[0]) == PM_JINGLE_COUNT,
              "PM_JINGLE_COUNT parity: Jingles");

namespace {

constexpr size_t kPoolSlots = 4;
constexpr size_t kPoolBytes = 256;

// Backs every const char* return. Not locked: only the owning app
// task calls these, so there is nothing to race with.
char g_pool[kPoolSlots][kPoolBytes];
size_t g_next_slot = 0;

char* pool_slot() {
  char* slot = g_pool[g_next_slot];
  g_next_slot = (g_next_slot + 1) % kPoolSlots;
  slot[0] = '\0';
  return slot;
}

// toCharArray always null terminates and truncates.
char* pool_string(const String& value) {
  char* slot = pool_slot();
  value.toCharArray(slot, kPoolBytes);
  return slot;
}

char* pool_borrowed(const char* value) {
  char* slot = pool_slot();
  if (value == nullptr) {
    return slot;
  }
  strncpy(slot, value, kPoolBytes - 1);
  slot[kPoolBytes - 1] = '\0';
  return slot;
}

}  // namespace

extern "C" {

uint32_t pm_app_abi(void) { return PM_APP_API_ABI; }

const char* pm_host_sdk_version(void) {
  return pocketmage_sdk_version;
}

int pm_app_at_exit(pm_app_exit_callback a0) {
  return static_cast<int>(P_APP.atExit(a0));
}

int pm_app_register_destructor(pm_app_destructor a0, void* a1, void* a2) {
  return static_cast<int>(P_APP.registerDestructor(a0, a1, a2));
}

void pm_app_finalize_destructors(void* a0) {
  P_APP.finalizeDestructors(a0);
}

void pm_audio_begin(void) {
  P_AUDIO.begin();
}

bool pm_audio_poll_usb(void) {
  return P_AUDIO.pollUsb();
}

void pm_audio_stop(void) {
  P_AUDIO.stop();
}

bool pm_audio_stop_checked(void) {
  return P_AUDIO.stopChecked();
}

void pm_audio_play(const char* a0) {
  P_AUDIO.play(a0);
}

void pm_audio_stop_playback(void) {
  P_AUDIO.stopPlayback();
}

void pm_audio_pause(void) {
  P_AUDIO.pause();
}

void pm_audio_resume(void) {
  P_AUDIO.resume();
}

void pm_audio_set_volume(int8_t a0) {
  P_AUDIO.setVolume(a0);
}

void pm_audio_set_volume_level(uint8_t a0) {
  P_AUDIO.setVolumeLevel(a0);
}

uint8_t pm_audio_get_volume_level(void) {
  return P_AUDIO.getVolumeLevel();
}

bool pm_audio_load_settings(void) {
  return P_AUDIO.loadSettings();
}

bool pm_audio_save_settings(void) {
  return P_AUDIO.saveSettings();
}

int pm_audio_get_state(void) {
  return static_cast<int>(P_AUDIO.getState());
}

bool pm_audio_is_playing(void) {
  return P_AUDIO.isPlaying();
}

bool pm_audio_owns_pin(void) {
  return P_AUDIO.ownsPin();
}

const char* pm_audio_get_current_path(void) {
  return pool_string(P_AUDIO.getCurrentPath());
}

const char* pm_audio_get_last_error(void) {
  return pool_string(P_AUDIO.getLastError());
}

int8_t pm_audio_get_volume(void) {
  return P_AUDIO.getVolume();
}

uint32_t pm_audio_get_position(void) {
  return P_AUDIO.getPosition();
}

uint64_t pm_audio_get_position_ms(void) {
  return P_AUDIO.getPositionMs();
}

uint32_t pm_audio_get_playback_id(void) {
  return P_AUDIO.getPlaybackId();
}

int pm_audio_get_playback_result(uint32_t a0) {
  return static_cast<int>(P_AUDIO.getPlaybackResult(a0));
}

void pm_audio_set_output(int a0) {
  if (a0 != static_cast<int>(AudioOutput::Piezo) && a0 != static_cast<int>(AudioOutput::Usb) && a0 != static_cast<int>(AudioOutput::Auto)) return;
  P_AUDIO.setOutput(static_cast<AudioOutput>(a0));
}

int pm_audio_get_output(void) {
  return static_cast<int>(P_AUDIO.getOutput());
}

int pm_audio_get_active_output(void) {
  return static_cast<int>(P_AUDIO.getActiveOutput());
}

bool pm_audio_usb_connected(void) {
  return P_AUDIO.usbConnected();
}

void pm_audio_dispatch_events(void) {
  P_AUDIO.dispatchEvents();
}

bool pm_bz_begin(int a0) {
  return BZ().begin(a0);
}

void pm_bz_end(void) {
  BZ().end();
}

void pm_bz_play_jingle(int a0) {
  if (a0 < 0 || a0 >= PM_JINGLE_COUNT) return;
  BZ().playJingle(*pm_jingle_table[a0]);
}

void pm_bz_drain_pending(void) {
  BZ().drainPending();
}

bool pm_clock_begin(void) {
  return CLOCK().begin();
}

void pm_clock_set_time_from_string(const char* a0) {
  CLOCK().setTimeFromString(String(a0));
}

bool pm_clock_valid(void) {
  return CLOCK().isValid();
}

int64_t pm_clock_epoch(void) {
  return CLOCK().epoch();
}

const char* pm_clock_timestamp(void) {
  return pool_string(CLOCK().timestamp());
}

void pm_clock_set_timeout_millis(long a0) {
  CLOCK().setTimeoutMillis(a0);
}

void pm_clock_set_prev_time_millis(long a0) {
  CLOCK().setPrevTimeMillis(a0);
}

void pm_eink_set_line_spacing(uint8_t a0) {
  EINK().setLineSpacing(a0);
}

void pm_eink_set_full_refresh_after(uint8_t a0) {
  EINK().setFullRefreshAfter(a0);
}

int pm_eink_width(void) {
  return static_cast<int>(EINK().width());
}

int pm_eink_height(void) {
  return static_cast<int>(EINK().height());
}

void pm_eink_clear(void) {
  EINK().clear();
}

void pm_eink_pixel(int a0, int a1, bool a2) {
  EINK().pixel(a0, a1, a2);
}

void pm_eink_rect(int a0, int a1, int a2, int a3, bool a4, bool a5) {
  EINK().rect(a0, a1, a2, a3, a4, a5);
}

void pm_eink_triangle(int a0, int a1, int a2, int a3, int a4, int a5, bool a6, bool a7) {
  EINK().triangle(a0, a1, a2, a3, a4, a5, a6, a7);
}

void pm_eink_refresh(void) {
  EINK().refresh();
}

void pm_eink_set_fast_full_refresh(bool a0) {
  EINK().setFastFullRefresh(a0);
}

void pm_eink_status_bar(const char* a0, bool a1) {
  EINK().statusBar(String(a0), a1);
}

void pm_eink_draw_status_bar(const char* a0) {
  EINK().drawStatusBar(String(a0));
}

void pm_eink_reset_display(bool a0, uint16_t a1) {
  EINK().resetDisplay(a0, a1);
}

int pm_eink_count_lines(const char* a0, size_t a1) {
  return static_cast<int>(EINK().countLines(String(a0), a1));
}

uint8_t pm_eink_get_font_height(void) {
  return EINK().getFontHeight();
}

int pm_eink_max_lines(void) {
  return static_cast<int>(EINK().maxLines());
}

uint16_t pm_eink_get_eink_text_width(const char* a0) {
  return EINK().getEinkTextWidth(String(a0));
}

uint8_t pm_eink_get_line_spacing(void) {
  return EINK().getLineSpacing();
}

void pm_eink_force_slow_full_update(bool a0) {
  EINK().forceSlowFullUpdate(a0);
}

void pm_eink_mark_panel_needs_full_refresh(void) {
  EINK().markPanelNeedsFullRefresh();
}

void pm_font_engine_draw_glyph(int a0, int a1, int a2, uint16_t a3, int a4) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return;
  if (a4 != static_cast<int>(FontStyle::Tiny) && a4 != static_cast<int>(FontStyle::Body) && a4 != static_cast<int>(FontStyle::BodyBold) && a4 != static_cast<int>(FontStyle::BodyItalic) && a4 != static_cast<int>(FontStyle::BodyBoldItalic) && a4 != static_cast<int>(FontStyle::Medium) && a4 != static_cast<int>(FontStyle::Small) && a4 != static_cast<int>(FontStyle::BodyNarrow) && a4 != static_cast<int>(FontStyle::Mono) && a4 != static_cast<int>(FontStyle::MonoBold) && a4 != static_cast<int>(FontStyle::MonoItalic) && a4 != static_cast<int>(FontStyle::MonoBoldItalic) && a4 != static_cast<int>(FontStyle::Sans) && a4 != static_cast<int>(FontStyle::SansBold) && a4 != static_cast<int>(FontStyle::SansItalic) && a4 != static_cast<int>(FontStyle::SansBoldItalic) && a4 != static_cast<int>(FontStyle::Caption) && a4 != static_cast<int>(FontStyle::Heading3) && a4 != static_cast<int>(FontStyle::Heading2) && a4 != static_cast<int>(FontStyle::Heading1) && a4 != static_cast<int>(FontStyle::Large) && a4 != static_cast<int>(FontStyle::OledWord) && a4 != static_cast<int>(FontStyle::Terminal) && a4 != static_cast<int>(FontStyle::TerminalBig) && a4 != static_cast<int>(FontStyle::ClockDigit)) return;
  FontEngine::drawGlyph(static_cast<DisplayTarget>(a0), a1, a2, a3, static_cast<FontStyle>(a4));
}

int pm_font_engine_char_width(int a0, uint16_t a1, int a2) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return {};
  if (a2 != static_cast<int>(FontStyle::Tiny) && a2 != static_cast<int>(FontStyle::Body) && a2 != static_cast<int>(FontStyle::BodyBold) && a2 != static_cast<int>(FontStyle::BodyItalic) && a2 != static_cast<int>(FontStyle::BodyBoldItalic) && a2 != static_cast<int>(FontStyle::Medium) && a2 != static_cast<int>(FontStyle::Small) && a2 != static_cast<int>(FontStyle::BodyNarrow) && a2 != static_cast<int>(FontStyle::Mono) && a2 != static_cast<int>(FontStyle::MonoBold) && a2 != static_cast<int>(FontStyle::MonoItalic) && a2 != static_cast<int>(FontStyle::MonoBoldItalic) && a2 != static_cast<int>(FontStyle::Sans) && a2 != static_cast<int>(FontStyle::SansBold) && a2 != static_cast<int>(FontStyle::SansItalic) && a2 != static_cast<int>(FontStyle::SansBoldItalic) && a2 != static_cast<int>(FontStyle::Caption) && a2 != static_cast<int>(FontStyle::Heading3) && a2 != static_cast<int>(FontStyle::Heading2) && a2 != static_cast<int>(FontStyle::Heading1) && a2 != static_cast<int>(FontStyle::Large) && a2 != static_cast<int>(FontStyle::OledWord) && a2 != static_cast<int>(FontStyle::Terminal) && a2 != static_cast<int>(FontStyle::TerminalBig) && a2 != static_cast<int>(FontStyle::ClockDigit)) return {};
  return static_cast<int>(FontEngine::charWidth(static_cast<DisplayTarget>(a0), a1, static_cast<FontStyle>(a2)));
}

int pm_font_engine_font_ascent(int a0, int a1) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return {};
  if (a1 != static_cast<int>(FontStyle::Tiny) && a1 != static_cast<int>(FontStyle::Body) && a1 != static_cast<int>(FontStyle::BodyBold) && a1 != static_cast<int>(FontStyle::BodyItalic) && a1 != static_cast<int>(FontStyle::BodyBoldItalic) && a1 != static_cast<int>(FontStyle::Medium) && a1 != static_cast<int>(FontStyle::Small) && a1 != static_cast<int>(FontStyle::BodyNarrow) && a1 != static_cast<int>(FontStyle::Mono) && a1 != static_cast<int>(FontStyle::MonoBold) && a1 != static_cast<int>(FontStyle::MonoItalic) && a1 != static_cast<int>(FontStyle::MonoBoldItalic) && a1 != static_cast<int>(FontStyle::Sans) && a1 != static_cast<int>(FontStyle::SansBold) && a1 != static_cast<int>(FontStyle::SansItalic) && a1 != static_cast<int>(FontStyle::SansBoldItalic) && a1 != static_cast<int>(FontStyle::Caption) && a1 != static_cast<int>(FontStyle::Heading3) && a1 != static_cast<int>(FontStyle::Heading2) && a1 != static_cast<int>(FontStyle::Heading1) && a1 != static_cast<int>(FontStyle::Large) && a1 != static_cast<int>(FontStyle::OledWord) && a1 != static_cast<int>(FontStyle::Terminal) && a1 != static_cast<int>(FontStyle::TerminalBig) && a1 != static_cast<int>(FontStyle::ClockDigit)) return {};
  return static_cast<int>(FontEngine::fontAscent(static_cast<DisplayTarget>(a0), static_cast<FontStyle>(a1)));
}

int pm_font_engine_font_descent(int a0, int a1) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return {};
  if (a1 != static_cast<int>(FontStyle::Tiny) && a1 != static_cast<int>(FontStyle::Body) && a1 != static_cast<int>(FontStyle::BodyBold) && a1 != static_cast<int>(FontStyle::BodyItalic) && a1 != static_cast<int>(FontStyle::BodyBoldItalic) && a1 != static_cast<int>(FontStyle::Medium) && a1 != static_cast<int>(FontStyle::Small) && a1 != static_cast<int>(FontStyle::BodyNarrow) && a1 != static_cast<int>(FontStyle::Mono) && a1 != static_cast<int>(FontStyle::MonoBold) && a1 != static_cast<int>(FontStyle::MonoItalic) && a1 != static_cast<int>(FontStyle::MonoBoldItalic) && a1 != static_cast<int>(FontStyle::Sans) && a1 != static_cast<int>(FontStyle::SansBold) && a1 != static_cast<int>(FontStyle::SansItalic) && a1 != static_cast<int>(FontStyle::SansBoldItalic) && a1 != static_cast<int>(FontStyle::Caption) && a1 != static_cast<int>(FontStyle::Heading3) && a1 != static_cast<int>(FontStyle::Heading2) && a1 != static_cast<int>(FontStyle::Heading1) && a1 != static_cast<int>(FontStyle::Large) && a1 != static_cast<int>(FontStyle::OledWord) && a1 != static_cast<int>(FontStyle::Terminal) && a1 != static_cast<int>(FontStyle::TerminalBig) && a1 != static_cast<int>(FontStyle::ClockDigit)) return {};
  return static_cast<int>(FontEngine::fontDescent(static_cast<DisplayTarget>(a0), static_cast<FontStyle>(a1)));
}

void pm_font_engine_draw_text_txt(int a0, int a1, int a2, const char* a3, uint8_t a4, uint8_t a5, uint8_t a6) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return;
  FontEngine::drawTextTxt(static_cast<DisplayTarget>(a0), a1, a2, a3, a4, a5, a6);
}

void pm_font_engine_draw_glyph_txt(int a0, int a1, int a2, uint16_t a3, uint8_t a4, uint8_t a5, uint8_t a6) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return;
  FontEngine::drawGlyphTxt(static_cast<DisplayTarget>(a0), a1, a2, a3, a4, a5, a6);
}

int pm_font_engine_text_width_txt(int a0, const char* a1, uint8_t a2, uint8_t a3, uint8_t a4) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return {};
  return static_cast<int>(FontEngine::textWidthTxt(static_cast<DisplayTarget>(a0), a1, a2, a3, a4));
}

int pm_font_engine_char_width_txt(int a0, uint16_t a1, uint8_t a2, uint8_t a3, uint8_t a4) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return {};
  return static_cast<int>(FontEngine::charWidthTxt(static_cast<DisplayTarget>(a0), a1, a2, a3, a4));
}

int pm_font_engine_font_height_txt(uint8_t a0, uint8_t a1, uint8_t a2) {
  return static_cast<int>(FontEngine::fontHeightTxt(a0, a1, a2));
}

void pm_i18n_set_language(int a0) {
  if (a0 != static_cast<int>(Lang::English) && a0 != static_cast<int>(Lang::French) && a0 != static_cast<int>(Lang::Spanish) && a0 != static_cast<int>(Lang::German)) return;
  I18n::setLanguage(static_cast<Lang>(a0));
}

bool pm_i18n_set_language_by_code(const char* a0) {
  return I18n::setLanguageByCode(a0);
}

int pm_i18n_language(void) {
  return static_cast<int>(I18n::language());
}

int pm_i18n_language_count(void) {
  return static_cast<int>(I18n::languageCount());
}

const char* pm_i18n_code(void) {
  return pool_borrowed(I18n::code());
}

const char* pm_i18n_code_at(int a0) {
  return pool_borrowed(I18n::code(a0));
}

const char* pm_i18n_native_name(void) {
  return pool_borrowed(I18n::nativeName());
}

const char* pm_i18n_native_name_at(int a0) {
  return pool_borrowed(I18n::nativeName(a0));
}

const char* pm_i18n_get(int a0) {
  if (a0 != static_cast<int>(StringID::STR_KB_SHIFT) && a0 != static_cast<int>(StringID::STR_KB_FN) && a0 != static_cast<int>(StringID::STR_KB_FN_SHIFT) && a0 != static_cast<int>(StringID::STR_KB_INIT_FAILED) && a0 != static_cast<int>(StringID::STR_KB_SPECIAL) && a0 != static_cast<int>(StringID::STR_KB_USB_CLOSE) && a0 != static_cast<int>(StringID::STR_KB_APP_CANCEL) && a0 != static_cast<int>(StringID::STR_KB_APP_TXT) && a0 != static_cast<int>(StringID::STR_KB_APP_FILEWIZ) && a0 != static_cast<int>(StringID::STR_KB_APP_USB) && a0 != static_cast<int>(StringID::STR_KB_APP_CHAT) && a0 != static_cast<int>(StringID::STR_KB_APP_SETTINGS) && a0 != static_cast<int>(StringID::STR_KB_APP_TASKS) && a0 != static_cast<int>(StringID::STR_KB_APP_CALENDAR) && a0 != static_cast<int>(StringID::STR_KB_APP_JOURNAL) && a0 != static_cast<int>(StringID::STR_KB_APP_LEXICON) && a0 != static_cast<int>(StringID::STR_KB_APP_TERMINAL) && a0 != static_cast<int>(StringID::STR_KB_APP_LOADER) && a0 != static_cast<int>(StringID::STR_GRID_TXT) && a0 != static_cast<int>(StringID::STR_GRID_FILEWIZ) && a0 != static_cast<int>(StringID::STR_GRID_USB) && a0 != static_cast<int>(StringID::STR_GRID_CHAT) && a0 != static_cast<int>(StringID::STR_GRID_SETTINGS) && a0 != static_cast<int>(StringID::STR_GRID_TASKS) && a0 != static_cast<int>(StringID::STR_GRID_CALENDAR) && a0 != static_cast<int>(StringID::STR_GRID_JOURNAL) && a0 != static_cast<int>(StringID::STR_GRID_LEXICON) && a0 != static_cast<int>(StringID::STR_GRID_TERMINAL) && a0 != static_cast<int>(StringID::STR_GRID_LOADER) && a0 != static_cast<int>(StringID::STR_INFO_USB) && a0 != static_cast<int>(StringID::STR_INFO_SNK) && a0 != static_cast<int>(StringID::STR_INFO_SD) && a0 != static_cast<int>(StringID::STR_OLED_LINE) && a0 != static_cast<int>(StringID::STR_FRAMES_LINES) && a0 != static_cast<int>(StringID::STR_DAY_SUNDAY) && a0 != static_cast<int>(StringID::STR_DAY_MONDAY) && a0 != static_cast<int>(StringID::STR_DAY_TUESDAY) && a0 != static_cast<int>(StringID::STR_DAY_WEDNESDAY) && a0 != static_cast<int>(StringID::STR_DAY_THURSDAY) && a0 != static_cast<int>(StringID::STR_DAY_FRIDAY) && a0 != static_cast<int>(StringID::STR_DAY_SATURDAY) && a0 != static_cast<int>(StringID::STR_MONTH_JAN) && a0 != static_cast<int>(StringID::STR_MONTH_FEB) && a0 != static_cast<int>(StringID::STR_MONTH_MAR) && a0 != static_cast<int>(StringID::STR_MONTH_APR) && a0 != static_cast<int>(StringID::STR_MONTH_MAY) && a0 != static_cast<int>(StringID::STR_MONTH_JUN) && a0 != static_cast<int>(StringID::STR_MONTH_JUL) && a0 != static_cast<int>(StringID::STR_MONTH_AUG) && a0 != static_cast<int>(StringID::STR_MONTH_SEP) && a0 != static_cast<int>(StringID::STR_MONTH_OCT) && a0 != static_cast<int>(StringID::STR_MONTH_NOV) && a0 != static_cast<int>(StringID::STR_MONTH_DEC) && a0 != static_cast<int>(StringID::STR_MONTH_ERR) && a0 != static_cast<int>(StringID::STR_INVALID) && a0 != static_cast<int>(StringID::STR_GOOD_SAVE) && a0 != static_cast<int>(StringID::STR_SAVED_PREFIX) && a0 != static_cast<int>(StringID::STR_ARROW) && a0 != static_cast<int>(StringID::STR_SD_NOT_DETECTED) && a0 != static_cast<int>(StringID::STR_SD_BRACKET_CLOSE) && a0 != static_cast<int>(StringID::STR_SD_NOT_DETECTED_START_FAIL) && a0 != static_cast<int>(StringID::STR_SD_COMPAT_MODE) && a0 != static_cast<int>(StringID::STR_SD_ALL_WORK_LOST) && a0 != static_cast<int>(StringID::STR_SD_INSERT_REBOOT) && a0 != static_cast<int>(StringID::STR_SD_SPI_NOT_DETECTED) && a0 != static_cast<int>(StringID::STR_SD_COMPAT_FAILED) && a0 != static_cast<int>(StringID::STR_SD_COMPAT_STARTED) && a0 != static_cast<int>(StringID::STR_SD_SAVE_FAILED) && a0 != static_cast<int>(StringID::STR_SD_META_WRITE_ERR) && a0 != static_cast<int>(StringID::STR_SD_LOAD_FAILED) && a0 != static_cast<int>(StringID::STR_SD_LOADING_FILE) && a0 != static_cast<int>(StringID::STR_SD_FILE_LOADED) && a0 != static_cast<int>(StringID::STR_SD_DELETE_FAILED) && a0 != static_cast<int>(StringID::STR_SD_RENAME_FAILED) && a0 != static_cast<int>(StringID::STR_SD_COPY_FAILED) && a0 != static_cast<int>(StringID::STR_SD_OP_FAILED) && a0 != static_cast<int>(StringID::STR_SD_LOAD_FAILED_SHORT) && a0 != static_cast<int>(StringID::STR_SYS_REBOOT_WARNING) && a0 != static_cast<int>(StringID::STR_SYS_PROCESS_INTERRUPTED) && a0 != static_cast<int>(StringID::STR_WIFI_IDLE) && a0 != static_cast<int>(StringID::STR_WIFI_STARTED) && a0 != static_cast<int>(StringID::STR_WIFI_CONNECTED) && a0 != static_cast<int>(StringID::STR_WIFI_AUTH_FAILED) && a0 != static_cast<int>(StringID::STR_WIFI_NETWORK_NOT_FOUND) && a0 != static_cast<int>(StringID::STR_WIFI_RECONNECTING) && a0 != static_cast<int>(StringID::STR_WIFI_MAX_RETRIES) && a0 != static_cast<int>(StringID::STR_WIFI_CONN_FAILED) && a0 != static_cast<int>(StringID::STR_WIFI_SCAN_DONE) && a0 != static_cast<int>(StringID::STR_WIFI_GOT_IP) && a0 != static_cast<int>(StringID::STR_WIFI_ENABLING) && a0 != static_cast<int>(StringID::STR_WIFI_CREATE_NETIF_FAILED) && a0 != static_cast<int>(StringID::STR_WIFI_INIT_FAILED) && a0 != static_cast<int>(StringID::STR_WIFI_SET_MODE_FAILED) && a0 != static_cast<int>(StringID::STR_WIFI_START_FAILED) && a0 != static_cast<int>(StringID::STR_WIFI_ENABLED) && a0 != static_cast<int>(StringID::STR_WIFI_DISABLING) && a0 != static_cast<int>(StringID::STR_WIFI_DISABLED) && a0 != static_cast<int>(StringID::STR_WIFI_SCANNING) && a0 != static_cast<int>(StringID::STR_WIFI_NO_SSID) && a0 != static_cast<int>(StringID::STR_WIFI_CONNECTING) && a0 != static_cast<int>(StringID::STR_WIFI_DISCONNECTING) && a0 != static_cast<int>(StringID::STR_FRAME_CURSOR) && a0 != static_cast<int>(StringID::STR_FRAME_TEST_1) && a0 != static_cast<int>(StringID::STR_FRAME_TEST_2) && a0 != static_cast<int>(StringID::STR_FRAME_TEST_3) && a0 != static_cast<int>(StringID::STR_FRAME_TEST_4) && a0 != static_cast<int>(StringID::STR_FRAME_TEST_5) && a0 != static_cast<int>(StringID::STR_FRAME_TEST_6) && a0 != static_cast<int>(StringID::STR_SETTINGS_DATE_UPDATED_TO) && a0 != static_cast<int>(StringID::STR_COMM_PEERS_PREFIX) && a0 != static_cast<int>(StringID::STR_TOUCHPAD_FAILED) && a0 != static_cast<int>(StringID::STR_SAVE_FAILED) && a0 != static_cast<int>(StringID::STR_GOTO_LINE) && a0 != static_cast<int>(StringID::STR_UTILS_SLEEP) && a0 != static_cast<int>(StringID::STR_UTILS_BATT_CRITICAL) && a0 != static_cast<int>(StringID::STR_UTILS_YN_SUFFIX) && a0 != static_cast<int>(StringID::STR_UTILS_PRESS_KEY) && a0 != static_cast<int>(StringID::STR_UTILS_CRASH_PREFIX) && a0 != static_cast<int>(StringID::STR_UTILS_CRASH_PANIC) && a0 != static_cast<int>(StringID::STR_UTILS_CRASH_WDT) && a0 != static_cast<int>(StringID::STR_UTILS_CRASH_TASK_WDT) && a0 != static_cast<int>(StringID::STR_UTILS_CRASH_INT_WDT) && a0 != static_cast<int>(StringID::STR_UTILS_CRASH_UNKNOWN) && a0 != static_cast<int>(StringID::STR_UTILS_CRASH_CODE_OPEN) && a0 != static_cast<int>(StringID::STR_UTILS_CRASH_CODE_CLOSE) && a0 != static_cast<int>(StringID::STR_UTILS_POWER_LOST) && a0 != static_cast<int>(StringID::STR_UTILS_TIME_SET) && a0 != static_cast<int>(StringID::STR_UTILS_SAVING_WORK) && a0 != static_cast<int>(StringID::STR_HOME_INVALID_NUMBER) && a0 != static_cast<int>(StringID::STR_HOME_D1_SILLY) && a0 != static_cast<int>(StringID::STR_HOME_SD_COMPAT_OFF) && a0 != static_cast<int>(StringID::STR_HOME_WELCOME) && a0 != static_cast<int>(StringID::STR_HOME_PMOS_PREFIX) && a0 != static_cast<int>(StringID::STR_HOME_SMELLS) && a0 != static_cast<int>(StringID::STR_HOME_YUCK) && a0 != static_cast<int>(StringID::STR_HOME_HEY) && a0 != static_cast<int>(StringID::STR_HOME_WHATS_UP) && a0 != static_cast<int>(StringID::STR_HOME_LOVE) && a0 != static_cast<int>(StringID::STR_HOME_IDK) && a0 != static_cast<int>(StringID::STR_HOME_CRASHING) && a0 != static_cast<int>(StringID::STR_HOME_TYPE_CMD) && a0 != static_cast<int>(StringID::STR_APPLOADER_EXTRACTING) && a0 != static_cast<int>(StringID::STR_APPLOADER_INSTALLING) && a0 != static_cast<int>(StringID::STR_APPLOADER_CHOOSE_SLOT) && a0 != static_cast<int>(StringID::STR_APPLOADER_APP_REMOVED) && a0 != static_cast<int>(StringID::STR_APPLOADER_SWAP_DELETE) && a0 != static_cast<int>(StringID::STR_APPLOADER_MACOS_META) && a0 != static_cast<int>(StringID::STR_APPLOADER_NOT_TAR) && a0 != static_cast<int>(StringID::STR_APPLOADER_INSTALL_FAILED) && a0 != static_cast<int>(StringID::STR_APPLOADER_INSTALL_COMPLETE) && a0 != static_cast<int>(StringID::STR_APPLOADER_TYPE_LETTER) && a0 != static_cast<int>(StringID::STR_FILEWIZ_EMPTY) && a0 != static_cast<int>(StringID::STR_FILEWIZ_ENTER_NEW_NAME) && a0 != static_cast<int>(StringID::STR_FILEWIZ_RENAMING) && a0 != static_cast<int>(StringID::STR_FILEWIZ_DELETE_Q) && a0 != static_cast<int>(StringID::STR_FILEWIZ_DELETING) && a0 != static_cast<int>(StringID::STR_FILEWIZ_ENTER_COPY_NAME) && a0 != static_cast<int>(StringID::STR_FILEWIZ_COPYING) && a0 != static_cast<int>(StringID::STR_FILEWIZ_SIZE_PREFIX) && a0 != static_cast<int>(StringID::STR_FILEWIZ_SIZE_SUFFIX) && a0 != static_cast<int>(StringID::STR_FILEWIZ_ERR_READING) && a0 != static_cast<int>(StringID::STR_FILEWIZ_SELECT_OP) && a0 != static_cast<int>(StringID::STR_FILEWIZ_SELECT_FILE) && a0 != static_cast<int>(StringID::STR_SETTINGS_TIME_UPDATED_TO) && a0 != static_cast<int>(StringID::STR_SETTINGS_TIME_UPDATED) && a0 != static_cast<int>(StringID::STR_SETTINGS_INVALID_FMT_HHMM) && a0 != static_cast<int>(StringID::STR_SETTINGS_INVALID_DATE) && a0 != static_cast<int>(StringID::STR_SETTINGS_DATE_UPDATED) && a0 != static_cast<int>(StringID::STR_SETTINGS_INVALID_FMT_YYYYMMDD) && a0 != static_cast<int>(StringID::STR_SETTINGS_INVALID_BANG) && a0 != static_cast<int>(StringID::STR_SETTINGS_UPDATED) && a0 != static_cast<int>(StringID::STR_SETTINGS_HUH) && a0 != static_cast<int>(StringID::STR_SETTINGS_LANG_PROMPT) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_BRIGHTNESS) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_TIMEOUT) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_MAX_FPS) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_MUTE) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_CLOCK) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_YEAR) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_POWER) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_FAST_REFRESH) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_DEBUG) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_HOME_BOOT) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_ALLOW_SD) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_LOCK) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_SET_PIN) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_SET_TIME) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_SET_DATE) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_SET_LANG) && a0 != static_cast<int>(StringID::STR_SETTINGS_CMD_PROMPT) && a0 != static_cast<int>(StringID::STR_SETTINGS_ENTER_CHANGE) && a0 != static_cast<int>(StringID::STR_SETTINGS_SPACE_TYPE) && a0 != static_cast<int>(StringID::STR_SETTINGS_EINK_HINT) && a0 != static_cast<int>(StringID::STR_LEX_INSTALL_DICT) && a0 != static_cast<int>(StringID::STR_LEX_LOADING) && a0 != static_cast<int>(StringID::STR_LEX_MISSING) && a0 != static_cast<int>(StringID::STR_LEX_NONE) && a0 != static_cast<int>(StringID::STR_LEX_TYPE_WORD) && a0 != static_cast<int>(StringID::STR_LEX_TYPE_NEW_WORD) && a0 != static_cast<int>(StringID::STR_JOURNAL_CHOOSE) && a0 != static_cast<int>(StringID::STR_USB_INIT) && a0 != static_cast<int>(StringID::STR_USB_HOST_FAIL) && a0 != static_cast<int>(StringID::STR_USB_SLOT_FAIL) && a0 != static_cast<int>(StringID::STR_USB_CARD_STRUCT_FAIL) && a0 != static_cast<int>(StringID::STR_USB_CARD_FAIL) && a0 != static_cast<int>(StringID::STR_USB_SDRESET) && a0 != static_cast<int>(StringID::STR_USB_CONNECT) && a0 != static_cast<int>(StringID::STR_USB_WAITING) && a0 != static_cast<int>(StringID::STR_USB_CONNECTED) && a0 != static_cast<int>(StringID::STR_USB_SUSPENDED) && a0 != static_cast<int>(StringID::STR_USB_EJECTED) && a0 != static_cast<int>(StringID::STR_USB_CARD_BUSY) && a0 != static_cast<int>(StringID::STR_USB_MSC_FAIL) && a0 != static_cast<int>(StringID::STR_USB_DEVICE_FAIL) && a0 != static_cast<int>(StringID::STR_CAL_TYPE_A_DATE) && a0 != static_cast<int>(StringID::STR_CAL_WEEK_HINT) && a0 != static_cast<int>(StringID::STR_CAL_EVENT_NAME) && a0 != static_cast<int>(StringID::STR_CAL_ATTACH_NOTE) && a0 != static_cast<int>(StringID::STR_CAL_EVENT_CREATED) && a0 != static_cast<int>(StringID::STR_CAL_EDIT_NAME) && a0 != static_cast<int>(StringID::STR_CAL_EDIT_NOTE) && a0 != static_cast<int>(StringID::STR_CAL_DELETE_Q) && a0 != static_cast<int>(StringID::STR_CAL_EVENT_DELETED) && a0 != static_cast<int>(StringID::STR_CAL_EVENT_SAVED) && a0 != static_cast<int>(StringID::STR_CAL_VIEW_HINT) && a0 != static_cast<int>(StringID::STR_CAL_STEP_NAME) && a0 != static_cast<int>(StringID::STR_CAL_STEP_START_DATE) && a0 != static_cast<int>(StringID::STR_CAL_STEP_START_TIME) && a0 != static_cast<int>(StringID::STR_CAL_STEP_DURATION) && a0 != static_cast<int>(StringID::STR_CAL_STEP_REPEAT) && a0 != static_cast<int>(StringID::STR_CAL_STEP_NOTE) && a0 != static_cast<int>(StringID::STR_CAL_DAY_HINT) && a0 != static_cast<int>(StringID::STR_CAL_STARTS) && a0 != static_cast<int>(StringID::STR_CAL_DUR) && a0 != static_cast<int>(StringID::STR_CAL_REP) && a0 != static_cast<int>(StringID::STR_CAL_MONTHLY_IN) && a0 != static_cast<int>(StringID::STR_COMM_ME) && a0 != static_cast<int>(StringID::STR_COMM_LOCAL_CHAT) && a0 != static_cast<int>(StringID::STR_COMM_CHAT) && a0 != static_cast<int>(StringID::STR_COMM_MSG_PREFIX) && a0 != static_cast<int>(StringID::STR_COMM_SELECT_ROOM) && a0 != static_cast<int>(StringID::STR_COMM_PLACEHOLDER) && a0 != static_cast<int>(StringID::STR_COMM_DIRECT) && a0 != static_cast<int>(StringID::STR_TERM_FILE_SAVED) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_POTION) && a0 != static_cast<int>(StringID::STR_TERM_ONLY_CTXT) && a0 != static_cast<int>(StringID::STR_TERM_FILE_NOT_FOUND) && a0 != static_cast<int>(StringID::STR_TERM_HELP_AVAILABLE) && a0 != static_cast<int>(StringID::STR_TERM_HELP_LS) && a0 != static_cast<int>(StringID::STR_TERM_HELP_CD) && a0 != static_cast<int>(StringID::STR_TERM_HELP_RM) && a0 != static_cast<int>(StringID::STR_TERM_HELP_RM_R) && a0 != static_cast<int>(StringID::STR_TERM_HELP_CP) && a0 != static_cast<int>(StringID::STR_TERM_HELP_MV) && a0 != static_cast<int>(StringID::STR_TERM_HELP_TOUCH) && a0 != static_cast<int>(StringID::STR_TERM_HELP_MKDIR) && a0 != static_cast<int>(StringID::STR_TERM_HELP_CLEAR) && a0 != static_cast<int>(StringID::STR_TERM_HELP_TXT) && a0 != static_cast<int>(StringID::STR_TERM_HELP_POTION) && a0 != static_cast<int>(StringID::STR_TERM_HELP_BREW) && a0 != static_cast<int>(StringID::STR_TERM_HELP_LINK) && a0 != static_cast<int>(StringID::STR_TERM_HELP_UNLINK) && a0 != static_cast<int>(StringID::STR_TERM_HELP_SETFONT) && a0 != static_cast<int>(StringID::STR_TERM_HELP_THEME) && a0 != static_cast<int>(StringID::STR_TERM_CD_NOT_DIR) && a0 != static_cast<int>(StringID::STR_TERM_CD_NO_SUCH) && a0 != static_cast<int>(StringID::STR_TERM_LS_DIR) && a0 != static_cast<int>(StringID::STR_TERM_LS_NOT_DIR) && a0 != static_cast<int>(StringID::STR_TERM_LS_NO_SUCH) && a0 != static_cast<int>(StringID::STR_TERM_PATH_NOT_DEFINED) && a0 != static_cast<int>(StringID::STR_TERM_RMDIR_FAILED) && a0 != static_cast<int>(StringID::STR_TERM_PATH_NOT_FOUND) && a0 != static_cast<int>(StringID::STR_TERM_OPEN_FAILED) && a0 != static_cast<int>(StringID::STR_TERM_NOT_FILE_RM) && a0 != static_cast<int>(StringID::STR_TERM_DELETE_FAILED) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_CP) && a0 != static_cast<int>(StringID::STR_TERM_SRC_NOT_FOUND) && a0 != static_cast<int>(StringID::STR_TERM_SRC_NOT_FILE) && a0 != static_cast<int>(StringID::STR_TERM_DEST_FAILED) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_MV) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_TOUCH) && a0 != static_cast<int>(StringID::STR_TERM_IS_DIR) && a0 != static_cast<int>(StringID::STR_TERM_CREATE_FILE_FAILED) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_TXT) && a0 != static_cast<int>(StringID::STR_TERM_ONLY_TXT) && a0 != static_cast<int>(StringID::STR_TERM_OPENING_PREFIX) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_LINK) && a0 != static_cast<int>(StringID::STR_TERM_LINKED) && a0 != static_cast<int>(StringID::STR_TERM_UNLINKED) && a0 != static_cast<int>(StringID::STR_TERM_LINK_NOT_FOUND) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_BREW) && a0 != static_cast<int>(StringID::STR_TERM_ONLY_C) && a0 != static_cast<int>(StringID::STR_TERM_BREW_READ_FAIL) && a0 != static_cast<int>(StringID::STR_TERM_FONT_LARGE) && a0 != static_cast<int>(StringID::STR_TERM_FONT_SMALL) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_SETFONT) && a0 != static_cast<int>(StringID::STR_TERM_THEME_LIGHT) && a0 != static_cast<int>(StringID::STR_TERM_THEME_DARK) && a0 != static_cast<int>(StringID::STR_TERM_USAGE_THEME) && a0 != static_cast<int>(StringID::STR_TERM_LINK_MISSING) && a0 != static_cast<int>(StringID::STR_TERM_LINK_READ_FAIL) && a0 != static_cast<int>(StringID::STR_TERM_HELP_LANG) && a0 != static_cast<int>(StringID::STR_TERM_LANG_SET) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_BODY) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_CODE) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_QUOTE) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_ULIST) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_OLIST) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_HRULE) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_BLANK_LINE) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_BLANK) && a0 != static_cast<int>(StringID::STR_TXT_FONT_SER) && a0 != static_cast<int>(StringID::STR_TXT_FONT_SAN) && a0 != static_cast<int>(StringID::STR_TXT_FONT_MON) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_BOLD_ITALIC) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_BOLD) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_ITALIC) && a0 != static_cast<int>(StringID::STR_TXT_STYLE_NORMAL) && a0 != static_cast<int>(StringID::STR_TXT_SAVE_FAILED_OPEN) && a0 != static_cast<int>(StringID::STR_TXT_FILE_LOADED) && a0 != static_cast<int>(StringID::STR_TXT_CREATE_FAILED_SD) && a0 != static_cast<int>(StringID::STR_TXT_CREATE_FAILED) && a0 != static_cast<int>(StringID::STR_TXT_CREATED) && a0 != static_cast<int>(StringID::STR_TXT_FONT_SERIF) && a0 != static_cast<int>(StringID::STR_TXT_FONT_SANS) && a0 != static_cast<int>(StringID::STR_TXT_FONT_MONO) && a0 != static_cast<int>(StringID::STR_TXT_INVALID_NAME) && a0 != static_cast<int>(StringID::STR_TXT_INPUT_FILENAME) && a0 != static_cast<int>(StringID::STR_TXT_NAME_NEW_FILE) && a0 != static_cast<int>(StringID::STR_TXT_BAD_FILETYPE) && a0 != static_cast<int>(StringID::STR_TASKS_ENTER_NAME) && a0 != static_cast<int>(StringID::STR_TASKS_ADDED) && a0 != static_cast<int>(StringID::STR_TASKS_ENTER_NEW_NAME) && a0 != static_cast<int>(StringID::STR_TASKS_UPDATING) && a0 != static_cast<int>(StringID::STR_TASKS_DELETE_Q) && a0 != static_cast<int>(StringID::STR_TASKS_DELETING) && a0 != static_cast<int>(StringID::STR_TASKS_COPYING) && a0 != static_cast<int>(StringID::STR_TASKS_SELECT_HINT) && a0 != static_cast<int>(StringID::STR_TASKS_NO_TASKS) && a0 != static_cast<int>(StringID::STR_TASKS_SET_DUE) && a0 != static_cast<int>(StringID::STR_TASKS_T_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_NO_MEM) && a0 != static_cast<int>(StringID::STR_SSH_CONNECTING_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_CONNECT_FAILED_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_DISCONNECTED) && a0 != static_cast<int>(StringID::STR_SSH_SESSION_CLOSED) && a0 != static_cast<int>(StringID::STR_SSH_FINGERPRINT) && a0 != static_cast<int>(StringID::STR_SSH_FIRST_CONNECTION) && a0 != static_cast<int>(StringID::STR_SSH_TRUST_HOST_KEY) && a0 != static_cast<int>(StringID::STR_SSH_HOST_KEY_NOT_TRUSTED) && a0 != static_cast<int>(StringID::STR_SSH_HOST_KEY_CHANGED) && a0 != static_cast<int>(StringID::STR_SSH_HOST_KEY_MITM) && a0 != static_cast<int>(StringID::STR_SSH_HOST_KEY_FAIL) && a0 != static_cast<int>(StringID::STR_SSH_HOST_KEY_VERIFY_FAIL) && a0 != static_cast<int>(StringID::STR_SSH_LOGIN_AS) && a0 != static_cast<int>(StringID::STR_SSH_LOGIN_CANCELLED) && a0 != static_cast<int>(StringID::STR_SSH_PASSWORD_FOR_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_AUTHENTICATING) && a0 != static_cast<int>(StringID::STR_SSH_AUTH_FAILED_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_CHANNEL_FAIL) && a0 != static_cast<int>(StringID::STR_SSH_SHELL_FAIL_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_DISCONNECTED_TAG) && a0 != static_cast<int>(StringID::STR_SSH_USAGE_WIFI) && a0 != static_cast<int>(StringID::STR_SSH_USAGE_WIFI_SUB) && a0 != static_cast<int>(StringID::STR_SSH_CONNECTING_SAVED_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_WIFI_CONNECT_CANCELLED) && a0 != static_cast<int>(StringID::STR_SSH_SCANNING_NETWORKS) && a0 != static_cast<int>(StringID::STR_SSH_SCAN_HINT) && a0 != static_cast<int>(StringID::STR_SSH_NO_SCAN_RESULTS) && a0 != static_cast<int>(StringID::STR_SSH_NETWORKS_FOUND_SUFFIX) && a0 != static_cast<int>(StringID::STR_SSH_WIFI_STATE_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_SSID_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_IP_PREFIX) && a0 != static_cast<int>(StringID::STR_SSH_WIFI_DISCONNECTED) && a0 != static_cast<int>(StringID::STR_SSH_INVALID_PORT) && a0 != static_cast<int>(StringID::STR_SSH_USAGE_SSH) && a0 != static_cast<int>(StringID::STR_SSH_USAGE_SSH_P) && a0 != static_cast<int>(StringID::STR_SSH_NO_WIFI) && a0 != static_cast<int>(StringID::STR_SSH_JOIN_NETWORK) && a0 != static_cast<int>(StringID::STR_SSH_NO_HOST) && a0 != static_cast<int>(StringID::STR_LOCK_PROMPT) && a0 != static_cast<int>(StringID::STR_LOCK_WRONG) && a0 != static_cast<int>(StringID::STR_LOCK_UNLOCKED) && a0 != static_cast<int>(StringID::STR_LOCK_ENABLED) && a0 != static_cast<int>(StringID::STR_LOCK_DISABLED) && a0 != static_cast<int>(StringID::STR_LOCK_PIN_SET) && a0 != static_cast<int>(StringID::STR_LOCK_PIN_INVALID) && a0 != static_cast<int>(StringID::STR_LOCK_ENTER_PIN) && a0 != static_cast<int>(StringID::STR_LOCK_CONFIRM_PIN) && a0 != static_cast<int>(StringID::STR_LOCK_MISMATCH) && a0 != static_cast<int>(StringID::STR_LOCK_HELP) && a0 != static_cast<int>(StringID::STR_SETTINGS_NAME_ONBOARD) && a0 != static_cast<int>(StringID::STR_ONBOARD_WELCOME) && a0 != static_cast<int>(StringID::STR_ONBOARD_WELCOME_SUB) && a0 != static_cast<int>(StringID::STR_ONBOARD_HINT_START) && a0 != static_cast<int>(StringID::STR_ONBOARD_LANG_TITLE) && a0 != static_cast<int>(StringID::STR_ONBOARD_HINT_SELECT) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFIASK_TITLE) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFIASK_BODY) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFIASK_NO) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFIASK_YES) && a0 != static_cast<int>(StringID::STR_ONBOARD_HINT_ASK) && a0 != static_cast<int>(StringID::STR_ONBOARD_HINT_NEXT) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFI_TITLE) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFI_SCANNING) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFI_NONE) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFI_PASS_PROMPT) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFI_CONNECTING) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFI_CONNECTED) && a0 != static_cast<int>(StringID::STR_ONBOARD_WIFI_FAILED) && a0 != static_cast<int>(StringID::STR_ONBOARD_TIME_TITLE) && a0 != static_cast<int>(StringID::STR_ONBOARD_TIME_CURRENT) && a0 != static_cast<int>(StringID::STR_ONBOARD_CTRL_TITLE) && a0 != static_cast<int>(StringID::STR_ONBOARD_KEY_1) && a0 != static_cast<int>(StringID::STR_ONBOARD_KEY_2) && a0 != static_cast<int>(StringID::STR_ONBOARD_KEY_3) && a0 != static_cast<int>(StringID::STR_ONBOARD_KEY_4) && a0 != static_cast<int>(StringID::STR_ONBOARD_KEY_5) && a0 != static_cast<int>(StringID::STR_ONBOARD_KEY_6) && a0 != static_cast<int>(StringID::STR_ONBOARD_CTRL_DESC_1) && a0 != static_cast<int>(StringID::STR_ONBOARD_CTRL_DESC_2) && a0 != static_cast<int>(StringID::STR_ONBOARD_CTRL_DESC_3) && a0 != static_cast<int>(StringID::STR_ONBOARD_CTRL_DESC_4) && a0 != static_cast<int>(StringID::STR_ONBOARD_CTRL_DESC_5) && a0 != static_cast<int>(StringID::STR_ONBOARD_CTRL_DESC_6) && a0 != static_cast<int>(StringID::STR_ONBOARD_SD_TITLE) && a0 != static_cast<int>(StringID::STR_ONBOARD_SD_OK) && a0 != static_cast<int>(StringID::STR_ONBOARD_SD_MISSING) && a0 != static_cast<int>(StringID::STR_ONBOARD_DONE_TITLE) && a0 != static_cast<int>(StringID::STR_ONBOARD_DONE_HINT) && a0 != static_cast<int>(StringID::STR_ONBOARD_SUM_LANG) && a0 != static_cast<int>(StringID::STR_ONBOARD_SUM_WIFI) && a0 != static_cast<int>(StringID::STR_ONBOARD_SUM_CLOCK) && a0 != static_cast<int>(StringID::STR_ONBOARD_SKIPPED) && a0 != static_cast<int>(StringID::STR_COMM_INIT_FAILED)) return "";
  return pool_borrowed(I18n::get(static_cast<StringID>(a0)));
}

const char* pm_i18n_month_name(int a0) {
  return pool_borrowed(I18n::monthName(a0));
}

const char* pm_i18n_day_name(int a0) {
  return pool_borrowed(I18n::dayName(a0));
}

const char* pm_i18n_app_name(int a0) {
  return pool_borrowed(I18n::appName(a0));
}

const char* pm_i18n_kb_app_name(int a0) {
  return pool_borrowed(I18n::kbAppName(a0));
}

const char* pm_i18n_normalize_command(const char* a0) {
  return pool_string(I18n::normalizeCommand(String(a0)));
}

bool pm_image_probe_region(const char* a0, struct pm_image_info* a1, uint32_t a2, uint32_t a3) {
  if (!a1) return {};
  ImageInfo native_a1{};
  if (!P_IMAGE.probe(a0, native_a1, a2, a3)) return false;
  a1->width = native_a1.width;
  a1->height = native_a1.height;
  a1->format = static_cast<int>(native_a1.format);
  return true;
}

bool pm_image_decode(const char* a0, uint8_t* a1, int a2, int a3, int a4, int a5, int a6, bool a7) {
  return P_IMAGE.decodeTo1Bit(a0, a1, a2, a3, a4, a5, a6, a7);
}

bool pm_image_decode_region_to_1bit(const char* a0, uint32_t a1, uint32_t a2, uint8_t* a3, int a4, int a5, int a6, int a7, int a8, bool a9) {
  return P_IMAGE.decodeRegionTo1Bit(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

bool pm_image_decode_from_file(const char* a0, uint8_t* a1, int a2, int a3, int a4, int a5, int a6, bool a7) {
  return P_IMAGE.decodeFromFile(a0, a1, a2, a3, a4, a5, a6, a7);
}

void pm_image_clear(void) {
  P_IMAGE.clear();
}

int pm_io_split_string_count(const char* a0, char a1) {
  return static_cast<int>(static_cast<int>(splitString(String(a0), a1).size()));
}

int pm_io_split_string_get(const char* a0, char a1, int index, char* out, size_t out_size) {
  { const auto& v = splitString(String(a0), a1);
    if (index < 0 || static_cast<size_t>(index) >= v.size()) return -1;
    const String& s = v[index];
    if (out_size == 0) return 0;
    const size_t n = s.length() < out_size - 1 ? s.length() : out_size - 1;
    memcpy(out, s.c_str(), n);
    out[n] = 0;
    return static_cast<int>(n); }
}

const char* pm_io_join_string(const char* const* a0_items, int a0_count, char a1) {
  return pool_string(joinString(std::vector<String>(a0_items, a0_items + a0_count), a1));
}

const char* pm_io_remove_char(const char* a0, char a1) {
  return pool_string(removeChar(String(a0), a1));
}

int pm_io_string_to_int(const char* a0, int a1) {
  return static_cast<int>(stringToInt(String(a0), a1));
}

void pm_kb_set_keyboard_state(int a0) {
  KB().setKeyboardState(a0);
}

int pm_kb_state(void) {
  return static_cast<int>(KB().getKeyboardState());
}

void pm_kb_toggle_shift(void) {
  KB().toggleShift();
}

void pm_kb_toggle_fn(void) {
  KB().toggleFn();
}

bool pm_kb_accept_key(void) {
  return KB().acceptKey();
}

char pm_kb_read(void) {
  return KB().updateKeypress();
}

void pm_kb_check_usbkb(void) {
  KB().checkUSBKB();
}

void pm_kb_flush(void) {
  KB().flush();
}

void pm_kb_set_tca8418_event(void) {
  KB().setTCA8418Event();
}

int pm_layout_eink_row_pitch(int a0) {
  if (a0 != static_cast<int>(FontStyle::Tiny) && a0 != static_cast<int>(FontStyle::Body) && a0 != static_cast<int>(FontStyle::BodyBold) && a0 != static_cast<int>(FontStyle::BodyItalic) && a0 != static_cast<int>(FontStyle::BodyBoldItalic) && a0 != static_cast<int>(FontStyle::Medium) && a0 != static_cast<int>(FontStyle::Small) && a0 != static_cast<int>(FontStyle::BodyNarrow) && a0 != static_cast<int>(FontStyle::Mono) && a0 != static_cast<int>(FontStyle::MonoBold) && a0 != static_cast<int>(FontStyle::MonoItalic) && a0 != static_cast<int>(FontStyle::MonoBoldItalic) && a0 != static_cast<int>(FontStyle::Sans) && a0 != static_cast<int>(FontStyle::SansBold) && a0 != static_cast<int>(FontStyle::SansItalic) && a0 != static_cast<int>(FontStyle::SansBoldItalic) && a0 != static_cast<int>(FontStyle::Caption) && a0 != static_cast<int>(FontStyle::Heading3) && a0 != static_cast<int>(FontStyle::Heading2) && a0 != static_cast<int>(FontStyle::Heading1) && a0 != static_cast<int>(FontStyle::Large) && a0 != static_cast<int>(FontStyle::OledWord) && a0 != static_cast<int>(FontStyle::Terminal) && a0 != static_cast<int>(FontStyle::TerminalBig) && a0 != static_cast<int>(FontStyle::ClockDigit)) return {};
  return static_cast<int>(einkRowPitch(static_cast<FontStyle>(a0)));
}

size_t pm_layout_slice_that_fits(const char* a0, size_t a1, int a2, int a3) {
  if (a3 != static_cast<int>(FontStyle::Tiny) && a3 != static_cast<int>(FontStyle::Body) && a3 != static_cast<int>(FontStyle::BodyBold) && a3 != static_cast<int>(FontStyle::BodyItalic) && a3 != static_cast<int>(FontStyle::BodyBoldItalic) && a3 != static_cast<int>(FontStyle::Medium) && a3 != static_cast<int>(FontStyle::Small) && a3 != static_cast<int>(FontStyle::BodyNarrow) && a3 != static_cast<int>(FontStyle::Mono) && a3 != static_cast<int>(FontStyle::MonoBold) && a3 != static_cast<int>(FontStyle::MonoItalic) && a3 != static_cast<int>(FontStyle::MonoBoldItalic) && a3 != static_cast<int>(FontStyle::Sans) && a3 != static_cast<int>(FontStyle::SansBold) && a3 != static_cast<int>(FontStyle::SansItalic) && a3 != static_cast<int>(FontStyle::SansBoldItalic) && a3 != static_cast<int>(FontStyle::Caption) && a3 != static_cast<int>(FontStyle::Heading3) && a3 != static_cast<int>(FontStyle::Heading2) && a3 != static_cast<int>(FontStyle::Heading1) && a3 != static_cast<int>(FontStyle::Large) && a3 != static_cast<int>(FontStyle::OledWord) && a3 != static_cast<int>(FontStyle::Terminal) && a3 != static_cast<int>(FontStyle::TerminalBig) && a3 != static_cast<int>(FontStyle::ClockDigit)) return {};
  return sliceThatFits(a0, a1, a2, static_cast<FontStyle>(a3));
}

const char* pm_layout_truncate_with_ellipsis(const char* a0, int a1, int a2, int a3) {
  if (a2 != static_cast<int>(FontStyle::Tiny) && a2 != static_cast<int>(FontStyle::Body) && a2 != static_cast<int>(FontStyle::BodyBold) && a2 != static_cast<int>(FontStyle::BodyItalic) && a2 != static_cast<int>(FontStyle::BodyBoldItalic) && a2 != static_cast<int>(FontStyle::Medium) && a2 != static_cast<int>(FontStyle::Small) && a2 != static_cast<int>(FontStyle::BodyNarrow) && a2 != static_cast<int>(FontStyle::Mono) && a2 != static_cast<int>(FontStyle::MonoBold) && a2 != static_cast<int>(FontStyle::MonoItalic) && a2 != static_cast<int>(FontStyle::MonoBoldItalic) && a2 != static_cast<int>(FontStyle::Sans) && a2 != static_cast<int>(FontStyle::SansBold) && a2 != static_cast<int>(FontStyle::SansItalic) && a2 != static_cast<int>(FontStyle::SansBoldItalic) && a2 != static_cast<int>(FontStyle::Caption) && a2 != static_cast<int>(FontStyle::Heading3) && a2 != static_cast<int>(FontStyle::Heading2) && a2 != static_cast<int>(FontStyle::Heading1) && a2 != static_cast<int>(FontStyle::Large) && a2 != static_cast<int>(FontStyle::OledWord) && a2 != static_cast<int>(FontStyle::Terminal) && a2 != static_cast<int>(FontStyle::TerminalBig) && a2 != static_cast<int>(FontStyle::ClockDigit)) return "";
  if (a3 != static_cast<int>(DisplayTarget::OLED) && a3 != static_cast<int>(DisplayTarget::EINK)) return "";
  return pool_string(truncateWithEllipsis(String(a0), a1, static_cast<FontStyle>(a2), static_cast<DisplayTarget>(a3)));
}

int pm_layout_word_wrap_count(const char* a0, int a1, int a2) {
  return static_cast<int>(static_cast<int>(wordWrap(String(a0), a1, static_cast<FontStyle>(a2)).size()));
}

int pm_layout_word_wrap_get(const char* a0, int a1, int a2, int index, char* out, size_t out_size) {
  { const auto& v = wordWrap(String(a0), a1, static_cast<FontStyle>(a2));
    if (index < 0 || static_cast<size_t>(index) >= v.size()) return -1;
    const String& s = v[index];
    if (out_size == 0) return 0;
    const size_t n = s.length() < out_size - 1 ? s.length() : out_size - 1;
    memcpy(out, s.c_str(), n);
    out[n] = 0;
    return static_cast<int>(n); }
}

bool pm_media_probe(const char* a0, struct pm_media_info* a1) {
  if (!a1) return {};
  MediaInfo native_a1{};
  if (!P_MEDIA.probe(a0, native_a1)) return false;
  memcpy(a1->title, native_a1.title, sizeof(a1->title));
  memcpy(a1->artist, native_a1.artist, sizeof(a1->artist));
  memcpy(a1->album, native_a1.album, sizeof(a1->album));
  a1->durationMs = native_a1.durationMs;
  a1->audioOffset = native_a1.audioOffset;
  a1->coverOffset = native_a1.coverOffset;
  a1->coverLength = native_a1.coverLength;
  a1->coverFormat = static_cast<int>(native_a1.coverFormat);
  return true;
}

void pm_oled_init(void) {
  OLED().init();
}

void pm_oled_send(void) {
  OLED().send();
}

void pm_oled_clear(void) {
  OLED().clear();
}

int pm_oled_width(void) {
  return static_cast<int>(OLED().width());
}

int pm_oled_height(void) {
  return static_cast<int>(OLED().height());
}

void pm_oled_box(int a0, int a1, int a2, int a3, bool a4) {
  OLED().box(a0, a1, a2, a3, a4);
}

void pm_oled_triangle(int a0, int a1, int a2, int a3, int a4, int a5, bool a6) {
  OLED().triangle(a0, a1, a2, a3, a4, a5, a6);
}

void pm_oled_oled_word(const char* a0, bool a1, bool a2, const char* a3) {
  OLED().oledWord(String(a0), a1, a2, String(a3));
}

void pm_oled_oled_line(const char* a0, int a1, bool a2, const char* a3, bool a4) {
  OLED().oledLine(String(a0), a1, a2, String(a3), a4);
}

void pm_oled_sysmsg(const char* a0, int a1) {
  OLED().sysMessage(String(a0), a1);
}

void pm_oled_oled_scroll(void) {
  OLED().oledScroll();
}

void pm_oled_info_bar(void) {
  OLED().infoBar();
}

void pm_oled_set_power_save(bool a0) {
  OLED().setPowerSave(a0);
}

bool pm_oled_power_save(void) {
  return OLED().getPowerSave();
}

void pm_sd_set_mode(int a0) {
  if (a0 != static_cast<int>(PocketmageSD::Mode::SDMMC) && a0 != static_cast<int>(PocketmageSD::Mode::SDSPI)) return;
  PM_SD().setMode(static_cast<PocketmageSD::Mode>(a0));
}

int pm_sd_get_mode(void) {
  return static_cast<int>(PM_SD().getMode());
}

void pm_sd_save_file(void) {
  PM_SD().saveFile();
}

void pm_sd_write_metadata(const char* a0) {
  PM_SD().writeMetadata(a0);
}

void pm_sd_load_file(bool a0) {
  PM_SD().loadFile(a0);
}

void pm_sd_del_file(const char* a0) {
  PM_SD().delFile(a0);
}

void pm_sd_delete_metadata(const char* a0) {
  PM_SD().deleteMetadata(a0);
}

void pm_sd_ren_file(const char* a0, const char* a1) {
  PM_SD().renFile(a0, a1);
}

void pm_sd_ren_metadata(const char* a0, const char* a1) {
  PM_SD().renMetadata(a0, a1);
}

void pm_sd_copy_file(const char* a0, const char* a1) {
  PM_SD().copyFile(a0, a1);
}

void pm_sd_append_to_file(const char* a0, const char* a1) {
  PM_SD().appendToFile(a0, a1);
}

void pm_sd_begin_io(void) {
  PM_SD().beginIO();
}

void pm_sd_end_io(void) {
  PM_SD().endIO();
}

bool pm_sd_get_no_sd(void) {
  return PM_SD().getNoSD();
}

void pm_sd_set_no_sd(bool a0) {
  PM_SD().setNoSD(a0);
}

const char* pm_sd_get_working_file(void) {
  return pool_string(PM_SD().getWorkingFile());
}

void pm_sd_set_working_file(const char* a0) {
  PM_SD().setWorkingFile(String(a0));
}

const char* pm_sd_get_editing_file(void) {
  return pool_string(PM_SD().getEditingFile());
}

void pm_sd_set_editing_file(const char* a0) {
  PM_SD().setEditingFile(String(a0));
}

const char* pm_sd_get_files_list_index(int a0) {
  return pool_string(PM_SD().getFilesListIndex(a0));
}

void pm_sd_set_files_list_index(int a0, const char* a1) {
  PM_SD().setFilesListIndex(a0, String(a1));
}

void pm_sd_list_dir(const char* a0) {
  PM_SD().listDir(a0);
}

void pm_sd_read_file(const char* a0) {
  PM_SD().readFile(a0);
}

const char* pm_sd_read_file_to_string(const char* a0) {
  return pool_string(PM_SD().readFileToString(a0));
}

void pm_sd_write_file(const char* a0, const char* a1) {
  PM_SD().writeFile(a0, a1);
}

void pm_sd_append_file(const char* a0, const char* a1) {
  PM_SD().appendFile(a0, a1);
}

void pm_sd_rename_file(const char* a0, const char* a1) {
  PM_SD().renameFile(a0, a1);
}

void pm_sd_delete_file(const char* a0) {
  PM_SD().deleteFile(a0);
}

bool pm_sd_read_binary_file(const char* a0, uint8_t* a1, size_t a2) {
  return PM_SD().readBinaryFile(a0, a1, a2);
}

bool pm_sd_append_binary_file(const char* a0, const uint8_t* a1, size_t a2) {
  return PM_SD().appendBinaryFile(a0, a1, a2);
}

size_t pm_sd_get_file_size(const char* a0) {
  return PM_SD().getFileSize(a0);
}

bool pm_sd_exists(const char* a0) {
  return PM_SD().exists(a0);
}

bool pm_sd_stat_is_dir(const char* a0) {
  return PM_SD().statIsDir(a0);
}

bool pm_sd_stat_path(const char* a0, size_t* a1, bool* a2) {
  return PM_SD().statPath(a0, a1, a2);
}

bool pm_sd_mkdir(const char* a0) {
  return PM_SD().mkdirPath(a0);
}

bool pm_sd_remove_file(const char* a0) {
  return PM_SD().removeFile(a0);
}

bool pm_sd_remove_tree(const char* a0) {
  return PM_SD().removeTree(a0);
}

bool pm_sd_rename_path(const char* a0, const char* a1) {
  return PM_SD().renamePath(a0, a1);
}

bool pm_sd_copy_file_raw(const char* a0, const char* a1) {
  return PM_SD().copyFileRaw(a0, a1);
}

bool pm_sd_write_binary_file(const char* a0, const uint8_t* a1, size_t a2) {
  return PM_SD().writeBinaryFile(a0, a1, a2);
}

int pm_sd_stream_open_mode(const char* a0, char a1) {
  return static_cast<int>(PM_SD().streamOpen(a0, a1));
}

size_t pm_sd_stream_read(int a0, void* a1, size_t a2) {
  return PM_SD().streamRead(a0, a1, a2);
}

size_t pm_sd_stream_write(int a0, const void* a1, size_t a2) {
  return PM_SD().streamWrite(a0, a1, a2);
}

bool pm_sd_stream_seek_from(int a0, long a1, int a2) {
  return PM_SD().streamSeek(a0, a1, a2);
}

size_t pm_sd_stream_tell(int a0) {
  return PM_SD().streamTell(a0);
}

size_t pm_sd_stream_size(int a0) {
  return PM_SD().streamSize(a0);
}

bool pm_sd_stream_close_checked(int a0) {
  return PM_SD().streamClose(a0);
}

int pm_sd_dir_open(const char* a0) {
  return static_cast<int>(PM_SD().dirEnumOpen(a0));
}

bool pm_sd_dir_entry(int a0, char* a1, size_t a2, bool* a3) {
  return PM_SD().dirEnumEntry(a0, a1, a2, a3);
}

void pm_sd_dir_close(void) {
  PM_SD().dirEnumClose();
}

const char* pm_sd_sd_path(const char* a0) {
  return pool_borrowed(PM_SD().sdPath(a0));
}

bool pm_sd_sd_exists(const char* a0) {
  return PM_SD().sdExists(a0);
}

bool pm_sd_sd_mkdir(const char* a0) {
  return PM_SD().sdMkdir(a0);
}

bool pm_sd_sd_remove(const char* a0) {
  return PM_SD().sdRemove(a0);
}

bool pm_sd_sd_rename(const char* a0, const char* a1) {
  return PM_SD().sdRename(a0, a1);
}

bool pm_sd_sd_get_file_info(const char* a0, size_t* a1) {
  return PM_SD().sdGetFileInfo(a0, a1);
}

void pm_sys_set_cpu_speed(int a0) {
  pocketmage::setCpuSpeed(a0);
}

void pm_activity(void) {
  pocketmage::reportActivity();
}

void pm_sys_deep_sleep(bool a0) {
  pocketmage::deepSleep(a0);
}

void pm_touch_update_scroll_from_touch(void) {
  TOUCH().updateScrollFromTouch();
}

bool pm_touch_update_scroll(int a0, unsigned long* a1, int a2) {
  if (!a1) return {};
  return TOUCH().updateScroll(a0, *a1, a2);
}

int pm_touch_get_scroll_vector(void) {
  return static_cast<int>(TOUCH().getScrollVector());
}

long pm_touch_get_dynamic_scroll(void) {
  return TOUCH().getDynamicScroll();
}

void pm_touch_set_dynamic_scroll(long a0) {
  TOUCH().setDynamicScroll(a0);
}

void pm_touch_set_prev_dynamic_scroll(long a0) {
  TOUCH().setPrevDynamicScroll(a0);
}

void pm_touch_set_last_touch(int a0) {
  TOUCH().setLastTouch(a0);
}

void pm_touch_set_last_touch_time(unsigned long a0) {
  TOUCH().setLastTouchTime(a0);
}

void pm_touch_reset_last_touch(void) {
  TOUCH().resetLastTouch();
}

long pm_touch_get_prev_dynamic_scroll(void) {
  return TOUCH().getPrevDynamicScroll();
}

int pm_touch_get_last_touch(void) {
  return static_cast<int>(TOUCH().getLastTouch());
}

unsigned long pm_touch_get_last_touch_time(void) {
  return TOUCH().getLastTouchTime();
}

int pm_touch_get_diff(void) {
  return static_cast<int>(TOUCH().getDiff());
}

void pm_ui_draw_transport_icon(int a0, int a1, int a2, int a3, int a4) {
  if (a0 != static_cast<int>(DisplayTarget::OLED) && a0 != static_cast<int>(DisplayTarget::EINK)) return;
  if (a4 != static_cast<int>(TransportIcon::Play) && a4 != static_cast<int>(TransportIcon::Pause) && a4 != static_cast<int>(TransportIcon::Stop)) return;
  drawTransportIcon(static_cast<DisplayTarget>(a0), a1, a2, a3, static_cast<TransportIcon>(a4));
}

void pm_ui_draw_scrollbar(int a0, int a1, int a2, int a3, int a4, int a5, int a6, bool a7, uint16_t a8, uint16_t a9) {
  drawScrollbar(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

void pm_ui_begin_eink_screen(bool a0) {
  beginEinkScreen(a0);
}

void pm_ui_end_eink_screen(const char* a0, int a1) {
  if (a1 != static_cast<int>(EinkRefresh::Normal) && a1 != static_cast<int>(EinkRefresh::ForceFull)) return;
  endEinkScreen(a0, static_cast<EinkRefresh>(a1));
}

void pm_ui_draw_list_item(int a0, int a1, const char* a2, int a3) {
  drawListItem(a0, a1, String(a2), a3);
}

int pm_ui_draw_chip_text(int a0, int a1, const char* a2, int a3, int a4, bool a5, int a6, int a7, int a8, int a9) {
  if (a3 != static_cast<int>(FontStyle::Tiny) && a3 != static_cast<int>(FontStyle::Body) && a3 != static_cast<int>(FontStyle::BodyBold) && a3 != static_cast<int>(FontStyle::BodyItalic) && a3 != static_cast<int>(FontStyle::BodyBoldItalic) && a3 != static_cast<int>(FontStyle::Medium) && a3 != static_cast<int>(FontStyle::Small) && a3 != static_cast<int>(FontStyle::BodyNarrow) && a3 != static_cast<int>(FontStyle::Mono) && a3 != static_cast<int>(FontStyle::MonoBold) && a3 != static_cast<int>(FontStyle::MonoItalic) && a3 != static_cast<int>(FontStyle::MonoBoldItalic) && a3 != static_cast<int>(FontStyle::Sans) && a3 != static_cast<int>(FontStyle::SansBold) && a3 != static_cast<int>(FontStyle::SansItalic) && a3 != static_cast<int>(FontStyle::SansBoldItalic) && a3 != static_cast<int>(FontStyle::Caption) && a3 != static_cast<int>(FontStyle::Heading3) && a3 != static_cast<int>(FontStyle::Heading2) && a3 != static_cast<int>(FontStyle::Heading1) && a3 != static_cast<int>(FontStyle::Large) && a3 != static_cast<int>(FontStyle::OledWord) && a3 != static_cast<int>(FontStyle::Terminal) && a3 != static_cast<int>(FontStyle::TerminalBig) && a3 != static_cast<int>(FontStyle::ClockDigit)) return {};
  return static_cast<int>(drawChipText(a0, a1, String(a2), static_cast<FontStyle>(a3), a4, a5, a6, a7, a8, a9));
}

void pm_wifi_begin(void) {
  P_WIFI.begin();
}

void pm_wifi_stop(void) {
  P_WIFI.stop();
}

void pm_wifi_enable(void) {
  P_WIFI.enable();
}

void pm_wifi_disable(void) {
  P_WIFI.disable();
}

void pm_wifi_scan(void) {
  P_WIFI.scan();
}

void pm_wifi_connect(const char* a0, const char* a1, bool a2) {
  P_WIFI.connect(a0, a1, a2);
}

void pm_wifi_disconnect(void) {
  P_WIFI.disconnect();
}

void pm_wifi_reconnect(void) {
  P_WIFI.reconnect();
}

int pm_wifi_get_state(void) {
  return static_cast<int>(P_WIFI.getState());
}

bool pm_wifi_is_connected(void) {
  return P_WIFI.isConnected();
}

bool pm_wifi_is_scanning(void) {
  return P_WIFI.isScanning();
}

const char* pm_wifi_get_status_message(void) {
  return pool_string(P_WIFI.getStatusMessage());
}

const char* pm_wifi_get_connected_ssid(void) {
  return pool_string(P_WIFI.getConnectedSSID());
}

const char* pm_wifi_get_ip_address(void) {
  return pool_string(P_WIFI.getIpAddress());
}

int pm_wifi_get_rssi(void) {
  return static_cast<int>(P_WIFI.getRssi());
}

const char* pm_wifi_get_last_error(void) {
  return pool_string(P_WIFI.getLastError());
}

uint16_t pm_wifi_get_scan_result_count(void) {
  return P_WIFI.getScanResultCount();
}

bool pm_wifi_has_saved_credentials(const char* a0) {
  return P_WIFI.hasSavedCredentials(a0);
}

bool pm_wifi_load_saved_credentials(const char* a0, char* a1, size_t a2) {
  return P_WIFI.loadSavedCredentials(a0, a1, a2);
}

void pm_wifi_clear_saved_credentials(const char* a0) {
  P_WIFI.clearSavedCredentials(a0);
}

void pm_wifi_dispatch_events(void) {
  P_WIFI.dispatchEvents();
}

}  // extern "C"
