# Agent Instructions for `src/lib`

Core reusable firmware modules live here.

- `shared.h` defines enums and IDs for animations, parameters (`ANIMATION_TYPE`, `ParameterID`, `BoolParameter`, `FloatParameter`, `IntParameter`, `MenuID`, `LED_STATE`).
- `parameterManager.*` encodes and decodes int/float/bool parameters for serial or ESP-NOW messages.
- `ledManager.*` wraps FastLED and owns multiple `StripState` instances.
- `stripState.*` represents a single LED strip and manages a list of `StripAnimation` objects.
- `animations.*` implements concrete animations derived from `StripAnimation`; each animation works within its configured LED range.
- Keep header (`.h`) and implementation (`.cpp`) files in sync and target C++17.
