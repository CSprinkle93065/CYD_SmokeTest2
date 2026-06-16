/**
 * @file pins.h
 * @brief CYD2USB v3 (ESP32-2432S028R) pin map
 *
 * Source: projects/CYD2USB_HelloLVGL/include/User_Setup.h
 *
 * This header defines the fixed wiring between the ESP32 module and the
 * onboard ST7789 display / XPT2046 touchscreen. Values are taken directly
 * from the verified CYD2USB_HelloLVGL reference project.
 */

#ifndef PINS_H
#define PINS_H

// ---------------------------------------------------------------------------
// ST7789 display (HSPI)
// ---------------------------------------------------------------------------
#define TFT_MOSI  13
#define TFT_MISO  12
#define TFT_SCK   14
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   -1    // Display reset tied to ESP32 board reset
#define TFT_BL    21    // Backlight control pin

// ---------------------------------------------------------------------------
// XPT2046 resistive touchscreen (VSPI)
// ---------------------------------------------------------------------------
#define TOUCH_CLK   25
#define TOUCH_MOSI  32
#define TOUCH_MISO  39
#define TOUCH_CS    33
#define TOUCH_IRQ   255 // IRQ disabled; touchscreen is polled

#endif // PINS_H
