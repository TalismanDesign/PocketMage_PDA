/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org),
 * Additions Copyright (c) 2020, Espressif Systems (Shanghai) PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once
/* Do not include tusb_option.h here: TinyUSB includes this file FROM tusb_option.h
 * after OPT_* are defined. Including tusb_option.h first causes CFG_* defaults then
 * redefinition warnings (CFG_TUD_ENABLED, CFG_TUSB_OS, CFG_TUD_MAX_SPEED, ...). */
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/*         */
/* KCONFIG */
/*         */

#ifndef CONFIG_TINYUSB_CDC_ENABLED
#   define CONFIG_TINYUSB_CDC_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_MSC_ENABLED
#   define CONFIG_TINYUSB_MSC_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_HID_ENABLED
#   define CONFIG_TINYUSB_HID_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_MIDI_ENABLED
#   define CONFIG_TINYUSB_MIDI_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_AUDIO_ENABLED
#   define CONFIG_TINYUSB_AUDIO_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_VIDEO_ENABLED
#   define CONFIG_TINYUSB_VIDEO_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_CUSTOM_CLASS_ENABLED
#   define CONFIG_TINYUSB_CUSTOM_CLASS_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_DFU_RT_ENABLED
#   define CONFIG_TINYUSB_DFU_RT_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_DFU_ENABLED
#   define CONFIG_TINYUSB_DFU_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_VENDOR_ENABLED
#   define CONFIG_TINYUSB_VENDOR_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_NCM_ENABLED
#   define CONFIG_TINYUSB_NCM_ENABLED 0
#endif

#ifndef CONFIG_TINYUSB_HOST_ENABLED
#   define CONFIG_TINYUSB_HOST_ENABLED 0
#endif

#if CONFIG_TINYUSB_ENABLED
#	define CFG_TUD_ENABLED 1
#endif

/*                      */
/* COMMON CONFIGURATION */
/*                      */
#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU				OPT_MCU_ESP32S2
#endif
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE
#define CFG_TUSB_OS                 OPT_OS_FREERTOS
#define BOARD_TUD_RHPORT			0

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#   define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#if CONFIG_IDF_TARGET_ESP32P4
#   define CFG_TUSB_MEM_ALIGN       TU_ATTR_ALIGNED(64)
#else
#   define CFG_TUSB_MEM_ALIGN       TU_ATTR_ALIGNED(4)
#endif
#endif

#if CONFIG_IDF_TARGET_ESP32P4
#define CFG_TUD_MAX_SPEED OPT_MODE_HIGH_SPEED
#else
#define CFG_TUD_MAX_SPEED OPT_MODE_FULL_SPEED
#endif

#define BOARD_TUD_MAX_SPEED   CFG_TUD_MAX_SPEED

/*                      */
/* DEVICE CONFIGURATION */
/*                      */

#define CFG_TUD_MAINTASK_SIZE 4096
#define CFG_TUD_ENDPOINT_SIZE  (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_ENDPOINT0_SIZE 64

// Enabled Drivers
#ifdef CONFIG_TINYUSB_CDC_MAX_PORTS
#define CFG_TUD_CDC           CONFIG_TINYUSB_CDC_MAX_PORTS
#else
#define CFG_TUD_CDC           0
#endif
#define CFG_TUD_MSC           CONFIG_TINYUSB_MSC_ENABLED
#define CFG_TUD_HID           CONFIG_TINYUSB_HID_ENABLED
#define CFG_TUD_MIDI          CONFIG_TINYUSB_MIDI_ENABLED
#define CFG_TUD_AUDIO         CONFIG_TINYUSB_AUDIO_ENABLED
#define CFG_TUD_VIDEO         CONFIG_TINYUSB_VIDEO_ENABLED
#define CFG_TUD_CUSTOM_CLASS  CONFIG_TINYUSB_CUSTOM_CLASS_ENABLED
#define CFG_TUD_DFU_RUNTIME   CONFIG_TINYUSB_DFU_RT_ENABLED
#define CFG_TUD_DFU           CONFIG_TINYUSB_DFU_ENABLED
#define CFG_TUD_VENDOR        CONFIG_TINYUSB_VENDOR_ENABLED
#define CFG_TUD_NCM           CONFIG_TINYUSB_NCM_ENABLED

// CDC FIFO size of TX and RX
#define CFG_TUD_CDC_RX_BUFSIZE 		CONFIG_TINYUSB_CDC_RX_BUFSIZE
#define CFG_TUD_CDC_TX_BUFSIZE 		CONFIG_TINYUSB_CDC_TX_BUFSIZE

// MSC Buffer size of Device Mass storage:
#define CFG_TUD_MSC_BUFSIZE 		CONFIG_TINYUSB_MSC_BUFSIZE

// HID buffer size Should be sufficient to hold ID (if any) + Data
#define CFG_TUD_HID_BUFSIZE 		CONFIG_TINYUSB_HID_BUFSIZE

// MIDI FIFO size of TX and RX
#define CFG_TUD_MIDI_RX_BUFSIZE		CONFIG_TINYUSB_MIDI_RX_BUFSIZE
#define CFG_TUD_MIDI_TX_BUFSIZE		CONFIG_TINYUSB_MIDI_TX_BUFSIZE

// The number of video streaming interfaces and  endpoint size
#define CFG_TUD_VIDEO_STREAMING     CONFIG_TINYUSB_VIDEO_STREAMING_IFS
#define CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE  CONFIG_TINYUSB_VIDEO_STREAMING_BUFSIZE

// DFU buffer size
#define CFG_TUD_DFU_XFER_BUFSIZE	CONFIG_TINYUSB_DFU_BUFSIZE

// VENDOR FIFO size of TX and RX
#define CFG_TUD_VENDOR_RX_BUFSIZE 	CONFIG_TINYUSB_VENDOR_RX_BUFSIZE
#define CFG_TUD_VENDOR_TX_BUFSIZE 	CONFIG_TINYUSB_VENDOR_TX_BUFSIZE

// AUDIO configuration
#define CFG_TUD_AUDIO_ENABLE_EP_IN                1  // TX
#define CFG_TUD_AUDIO_ENABLE_EP_OUT               1  // RX
#define CFG_TUD_AUDIO_MAX_N_CHANNELS              2  // Stereo
#define CFG_TUD_AUDIO_MAX_N_BYTES_PER_SAMPLE      4  // 24bit/32bit

#if CONFIG_IDF_TARGET_ESP32P4
#define CFG_TUD_AUDIO_ENABLE_INTERRUPT_EP         1  // Allow volume controlled by on-baord button (UAC2)
#define CFG_TUD_AUDIO_MAX_SAMPLE_RATE             192000
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX         TUD_AUDIO_EP_SIZE(true, CFG_TUD_AUDIO_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_MAX_N_BYTES_PER_SAMPLE, CFG_TUD_AUDIO_MAX_N_CHANNELS)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ      (32 * CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX)
#else
#define CFG_TUD_AUDIO_MAX_SAMPLE_RATE             96000
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX         TUD_AUDIO_EP_SIZE(false, CFG_TUD_AUDIO_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_MAX_N_BYTES_PER_SAMPLE, CFG_TUD_AUDIO_MAX_N_CHANNELS)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ      (4 * CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX)
#endif

#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX        CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ     CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ

/*                      */
/*  HOST CONFIGURATION  */
/*                      */

#if CONFIG_TINYUSB_HOST_ENABLED
#define CFG_TUH_ENABLED             CFG_TUD_ENABLED
#define CFG_TUSB_RHPORT1_MODE       OPT_MODE_HOST
#define BOARD_TUH_RHPORT            1
#define BOARD_TUH_MAX_SPEED         CFG_TUD_MAX_SPEED
#define CFG_TUH_ENUMERATION_BUFSIZE 256

#define CFG_TUH_HUB                 2 // number of supported hubs
#define CFG_TUH_CDC                 1 // CDC ACM
#define CFG_TUH_CDC_FTDI            1 // FTDI Serial.  FTDI is not part of CDC class, only to re-use CDC driver API
#define CFG_TUH_CDC_CP210X          1 // CP210x Serial. CP210X is not part of CDC class, only to re-use CDC driver API
#define CFG_TUH_CDC_CH34X           1 // CH340 or CH341 Serial. CH34X is not part of CDC class, only to re-use CDC driver API
/* Max simultaneous HID interfaces (TinyUSB hid_host.c: _hidh_itf[CFG_TUH_HID]). Value 1 allows only one iface stack-wide. */
#define CFG_TUH_HID                 (3*CFG_TUH_DEVICE_MAX) // typical keyboard + mouse device can have 3-4 HID interfaces
#define CFG_TUH_MSC                 1
#define CFG_TUH_MIDI                1 // MIDI host driver; midi_host.c is already compiled, this enables it
//#define CFG_TUH_VENDOR              3

#define CFG_TUH_DEVICE_MAX          (3*CFG_TUH_HUB + 1)

//------------- HID -------------//
#define CFG_TUH_HID_EPIN_BUFSIZE    64
#define CFG_TUH_HID_EPOUT_BUFSIZE   64

//------------- CDC -------------//
#define CFG_TUH_CDC_LINE_CONTROL_ON_ENUM    0x03
#define CFG_TUH_CDC_LINE_CODING_ON_ENUM   { 115200, CDC_LINE_CODING_STOP_BITS_1, CDC_LINE_CODING_PARITY_NONE, 8 }
#else
#define CFG_TUH_ENABLED             0
#endif /* CONFIG_TINYUSB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif
