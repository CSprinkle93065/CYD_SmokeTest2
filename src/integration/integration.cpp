/**
 * @file integration.cpp
 * @brief UI-to-state-machine glue implementation.
 *
 * Wires LVGL control events to state-machine events and reflects state-machine
 * outputs to the display. Keeps business logic out of the integration layer.
 */

#include "integration.h"
#include "ui/ui.h"
#include <Arduino.h>

namespace integration {

namespace {

// Bound state-machine instance. Single-threaded Arduino loop; no locking needed.
app::AppStateMachine* g_sm = nullptr;
app::State g_last_state = app::State::Initializing;

} // anonymous namespace

void bind(app::AppStateMachine& sm)
{
    g_sm = &sm;
    g_last_state = sm.current();
    ui_register_flip_callback(on_flip_button_clicked);
}

void on_flip_button_clicked()
{
    if (g_sm == nullptr) {
        return;
    }
    Serial.println("INTEGRATION: Flip pressed -> evButtonClick");
    g_sm->dispatch(app::Event::evButtonClick, g_sm->context());
}

void update()
{
    if (g_sm == nullptr) {
        return;
    }

    app::State current = g_sm->current();

    // When the state machine moves into Flipping, automatically complete the
    // flip by posting evFlipComplete. The actual coin-flip business logic runs
    // as the Flipping -> ResultShown action in the state machine.
    if (current == app::State::Flipping && g_last_state != app::State::Flipping) {
        Serial.println("INTEGRATION: Completing flip -> evFlipComplete");
        g_sm->dispatch(app::Event::evFlipComplete, g_sm->context());
        current = g_sm->current();
    }

    // Reflect error-state entry through the UI layer's error screen.
    if (current != g_last_state) {
        if (current == app::State::Error_Display) {
            ui_show_error("ERR: Display init failed");
        } else if (current == app::State::Error_Touch) {
            ui_show_error("ERR: Touch init failed");
        }
        g_last_state = current;
    }
}

} // namespace integration
