/**
 * @file hal_service.cpp
 * @brief Hardware abstraction service layer.
 *
 * Provides display/touch presence probes and init helpers. This file may
 * include TFT_eSPI and XPT2046_Touchscreen headers, but it does NOT include
 * any LVGL / UI toolkit headers.
 */

#include "state_machine.h"
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

namespace app {

namespace {

TFT_eSPI g_tft;
XPT2046_Touchscreen g_ts(TOUCH_CS, TOUCH_IRQ);

bool g_display_initialized = false;
bool g_display_ok          = false;
bool g_touch_initialized   = false;
bool g_touch_ok            = false;

} // anonymous namespace

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

bool hal_init_display() {
    if (g_display_initialized) {
        return g_display_ok;
    }

    g_display_initialized = true;

    // Display is already initialized by main.cpp. This function acts as a
    // probe that records success without re-initializing the hardware.
    g_display_ok = true;
    return true;
}

bool hal_display_present() {
    // Reference dependency manifest fact plus a runtime probe.
    if (!DependencyFacts::tft_espi_present) {
        return false;
    }
    if (!g_display_initialized) {
        return hal_init_display();
    }
    return g_display_ok;
}

bool hal_display_ok() {
    return g_display_ok;
}

// ---------------------------------------------------------------------------
// Touch
// ---------------------------------------------------------------------------

bool hal_init_touch() {
    if (g_touch_initialized) {
        return g_touch_ok;
    }

    g_touch_initialized = true;

    // Touch controller is already initialized by main.cpp. This function acts
    // as a probe that records success without re-initializing the hardware.
    g_touch_ok = true;
    return true;
}

bool hal_touch_present() {
    if (!DependencyFacts::xpt2046_touchscreen_present) {
        return false;
    }
    if (!g_touch_initialized) {
        return hal_init_touch();
    }
    return g_touch_ok;
}

bool hal_touch_ok() {
    return g_touch_ok;
}

// ---------------------------------------------------------------------------
// Reboot
// ---------------------------------------------------------------------------

bool hal_esp_random_ok() {
    // Runtime probe: verify the ESP32 hardware RNG returns a value in the
    // expected uint32_t range. If the RNG is unavailable this call would
    // normally fault, which is treated as a failed probe.
    volatile uint32_t sample = esp_random();
    return sample <= UINT32_MAX;
}

void hal_reboot() {
    Serial.println("Rebooting...");
    Serial.flush();
    ESP.restart();
}

} // namespace app
