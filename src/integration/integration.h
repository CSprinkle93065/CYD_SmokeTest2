/**
 * @file integration.h
 * @brief UI-to-state-machine glue layer for CYD_SmokeTest2.
 *
 * This module binds LVGL UI events to the firmware state machine and reflects
 * state-machine outputs back to the UI. It contains no business logic; all
 * decisions remain in the state_machine layer.
 */

#ifndef INTEGRATION_H
#define INTEGRATION_H

#include "state_machine/state_machine.h"

namespace integration {

/**
 * @brief Bind the UI layer to an application state-machine instance.
 *
 * Registers the Flip Coin button callback and stores the state-machine
 * reference for use by update(). Must be called before the first loop()
 * iteration.
 */
void bind(app::AppStateMachine& sm);

/**
 * @brief Drive automatic state-machine outputs once per main loop.
 *
 * Handles:
 *   - Completing a flip when the state machine enters Flipping.
 *   - Showing the error screen when the state machine enters an error state.
 */
void update();

/**
 * @brief LVGL button-click entry point.
 *
 * Dispatches evButtonClick to the bound state machine. Registered as the
 * ui_flip_callback_t via ui_register_flip_callback().
 */
void on_flip_button_clicked();

} // namespace integration

#endif // INTEGRATION_H
