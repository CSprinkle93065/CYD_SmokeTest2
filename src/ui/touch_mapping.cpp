/**
 * @file touch_mapping.cpp
 * @brief LVGL pointer input device mapping for the XPT2046 touchscreen.
 *
 * Bridges the XPT2046_Touchscreen driver to LVGL's input device subsystem.
 * The caller owns the touchscreen instance and must initialize it; this file
 * only registers the LVGL read callback and logs touch events to serial.
 */

#include "ui.h"

#include <Arduino.h>
#include <lvgl.h>
#include <XPT2046_Touchscreen.h>

static XPT2046_Touchscreen* touchscreen_instance = nullptr;

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
        data->point.x = p.x;
        data->point.y = p.y;
        data->state = LV_INDEV_STATE_PRESSED;

        Serial.printf("TOUCH x=%d y=%d z=%d\n", p.x, p.y, p.z);
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
