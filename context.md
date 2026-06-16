# CYD_SmokeTest2 — Release Context

| Field | Value |
|-------|-------|
| Project | CYD_SmokeTest2 |
| Version | v0.1.1 |
| Branch | main |
| Release Date | 2026-06-16 |
| Workflow ID | wvc_20260616_130541 |
| Revision Type | bug_fix |
| GitHub Release | https://github.com/CSprinkle93065/CYD_SmokeTest2/releases/tag/v0.1.1 |
| GitHub Repository | https://github.com/CSprinkle93065/CYD_SmokeTest2 |

---

## App State Summary

- Firmware boots on CYD2USB v3 (ESP32-2432S028R) and in the Wokwi simulator without fatal errors.
- Wokwi boot test (T01_BOOT) passes: the device emits `Coin Toss setup done` within the timeout.
- The LVGL UI displays the title, result label, and "Flip Coin" button.
- Touch input is mapped from raw XPT2046 coordinates to 240×320 screen pixels; the coin-flip action is verified on physical hardware.
- Random selection uses the ESP32 hardware RNG (`esp_random()`) returning `HEADS` or `TAILS`.

---

## v0.1.1 Bug Fixes

| Bug | Symptom | Fix |
|-----|---------|-----|
| Double HAL initialization | `RTCWDT_RTC_RESET` boot loop | `hal_init_display()` and `hal_init_touch()` now record success without re-initializing hardware already initialized by `main.cpp`. |
| Unmapped touch coordinates | Flip Coin button ignored touch | Raw 12-bit XPT2046 values are mapped to screen pixels in `touch_mapping.cpp` using calibration constants from `hal_config.h`. |

---

## Flash Instructions

1. Download the release asset `CYD_SmokeTest2_v0.1.1.zip` or use the attached `firmware.bin`.
2. With the CYD2USB v3 board unplugged, hold the **BOOT** button.
3. Plug in USB while holding BOOT, keep BOOT pressed for ~2 seconds, then release.
4. Flash with PlatformIO or `esptool.py --before no_reset --after hard_reset write_flash 0x1000 firmware.bin`.
5. Open the serial monitor at 115200 baud with `--rts 0 --dtr 0` to avoid holding the ESP32 in reset.
6. After flashing, reset or power-cycle the board. You should see:

```text
Coin Toss boot
Coin Toss setup done
```

---

## Known Limitations

- **Wokwi XPT2046 unsupported:** The Wokwi simulator reports `[unknown-part-type] Unknown part type "wokwi-xpt2046"`. Automated touch-driven tests (e.g., tapping the Flip Coin button) cannot be run in simulation.
- **No host test harness:** There is no native/host PlatformIO test environment, so `coin_flip()` is verified by code review rather than automated unit test.
- **Per-unit touch calibration:** The default XPT2046 calibration constants are starting values for CYD2USB v3 portrait orientation (USB at bottom); minor per-unit adjustment may be required.
- **Manual tests not executed:** State-transition tests T03–T10 and the display/touch error-injection tests are documented but require manual execution on hardware.

---

## Fork Branch Registry

| Fork Owner | Branch | Purpose | Status |
|------------|--------|---------|--------|
| (placeholder) | (placeholder) | (placeholder) | (placeholder) |

Add new forks to this table when branching for custom features or hardware variants.
