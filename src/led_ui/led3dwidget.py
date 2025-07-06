from PyQt5.QtWidgets import QWidget, QVBoxLayout, QCheckBox
from PyQt5.QtCore import Qt
import pyqtgraph as pg
import pyqtgraph.opengl as gl
from PyQt5.QtGui import QColor
import numpy as np


class LED3DWidget(QWidget):
    def __init__(self, console, nodes, led_count):
        super().__init__()
        self.console = console
        # maintain a map of strip -> {"nodes": [(index,x,y,z)],
        #  "led_count": int, "led_colors": [(r,g,b)]}
        self.strips = {0: {"nodes": list(nodes),
                           "led_count": led_count,
                           "led_colors": [(255, 0, 0)] * led_count}}
        self.layout = QVBoxLayout(self)
        # orientation flag -- incoming coordinates use Y as the up axis
        # but GLViewWidget uses Z-up, so swap Y and Z when plotting
        self.y_up = True

        self.simulate_checkbox = QCheckBox('Simulate')
        self.simulate_checkbox.setChecked(False)
        self.simulate_checkbox.stateChanged.connect(self._sim_state_changed)
        self.updateNodesButton = pg.QtWidgets.QPushButton('Update Nodes')
        self.updateNodesButton.clicked.connect(self.request_nodes)
        self.view = gl.GLViewWidget()
        self.layout.addWidget(self.view)
        self.view.opts['distance'] = 4
        self.scatter = gl.GLScatterPlotItem()
        self.view.addItem(self.scatter)

        self.layout.addWidget(self.updateNodesButton)

        self._rebuild_positions()
        self._update_scatter()
        self._update_camera()
        self._sim_state_changed(self.simulate_checkbox.checkState())

    def request_nodes(self):
        # request nodes for all strips; device will broadcast
        self.console.send_cmd("get_nodes")

    def _sim_state_changed(self, state):
        enabled = state == Qt.Checked
        if enabled:
            self.console.send_cmd("simulate:1")
        else:
            self.console.send_cmd("simulate:-1")
        self.setVisible(enabled)

    def _rebuild_positions(self):
        all_pos = []
        for strip_index in sorted(self.strips.keys()):
            strip = self.strips[strip_index]
            nodes = list(strip.get("nodes", []))
            led_count = strip.get("led_count", 0)
            if nodes:
                last_idx = nodes[-1][0]
                if last_idx < led_count:
                    first = nodes[0]
                    nodes.append((led_count, first[1], first[2], first[3]))
            else:
                continue

            pos = []
            for i in range(len(nodes) - 1):
                start_n = nodes[i]
                end_n = nodes[i + 1]
                start_i = start_n[0]
                end_i = end_n[0]
                start_p = np.array(start_n[1:], dtype=float)
                end_p = np.array(end_n[1:], dtype=float)
                if self.y_up:
                    start_p = start_p[[0, 2, 1]]
                    end_p = end_p[[0, 2, 1]]
                count = max(0, end_i - start_i)
                for j in range(count):
                    t = j / max(1, count)
                    p = start_p + t * (end_p - start_p)
                    pos.append(p)
            strip["positions"] = np.array(pos, dtype=float)
            all_pos.append(strip["positions"])

        if all_pos:
            self.positions = np.concatenate(all_pos, axis=0)
        else:
            self.positions = np.zeros((0, 3))

        self._update_camera()

    def _update_camera(self):
        """Zoom and center the view so all points are visible."""
        if self.positions.size == 0:
            return
        mins = self.positions.min(axis=0)
        maxs = self.positions.max(axis=0)
        center = (mins + maxs) / 2.0
        span = np.linalg.norm(maxs - mins)
        try:
            self.view.opts['center'] = pg.Vector(center[0], center[1], center[2])
        except Exception:
            # fallback if pg.Vector is unavailable
            self.view.opts['center'] = center
        self.view.opts['distance'] = max(span * 1.2, 1.0)

    def _update_scatter(self):
        color_arrays = []
        for strip_index in sorted(self.strips.keys()):
            strip = self.strips[strip_index]
            leds = strip.get("led_colors", [])
            display = []
            for r, g, b in leds:
                if r == 0 and g == 0 and b == 0:
                    disp = QColor(50, 50, 50)
                else:
                    disp = QColor(r, g, b)
                display.append(disp.getRgbF())
            color_arrays.append(np.array(display, dtype=float))
        if color_arrays:
            colors = np.concatenate(color_arrays, axis=0)
        else:
            colors = np.zeros((0, 4))
        self.scatter.setData(pos=self.positions, size=5, color=colors)

    def process_string(self, string):
        if string.startswith("sim:"):
            data = string.split("sim:", 1)[1].strip()
            strip_index = 0
            if ':' in data and data.split(':', 1)[0].isdigit():
                prefix, data = data.split(':', 1)
                strip_index = int(prefix)
            colors = self.parse_rle(data)
            strip = self.strips.setdefault(strip_index, {
                "nodes": [],
                "led_count": len(colors),
                "led_colors": colors,
            })
            strip["led_colors"] = colors
            strip["led_count"] = len(colors)
            self._rebuild_positions()
            self._update_scatter()
            self._update_camera()
            return True
        if string.startswith("nodes:"):
            parts = string.split(":")
            if len(parts) >= 4:
                strip_index = int(parts[1])
                led_count = int(parts[2])
                node_count = int(parts[3])
                nodes = []
                for i in range(node_count):
                    if 4 + i < len(parts):
                        node_str = parts[4 + i]
                        if node_str:
                            n_parts = node_str.split(",")
                            if len(n_parts) >= 4:
                                idx = int(n_parts[0])
                                coords = tuple(float(x) for x in n_parts[1:4])
                                nodes.append((idx, *coords))
                strip = self.strips.setdefault(strip_index, {
                    "nodes": nodes,
                    "led_count": led_count,
                    "led_colors": [(255, 0, 0)] * led_count,
                })
                strip["nodes"] = nodes
                strip["led_count"] = led_count
                if len(strip["led_colors"]) < led_count:
                    strip["led_colors"].extend(
                        [(255, 0, 0)] * (led_count - len(strip["led_colors"]))
                    )
                elif len(strip["led_colors"]) > led_count:
                    strip["led_colors"] = strip["led_colors"][:led_count]
                self._rebuild_positions()
                self._update_scatter()
                self._update_camera()
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
                leds.extend(
                    [(qcolor.red(), qcolor.green(), qcolor.blue())] * int(count))
        return leds
