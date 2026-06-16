/**
 * @file state_machine.h
 * @brief Hand-written state machine engine, business logic API, and HAL service
 *        layer for CYD_SmokeTest2.
 *
 * This layer owns firmware behavior only. It deliberately does NOT include any
 * LVGL or UI toolkit headers. UI actions are dispatched through function
 * pointers registered by the UI layer (Stage 13).
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "state_machine_generated.h"
#include "hal_config.h"
#include "pins.h"

namespace app {

// -----------------------------------------------------------------------------
// Dependency manifest facts (read-only constants derived from
// dependency_manifest.json). Guards MUST reference these facts or runtime
// probes rather than hard-coded assumptions.
// -----------------------------------------------------------------------------
struct DependencyFacts {
    static constexpr bool framework_arduino_esp32_present = true;
    static constexpr bool platform_espressif32_present    = true;
    static constexpr bool board_esp32dev_present          = true;
    static constexpr bool lvgl_present                    = true;
    static constexpr bool tft_espi_present                = true;
    static constexpr bool xpt2046_touchscreen_present     = true;
    static constexpr bool serial_required                 = true;
    static constexpr bool reboot_available                = true;
};

// -----------------------------------------------------------------------------
// Coin-toss business logic
// -----------------------------------------------------------------------------
enum class coin_side_t {
    HEADS,
    TAILS
};

coin_side_t coin_flip();
const char* coin_side_to_string(coin_side_t side);
void log_coin_result(coin_side_t side);

// -----------------------------------------------------------------------------
// Runtime hardware / application status
// -----------------------------------------------------------------------------
struct HardwareStatus {
    bool framework_ready = false;
    bool libs_ready      = false;
    bool serial_ok       = false;
    bool display_ok      = false;
    bool touch_ok        = false;
    bool btnFlip_enabled = false;
    bool ui_ready        = false;
    bool esp_random_ok   = true;
    bool reboot_available = true;
};

struct AppContext : Context {
    HardwareStatus hw;
    coin_side_t last_result = coin_side_t::HEADS;
};

// -----------------------------------------------------------------------------
// UI callbacks (registered by the UI layer; no LVGL dependency here)
// -----------------------------------------------------------------------------
using ui_create_fn_t     = void (*)();
using ui_set_result_fn_t = void (*)(const char* text);

// -----------------------------------------------------------------------------
// HAL service prototypes (implemented in hal_service.cpp)
// -----------------------------------------------------------------------------
bool hal_init_display();
bool hal_display_present();
bool hal_display_ok();

bool hal_init_touch();
bool hal_touch_present();
bool hal_touch_ok();

bool hal_esp_random_ok();

void hal_reboot();

// -----------------------------------------------------------------------------
// Concrete state machine
// -----------------------------------------------------------------------------
class AppStateMachine : public StateMachine {
public:
    AppStateMachine();

    void set_ui_callbacks(ui_create_fn_t create, ui_set_result_fn_t set_result);
    void probe_hardware();

    AppContext& context() { return ctx_; }
    const AppContext& context() const { return ctx_; }

protected:
    // Guards (dependency manifest facts + runtime probes)
    bool guard_serial_required(const Context& ctx) const override;
    bool guard_TFT_eSPI_present(const Context& ctx) const override;
    bool guard_TFT_eSPI_missing_or_not_display_ok(const Context& ctx) const override;
    bool guard_XPT2046_present(const Context& ctx) const override;
    bool guard_XPT2046_missing_or_not_touch_ok(const Context& ctx) const override;
    bool guard_btnFlip_enabled(const Context& ctx) const override;
    bool guard_esp_random_ok(const Context& ctx) const override;
    bool guard_ui_ready(const Context& ctx) const override;
    bool guard_runtime_display_fail(const Context& ctx) const override;
    bool guard_runtime_touch_fail(const Context& ctx) const override;
    bool guard_reboot_available(const Context& ctx) const override;

    // Actions
    void init_serial() override;
    void init_tft() override;
    void ui_create() override;
    void on_flip_button_clicked() override;
    void coin_flip_ui_set_result_log_coin_result() override;
    void ui_set_result_Tap_Flip() override;
    void on_display_init_failed() override;
    void on_touch_init_failed() override;
    void reboot() override;

private:
    ui_create_fn_t ui_create_fn_     = nullptr;
    ui_set_result_fn_t ui_set_result_fn_ = nullptr;
    AppContext ctx_;
};

// -----------------------------------------------------------------------------
// Application bootstrap helper (called from setup())
// -----------------------------------------------------------------------------
void app_init(AppStateMachine& sm);

} // namespace app

#endif // STATE_MACHINE_H
