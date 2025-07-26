from PyQt5.QtWidgets import QWidget, QVBoxLayout, QCheckBox
from PyQt5.QtCore import Qt
import pyqtgraph as pg
import pyqtgraph.opengl as gl
from PyQt5.QtGui import QColor
from parameter_menu import ParameterIDMap
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

        self.current_shape = None
        self.shape_item = None
        self.shape_params = {
            "pos_x": 0.0,
            "pos_y": 0.0,
            "pos_z": 0.0,
            "radius": 1.0,
            "thickness": 1.0,
        }

        self.layout.addWidget(self.updateNodesButton)

        self._rebuild_positions()
        self._update_scatter()
        self._update_camera()
        self._sim_state_changed(self.simulate_checkbox.checkState())
        self.load_nodes("led_nodes_tess.txt")

    def request_nodes(self):
        # request nodes for all strips; device will broadcast
        self.console.send_cmd("get_nodes")

    def _sim_state_changed(self, state):
        enabled = state == Qt.Checked
        if enabled:
            self.console.send_cmd("simulate:5")
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
                # Even if there are no nodes, ensure the strip has a properly shaped positions array
                strip["positions"] = np.zeros((0, 3), dtype=float)
                all_pos.append(strip["positions"])
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

            # Ensure positions array is always 2D with proper shape
            if pos:
                strip["positions"] = np.array(pos, dtype=float)
            else:
                strip["positions"] = np.zeros((0, 3), dtype=float)
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
            self.view.opts['center'] = pg.Vector(
                center[0], center[1], center[2])
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

            # Ensure color array is always 2D with proper shape
            if display:
                color_arrays.append(np.array(display, dtype=float))
            else:
                color_arrays.append(np.zeros((0, 4), dtype=float))

        if color_arrays:
            colors = np.concatenate(color_arrays, axis=0)
        else:
            colors = np.zeros((0, 4))
        self.scatter.setData(pos=self.positions, size=5, color=colors)

    def save_nodes(self):
        """Save the current nodes to a file."""
        with open("led_nodes.txt", "w") as f:
            for strip_index, strip in self.strips.items():
                nodes = strip.get("nodes", [])
                led_count = strip.get("led_count", 0)
                f.write(f"segments:{strip_index}:{led_count}:{len(nodes)}:")
                for idx, x, y, z in nodes:
                    if (idx == len(nodes)-2):
                        f.write(f"{idx},{x},{y},{z}")
                    else:
                        f.write(f"{idx},{x},{y},{z}:")
                f.write("\n")

    def load_nodes(self, filename):
        """Load nodes from a file."""
        # clear existing strips
        self.strips.clear()
        self.strips[0] = {"nodes": [], "led_count": 0, "led_colors": []}
        self.positions = np.zeros((0, 3))
        self.scatter.setData(pos=self.positions, size=5, color=[])
        self.shape_item = None
        try:
            with open(filename,
                      "r") as f:
                for line in f:
                    if self.process_string(line.strip()):
                        continue
            self._rebuild_positions()
            self._update_scatter()
            self._update_camera()
            print(f"Loaded nodes from {filename}")
        except Exception as e:
            print(f"Error loading nodes from {filename}: {e}")

    def process_string(self, string):
        if string.startswith("sim:"):
            # data = string.split("sim:", 1)[1].strip()
            # strip_index = 0
            # if ':' in data and data.split(':', 1)[0].isdigit():
            #     prefix, data = data.split(':', 1)
            #     strip_index = int(prefix)
            # colors = self.parse_rle(data)
            # strip = self.strips.setdefault(strip_index, {
            #     "nodes": [],
            #     "led_count": len(colors),
            #     "led_colors": colors,
            # })
            # strip["led_colors"] = colors
            # strip["led_count"] = len(colors)
            # self._rebuild_positions()
            # self._update_scatter()
            # self._update_camera()
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
                self.save_nodes()
                return True

        if string.startswith("segments:"):
            parts = string.split(":")

            if len(parts) >= 3:
                segmentName = parts[1]
                rest = parts[2:]
                nodes = []
                led_count = 0
                print(f"Processing segment: {segmentName} with data: {rest}")
                if len(rest) < 1:
                    print("No nodes found in segment data.")
                    return False
                for node_str in rest:
                    if node_str and len(node_str) > 3:
                        n_parts = node_str.split(",")
                        print(f"Processing node: {n_parts}")
                        if len(n_parts) >= 4:
                            idx = int(n_parts[0])
                            coords = tuple(float(x) for x in n_parts[1:4])
                            nodes.append((idx, *coords))
                            led_count = max(led_count, idx + 1)

                strip = self.strips.setdefault(len(self.strips), {
                    "nodes": nodes,
                    "led_count": led_count,
                    "led_colors": [(255, 0, 0)] * led_count,
                })
                strip["nodes"] = nodes
                # strip["led_count"] = led_count
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

    # ------------------------------------------------------------------
    def animation_changed(self, name: str):
        name = name.lower()
        if name == "sphere":
            self.current_shape = "sphere"
        elif name == "plane":
            self.current_shape = "plane"
        else:
            self.current_shape = None
        self._update_shape()

    def parameter_changed(self, pname: str, val):

        print(f"Parameter changed: {pname} = {val}")
        if pname == "PARAM_POS_X":
            self.shape_params["pos_x"] = float(val)
        elif pname == "PARAM_POS_Y":
            self.shape_params["pos_y"] = float(val)
        elif pname == "PARAM_POS_Z":
            self.shape_params["pos_z"] = float(val)
        elif pname == "PARAM_RADIUS":
            self.shape_params["radius"] = float(val)
        elif pname == "PARAM_THICKNESS":
            self.shape_params["thickness"] = float(val)
        self._update_shape()

    def _update_shape(self):
        if self.shape_item:
            self.view.removeItem(self.shape_item)
            self.shape_item = None

        if self.current_shape == "sphere":
            md = gl.MeshData.sphere(rows=16, cols=16)
            item = gl.GLMeshItem(meshdata=md, smooth=True,
                                 color=(1, 0, 0, 0.3), drawEdges=False)
            item.setGLOptions("additive")
            r = self.shape_params.get("radius", 1.0)
            item.scale(r, r, r)
            pos = np.array([
                self.shape_params.get("pos_x", 0.0),
                self.shape_params.get("pos_y", 0.0),
                self.shape_params.get("pos_z", 0.0),
            ], dtype=float)
            if self.y_up:
                pos = pos[[0, 2, 1]]
            item.translate(pos[0], pos[1], pos[2])
            self.shape_item = item
            self.view.addItem(item)
        elif self.current_shape == "plane":
            if self.positions.size == 0:
                size_x = size_y = 1.0
            else:
                mins = self.positions.min(axis=0)
                maxs = self.positions.max(axis=0)
                size_x = maxs[0] - mins[0]
                size_y = maxs[1] - mins[1]
            verts = np.array([
                [-size_x / 2, -size_y / 2, 0],
                [size_x / 2, -size_y / 2, 0],
                [size_x / 2, size_y / 2, 0],
                [-size_x / 2, size_y / 2, 0],
            ])
            faces = np.array([[0, 1, 2], [0, 2, 3]])
            md = gl.MeshData(vertexes=verts, faces=faces)
            item = gl.GLMeshItem(meshdata=md, smooth=False,
                                 color=(0, 0, 1, 0.3), drawEdges=False)
            item.setGLOptions("additive")
            pos = np.array([
                self.shape_params.get("pos_x", 0.0),
                self.shape_params.get("pos_y", 0.0),
                self.shape_params.get("pos_z", 0.0),
            ], dtype=float)
            if self.y_up:
                pos = pos[[0, 2, 1]]
            item.translate(pos[0], pos[1], pos[2])
            self.shape_item = item
            self.view.addItem(item)
