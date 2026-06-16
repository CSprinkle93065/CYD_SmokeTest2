# CYD_SmokeTest2 — Coin Toss App Definition

**Project:** CYD_SmokeTest2  
**Version:** 0.1.0  
**Workflow ID:** wvc_20260616_043904  
**Revision Type:** new_project  
**Board Version:** CYD2USB v3 (ESP32-2432S028R)  
**Target Environment:** Wokwi simulation + physical CYD2USB v3  
**Simulator:** Wokwi (required)

---

## 1. Application Overview

A minimal CYD2USB v3 firmware app that simulates a coin toss. The user taps a single on-screen button; the firmware randomly selects **Heads** or **Tails**, updates the result label, and prints a serial trace in the form `COIN: Heads` or `COIN: Tails`. The app is intentionally small so it can serve as a smoke-test template for the CYD2USB VibeCoder workflow.

### Goals
- Boot on CYD2USB v3 and in Wokwi without fatal errors.
- Display a title, a result label, and one tappable button.
- Respond to a touch event by running a fair random coin flip.
- Emit a deterministic serial trace for automated Wokwi verification.

### Scope
- One screen, no navigation, no persistence of historical results.
- No audio, no LED feedback, no animation beyond a label text change.
- Random source: ESP32 `esp_random()` hardware RNG (confirmed design decision; hardware RNG is the simplest fair source on ESP32 and matches the prior CYD_SmokeTest project).

---

## 2. UI/Display Layout

### Screen
- **Resolution:** 240 × 320 (portrait, USB connector at the bottom) (confirmed during Stage 2 interview).
- **Background:** dark theme / black screen (confirmed during Stage 2 interview).
- **Layout:** centered vertical LVGL flex layout (confirmed during Stage 2 interview).

### Controls

| Control | Type | Position | Style | Purpose |
|---------|------|----------|-------|---------|
| `lblTitle` | `lv_label` | Top-center, y ≈ 40 px | Font: Montserrat 24, color white | Static title text `"Coin Toss"` |
| `lblResult` | `lv_label` | Center | Font: Montserrat 32, color orange | Dynamic result text; initial `"Tap Flip"`, then `"Heads"` or `"Tails"` |
| `btnFlip` | `lv_btn` | Bottom-center, y ≈ 180 px | Size 100 × 60 px, label `"Flip Coin"` | Triggers the coin flip when tapped |

### Visual States
- **Init:** screen black, no UI visible (during hardware init).
- **Ready:** title = `"Coin Toss"`, result = `"Tap Flip"`, button enabled.
- **Result:** title unchanged, result = `"Heads"` or `"Tails"`, button enabled.
- **Error:** full-screen red label with error code; button hidden/disabled (confirmed during Stage 2 interview; defensive state for init failure, matching prior CYD_SmokeTest project).

---

## 3. User Actions

| ID | Action | Input | Precondition | System Response | API Function |
|----|--------|-------|--------------|-----------------|--------------|
| UA-01 | Power on / boot | Device reset | None | Initialize serial, LVGL, TFT, touch, and UI; log boot message `Coin Toss setup done` | `app_init()` |
| UA-02 | Tap "Flip Coin" | Touch release on `btnFlip` | System in `Ready` or `ResultShown` state | Generate random Heads/Tails, update `lblResult`, print `COIN: <result>` to Serial | `on_flip_button_clicked()` → `coin_flip()` → `ui_set_result()` → `log_coin_result()` |
| UA-03 | Observe result | Read screen | After UA-02 | `lblResult` displays `"Heads"` or `"Tails"` | `ui_set_result(const char* text)` |

---

## 4. Data Model / Persistence

This app does **not** persist data across resets.

### Runtime Data

| Name | Type | Scope | Description |
|------|------|-------|-------------|
| `current_result` | `coin_side_t` (`enum { HEADS, TAILS }`) | Global/static | Last coin-toss result; updated on every flip |
| `ui_ready` | `bool` | Global/static | True after UI creation succeeds |

### No Persistence Required
- No score history, no settings, no NVS/SPIFFS/SD usage.

---

## 5. API Function List

### Application Layer

| Function | Signature | Trigger | Description |
|----------|-----------|---------|-------------|
| `app_init` | `void app_init(void)` | `setup()` | Initialize serial, LVGL, TFT, touch, and UI. Logs `"Coin Toss setup done"` on success. |
| `app_run` | `void app_run(void)` | `loop()` | Poll touch, run LVGL timer handler, dispatch events. |

### UI Layer

| Function | Signature | Trigger | Description |
|----------|-----------|---------|-------------|
| `ui_create` | `void ui_create(void)` | `app_init()` success | Create `lblTitle`, `lblResult`, `btnFlip` and bind button event. |
| `ui_set_result` | `void ui_set_result(const char* text)` | State transition to `ResultShown` | Update `lblResult` text. |
| `ui_show_error` | `void ui_show_error(const char* msg)` | Init failure | Clear screen and show a red error label. |

### Business Logic Layer

| Function | Signature | Trigger | Description |
|----------|-----------|---------|-------------|
| `coin_flip` | `coin_side_t coin_flip(void)` | `btnFlip` click | Return pseudo-random `HEADS` or `TAILS` using `esp_random()`. |
| `coin_side_to_string` | `const char* coin_side_to_string(coin_side_t side)` | Logging / UI update | Map `HEADS` → `"Heads"`, `TAILS` → `"Tails"`. |
| `log_coin_result` | `void log_coin_result(coin_side_t side)` | After flip | Print `"COIN: Heads"` or `"COIN: Tails"` to Serial. |

### Event Handlers

| Function | Signature | Trigger | Description |
|----------|-----------|---------|-------------|
| `on_flip_button_clicked` | `static void on_flip_button_clicked(lv_event_t* e)` | LVGL `LV_EVENT_CLICKED` on `btnFlip` | Call `coin_flip()` and update UI/serial log. |
| `on_display_init_failed` | `void on_display_init_failed(void)` | TFT init error | Transition to error state, log error. |
| `on_touch_init_failed` | `void on_touch_init_failed(void)` | Touch init error | Transition to error state, log error. |

---

## 6. Toolchain

| Component | Value | Notes |
|-----------|-------|-------|
| Language | C/C++ (Arduino-ESP32) | Matches CYD2USB_HelloLVGL reference |
| Framework | Arduino | `framework = arduino` in `platformio.ini` |
| Platform | `espressif32` | PlatformIO |
| Board | `esp32dev` | Equivalent to ESP32-2432S028R in PIO |
| GUI library | LVGL 8.4.0 | `lvgl@8.4.0` |
| Display driver | TFT_eSPI 2.5.43 | `bodmer/TFT_eSPI@^2.5.43` |
| Touch driver | XPT2046_Touchscreen | `paulstoffregen/XPT2046_Touchscreen` |
| Build tool | PlatformIO `pio run` | Produces `.pio/build/esp32dev/firmware.bin` |
| Simulator | Wokwi CLI | `wokwi-cli` with `wokwi/diagram.json` and `wokwi/scenario.yaml` (accepted as verification artifact during Stage 2 interview) |
| Serial monitor | 115200 baud | Matches reference `platformio.ini` |

### Build Flags (from CYD2USB_HelloLVGL reference)
- `-D USER_SETUP_LOADED`
- `-include "${PROJECT_DIR}/include/User_Setup.h"`
- `-include "${PROJECT_DIR}/include/lv_conf.h"`
- `-D LV_CONF_INCLUDE_SIMPLE`

### Board Build Parameters (from CYD2USB_HelloLVGL reference)
- `board_build.f_cpu = 240000000L`
- `board_build.f_flash = 40000000L`
- `board_build.flash_mode = dio`
- `upload_speed = 460800`

---

## 7. Hardware Profile

### Board
- **Version:** CYD2USB v3 (ESP32-2432S028R)
- **Display controller:** ST7789, 240 × 320 native, used in portrait 240 × 320
- **Touch controller:** XPT2046 resistive touchscreen
- **Backlight:** GPIO 21

### Pin Map (from `projects/CYD2USB_HelloLVGL/include/User_Setup.h` and `wokwi/diagram.json`)

| Function | GPIO | Notes |
|----------|------|-------|
| TFT_MOSI | 13 | HSPI (per `USE_HSPI_PORT`) |
| TFT_MISO | 12 | HSPI |
| TFT_SCK  | 14 | HSPI |
| TFT_CS   | 15 | HSPI |
| TFT_DC   | 2  | HSPI |
| TFT_BL   | 21 | Backlight output |
| TFT_RST  | -1 | Tied to ESP32 board reset |
| TOUCH_CLK  | 25  | VSPI — fixed hardware fact from CYD2USB_HelloLVGL reference (`diagram.json` / `User_Setup.h`) |
| TOUCH_MOSI | 32  | VSPI — fixed hardware fact from CYD2USB_HelloLVGL reference (`diagram.json` / `User_Setup.h`) |
| TOUCH_MISO | 39  | VSPI (VN) — fixed hardware fact from CYD2USB_HelloLVGL reference (`diagram.json` / `User_Setup.h`) |
| TOUCH_CS   | 33  | VSPI — fixed hardware fact from CYD2USB_HelloLVGL reference (`diagram.json` / `User_Setup.h`) |
| TOUCH_IRQ  | 255 | IRQ disabled — fixed hardware fact from CYD2USB_HelloLVGL reference; XPT2046_Touchscreen is polled without IRQ |

### Wokwi Delta
- Wokwi uses the `wokwi-ili9341` part as a generic 240 × 320 framebuffer in place of the physical ST7789 panel.
- The `wokwi-xpt2046` part is used for touch input.
- Color byte order and gamma may differ from the physical board; visual correctness is validated on hardware.

### Reserved Pins
- Do not use GPIO 0, 2, 4, 5, 12, 15 for user peripherals without checking boot-mode impact.
- GPIO 34–39 are input-only.
- No additional peripherals are required by this app.

### Free / External Pin List
- No external pins are used by this app beyond the onboard display and touch interfaces listed above.
