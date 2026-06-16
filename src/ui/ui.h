/**
 * @file ui.h
 * @brief Public UI API for the CYD_SmokeTest2 coin-toss app.
 *
 * The UI layer owns presentation only: screens, widgets, theme, touch input
 * mapping, and event emission. It does NOT contain business logic such as
 * coin_flip() or state transitions.
 */

#ifndef UI_H
#define UI_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration so the UI can register a hardware touchscreen instance
   without pulling XPT2046_Touchscreen.h into every consumer. */
class XPT2046_Touchscreen;

/** Callback signature emitted when the Flip Coin button is clicked. */
typedef void (*ui_flip_callback_t)(void);

/**
 * @brief Create the primary screen and all widgets.
 *
 * Must be called after LVGL has been initialized and a display driver has been
 * registered. The screen is set to a dark background.
 */
void ui_create(void);

/**
 * @brief Update the result label text.
 * @param text New text to display (e.g. "Heads" or "Tails").
 */
void ui_set_result(const char* text);

/**
 * @brief Clear the screen and show a full-screen red error label.
 *
 * Hides the title, result label, and Flip Coin button.
 * @param msg Error message to display.
 */
void ui_show_error(const char* msg);

/**
 * @brief Restore the UI to its ready state.
 *
 * Resets title/result text, re-enables the flip button, and hides any error.
 */
void ui_reset(void);

/**
 * @brief Register the callback invoked when the Flip Coin button is clicked.
 * @param callback Function to call from the LV_EVENT_CLICKED handler.
 */
void ui_register_flip_callback(ui_flip_callback_t callback);

/**
 * @brief Register the XPT2046 touchscreen as an LVGL pointer input device.
 *
 * The UI layer configures the LVGL input device read callback; the caller
 * retains ownership of the touchscreen instance and must initialize it first.
 * @param touchscreen Pointer to an initialized XPT2046_Touchscreen object.
 */
void ui_register_touchscreen(XPT2046_Touchscreen* touchscreen);

#ifdef __cplusplus
}
#endif

#endif // UI_H
