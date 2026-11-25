#pragma once

/** Panic capture into RTC memory and report export after reboot. */
namespace pm_crash {

/** Install the panic hook and check the retained record. Call once from setup(),
 * before PocketMage_INIT. Requires the -Wl,--wrap=esp_panic_handler linker flag.
 */
void begin();
/** Sample OS state, save a pending report to /logs and replay it on serial reconnect.
 * Call from the OS loop after PocketMage_INIT. A failed save retries after five
 * seconds. May block on SD I/O.
 */
void poll();

}  // namespace pm_crash
