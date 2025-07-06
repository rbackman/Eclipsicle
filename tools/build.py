#!/usr/bin/env python3
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT.parent / 'src'
TESTS = ROOT.parent / 'tests'
STUB = ROOT / 'arduino_stub'


def run(cmd):
    print(' '.join(str(c) for c in cmd))
    subprocess.check_call(cmd)


def build_simulate_bricks():
    run(['g++', '-std=c++17', str(ROOT/'simulate_bricks.cpp'), str(TESTS/'FallingBricks.cpp'), '-o', str(ROOT/'simulate_bricks')])


def build_simulate_strip():
    cmd = [
        'g++', '-std=c++17',
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
    build_simulate_bricks()
    build_simulate_strip()


if __name__ == '__main__':
    main()
