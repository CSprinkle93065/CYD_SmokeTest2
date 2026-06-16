# CYD_SmokeTest2 — QA Results (Re-run after Test Plan Update)

| Field | Value |
|-------|-------|
| Workflow ID | `wvc_20260616_043904` |
| Project | `CYD_SmokeTest2` |
| Version | `0.1.0` |
| Revision type | `new_project` |
| QA Stage | 15 — QA Agent (re-run) |
| Date | 2026-06-16 |
| Tester | MAOA QA Agent (automated) |

---

## 1. Scope

This report covers the updated structured test plan in `docs/test_plan.json`.

- **Automated tests only:** tests whose `environment` is `wokwi` or `target` are executed.
- **Manual / code_review tests:** are documented but **not executed**, using the manual procedure from `test_plan.json`.
- **Build metrics:** captured from `pio run`.
- **No source code was modified** to make any test pass.

For this project the only automated test is **T01_BOOT** (`environment: wokwi`).
All other tests are manual or code-review only.

---

## 2. Build Results (`pio run`)

```bash
cd projects/CYD_SmokeTest2
source ../../.venv/Scripts/activate
pio run -e esp32dev
```

**Result: PASS**

```text
Processing esp32dev (platform: espressif32; board: esp32dev; framework: arduino)
...
RAM:   [===       ]  25.9% (used 84884 bytes from 327680 bytes)
Flash: [====      ]  44.1% (used 577817 bytes from 1310720 bytes)
========================= [SUCCESS] Took 4.92 seconds =========================
```

| Metric | Value |
|--------|-------|
| Environment | `esp32dev` |
| Board | Espressif ESP32 Dev Module (240 MHz, 320 KB RAM, 4 MB Flash) |
| Firmware binary | `.pio/build/esp32dev/firmware.bin` (578,176 bytes) |
| Static RAM used | 84,884 bytes / 327,680 bytes (25.9%) |
| Flash used (app partition) | 577,817 bytes / 1,310,720 bytes (44.1%) |
| Estimated runtime free heap | ~242,796 bytes (327,680 - 84,884) before LVGL/task allocators. Not measured at runtime because the firmware does not emit `ESP.getFreeHeap()`. |

---

## 3. Automated Test Execution

### 3.1 T01_BOOT — Wokwi boot smoke test

| Attribute | Value |
|-----------|-------|
| Test ID | T01_BOOT |
| Sequence | POSITIVE_PATH |
| Environment | `wokwi` |
| Starting state | `[*]` |

**Command executed:**

```bash
wokwi-cli \
  --diagram-file wokwi/diagram.json \
  --elf .pio/build/esp32dev/firmware.elf \
  --timeout 15000 \
  --expect-text "Coin Toss setup done" \
  --serial-log-file wokwi/wokwi_boot.log
```

The `WOKWI_CLI_TOKEN` environment variable was sourced from the workspace `.env` file before invoking `wokwi-cli`.

**Result: PASS**

```text
Wokwi CLI v0.26.1 (9d71b975b7eb)
Diagram issues in ...\wokwi\diagram.json:
✗ [unknown-part-type] Unknown part type "wokwi-xpt2046" for part "touch" (touch)
ℹ [unsupported-part] Part "esp" uses undocumented type "wokwi-esp32-devkit-v1". This part may change or be removed in future versions. (esp)
Connected to Wokwi Simulation API 1.0.0-20260614-gca82895e
Starting simulation...
ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
...
Coin Toss boot
Coin Toss setup done
TOUCH x=-4096 y=8191 z=4095

Expected text found: "Coin Toss setup done"
TEST PASSED.
```

The firmware reaches the `Ready` state and emits the required serial message.

**Captured boot trace** (`wokwi/wokwi_boot.log`):

```text
ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1156
load:0x40078000,len:11456
ho 0 tail 12 room 4
load:0x40080400,len:2972
entry 0x400805dc
Coin Toss boot
Coin Toss setup done
TOUCH x=-4096 y=8191 z=4095
```

The `wokwi-xpt2046` diagram warning is informational for T01; it does not affect the boot-only assertion and is the reason the flip sequence cannot be automated in Wokwi.

---

## 4. Manual and Code-Review Tests (NOT EXECUTED)

The following tests were **not executed** in this QA run because they require manual hardware interaction or source-code review. Each row reproduces the manual procedure and expected behavioral outcome from `docs/test_plan.json`.

| Test ID | Sequence | Environment (plan) | Starting state | Manual / Review procedure | Expected behavioral outcome | Status |
|---------|----------|--------------------|----------------|---------------------------|-----------------------------|--------|
| T02_COIN_FLIP_HOST | CODE_REVIEW | `code_review`¹ | N/A | Review `src/state_machine/coin_logic.cpp` and observe the `COIN:` output during the on-target T03 flip test. | `coin_flip()` returns only `coin_side_t::HEADS` or `coin_side_t::TAILS`; implementation uses `esp_random() % 2u`; distribution is non-deterministic across runs. | NOT_EXECUTED / CODE_REVIEW |
| T03_FLIP_SEQUENCE | POSITIVE_PATH | `manual` | Ready | Power-cycle target, wait for `Coin Toss setup done`, then tap the on-screen **Flip Coin** button twice. | Each tap prints `UI: Flip button clicked` → `INTEGRATION: Flip pressed -> evButtonClick` → `COIN: Heads` or `COIN: Tails`. `lblResult` updates from `Tap Flip` to the result. `btnFlip` is disabled only while in `Flipping`. | NOT_EXECUTED / MANUAL |
| T04_RESET | POSITIVE_PATH | `manual` | ResultShown | Trigger `evReset` (reset request when `ui_ready` is true) via the configured reset mechanism. | System returns to `Ready`; `lblResult` shows `Tap Flip`; `lblTitle` remains `Coin Toss`; `btnFlip` visible and enabled; no error screen. | NOT_EXECUTED / MANUAL |
| T05_DISPLAY_INIT_FAIL | DISPLAY_INIT_ERROR | `manual` | InitDisplay | Power-cycle with display disconnected or an invalid `User_Setup.h`. | System enters `Error_Display`; full-screen red `ERR: Display init failed`; title/result/button hidden/disabled; error logged. | NOT_EXECUTED / MANUAL |
| T06_TOUCH_INIT_FAIL | TOUCH_INIT_ERROR | `manual` | InitTouch | Power-cycle with XPT2046 CS/TCLK/TDIN disconnected. | System enters `Error_Touch`; full-screen red `ERR: Touch init failed`; title/result/button hidden/disabled; error logged. | NOT_EXECUTED / MANUAL |
| T07_RUNTIME_DISPLAY_FAIL | POSITIVE_PATH | `manual` | Ready | After boot, briefly disconnect the display data cable or force a TFT driver fault. | System transitions `Ready -> Error_Display` and logs the runtime failure. | NOT_EXECUTED / MANUAL |
| T08_RUNTIME_TOUCH_FAIL | RUNTIME_TOUCH_ERROR | `manual` | Ready | After boot, briefly disconnect the touch controller or force a touch driver fault. | System transitions `Ready -> Error_Touch` and logs the runtime failure. | NOT_EXECUTED / MANUAL |
| T09_ERROR_REBOOT_DISPLAY | DISPLAY_INIT_ERROR | `manual` | Error_Display | Press the board reset button while in `Error_Display`. | System enters `Halted` and reboots; screen goes black; after restart `Coin Toss setup done` reappears and app returns to `Ready`. | NOT_EXECUTED / MANUAL |
| T10_ERROR_REBOOT_TOUCH | TOUCH_INIT_ERROR | `manual` | Error_Touch | Press the board reset button while in `Error_Touch`. | System enters `Halted` and reboots; screen goes black; after restart `Coin Toss setup done` reappears and app returns to `Ready`. | NOT_EXECUTED / MANUAL |

¹ `test_plan.json` stores T02 with `environment: manual`, but its `sequence_id` is `CODE_REVIEW` and its `automation_method` describes a code-review compensating check. This report treats T02 as a documented code-review item and does not execute it.

---

## 5. Code Review Evidence (T02)

Because no host/native PlatformIO test environment exists, the production implementation of `coin_flip()` was reviewed:

- File: `src/state_machine/coin_logic.cpp`
- Implementation: `return (esp_random() % 2u == 0u) ? coin_side_t::HEADS : coin_side_t::TAILS;`
- Result set: `{HEADS, TAILS}`
- No other values are possible.
- `esp_random()` is the ESP32 hardware RNG, so the distribution is non-deterministic across runs.

This satisfies the behavioral intent of T02, but it is a review, not an automated execution.

---

## 6. Quality Gate Assessment

| Gate | Criterion | Verdict | Notes |
|------|-----------|---------|-------|
| G15.1 | Every automated test in `test_plan.json` passes. Zero failures. Zero collection errors. | **PASS** | Only T01_BOOT is automated; it passes. No other automated tests exist. |
| G15.2 | Every test verifies a behavioral outcome. | **PASS** | T01 verifies the boot serial message/state. T02 verifies the coin-flip return-value contract. Manual tests verify state transitions, screen content, and serial output rather than widget existence alone. |
| G15.3 | Manual/code_review verification procedures are documented. | **PASS** | Section 4 documents the manual/code-review procedure and expected outcome for every non-automated test (T02–T10). |
| G15.4 | Flash and RAM usage metrics captured. | **PASS** | Build metrics recorded in Section 2. |
| G15.5 | Wokwi trace summary captured for simulator tests. | **PASS** | T01 boot trace captured in `wokwi/wokwi_boot.log` and excerpted in Section 3.1. |

---

## 7. Untestable Transitions

Per `test_plan.json`:

| Transition | Event | Reason |
|------------|-------|--------|
| Ready -> Flipping | evButtonClick | Wokwi CLI does not model the XPT2046 touch controller. |
| ResultShown -> Flipping | evButtonClick | Wokwi CLI does not model the XPT2046 touch controller. |

These transitions are therefore covered only by the manual T03 procedure on physical CYD2USB hardware.

---

## 8. Issues and Recommendations

1. **Wokwi XPT2046 part unsupported**
   - Impact: Blocks automated touch-driven tests (T03) in simulation.
   - Status: Accepted; T03 is now classified as `manual` in the updated test plan.

2. **No host/native PlatformIO test environment for `coin_flip()`**
   - Impact: T02 cannot be executed automatically.
   - Status: Accepted; T02 is now classified as `code_review` and verified by inspecting `src/state_machine/coin_logic.cpp`.

3. **Runtime heap not measured**
   - The firmware does not print `ESP.getFreeHeap()`. Add a one-time print after `setup()` if runtime free-heap reporting is required.

---

## 9. Artifacts Produced

- `c:/Users/terav/WORKSPACE_MAOA_WinAppVibeCoder -Kimi/projects/CYD_SmokeTest2/docs/qa_results.md`
- `c:/Users/terav/WORKSPACE_MAOA_WinAppVibeCoder -Kimi/projects/CYD_SmokeTest2/wokwi/wokwi_boot.log`

---

## 10. Overall QA Verdict

**PASS** — The only automated test (T01_BOOT) passes, the firmware builds cleanly, flash/RAM metrics are captured, the Wokwi boot trace is recorded, and all manual/code-review tests are documented with their procedures and expected behavioral outcomes.
