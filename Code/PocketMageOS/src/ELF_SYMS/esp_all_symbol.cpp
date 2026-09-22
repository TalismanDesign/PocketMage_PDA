/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Generated from the curated PocketMage SDK export list.
 * DO NOT EDIT: regenerate with tools/symbols.py.
 */

#include <stddef.h>

#include "private/elf_symbol.h"

#include <pocketmage_oled/pocketmage_oled.h>
#include <pocketmage_kb/pocketmage_kb.h>
#include <pocketmage_eink/pocketmage_eink.h>
#include <pocketmage_bz/pocketmage_bz.h>
#include <pocketmage_clock/pocketmage_clock.h>
#include <pocketmage_sd/pocketmage_sd.h>

/* Available ELF symbols table: g_customer_elfsyms */
/* C linkage: the loader core is C and references this unmangled. */
/* Explicit extern: namespace-scope const defaults to internal
 * linkage in C++, which would let the compiler discard the table. */

extern "C" {

extern const struct esp_elfsym g_customer_elfsyms[] = {
    ESP_ELFSYM_EXPORT(OLED),
    ESP_ELFSYM_EXPORT(KB),
    ESP_ELFSYM_EXPORT(EINK),
    ESP_ELFSYM_EXPORT(BZ),
    ESP_ELFSYM_EXPORT(CLOCK),
    ESP_ELFSYM_EXPORT(PM_SDAUTO),
    ESP_ELFSYM_END
};
}
