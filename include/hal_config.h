/**
 * @file hal_config.h
 * @brief Hardware abstraction constants for CYD_SmokeTest2 on CYD2USB v3
 *
 * Source: projects/CYD2USB_HelloLVGL/include/lv_conf.h and docs/definition.md
 *
 * This header captures the display and touch parameters that application code
 * depends on. TFT_eSPI display setup is supplied by include/User_Setup.h; LVGL
 * configuration is supplied by include/lv_conf.h.
 */

#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------
#define HAL_DISPLAY_CONTROLLER  ST7789
#define HAL_DISPLAY_WIDTH       240
#define HAL_DISPLAY_HEIGHT      320
#define HAL_DISPLAY_ORIENTATION 0   // Portrait, USB connector at the bottom
#define HAL_DISPLAY_COLOR_DEPTH 16  // RGB565
#define HAL_BACKLIGHT_PIN       21
#define HAL_BACKLIGHT_ACTIVE    HIGH

// ---------------------------------------------------------------------------
// Touch
// ---------------------------------------------------------------------------
#define HAL_TOUCH_CONTROLLER    XPT2046
#define HAL_TOUCH_SPI_FREQUENCY 2500000
#define HAL_TOUCH_IRQ_ENABLED   0

// ---------------------------------------------------------------------------
// Serial
// ---------------------------------------------------------------------------
#define HAL_SERIAL_BAUD         115200

#endif // HAL_CONFIG_H
