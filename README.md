# Eclipsicle

Eclipsicle is an ESP32 based light control project. The code is organized as a PlatformIO project with multiple build environments for different boards (master, slaves, tests).  A small Python UI for development lives under `src/led_ui`.

## Building

Install [PlatformIO](https://platformio.org/) and run for the desired environment. For example, to build the master firmware:

```bash
pio run -e master
```

The `platformio.ini` file lists additional environments such as `slave_with_motor` or `master_with_audio` which enable optional features like audio playback or motor control.

## Source overview

* `src/` – firmware sources
  * `master.cpp` / `slave.cpp` – entry points for the two board roles
  * `ledManager.*` and `stripState.*` – LED strip state machine and animations
  * `sensors.*` – abstraction for buttons, sliders and other inputs
  * `meshnet.*` – simple ESP‑Now based communication
* `src/led_ui` – Python desktop tools for testing animations

The code uses a modular `ParameterManager` class so that each subsystem can expose tunable parameters.  See `shared.h` for enums describing menus, parameters and message types.

