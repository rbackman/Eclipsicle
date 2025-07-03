from PyQt5.QtWidgets import QWidget, QVBoxLayout, QGraphicsView, QGraphicsScene, QGraphicsEllipseItem, QCheckBox, QSpinBox, QHBoxLayout
from PyQt5.QtCore import Qt, pyqtSignal, QRectF, QSize
from PyQt5.QtGui import QColor, QBrush, QPen, QPainter


class LEDSimWidget(QWidget):
    update_led_signal = pyqtSignal(str)

    def __init__(self, console):
        super().__init__()
        self.console = console
        self.layout = QHBoxLayout()
        self.setLayout(self.layout)

        self.leds = []
        self.led_count = 0
        self.simCountSpinbox = QSpinBox()
        self.simCountSpinbox.setRange(-1, 10000)
        self.simCountSpinbox.setValue(1)

        self.simulate_checkbox = QCheckBox('Simulate')
        self.simulate_checkbox.setChecked(False)
        self.simulate_checkbox.stateChanged.connect(self._sim_state_changed)
        self.scene = QGraphicsScene(self)
        self.view = QGraphicsView(self.scene)
        self.view.setRenderHint(QPainter.Antialiasing)
        self.view.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.view.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setMaximumHeight(64)
        self.layout.addWidget(self.simulate_checkbox)
        self.layout.addWidget(self.simCountSpinbox)
        self.layout.addWidget(self.view)
        self._sim_state_changed(self.simulate_checkbox.checkState())

    def _sim_state_changed(self, state):
        enabled = state == Qt.Checked
        if enabled:
            self.console.send_cmd(
                f"simulate:{self.simCountSpinbox.value()}")
        else:
            self.console.send_cmd("simulate:-1")
        self.setVisible(enabled)

    def process_string(self, string):
        if string.startswith("sim:"):
            # check if there is multiple sim: in the string
            compressed_data = string.split("sim:")[1].strip()

            self.update_leds(compressed_data)
            return True
        return False

    def update_leds(self, compressed_data):
        self.leds = self.parse_rle(compressed_data)
        self.led_count = len(self.leds)
        self.draw_leds()

    def parse_rle(self, data):
        leds = []
        for segment in data.split(";"):
            if segment:
                color, count = segment.split(":")
                hc, vc = color.split(",")
                hue = int(hc)
                value = int(vc)
                color = QColor.fromHsv(hue, 255, value)
                leds.extend([color] * int(count))
        return leds

    def draw_leds(self):
        self.scene.clear()
        if self.led_count == 0:
            return

        # Get available view width and height
        view_size: QSize = self.view.viewport().size()
        width = view_size.width()
        height = view_size.height()

        padding = 2
        max_diameter = min(height - 2 * padding, width //
                           self.led_count - padding)
        diameter = max(6, max_diameter)

        for i, color in enumerate(self.leds):
            x = i * (diameter + padding)
            y = (height - diameter) / 2
            ellipse = QGraphicsEllipseItem(
                QRectF(x, y, diameter, height - 2 * padding))
            ellipse.setBrush(QBrush(color))
            ellipse.setPen(QPen(Qt.NoPen))
            self.scene.addItem(ellipse)

        # Resize scene to fit content
        self.scene.setSceneRect(0, 0, self.led_count *
                                (diameter + padding), height)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.draw_leds()  # Redraw on resize
