#!/usr/bin/env python3

import sys
import os
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))

# Mock the console since we're just testing parsing


class MockConsole:
    def send_cmd(self, cmd):
        pass


try:
    from PyQt5.QtWidgets import QApplication
    from led3dwidget import LED3DWidget

    app = QApplication(sys.argv)
    widget = LED3DWidget(MockConsole(), [], 0)

    print("Testing segment loading...")
    widget.load_nodes('led_nodes_tess.txt')
    print('Successfully loaded segments file!')
    print(f'Number of strips: {len(widget.strips)}')

    for i, strip in widget.strips.items():
        print(
            f'Strip {i}: {len(strip["nodes"])} nodes, {strip["led_count"]} LEDs')
        print(
            f'  Positions shape: {strip.get("positions", np.array([])).shape}')

    print(f'Total positions shape: {widget.positions.shape}')

except Exception as e:
    print(f'Error: {e}')
    import traceback
    traceback.print_exc()
