#pragma once

inline bool usbPhyBusy = false, serialPhy = true;
using usb_phy_status_t = int;
constexpr int ESP_ERR_INVALID_STATE = 0x103;
constexpr int USB_PHY_TARGET_INT = 0, USB_PHY_STATUS_FREE = 0;
constexpr int PERIPH_USB_MODULE = 0;
constexpr int RTC_CNTL_USB_CONF_REG = 0, USB_SERIAL_JTAG_CONF0_REG = 1;
constexpr unsigned RTC_CNTL_SW_HW_USB_PHY_SEL = 1, RTC_CNTL_SW_USB_PHY_SEL = 2;
constexpr unsigned RTC_CNTL_USB_PAD_ENABLE = 4, USB_SERIAL_JTAG_PHY_SEL = 8;
constexpr unsigned USB_SERIAL_JTAG_PAD_PULL_OVERRIDE = 16, USB_SERIAL_JTAG_DP_PULLUP = 32;
constexpr unsigned USB_SERIAL_JTAG_DM_PULLUP = 64, USB_SERIAL_JTAG_DP_PULLDOWN = 128;
constexpr unsigned USB_SERIAL_JTAG_DM_PULLDOWN = 256, USB_SERIAL_JTAG_USB_PAD_ENABLE = 512;
inline unsigned usbRegisters[2] = {};
inline void CLEAR_PERI_REG_MASK(int reg, unsigned mask) { usbRegisters[reg] &= ~mask; }
inline void SET_PERI_REG_MASK(int reg, unsigned mask) {
  usbRegisters[reg] |= mask;
  serialPhy = reg == USB_SERIAL_JTAG_CONF0_REG && (mask & USB_SERIAL_JTAG_USB_PAD_ENABLE);
}
inline void periph_module_reset(int) { serialPhy = false; }
inline void periph_module_disable(int) {}
inline int usb_phy_get_phy_status(int, usb_phy_status_t *status) {
  *status = usbPhyBusy ? 1 : USB_PHY_STATUS_FREE;
  return ESP_OK;
}
