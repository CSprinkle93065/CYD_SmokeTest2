# CYD_SmokeTest2 — Release Context

| Field | Value |
|-------|-------|
| Project | CYD_SmokeTest2 |
| Version | v0.1.0 |
| Branch | main |
| Release Date | 2026-06-16 |
| Workflow ID | wvc_20260616_043904 |
| Revision Type | new_project |
| GitHub Release | https://github.com/CSprinkle93065/CYD_SmokeTest2/releases/tag/v0.1.0 |
| GitHub Repository | https://github.com/CSprinkle93065/CYD_SmokeTest2 |

---

## App State Summary

- Firmware boots on CYD2USB v3 (ESP32-2432S028R) and in the Wokwi simulator without fatal errors.
- Wokwi boot test (T01_BOOT) passes: the device emits `Coin Toss setup done` within the timeout.
- The LVGL UI displays the title, result label, and "Flip Coin" button.
- Touch input is mapped to the XPT2046 controller; the coin-flip action is verified on physical hardware because Wokwi does not model the XPT2046 touch controller.
- Random selection uses the ESP32 hardware RNG (`esp_random()`) returning `HEADS` or `TAILS`.

---

## Known Limitations

- **Wokwi XPT2046 unsupported:** The Wokwi simulator reports `[unknown-part-type] Unknown part type "wokwi-xpt2046"`. Automated touch-driven tests (e.g., tapping the Flip Coin button) cannot be run in simulation.
- **No host test harness:** There is no native/host PlatformIO test environment, so `coin_flip()` is verified by code review rather than automated unit test.
- **Touch coordinate flip requires hardware validation:** The touch mapping was configured for the CYD2USB v3 portrait orientation (USB connector at the bottom); confirm on physical hardware that touch coordinates match the display.
- **Manual tests not executed:** State-transition tests T03–T10 and the display/touch error-injection tests are documented but require manual execution on hardware.

---

## Fork Branch Registry

| Fork Owner | Branch | Purpose | Status |
|------------|--------|---------|--------|
| (placeholder) | (placeholder) | (placeholder) | (placeholder) |

Add new forks to this table when branching for custom features or hardware variants.
