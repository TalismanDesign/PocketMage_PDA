// Host wrappers backing pm_sdk_app.h. Regenerate, do not edit.
// Each wrapper is extern "C", so SDK mangling stays local to this file.

#include <globals.h>

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

bool pm_bz_begin(int a0) {
  return BZ().begin(a0);
}

void pm_bz_end() {
  BZ().end();
}

bool pm_clock_begin() {
  return CLOCK().begin();
}

void pm_clock_set_time_from_string(const char* a0) {
  CLOCK().setTimeFromString(String(a0));
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

uint8_t pm_eink_get_font_height() {
  return EINK().getFontHeight();
}

int pm_eink_max_lines() {
  return static_cast<int>(EINK().maxLines());
}

uint16_t pm_eink_get_eink_text_width(const char* a0) {
  return EINK().getEinkTextWidth(String(a0));
}

uint8_t pm_eink_get_line_spacing() {
  return EINK().getLineSpacing();
}

void pm_eink_force_slow_full_update(bool a0) {
  EINK().forceSlowFullUpdate(a0);
}

int pm_font_engine_char_width(int a0, uint16_t a1, int a2) {
  return static_cast<int>(FontEngine::charWidth(static_cast<DisplayTarget>(a0), a1, static_cast<FontStyle>(a2)));
}

int pm_font_engine_font_ascent(int a0, int a1) {
  return static_cast<int>(FontEngine::fontAscent(static_cast<DisplayTarget>(a0), static_cast<FontStyle>(a1)));
}

int pm_font_engine_font_descent(int a0, int a1) {
  return static_cast<int>(FontEngine::fontDescent(static_cast<DisplayTarget>(a0), static_cast<FontStyle>(a1)));
}

int pm_font_engine_font_height_txt(uint8_t a0, uint8_t a1, uint8_t a2) {
  return static_cast<int>(FontEngine::fontHeightTxt(a0, a1, a2));
}

void pm_i18n_set_language(int a0) {
  I18n::setLanguage(static_cast<Lang>(a0));
}

bool pm_i18n_set_language_by_code(const char* a0) {
  return I18n::setLanguageByCode(a0);
}

int pm_i18n_language() {
  return static_cast<int>(I18n::language());
}

int pm_i18n_language_count() {
  return static_cast<int>(I18n::languageCount());
}

const char* pm_i18n_code() {
  return pool_borrowed(I18n::code());
}

const char* pm_i18n_code_at(int a0) {
  return pool_borrowed(I18n::code(a0));
}

const char* pm_i18n_native_name() {
  return pool_borrowed(I18n::nativeName());
}

const char* pm_i18n_native_name_at(int a0) {
  return pool_borrowed(I18n::nativeName(a0));
}

const char* pm_i18n_get(int a0) {
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

void pm_kb_toggle_shift() {
  KB().toggleShift();
}

void pm_kb_toggle_fn() {
  KB().toggleFn();
}

bool pm_kb_accept_key() {
  return KB().acceptKey();
}

void pm_kb_check_usbkb() {
  KB().checkUSBKB();
}

void pm_kb_flush() {
  KB().flush();
}

int pm_layout_eink_row_pitch(int a0) {
  return static_cast<int>(einkRowPitch(static_cast<FontStyle>(a0)));
}

size_t pm_layout_slice_that_fits(const char* a0, size_t a1, int a2, int a3) {
  return sliceThatFits(a0, a1, a2, static_cast<FontStyle>(a3));
}

const char* pm_layout_truncate_with_ellipsis(const char* a0, int a1, int a2, int a3) {
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

void pm_oled_oled_word(const char* a0, bool a1, bool a2, const char* a3) {
  OLED().oledWord(String(a0), a1, a2, String(a3));
}

void pm_oled_oled_line(const char* a0, int a1, bool a2, const char* a3, bool a4) {
  OLED().oledLine(String(a0), a1, a2, String(a3), a4);
}

void pm_oled_oled_scroll() {
  OLED().oledScroll();
}

void pm_oled_info_bar() {
  OLED().infoBar();
}

int pm_sd_get_mode() {
  return static_cast<int>(PM_SD().getMode());
}

void pm_sd_save_file() {
  PM_SD().saveFile();
}

void pm_sd_write_metadata(const char* a0) {
  PM_SD().writeMetadata(String(a0));
}

void pm_sd_load_file(bool a0) {
  PM_SD().loadFile(a0);
}

void pm_sd_del_file(const char* a0) {
  PM_SD().delFile(String(a0));
}

void pm_sd_delete_metadata(const char* a0) {
  PM_SD().deleteMetadata(String(a0));
}

void pm_sd_ren_file(const char* a0, const char* a1) {
  PM_SD().renFile(String(a0), String(a1));
}

void pm_sd_ren_metadata(const char* a0, const char* a1) {
  PM_SD().renMetadata(String(a0), String(a1));
}

void pm_sd_copy_file(const char* a0, const char* a1) {
  PM_SD().copyFile(String(a0), String(a1));
}

void pm_sd_append_to_file(const char* a0, const char* a1) {
  PM_SD().appendToFile(String(a0), String(a1));
}

void pm_sd_begin_io() {
  PM_SD().beginIO();
}

void pm_sd_end_io() {
  PM_SD().endIO();
}

bool pm_sd_get_no_sd() {
  return PM_SD().getNoSD();
}

const char* pm_sd_get_working_file() {
  return pool_string(PM_SD().getWorkingFile());
}

const char* pm_sd_get_editing_file() {
  return pool_string(PM_SD().getEditingFile());
}

const char* pm_sd_get_files_list_index(int a0) {
  return pool_string(PM_SD().getFilesListIndex(a0));
}

void pm_sd_list_dir(const char* a1) {
  PM_SD().listDir(*global_fs, a1);
}

void pm_sd_read_file(const char* a1) {
  PM_SD().readFile(*global_fs, a1);
}

const char* pm_sd_read_file_to_string(const char* a1) {
  return pool_string(PM_SD().readFileToString(*global_fs, a1));
}

void pm_sd_write_file(const char* a1, const char* a2) {
  PM_SD().writeFile(*global_fs, a1, a2);
}

void pm_sd_append_file(const char* a1, const char* a2) {
  PM_SD().appendFile(*global_fs, a1, a2);
}

void pm_sd_rename_file(const char* a1, const char* a2) {
  PM_SD().renameFile(*global_fs, a1, a2);
}

void pm_sd_delete_file(const char* a1) {
  PM_SD().deleteFile(*global_fs, a1);
}

bool pm_sd_read_binary_file(const char* a0, uint8_t* a1, size_t a2) {
  return PM_SD().readBinaryFile(a0, a1, a2);
}

size_t pm_sd_get_file_size(const char* a0) {
  return PM_SD().getFileSize(a0);
}

void pm_touch_update_scroll_from_touch() {
  TOUCH().updateScrollFromTouch();
}

bool pm_touch_update_scroll(int a0, unsigned long* a1, int a2) {
  return TOUCH().updateScroll(a0, *a1, a2);
}

int pm_touch_get_scroll_vector() {
  return static_cast<int>(TOUCH().getScrollVector());
}

long pm_touch_get_dynamic_scroll() {
  return TOUCH().getDynamicScroll();
}

long pm_touch_get_prev_dynamic_scroll() {
  return TOUCH().getPrevDynamicScroll();
}

int pm_touch_get_last_touch() {
  return static_cast<int>(TOUCH().getLastTouch());
}

unsigned long pm_touch_get_last_touch_time() {
  return TOUCH().getLastTouchTime();
}

int pm_touch_get_diff() {
  return static_cast<int>(TOUCH().getDiff());
}

void pm_ui_draw_scrollbar(int a0, int a1, int a2, int a3, int a4, int a5, int a6, bool a7, uint16_t a8, uint16_t a9) {
  drawScrollbar(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

void pm_ui_begin_eink_screen(bool a0) {
  beginEinkScreen(a0);
}

void pm_ui_end_eink_screen(const char* a0, int a1) {
  endEinkScreen(a0, static_cast<EinkRefresh>(a1));
}

void pm_ui_draw_list_item(int a0, int a1, const char* a2, int a3) {
  drawListItem(a0, a1, String(a2), a3);
}

int pm_ui_draw_chip_text(int a0, int a1, const char* a2, int a3, int a4, bool a5, int a6, int a7, int a8, int a9) {
  return static_cast<int>(drawChipText(a0, a1, String(a2), static_cast<FontStyle>(a3), a4, a5, a6, a7, a8, a9));
}

void pm_wifi_begin() {
  P_WIFI.begin();
}

void pm_wifi_stop() {
  P_WIFI.stop();
}

void pm_wifi_enable() {
  P_WIFI.enable();
}

void pm_wifi_disable() {
  P_WIFI.disable();
}

void pm_wifi_scan() {
  P_WIFI.scan();
}

void pm_wifi_connect(const char* a0, const char* a1, bool a2) {
  P_WIFI.connect(a0, a1, a2);
}

void pm_wifi_disconnect() {
  P_WIFI.disconnect();
}

void pm_wifi_reconnect() {
  P_WIFI.reconnect();
}

int pm_wifi_get_state() {
  return static_cast<int>(P_WIFI.getState());
}

bool pm_wifi_is_connected() {
  return P_WIFI.isConnected();
}

bool pm_wifi_is_scanning() {
  return P_WIFI.isScanning();
}

const char* pm_wifi_get_status_message() {
  return pool_string(P_WIFI.getStatusMessage());
}

const char* pm_wifi_get_connected_ssid() {
  return pool_string(P_WIFI.getConnectedSSID());
}

const char* pm_wifi_get_ip_address() {
  return pool_string(P_WIFI.getIpAddress());
}

int pm_wifi_get_rssi() {
  return static_cast<int>(P_WIFI.getRssi());
}

const char* pm_wifi_get_last_error() {
  return pool_string(P_WIFI.getLastError());
}

uint16_t pm_wifi_get_scan_result_count() {
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

void pm_wifi_dispatch_events() {
  P_WIFI.dispatchEvents();
}

}  // extern "C"
