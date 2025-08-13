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
        # Prefer the freshly built library under tools/led_sim. A stale
        # copy alongside this file can be missing newer symbols.
        Path(__file__).resolve().parents[2] / "tools" / "led_sim",
        Path(__file__).resolve().parent,
    ]

    for folder in base_paths:
        for name in (lib_name, alt_name):
            path = folder / name
            if path.exists():
                return ctypes.CDLL(str(path))
    raise OSError(f"Could not locate {lib_name}. Build it under tools/led_sim.")


_LIB = _load_lib()


class ParameterMessage(ctypes.Structure):
    """Mirror of the C++ parameter_message struct."""

    _fields_ = [
        ("type", ctypes.c_int),
        ("value", ctypes.c_int),
        ("boolValue", ctypes.c_bool),
        ("floatValue", ctypes.c_float),
        ("paramID", ctypes.c_int),
    ]


MESSAGE_TYPE_PARAMETER = 3
MESSAGE_TYPE_BOOL_PARAMETER = 4

# Set up function prototypes up front so a missing symbol fails fast and we
# avoid re-specifying types on each call.
_LIB.stripsim_create.restype = ctypes.c_void_p
_LIB.stripsim_destroy.argtypes = [ctypes.c_void_p]
_LIB.stripsim_set_animation.argtypes = [ctypes.c_void_p, ctypes.c_int]
_LIB.stripsim_update.argtypes = [ctypes.c_void_p]
_LIB.stripsim_get_rle.argtypes = [ctypes.c_void_p]
_LIB.stripsim_get_rle.restype = ctypes.c_char_p
_LIB.stripsim_command.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
_LIB.stripsim_command.restype = ctypes.c_bool
_LIB.stripsim_parameter.argtypes = [
    ctypes.c_void_p,
    ctypes.POINTER(ParameterMessage),
]
_LIB.stripsim_parameter.restype = ctypes.c_bool


class StripSim:
    """Wrapper class exposing the minimal API used by the Python UI."""

    def __init__(self, led_count: int):
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
        return _LIB.stripsim_get_rle(ctypes.c_void_p(self._obj)).decode("utf-8")

    def handle_cmd(self, cmd: str) -> bool:
        if cmd.startswith("p:"):
            parts = cmd.split(":", 2)
            if len(parts) >= 3:
                try:
                    pid = int(parts[1])
                except ValueError:
                    return False
                raw = parts[2]
                msg = ParameterMessage()
                msg.paramID = pid
                raw_lc = raw.lower()
                if raw_lc in {"true", "false"}:
                    msg.type = MESSAGE_TYPE_BOOL_PARAMETER
                    msg.boolValue = raw_lc == "true" or raw == "1"
                    msg.value = int(msg.boolValue)
                    msg.floatValue = float(msg.boolValue)
                else:
                    try:
                        val = float(raw)
                    except ValueError:
                        return False
                    msg.type = MESSAGE_TYPE_PARAMETER
                    msg.floatValue = val
                    msg.value = int(val)
                    msg.boolValue = bool(msg.value)
                return bool(
                    _LIB.stripsim_parameter(
                        ctypes.c_void_p(self._obj),
                        ctypes.byref(msg),
                    )
                )
            return False
        return bool(
            _LIB.stripsim_command(
                ctypes.c_void_p(self._obj),
                ctypes.c_char_p(cmd.encode("utf-8")),
            )
        )

