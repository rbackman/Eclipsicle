"""Thin ctypes wrapper around the C++ LED simulation library."""

import ctypes
import sys
from pathlib import Path


def _load_lib() -> ctypes.CDLL:
    names = {
        "win32": "led_sim.dll",
        "darwin": "libled_sim.dylib",
    }
    lib_name = names.get(sys.platform, "libled_sim.so")
    alt_name = "led_sim.so" if sys.platform not in ("win32", "darwin") else lib_name

    base_paths = [
        Path(__file__).resolve().parent,
        Path(__file__).resolve().parents[2] / "tools" / "led_sim",
    ]

    for folder in base_paths:
        for name in (lib_name, alt_name):
            path = folder / name
            if path.exists():
                return ctypes.CDLL(str(path))
    raise OSError(f"Could not locate {lib_name}. Build it under tools/led_sim.")


_LIB = _load_lib()


class StripSim:
    """Wrapper class exposing the minimal API used by the Python UI."""

    def __init__(self, led_count: int):
        _LIB.stripsim_create.restype = ctypes.c_void_p
        self._obj = _LIB.stripsim_create(ctypes.c_int(led_count))

    def __del__(self):
        if getattr(self, "_obj", None):
            _LIB.stripsim_destroy(ctypes.c_void_p(self._obj))
            self._obj = None

    def set_animation(self, anim: int) -> None:
        _LIB.stripsim_set_animation(ctypes.c_void_p(self._obj), ctypes.c_int(anim))

    def update(self) -> None:
        _LIB.stripsim_update(ctypes.c_void_p(self._obj))

    def get_rle(self) -> str:
        _LIB.stripsim_get_rle.restype = ctypes.c_char_p
        return _LIB.stripsim_get_rle(ctypes.c_void_p(self._obj)).decode("utf-8")

