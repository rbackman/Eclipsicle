from PyQt5.QtWidgets import QWidget, QVBoxLayout, QCheckBox, QSpinBox
from PyQt5.QtCore import Qt
import pyqtgraph as pg
import pyqtgraph.opengl as gl
from PyQt5.QtGui import QColor
import numpy as np

class LED3DWidget(QWidget):
    def __init__(self, console, nodes, led_count):
        super().__init__()
        self.console = console
        self.nodes = nodes
        self.led_count = led_count
        self.layout = QVBoxLayout(self)

        self.simulate_checkbox = QCheckBox('Simulate')
        self.simulate_checkbox.setChecked(False)
        self.simCountSpinbox = QSpinBox()
        self.simCountSpinbox.setRange(-1, 10000)
        self.simCountSpinbox.setValue(1)
        self.simulate_checkbox.stateChanged.connect(self._sim_state_changed)
        self.layout.addWidget(self.simCountSpinbox)

        self.view = gl.GLViewWidget()
        self.layout.addWidget(self.view)
        self.view.opts['distance'] = 4
        self.scatter = gl.GLScatterPlotItem()
        self.view.addItem(self.scatter)

        self.led_colors = [(255, 0, 0)] * self.led_count
        self._rebuild_positions()
        self._update_scatter()
        self._sim_state_changed(self.simulate_checkbox.checkState())

    def _sim_state_changed(self, state):
        enabled = state == Qt.Checked
        if enabled:
            self.console.send_cmd(
                f"simulate:{self.simCountSpinbox.value()}")
        else:
            self.console.send_cmd("simulate:-1")
        self.setVisible(enabled)

    def _rebuild_positions(self):
        pos = []
        for i in range(len(self.nodes) - 1):
            start_n = self.nodes[i]
            end_n = self.nodes[i + 1]
            start_i = start_n[0]
            end_i = end_n[0]
            start_p = np.array(start_n[1:])
            end_p = np.array(end_n[1:])
            count = end_i - start_i
            for j in range(count):
                t = j / max(1, count)
                p = start_p + t * (end_p - start_p)
                pos.append(p)
        self.positions = np.array(pos)

    def _update_scatter(self):
        colors = [QColor(r, g, b).getRgbF() for r, g, b in self.led_colors]
        self.scatter.setData(pos=self.positions, size=5, color=colors)

    def process_string(self, string):
        if string.startswith("sim:"):
            compressed_data = string.split("sim:")[1].strip()
            self.led_colors = self.parse_rle(compressed_data)
            self._update_scatter()
            return True
        return False

    def parse_rle(self, data):
        leds = []
        for segment in data.split(";"):
            if segment:
                color, count = segment.split(":")
                hc, vc = color.split(",")
                hue_byte = int(hc)
                hue = int(hue_byte * 359 / 255)
                value = int(vc)
                qcolor = QColor.fromHsv(hue, 255, value)
                leds.extend([(qcolor.red(), qcolor.green(), qcolor.blue())] * int(count))
        return leds

