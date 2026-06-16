# Assessment: Firmware Logic Code Critic (Stage 12)

**Project:** CYD_SmokeTest2  
**Version:** 0.1.0  
**Workflow ID:** wvc_20260616_043904  
**Parallel Track:** firmware_logic  

**Verdict:** GO

---

## Findings

### G12.1 — Every transition in `state_machine.puml` has a corresponding code path

**[PASS]** All 14 transitions declared in `docs/state_machine.puml` are represented in code:

| # | PUML Transition | Code Location |
|---|-----------------|---------------|
| 1 | `[*] -> Initializing` (boot / app_init) | `StateMachine` constructor sets `state_ = State::Initializing`; `app::app_init()` performs boot-time probes |
| 2 | `Initializing -> InitDisplay` | `state_machine_generated.cpp:49-52` |
| 3 | `InitDisplay -> InitTouch` | `state_machine_generated.cpp:31-33` |
| 4 | `InitDisplay -> Error_Display` | `state_machine_generated.cpp:34-37` |
| 5 | `InitTouch -> Ready` | `state_machine_generated.cpp:40-42` |
| 6 | `InitTouch -> Error_Touch` | `state_machine_generated.cpp:43-46` |
| 7 | `Ready -> Flipping` | `state_machine_generated.cpp:55-57` |
| 8 | `Flipping -> ResultShown` | `state_machine_generated.cpp:23-26` |
| 9 | `ResultShown -> Flipping` | `state_machine_generated.cpp:67-69` |
| 10 | `ResultShown -> Ready` | `state_machine_generated.cpp:70-72` |
| 11 | `Ready -> Error_Display` | `state_machine_generated.cpp:58-60` |
| 12 | `Ready -> Error_Touch` | `state_machine_generated.cpp:61-63` |
| 13 | `Error_Display -> Halted` | `state_machine_generated.cpp:11-14` |
| 14 | `Error_Touch -> Halted` | `state_machine_generated.cpp:17-20` |

The `StateMachine::dispatch(Event, Context&)` switch statement covers every state/event/guard combination shown in the diagram.

### G12.2 — Guards reference dependency manifest facts or runtime probes, not hardcoded assumptions

**[PASS]** Every implemented guard evaluates either a member of `DependencyFacts` (documented as derived from `dependency_manifest.json`) or a live runtime probe:

- `DependencyFacts::*` constants encode manifest facts (framework, platform, libraries, serial requirement, reboot availability).
- Runtime probes are provided by `hal_*` helpers and `HardwareStatus` flags (`ctx_.hw.display_ok`, `ctx_.hw.touch_ok`, `ctx_.hw.btnFlip_enabled`, `ctx_.hw.ui_ready`).
- No guard contains a bare literal `true`/`false` or an unchecked assumption.

Caveat: the `framework_ready && libs_ready` guard on the `[*] -> Initializing` transition is not explicitly evaluated inside the state machine; instead, `app_init()` populates `HardwareStatus` from `DependencyFacts` before the first dispatch. This is acceptable because the failure path for missing framework/libraries is not modeled in the PUML (no boot-error state exists).

### G12.3 — No LVGL or UI toolkit imports in state machine code

**[PASS]** No file under `src/state_machine/` includes LVGL headers (`lvgl.h`, `lv_*.h`) or any UI toolkit. UI side effects are injected through function pointers (`ui_create_fn_t`, `ui_set_result_fn_t`) registered by the UI layer. `hal_service.cpp` legitimately includes `TFT_eSPI.h` and `XPT2046_Touchscreen.h` for hardware probing, as permitted by its own header comment, and explicitly avoids LVGL.

### G12.4 — All external calls have defined error-state transitions; no silent exception swallowing

**[PASS]** The state machine dispatches to error states for every modeled failure mode:

- Display init failure → `Error_Display` (`InitDisplay` and `Ready`).
- Touch init failure → `Error_Touch` (`InitTouch` and `Ready`).
- Reboot request → `Halted` (`Error_Display`, `Error_Touch`).

There are no `try`/`catch` blocks in the state machine layer and therefore no silent exception swallowing. Arduino-ESP32 builds normally run with exceptions disabled; external calls that fault are expected to trigger a watchdog/reset rather than be caught silently.

### G12.5 — Generated code matches `state_machine.puml` transition-for-transition

**[PASS]** `state_machine_generated.cpp` is a direct mechanical encoding of `docs/state_machine.puml`. State enum, event enum, guard virtual methods, action virtual methods, and the `dispatch` switch all align one-to-one with the PUML transitions, guards, and actions listed in the table for G12.1. The generated header also declares `State::Initializing` as the default state, matching the `[*]` pseudo-state.

### G12.6 — No blocking delays in state-machine update loops

**[PASS]** A search for `delay(`, `vTaskDelay`, `sleep`, and long `while` loops across `src/state_machine/` returned no matches. `StateMachine::dispatch()` is a pure, non-blocking switch statement. The only potentially blocking operations are inside the TFT_eSPI/XPT2046 driver calls in `hal_service.cpp`, which are outside the state machine loop.

### G12.7 — All GPIO access goes through HAL/pins.h layer; ISR and watchdog safe

**[PASS]** All GPIO identifiers used in the firmware logic layer are defined in `include/pins.h` (`TFT_MOSI`, `TFT_SCK`, `TOUCH_CS`, `TOUCH_CLK`, etc.). `hal_service.cpp` consumes these constants for SPI/touch setup; the state machine itself never touches GPIO registers. The dispatch function contains no loops, no delays, and no ISR-disabled critical sections, so it is watchdog safe.

---

## Notes

- The `DependencyFacts` values are hard-coded as `constexpr bool` in `state_machine.h`. They mirror the current `dependency_manifest.json` and are clearly labeled as manifest-derived facts. Future revisions should consider generating this struct from the manifest to avoid drift, but the current implementation satisfies the gate criterion.
- The `[*] -> Initializing` boot guard (`framework_ready && libs_ready`) is implicit rather than evaluated inside `dispatch()`. Because the PUML does not define a boot-failure terminal state, this simplification is consistent with the design.
