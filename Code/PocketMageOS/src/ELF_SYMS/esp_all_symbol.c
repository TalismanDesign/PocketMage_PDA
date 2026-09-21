/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Generated from the curated PocketMage SDK export list.
 */

#include <stddef.h>

#include "private/elf_symbol.h"

/* Extern declarations from curated export list */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
extern int OLED;
extern int KB;
extern int EINK;
extern int BZ;
extern int CLOCK;
extern int PM_SDAUTO;
#pragma GCC diagnostic pop

/* Available ELF symbols table: g_customer_elfsyms */

const struct esp_elfsym g_customer_elfsyms[] = {
    ESP_ELFSYM_EXPORT(OLED),
    ESP_ELFSYM_EXPORT(KB),
    ESP_ELFSYM_EXPORT(EINK),
    ESP_ELFSYM_EXPORT(BZ),
    ESP_ELFSYM_EXPORT(CLOCK),
    ESP_ELFSYM_EXPORT(PM_SDAUTO),
    ESP_ELFSYM_END
};
