# Agent Instructions for `src`

This folder holds the firmware entry points and support files.

- `slave.cpp` manages several LED strips and responds to commands over serial or ESP-NOW.
- Other mains such as `master.cpp`, `controller.cpp`,   `light_sword.cpp`, and `tess_fragment.cpp` are selected by build environments in `platformio.ini`.
- Common utilities and drivers live under `src/lib/`.
- Pin mappings and board-specific configuration reside in `boardConfigs/`.
- All firmware is compiled as C++17 and uses macros from `platformio.ini` like `USE_LEDS`, `MASTER_BOARD`, or `MESH_NET` to enable features.
