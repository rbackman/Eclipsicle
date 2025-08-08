# Agent Instructions for `src/led_ui`

This directory contains the PyQt-based desktop tools.

- Target Python 3 and follow PEP8 where reasonable.
- Install dependencies using `environment.yml` (conda) or pip equivalents.
- `parameter_map.json` mirrors firmware parameter IDs; regenerate it by having the ESP confirm parameters to the UI.
- Preset configurations live in `data/configurations` and animation scripts in `data/animations`.
- Use `main.py` for live device control and `offline_main.py` or `sim_main.py` for local simulation.
