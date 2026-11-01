// App-facing C ABI for external ELF apps. Regenerate, do not edit.
// String returns stay valid for the next POOL_SLOTS string-returning calls.

#ifndef PM_SDK_APP_H
#define PM_SDK_APP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Bump on any incompatible change here. Apps compare at startup.
#define PM_APP_API_ABI 2u

// Mirrors DisplayTarget, values read from the SDK header.
enum pm_target {
  PM_TARGET_OLED = 0,
  PM_TARGET_EINK = 1,
};

// Mirrors FontStyle, values read from the SDK header.
enum pm_style {
  PM_STYLE_TINY = 0,
  PM_STYLE_BODY = 1,
  PM_STYLE_BODY_BOLD = 2,
  PM_STYLE_BODY_ITALIC = 3,
  PM_STYLE_BODY_BOLD_ITALIC = 4,
  PM_STYLE_MEDIUM = 5,
  PM_STYLE_SMALL = 6,
  PM_STYLE_BODY_NARROW = 7,
  PM_STYLE_MONO = 8,
  PM_STYLE_MONO_BOLD = 9,
  PM_STYLE_MONO_ITALIC = 10,
  PM_STYLE_MONO_BOLD_ITALIC = 11,
  PM_STYLE_SANS = 12,
  PM_STYLE_SANS_BOLD = 13,
  PM_STYLE_SANS_ITALIC = 14,
  PM_STYLE_SANS_BOLD_ITALIC = 15,
  PM_STYLE_CAPTION = 16,
  PM_STYLE_HEADING3 = 17,
  PM_STYLE_HEADING2 = 18,
  PM_STYLE_HEADING1 = 19,
  PM_STYLE_LARGE = 20,
  PM_STYLE_OLED_WORD = 21,
  PM_STYLE_TERMINAL = 22,
  PM_STYLE_TERMINAL_BIG = 23,
  PM_STYLE_CLOCK_DIGIT = 24,
  PM_STYLE_COUNT = 25,
};

// Mirrors Lang, values read from the SDK header.
enum pm_lang {
  PM_LANG_ENGLISH = 0,
  PM_LANG_FRENCH = 1,
  PM_LANG_SPANISH = 2,
  PM_LANG_GERMAN = 3,
  PM_LANG_COUNT = 4,
};

// Mirrors EinkRefresh, values read from the SDK header.
enum pm_refresh {
  PM_REFRESH_NORMAL = 0,
  PM_REFRESH_FORCE_FULL = 1,
};

// Mirrors Jingles, values read from the SDK header.
enum pm_jingle {
  PM_JINGLE_STARTUP = 0,
  PM_JINGLE_SHUTDOWN = 1,
  PM_JINGLE_COUNT = 2,
};

// version

// ABI identifier of the running host.
uint32_t pm_app_abi(void);

// SDK version string the running host was built with.
const char* pm_host_sdk_version(void);

// Declared here, not via pocketmage_globals.h: the export table pulls
// the address and globals.h clashes with its bare newlib prototypes.
#ifdef __cplusplus
extern "C" {
#endif
extern const char pocketmage_sdk_version[];
#ifdef __cplusplus
}
#endif

// Host primitives exported from the firmware core.
#ifdef __cplusplus
extern "C" {
#endif
void delay(unsigned long ms);
#ifdef __cplusplus
}  // extern "C"
#endif

// bz

bool pm_bz_begin(int a0);
void pm_bz_end();
// Defined by hand in pm_app_api.cpp.
void pm_bz_play_jingle(int a0);

// clock

bool pm_clock_begin();
void pm_clock_set_time_from_string(const char* a0);
// Defined by hand in pm_app_api.cpp.
bool pm_clock_valid();

// eink

// Defined by hand in pm_app_api.cpp.
void pm_eink_refresh();
// Defined by hand in pm_app_api.cpp.
void pm_eink_set_fast_full_refresh(bool a0);
void pm_eink_status_bar(const char* a0, bool a1);
void pm_eink_draw_status_bar(const char* a0);
void pm_eink_reset_display(bool a0, uint16_t a1);
int pm_eink_count_lines(const char* a0, size_t a1);
uint8_t pm_eink_get_font_height();
int pm_eink_max_lines();
uint16_t pm_eink_get_eink_text_width(const char* a0);
uint8_t pm_eink_get_line_spacing();
void pm_eink_force_slow_full_update(bool a0);

// font_engine

int pm_font_engine_char_width(int a0, uint16_t a1, int a2);
int pm_font_engine_font_ascent(int a0, int a1);
int pm_font_engine_font_descent(int a0, int a1);
int pm_font_engine_font_height_txt(uint8_t a0, uint8_t a1, uint8_t a2);

// i18n

void pm_i18n_set_language(int a0);
bool pm_i18n_set_language_by_code(const char* a0);
int pm_i18n_language();
int pm_i18n_language_count();
const char* pm_i18n_code();
const char* pm_i18n_code_at(int a0);
const char* pm_i18n_native_name();
const char* pm_i18n_native_name_at(int a0);
const char* pm_i18n_get(int a0);
const char* pm_i18n_month_name(int a0);
const char* pm_i18n_day_name(int a0);
const char* pm_i18n_app_name(int a0);
const char* pm_i18n_kb_app_name(int a0);
const char* pm_i18n_normalize_command(const char* a0);

// kb

// Defined by hand in pm_app_api.cpp.
int pm_kb_state();
void pm_kb_toggle_shift();
void pm_kb_toggle_fn();
bool pm_kb_accept_key();
// Defined by hand in pm_app_api.cpp.
char pm_kb_read();
void pm_kb_check_usbkb();
void pm_kb_flush();

// misc

// Defined by hand in pm_app_api.cpp.
int pm_text_width(int a0, const char* a1, int a2);
// Defined by hand in pm_app_api.cpp.
int pm_font_height(int a0, int a1);
// Defined by hand in pm_app_api.cpp.
void pm_text_color(int a0, uint16_t a1);
int pm_io_split_string_count(const char* a0, char a1);
int pm_io_split_string_get(const char* a0, char a1, int index, char* out, size_t out_size);
const char* pm_io_join_string(const char* const* a0_items, int a0_count, char a1);
const char* pm_io_remove_char(const char* a0, char a1);
int pm_io_string_to_int(const char* a0, int a1);
int pm_layout_eink_row_pitch(int a0);
size_t pm_layout_slice_that_fits(const char* a0, size_t a1, int a2, int a3);
const char* pm_layout_truncate_with_ellipsis(const char* a0, int a1, int a2, int a3);
int pm_layout_word_wrap_count(const char* a0, int a1, int a2);
int pm_layout_word_wrap_get(const char* a0, int a1, int a2, int index, char* out, size_t out_size);
void pm_ui_draw_scrollbar(int a0, int a1, int a2, int a3, int a4, int a5, int a6, bool a7, uint16_t a8, uint16_t a9);
void pm_ui_begin_eink_screen(bool a0);
void pm_ui_end_eink_screen(const char* a0, int a1);
void pm_ui_draw_list_item(int a0, int a1, const char* a2, int a3);
int pm_ui_draw_chip_text(int a0, int a1, const char* a2, int a3, int a4, bool a5, int a6, int a7, int a8, int a9);

// oled

void pm_oled_oled_word(const char* a0, bool a1, bool a2, const char* a3);
void pm_oled_oled_line(const char* a0, int a1, bool a2, const char* a3, bool a4);
// Defined by hand in pm_app_api.cpp.
void pm_oled_sysmsg(const char* a0, int a1);
void pm_oled_oled_scroll();
void pm_oled_info_bar();
// Defined by hand in pm_app_api.cpp.
void pm_oled_set_power_save(bool a0);
// Defined by hand in pm_app_api.cpp.
bool pm_oled_power_save();

// sd

int pm_sd_get_mode();
void pm_sd_save_file();
void pm_sd_write_metadata(const char* a0);
void pm_sd_load_file(bool a0);
void pm_sd_del_file(const char* a0);
void pm_sd_delete_metadata(const char* a0);
void pm_sd_ren_file(const char* a0, const char* a1);
void pm_sd_ren_metadata(const char* a0, const char* a1);
void pm_sd_copy_file(const char* a0, const char* a1);
void pm_sd_append_to_file(const char* a0, const char* a1);
void pm_sd_begin_io();
void pm_sd_end_io();
bool pm_sd_get_no_sd();
const char* pm_sd_get_working_file();
const char* pm_sd_get_editing_file();
const char* pm_sd_get_files_list_index(int a0);
void pm_sd_list_dir(const char* a1);
void pm_sd_read_file(const char* a1);
const char* pm_sd_read_file_to_string(const char* a1);
void pm_sd_write_file(const char* a1, const char* a2);
void pm_sd_append_file(const char* a1, const char* a2);
void pm_sd_rename_file(const char* a1, const char* a2);
void pm_sd_delete_file(const char* a1);
bool pm_sd_read_binary_file(const char* a0, uint8_t* a1, size_t a2);
size_t pm_sd_get_file_size(const char* a0);

// touch

void pm_touch_update_scroll_from_touch();
bool pm_touch_update_scroll(int a0, unsigned long* a1, int a2);
int pm_touch_get_scroll_vector();
long pm_touch_get_dynamic_scroll();
long pm_touch_get_prev_dynamic_scroll();
int pm_touch_get_last_touch();
unsigned long pm_touch_get_last_touch_time();
int pm_touch_get_diff();

// wifi

void pm_wifi_begin();
void pm_wifi_stop();
void pm_wifi_enable();
void pm_wifi_disable();
void pm_wifi_scan();
void pm_wifi_connect(const char* a0, const char* a1, bool a2);
void pm_wifi_disconnect();
void pm_wifi_reconnect();
int pm_wifi_get_state();
bool pm_wifi_is_connected();
bool pm_wifi_is_scanning();
const char* pm_wifi_get_status_message();
const char* pm_wifi_get_connected_ssid();
const char* pm_wifi_get_ip_address();
int pm_wifi_get_rssi();
const char* pm_wifi_get_last_error();
uint16_t pm_wifi_get_scan_result_count();
bool pm_wifi_has_saved_credentials(const char* a0);
bool pm_wifi_load_saved_credentials(const char* a0, char* a1, size_t a2);
void pm_wifi_clear_saved_credentials(const char* a0);
void pm_wifi_dispatch_events();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // PM_SDK_APP_H
