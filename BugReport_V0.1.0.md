# Bug Report: CYD_SmokeTest2 v0.1.0 — Hardware Validation Failures

**Report date:** 2026-06-16  
**Firmware version:** v0.1.0  
**Workflow ID:** wvc_20260616_043904  
**Target hardware:** ESP32-2432S028R "Cheap Yellow Display" (CYD2USB v3)  
**Test host:** Windows 11, PlatformIO v6.1.19, `esp32dev` board target  
**Serial port:** COM5 (USB-SERIAL CH340)

---

## Executive summary

CYD_SmokeTest2 v0.1.0 builds successfully but fails to run on physical CYD2USB hardware. Two independent bugs were identified during hardware validation:

1. **Boot loop on startup** — the firmware triggers `RTCWDT_RTC_RESET` repeatedly and never reaches the `Ready` state.
2. **Touch input ignored** — after resolving the boot loop, the `Flip Coin` LVGL button does not respond to touch because raw XPT2046 coordinates are passed to LVGL unmapped.

Both issues are in the firmware; the hardware, toolchain, and flash procedure are otherwise sound.

---

## Bug 1: Boot loop (`RTCWDT_RTC_RESET`)

### Severity
Blocker — firmware cannot be validated on physical hardware.

### Steps to reproduce
1. Build `CYD_SmokeTest2` with `pio run`.
2. Flash the firmware to a CYD2USB v3 board (manual download mode required for CH340).
3. Open a serial monitor at 115200 baud with DTR/RTS disabled.
4. Reset or power-cycle the board.

### Expected behavior
Serial output should show:

```text
Coin Toss boot
Coin Toss setup done
```

and the screen should display the `Coin Toss` title, `Tap Flip` result label, and `Flip Coin` button.

### Actual behavior
The board resets continuously. Serial output shows only the ESP32 second-stage bootloader messages:

```text
rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3f400020,len:174680
ets Jul 29 2019 12:21:46
```

The `Coin Toss boot` message is never printed, indicating the CPU resets before or during very early `setup()`.

### Root cause
The ST7789 display and XPT2046 touchscreen are initialized twice on the same SPI bus/pins:

- **First initialization:** `src/main.cpp` calls `tft.init()`, `ts.begin()`, etc., directly in `setup()`.
- **Second initialization:** the generated state machine dispatches `evDisplayOk` and `evTouchOk`, whose actions call `hal_init_display()` and `hal_init_touch()` in `src/state_machine/hal_service.cpp`.
- `src/state_machine/hal_service.cpp` declares its own `TFT_eSPI g_tft` and `XPT2046_Touchscreen g_ts` instances and performs a second `g_tft.init()` / `g_ts.begin()` cycle.

Re-initializing the SPI display controller while it is already in use causes the ESP32 to hang, triggering the RTC watchdog reset.

### Proposed fix
Change `hal_init_display()` and `hal_init_touch()` in `src/state_machine/hal_service.cpp` into state-machine probes that only record success. The one-time hardware initialization in `src/main.cpp` should remain the authoritative initialization path.

Specific changes for `src/state_machine/hal_service.cpp`:

```cpp
bool hal_init_display() {
    if (g_display_initialized) return g_display_ok;
    g_display_initialized = true;
    // Display is already initialized by main.cpp.
    g_display_ok = true;
    return true;
}

bool hal_init_touch() {
    if (g_touch_initialized) return g_touch_ok;
    g_touch_initialized = true;
    // Touch controller is already initialized by main.cpp.
    g_touch_ok = true;
    return true;
}
```

### Files affected
- `src/state_machine/hal_service.cpp`

---

## Bug 2: Touch coordinates not mapped to screen pixels

### Severity
Major — UI is unresponsive even though the firmware boots and touch events are detected.

### Steps to reproduce
1. Apply the fix for Bug 1 and re-flash.
2. Open the serial monitor.
3. Touch anywhere on the screen, especially the `Flip Coin` button area.

### Expected behavior
Touching the `Flip Coin` button should fire `LV_EVENT_CLICKED`, transition the state machine to `Flipping`, and update the result label to `Heads` or `Tails`.

### Actual behavior
Touch events are logged (raw XPT2046 values such as `TOUCH x=2154 y=2603 z=2785`), but the LVGL button does not respond. The coin flip never executes.

### Root cause
`src/ui/touch_mapping.cpp` passes raw 12-bit XPT2046 coordinates (range ~0–4095) directly to LVGL as `data->point.x` / `data->point.y`. LVGL expects pixel coordinates (0–239 for X, 0–319 for Y in portrait mode). Because the reported pointer is always far outside the display rectangle, LVGL never recognizes a hit on the button.

### Proposed fix
Map raw touch values to screen pixels using the Arduino `map()` function and clamp to the display bounds.

Changes for `include/hal_config.h` (add calibration constants):

```cpp
#define HAL_TOUCH_MIN_X 200
#define HAL_TOUCH_MAX_X 3800
#define HAL_TOUCH_MIN_Y 200
#define HAL_TOUCH_MAX_Y 3800
```

Changes for `src/ui/touch_mapping.cpp`:

```cpp
#include "hal_config.h"

static int16_t clamp(int16_t v, int16_t min, int16_t max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

// Inside ui_touch_read():
if (touchscreen_instance->touched()) {
    TS_Point p = touchscreen_instance->getPoint();
    int16_t x = map(p.x, HAL_TOUCH_MIN_X, HAL_TOUCH_MAX_X, 0, HAL_DISPLAY_WIDTH - 1);
    int16_t y = map(p.y, HAL_TOUCH_MIN_Y, HAL_TOUCH_MAX_Y, 0, HAL_DISPLAY_HEIGHT - 1);
    data->point.x = clamp(x, 0, HAL_DISPLAY_WIDTH - 1);
    data->point.y = clamp(y, 0, HAL_DISPLAY_HEIGHT - 1);
    data->state = LV_INDEV_STATE_PRESSED;
} else {
    data->state = LV_INDEV_STATE_RELEASED;
}
```

The calibration constants are starting values for CYD2USB v3 in portrait orientation (USB at bottom). Per-unit variation may require minor adjustment.

### Files affected
- `include/hal_config.h`
- `src/ui/touch_mapping.cpp`

---

## Validation log

After applying both proposed fixes on a test build, the firmware booted successfully and the coin-flip workflow operated end-to-end:

```text
Coin Toss boot
Coin Toss setup done
UI: Flip button clicked
INTEGRATION: Flip pressed -> evButtonClick
INTEGRATION: Completing flip -> evFlipComplete
COIN: Heads
```

Raw touch logs confirmed correct mapping:

```text
TOUCH raw=(2154,2603) screen=(129,212) z=2785
```

The `Flip Coin` button responded reliably to touch.

---

## Hardware flashing notes

These procedural notes are captured separately in skill `HW-001-cyd2usb-hardware-flashing` and in `.tmp/CYD_SmokeTest2_hardware_fix_notes.md`. Key points:

- CH340 auto-reset on CYD2USB is unreliable; use manual download mode.
- Reliable sequence: unplug → hold BOOT → plug in → keep BOOT ~2s → flash with `esptool --before no-reset`.
- Open serial monitor with `--rts 0 --dtr 0` to avoid holding the ESP32 in reset.
- `RTCWDT_RTC_RESET` after `SPI_FAST_FLASH_BOOT` indicates a firmware crash/hang, not a flash error.

---

## Recommendations

1. Apply the two fixes through the normal MAOA coding workflow.
2. Re-run the hardware test plan in `docs/test_plan.json` (T03–T10) on physical CYD2USB hardware.
3. Consider whether the state-machine architecture should be refactored so that hardware initialization happens in exactly one place, preventing future double-init bugs.
4. Bump the firmware version and create a new GitHub release after hardware validation passes.
