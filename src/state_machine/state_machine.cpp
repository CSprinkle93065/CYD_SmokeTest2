/**
 * @file state_machine.cpp
 * @brief Thin wrapper/engine that drives the generated state machine.
 *
 * Implements the concrete guards and actions. UI actions are forwarded to
 * registered callbacks so that this translation unit remains free of LVGL.
 */

#include "state_machine.h"
#include <Arduino.h>

namespace app {

AppStateMachine::AppStateMachine() = default;

void AppStateMachine::set_ui_callbacks(ui_create_fn_t create,
                                       ui_set_result_fn_t set_result) {
    ui_create_fn_     = create;
    ui_set_result_fn_ = set_result;
}

void AppStateMachine::probe_hardware() {
    ctx_.hw.framework_ready = DependencyFacts::framework_arduino_esp32_present;
    ctx_.hw.libs_ready      = DependencyFacts::lvgl_present
                           && DependencyFacts::tft_espi_present
                           && DependencyFacts::xpt2046_touchscreen_present;
    ctx_.hw.reboot_available = DependencyFacts::reboot_available;
    ctx_.hw.esp_random_ok    = hal_esp_random_ok();

    // Runtime probes update the live status flags.
    ctx_.hw.display_ok = hal_display_present();
    ctx_.hw.touch_ok   = hal_touch_present();
}

// ---------------------------------------------------------------------------
// Guards: every guard MUST evaluate either a dependency manifest fact or a
// runtime probe (or both). No silently true hard-coded assumptions.
// ---------------------------------------------------------------------------

bool AppStateMachine::guard_serial_required(const Context&) const {
    return DependencyFacts::serial_required;
}

bool AppStateMachine::guard_TFT_eSPI_present(const Context&) const {
    return DependencyFacts::tft_espi_present && hal_display_present();
}

bool AppStateMachine::guard_TFT_eSPI_missing_or_not_display_ok(const Context&) const {
    return !DependencyFacts::tft_espi_present || !hal_display_ok();
}

bool AppStateMachine::guard_XPT2046_present(const Context&) const {
    return DependencyFacts::xpt2046_touchscreen_present && hal_touch_present();
}

bool AppStateMachine::guard_XPT2046_missing_or_not_touch_ok(const Context&) const {
    return !DependencyFacts::xpt2046_touchscreen_present || !hal_touch_ok();
}

bool AppStateMachine::guard_btnFlip_enabled(const Context&) const {
    return ctx_.hw.btnFlip_enabled;
}

bool AppStateMachine::guard_esp_random_ok(const Context&) const {
    return DependencyFacts::platform_espressif32_present && hal_esp_random_ok();
}

bool AppStateMachine::guard_ui_ready(const Context&) const {
    return ctx_.hw.ui_ready;
}

bool AppStateMachine::guard_runtime_display_fail(const Context&) const {
    return !ctx_.hw.display_ok;
}

bool AppStateMachine::guard_runtime_touch_fail(const Context&) const {
    return !ctx_.hw.touch_ok;
}

bool AppStateMachine::guard_reboot_available(const Context&) const {
    return DependencyFacts::reboot_available;
}

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

void AppStateMachine::init_serial() {
    Serial.begin(HAL_SERIAL_BAUD);
    ctx_.hw.serial_ok = true;
}

void AppStateMachine::init_tft() {
    ctx_.hw.display_ok = hal_init_display();
}

void AppStateMachine::ui_create() {
    if (ui_create_fn_) {
        ui_create_fn_();
    }
    ctx_.hw.ui_ready       = true;
    ctx_.hw.btnFlip_enabled = true;
}

void AppStateMachine::on_flip_button_clicked() {
    // When the flip button is clicked in Ready or ResultShown, the state
    // machine moves to Flipping. The actual coin flip happens on evFlipComplete.
    ctx_.hw.btnFlip_enabled = false;
}

void AppStateMachine::coin_flip_ui_set_result_log_coin_result() {
    ctx_.last_result = coin_flip();
    log_coin_result(ctx_.last_result);
    if (ui_set_result_fn_) {
        ui_set_result_fn_(coin_side_to_string(ctx_.last_result));
    }
    ctx_.hw.btnFlip_enabled = true;
}

void AppStateMachine::ui_set_result_Tap_Flip() {
    if (ui_set_result_fn_) {
        ui_set_result_fn_("Tap Flip");
    }
}

void AppStateMachine::on_display_init_failed() {
    ctx_.hw.display_ok      = false;
    ctx_.hw.btnFlip_enabled = false;
    ctx_.hw.ui_ready        = false;
    Serial.println("ERR: Display init failed");
}

void AppStateMachine::on_touch_init_failed() {
    ctx_.hw.touch_ok        = false;
    ctx_.hw.btnFlip_enabled = false;
    ctx_.hw.ui_ready        = false;
    Serial.println("ERR: Touch init failed");
}

void AppStateMachine::reboot() {
    hal_reboot();
}

// ---------------------------------------------------------------------------
// Application bootstrap helper
// ---------------------------------------------------------------------------

void app_init(AppStateMachine& sm) {
    Serial.begin(HAL_SERIAL_BAUD);
    sm.probe_hardware();
}

} // namespace app
