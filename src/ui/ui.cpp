/**
 * @file ui.cpp
 * @brief LVGL screen, widgets, theme, and button event emitter.
 *
 * Presentation layer for the CYD_SmokeTest2 coin-toss app. All business logic
 * is delegated to the registered flip callback.
 */

#include "ui.h"

#include <Arduino.h>
#include <lvgl.h>

// ---------------------------------------------------------------------------
// Widget handles
// ---------------------------------------------------------------------------
static lv_obj_t* screen = nullptr;
static lv_obj_t* lblTitle = nullptr;
static lv_obj_t* lblResult = nullptr;
static lv_obj_t* btnFlip = nullptr;
static lv_obj_t* btnFlipLabel = nullptr;
static lv_obj_t* lblError = nullptr;

// ---------------------------------------------------------------------------
// Event callback registered by the application/state-machine layer
// ---------------------------------------------------------------------------
static ui_flip_callback_t flip_callback = nullptr;

// ---------------------------------------------------------------------------
// Style helpers
// ---------------------------------------------------------------------------
static void apply_dark_screen_theme(void)
{
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
}

static void style_title_label(lv_obj_t* label)
{
    lv_label_set_text(label, "Coin Toss");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 40);
}

static void style_result_label(lv_obj_t* label)
{
    lv_label_set_text(label, "Tap Flip");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFA500), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

static void style_flip_button(lv_obj_t* button, lv_obj_t* label)
{
    lv_obj_set_size(button, 100, 60);
    // Position the button so its top edge is at y ~ 180 px in portrait 240x320.
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -80);

    lv_label_set_text(label, "Flip Coin");
    lv_obj_center(label);
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------
static void on_flip_button_clicked(lv_event_t* event)
{
    (void)event;

    // Log every touch-triggered button click to serial for Wokwi verification.
    Serial.println("UI: Flip button clicked");

    if (flip_callback != nullptr) {
        flip_callback();
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void ui_create(void)
{
    screen = lv_scr_act();
    apply_dark_screen_theme();

    lblTitle = lv_label_create(screen);
    style_title_label(lblTitle);

    lblResult = lv_label_create(screen);
    style_result_label(lblResult);

    btnFlip = lv_btn_create(screen);
    btnFlipLabel = lv_label_create(btnFlip);
    style_flip_button(btnFlip, btnFlipLabel);

    lv_obj_add_event_cb(btnFlip, on_flip_button_clicked, LV_EVENT_CLICKED, nullptr);
}

void ui_set_result(const char* text)
{
    if (lblResult != nullptr && text != nullptr) {
        lv_label_set_text(lblResult, text);
    }
}

void ui_show_error(const char* msg)
{
    if (screen == nullptr) {
        return;
    }

    // Hide the normal UI controls.
    if (lblTitle != nullptr) {
        lv_obj_add_flag(lblTitle, LV_OBJ_FLAG_HIDDEN);
    }
    if (lblResult != nullptr) {
        lv_obj_add_flag(lblResult, LV_OBJ_FLAG_HIDDEN);
    }
    if (btnFlip != nullptr) {
        lv_obj_add_flag(btnFlip, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_state(btnFlip, LV_STATE_DISABLED);
    }

    // Lazily create the error label on first use.
    if (lblError == nullptr) {
        lblError = lv_label_create(screen);
        lv_obj_set_style_text_color(lblError, lv_color_hex(0xFF0000), LV_PART_MAIN);
        lv_obj_set_style_text_font(lblError, &lv_font_montserrat_24, LV_PART_MAIN);
        lv_label_set_long_mode(lblError, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(lblError, 200);
    }

    lv_label_set_text(lblError, msg);
    lv_obj_center(lblError);
    lv_obj_clear_flag(lblError, LV_OBJ_FLAG_HIDDEN);
}

void ui_reset(void)
{
    if (lblError != nullptr) {
        lv_obj_add_flag(lblError, LV_OBJ_FLAG_HIDDEN);
    }

    if (lblTitle != nullptr) {
        lv_label_set_text(lblTitle, "Coin Toss");
        lv_obj_clear_flag(lblTitle, LV_OBJ_FLAG_HIDDEN);
    }

    if (lblResult != nullptr) {
        lv_label_set_text(lblResult, "Tap Flip");
        lv_obj_clear_flag(lblResult, LV_OBJ_FLAG_HIDDEN);
    }

    if (btnFlip != nullptr) {
        lv_obj_clear_flag(btnFlip, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(btnFlip, LV_STATE_DISABLED);
    }
}

void ui_register_flip_callback(ui_flip_callback_t callback)
{
    flip_callback = callback;
}
