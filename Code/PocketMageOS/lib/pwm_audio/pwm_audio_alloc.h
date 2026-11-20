#pragma once

/*
 * Force-included into the vendored pwm_audio.c build only. When the ring
 * allocation fails, rb_create() frees its uninitialized rb->buf, so zero every
 * allocation. Drop this once the pinned esp-iot-solution fixes rb_create().
 */
#include <esp_heap_caps.h>

#define heap_caps_malloc(size, caps) heap_caps_calloc(1, (size), (caps))
