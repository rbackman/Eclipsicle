# Eclipsicle

Eclipsicle is an ESP32 based light control project. The code is organized as a PlatformIO project with multiple build environments for different boards (master, slaves, tests).  A small Python UI for development lives under `src/led_ui`.

## Building

Install the PlatformIO extension in VSCode to build and upload directly from the IDE.

Install [PlatformIO](https://platformio.org/) and run for the desired environment. For example, to build the master firmware:

```bash
pio run -e master
```

The `platformio.ini` file lists additional environments such as `slave_with_motor` or `master_with_audio` which enable optional features like audio playback or motor control. The file lists multiple "env" sections for different boards and options.

## Source overview

* `src/` – firmware sources
  * `master.cpp` / `slave.cpp` – entry points for the two board roles
  * `ledManager.*` and `stripState.*` – LED strip state machine and animations
  * `sensors.*` – abstraction for buttons, sliders and other inputs
  * `meshnet.*` – simple ESP‑Now based communication
* `src/led_ui` – Python desktop tools for testing animations
The UI environment setup is described in `src/led_ui/README.md`.

There are two major components:
1. Firmware written in C++ for the ESP32 (master and slave roles).
2. A PyQt desktop interface in `src/led_ui` that communicates with the device.

The code uses a modular `ParameterManager` class so that each subsystem can expose tunable parameters.  See `shared.h` for enums describing menus, parameters and message types. Parameters are exchanged over serial/Mesh using the IDs from this header so that the C++ firmware and Python UI stay in sync.

Recent updates added a few new LED patterns including a falling bricks build-up effect and a "nebula" gradient that uses noise for subtle color variation. The bricks pattern now supports a direction toggle and hue variance, while the nebula effect fades in using a noise driven brightness.

### UI Data Files

Parameter presets are stored under `src/led_ui/data/configurations` while animation script files live in `src/led_ui/data/animations`.  Presets now store only values that differ from the defaults in `parameter_map.json`.

Animation scripts (`*.led`) begin with a `ConfigFile:` line naming the base parameter preset. Parameters listed under the `Parameters:` section override values from that preset and `Animations:` describes the sequence of animations for the strip.

Within the UI, open an animation file and use the **Send** button to transmit it to the device. Newlines are encoded as `|` characters when sent via the `script:` command.

