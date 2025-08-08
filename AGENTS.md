# Agent Instructions

This repository controls addressable LEDs with an ESP32-S3 board and a small PyQt UI.

## Layout
- `platformio.ini` defines numerous build environments (e.g. `master`, `controller_slave`, `tesseratica_controller`, `lightsword`, `led_basic`, `rgbw_test`, `esp32s3_qemu`). Build with `pio run -e <env>`.
- Firmware sources live under `src/`. Reusable modules reside in `src/lib` while Python utilities and the UI live in `src/led_ui`.
- Host-side unit tests are in `tests/`.
- C++ simulators and helper tools are in `tools/`.

## Coding
- Firmware is C++17 using the Arduino framework.
- Python code targets Python 3 and should follow PEP8 style where practical.

## Testing
- After changing firmware or library code run the host tests:
  ```bash
  cd tests
  make
  ./falling_bricks_test && ./strip_state_test
  ```
- Simulation helpers in `tools` can also be rebuilt with `make` when modified.

## Parameters
- Shared enums and IDs for messages and parameters are declared in `src/lib/shared.h`.
- When new parameters or animations are added, update the UI's `src/led_ui/data/parameter_map.json` to keep the firmware and Python UI in sync.
