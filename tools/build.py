#!/usr/bin/env python3
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT.parent / 'src'
TESTS = ROOT.parent / 'tests'
STUB = ROOT / 'arduino_stub'


def run(cmd):
    print(' '.join(str(c) for c in cmd))
    subprocess.check_call(cmd)


def detect_compiler():
    for c in ("g++", "clang++"):
        path = shutil.which(c)
        if path:
            return path
    raise RuntimeError(
        "No C++ compiler found. Install g++ or clang++ and ensure it is in PATH"
    )


def copy_windows_runtime(compiler: str, dest: Path) -> None:
    """Copy MinGW runtime DLLs next to our binaries on Windows."""
    if sys.platform != 'win32':
        return
    bin_dir = Path(compiler).resolve().parent
    for name in ("libstdc++-6.dll", "libgcc_s_seh-1.dll", "libwinpthread-1.dll"):
        src = bin_dir / name
        if src.exists():
            shutil.copy2(src, dest / name)


def build_simulate_bricks(compiler: str) -> None:
    run(
        [
            compiler,
            '-std=c++17',
            str(ROOT / 'simulate_bricks.cpp'),
            str(TESTS / 'FallingBricks.cpp'),
            '-o',
            str(ROOT / 'simulate_bricks'),
        ]
    )


def build_simulate_strip(compiler: str) -> None:
    cmd = [
        compiler,
        '-std=c++17',
        '-I' + str(SRC / 'lib'),
        '-I' + str(STUB),
        '-DUSE_LEDS',
        str(ROOT / 'simulate_strip.cpp'),
        str(SRC / 'lib' / 'stripState.cpp'),
        str(SRC / 'lib' / 'animations.cpp'),
        str(SRC / 'lib' / 'shared.cpp'),
        str(SRC / 'lib' / 'parameterManager.cpp'),
        '-o',
        str(ROOT / 'simulate_strip'),
    ]
    run(cmd)


def build_led_sim_library(compiler: str) -> None:
    lib_name = {
        'win32': 'led_sim.dll',
        'darwin': 'libled_sim.dylib',
    }.get(sys.platform, 'libled_sim.so')
    out_path = ROOT / 'led_sim' / lib_name
    cmd = [
        compiler,
        '-std=c++17',
        '-shared',
        '-I' + str(SRC / 'lib'),
        '-I' + str(STUB),
        '-DUSE_LEDS',
        str(ROOT / 'led_sim' / 'strip_sim.cpp'),
        str(SRC / 'lib' / 'animations.cpp'),
        str(SRC / 'lib' / 'stripState.cpp'),
        str(SRC / 'lib' / 'parameterManager.cpp'),
        str(SRC / 'lib' / 'shared.cpp'),
        '-o',
        str(out_path),
    ]
    if os.name != 'nt':
        cmd.insert(2, '-fPIC')
    run(cmd)
    if sys.platform == 'win32':
        copy_windows_runtime(compiler, out_path.parent)


def main() -> None:
    try:
        compiler = detect_compiler()
    except RuntimeError as exc:
        print(exc)
        sys.exit(1)
    build_simulate_bricks(compiler)
    build_simulate_strip(compiler)
    build_led_sim_library(compiler)
    if sys.platform == 'win32':
        copy_windows_runtime(compiler, ROOT)


if __name__ == '__main__':
    main()

