"""Shared helpers and paths used across the PyQt tools."""

import json
import os

# Location of the repository root data folder
DATA_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "data")
)

# JSON files exchanged with the firmware
PARAM_MAP_FILE = os.path.join(DATA_DIR, "parameter_map.json")
ANIM_MAP_FILE = os.path.join(DATA_DIR, "animation_map.json")


def get_param_name(param_id: str) -> str:
    """Get the parameter name from the ID."""
    # remove PARAM_ prefix and convert to lowercase
    return param_id.replace("PARAM_", "").lower().replace("_", " ").capitalize()


def load_json(path: str) -> dict:
    """Utility to load a JSON file, returning an empty dict on failure."""
    try:
        with open(path, "r") as f:
            return json.load(f)
    except Exception:
        return {}


def save_json(path: str, data: dict) -> None:
    """Write JSON data to disk, ignoring write errors."""
    try:
        with open(path, "w") as f:
            json.dump(data, f, indent=2)
    except Exception:
        pass

