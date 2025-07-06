#!/usr/bin/env python3
import subprocess
import shutil
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
    raise RuntimeError("No C++ compiler found. Install g++ or clang++ and ensure it is in PATH")


def build_simulate_bricks(compiler):
    run([compiler, '-std=c++17', str(ROOT/'simulate_bricks.cpp'), str(TESTS/'FallingBricks.cpp'), '-o', str(ROOT/'simulate_bricks')])


def build_simulate_strip(compiler):
    cmd = [
        compiler, '-std=c++17',
        '-I'+str(SRC),
        '-I'+str(STUB),
        '-DUSE_LEDS',
        str(ROOT/'simulate_strip.cpp'),
        str(SRC/'stripState.cpp'),
        str(SRC/'animations.cpp'),
        str(SRC/'shared.cpp'),
        str(SRC/'parameterManager.cpp'),
        '-o', str(ROOT/'simulate_strip')
    ]
    run(cmd)


def main():
    try:
        compiler = detect_compiler()
    except RuntimeError as exc:
        print(exc)
        sys.exit(1)
    build_simulate_bricks(compiler)
    build_simulate_strip(compiler)


if __name__ == '__main__':
    main()

