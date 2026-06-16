/**
 * @file main.cpp
 * @brief Application entry point for CYD_SmokeTest2.
 *
 * Initializes serial, the LVGL graphics library, the TFT_eSPI display driver,
 * and the XPT2046 touchscreen, then wires the UI and state-machine layers
 * together through the integration module.
 */

#include <Arduino.h>

#include "pins.h"
#include "hal_config.h"

#include <lvgl.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "ui/ui.h"
#include "state_machine/state_machine.h"
#include "integration/integration.h"

// ---------------------------------------------------------------------------
// Hardware instances
// ---------------------------------------------------------------------------
static TFT_eSPI tft;
static XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// ---------------------------------------------------------------------------
// LVGL display buffer and driver
// ---------------------------------------------------------------------------
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[HAL_DISPLAY_WIDTH * 24];

static void my_disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(reinterpret_cast<uint16_t*>(&color_p->full), w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

// ---------------------------------------------------------------------------
// Application state machine
// ---------------------------------------------------------------------------
static app::AppStateMachine g_sm;

void setup()
{
    Serial.begin(HAL_SERIAL_BAUD);
    while (!Serial && millis() < 500) {
        ;
    }
    Serial.println("Coin Toss boot");

    lv_init();

    // Initialize display and register it with LVGL.
    tft.init();
    tft.setRotation(HAL_DISPLAY_ORIENTATION);
    tft.fillScreen(TFT_BLACK);

#if defined(TFT_BL) && TFT_BL >= 0
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HAL_BACKLIGHT_ACTIVE);
#endif

    lv_disp_draw_buf_init(&draw_buf, buf, nullptr, sizeof(buf) / sizeof(lv_color_t));

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = HAL_DISPLAY_WIDTH;
    disp_drv.ver_res = HAL_DISPLAY_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize touchscreen and register it with LVGL via the UI layer.
    SPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    ts.begin();
    ts.setRotation(HAL_DISPLAY_ORIENTATION);
    ui_register_touchscreen(&ts);

    // Wire UI callbacks into the state machine and bind the integration layer.
    g_sm.set_ui_callbacks(ui_create, ui_set_result);
    integration::bind(g_sm);

    // Bootstrap the state machine and drive it through the init sequence.
    app::app_init(g_sm);
    g_sm.dispatch(app::Event::evSerialOk, g_sm.context());
    g_sm.dispatch(app::Event::evDisplayOk, g_sm.context());
    g_sm.dispatch(app::Event::evTouchOk, g_sm.context());

    if (g_sm.current() == app::State::Ready) {
        Serial.println("Coin Toss setup done");
    } else {
        Serial.println("Coin Toss setup incomplete");
    }
}

static uint32_t last_tick = 0;

void loop()
{
    uint32_t now = millis();
    lv_tick_inc(now - last_tick);
    last_tick = now;

    lv_timer_handler();
    integration::update();

    delay(5);
}
