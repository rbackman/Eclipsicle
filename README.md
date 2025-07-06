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

Recent updates added a few LED patterns including a falling bricks build-up effect and a "nebula" gradient that uses noise for subtle color variation. The bricks animation now supports a direction toggle and per-brick hue variance with a gradient towards `HueEnd`, while the nebula effect fades in using a noise-driven brightness.

### UI Data Files

Parameter presets are stored under `src/led_ui/data/configurations` while animation script files live in `src/led_ui/data/animations`.  Presets now store only values that differ from the defaults in `parameter_map.json`.

Animation scripts (`*.led`) begin with a `ConfigFile:` line naming the base parameter preset. Parameters listed under the `Parameters:` section override values from that preset and `Animations:` describes the sequence of animations for the strip.  A `Variables:` section may optionally appear before `Parameters:` to define reusable values that can be referenced elsewhere by name.  Variable values can include simple expressions using `+`, `-`, `*` and `/` and may reference previously defined variables.

Within the UI, open an animation file and use the **Send** button to transmit it to the device. Newlines are encoded as `|` characters when sent via the `script:` command.

### Simulating with QEMU

PlatformIO can run the firmware under QEMU for automated testing.  An example
environment called `esp32s3_qemu` is provided in `platformio.ini`.  Build and
test it without uploading using:

```bash
pio test -e esp32s3_qemu --without-uploading
```

The `tool-qemu-xtensa` package used by this environment currently only
provides binaries for Linux hosts. Windows users should run PlatformIO inside
a WSL or other Linux environment in order to use QEMU.

Refer to the [PlatformIO QEMU documentation](https://docs.platformio.org/en/latest/advanced/unit-testing/simulators/qemu.html)
for more details on the simulator setup.

### Building a standalone simulator

The PyQt interface can be packaged as a Windows executable using
[PyInstaller](https://pyinstaller.org/).  First install the UI
dependencies including the `pyinstaller` package:

```bash
conda env create -f src/led_ui/environment.yml
conda activate led-ui
```

Then build the executable from `offline_main.py` using the helper script.
This step also compiles the small C++ simulators used for unit tests so
they can run without a `make` installation:

```bash
python src/led_ui/build_exe.py
```

The resulting binary will be placed under `dist/` and the simulator
executables appear in `tools/`.  This allows running the animation UI and
logic locally without connecting to an ESP32.

To quickly test the C++ animation logic on the host PC you can also build
the small demo programs in `tools/`.  A Python helper script builds both
executables without requiring `make`:

```bash
python tools/build.py
./tools/simulate_bricks
./tools/simulate_strip
```

This step requires a C++ compiler (`g++` or `clang++`) to be available on
your PATH. On Windows you can install the
[MSYS2 toolchain](https://www.msys2.org/) or the Visual Studio Build Tools
to provide one.

