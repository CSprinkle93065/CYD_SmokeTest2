/**
 * @file touch_mapping.cpp
 * @brief LVGL pointer input device mapping for the XPT2046 touchscreen.
 *
 * Bridges the XPT2046_Touchscreen driver to LVGL's input device subsystem.
 * The caller owns the touchscreen instance and must initialize it; this file
 * only registers the LVGL read callback and logs touch events to serial.
 */

#include "ui.h"
#include "hal_config.h"

#include <Arduino.h>
#include <lvgl.h>
#include <XPT2046_Touchscreen.h>

static XPT2046_Touchscreen* touchscreen_instance = nullptr;

static int16_t clamp(int16_t v, int16_t min, int16_t max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

/**
 * @brief LVGL input device read callback.
 *
 * Reads the XPT2046 state and maps it to LVGL pointer coordinates. Raw touch
 * coordinates are logged to serial every time a press is detected so Wokwi
 * tests can observe touch activity.
 */
static void ui_touch_read(lv_indev_drv_t* drv, lv_indev_data_t* data)
{
    (void)drv;

    if (touchscreen_instance == nullptr) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    if (touchscreen_instance->touched()) {
        TS_Point p = touchscreen_instance->getPoint();
        int16_t x = map(p.x, HAL_TOUCH_MIN_X, HAL_TOUCH_MAX_X, 0, HAL_DISPLAY_WIDTH - 1);
        int16_t y = map(p.y, HAL_TOUCH_MIN_Y, HAL_TOUCH_MAX_Y, 0, HAL_DISPLAY_HEIGHT - 1);
        data->point.x = clamp(x, 0, HAL_DISPLAY_WIDTH - 1);
        data->point.y = clamp(y, 0, HAL_DISPLAY_HEIGHT - 1);
        data->state = LV_INDEV_STATE_PRESSED;

        Serial.printf("TOUCH raw=(%d,%d) screen=(%d,%d) z=%d\n", p.x, p.y, data->point.x, data->point.y, p.z);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void ui_register_touchscreen(XPT2046_Touchscreen* touchscreen)
{
    touchscreen_instance = touchscreen;

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = ui_touch_read;
    lv_indev_drv_register(&indev_drv);
}
