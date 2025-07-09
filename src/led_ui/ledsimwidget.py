from PyQt5.QtWidgets import (
    QWidget, QGraphicsView, QGraphicsScene, QGraphicsRectItem,
    QCheckBox, QSpinBox, QHBoxLayout, QToolTip
)
from PyQt5.QtCore import Qt, pyqtSignal, QRectF, QSize
from PyQt5.QtGui import QColor, QBrush, QPen, QPainter


class LedRectItem(QGraphicsRectItem):
    """Rectangle item representing a single LED with click feedback."""

    def __init__(self, rect: QRectF, color: QColor, value: int):
        super().__init__(rect)
        self.orig_value = value
        self.setBrush(QBrush(color))
        self.setPen(QPen(Qt.NoPen))

    def mousePressEvent(self, event):
        QToolTip.showText(event.screenPos(), f"Value: {self.orig_value}")
        super().mousePressEvent(event)


class LEDSimWidget(QWidget):
    update_led_signal = pyqtSignal(str)

    def __init__(self, console):
        super().__init__()
        self.console = console
        self.layout = QHBoxLayout()
        self.setLayout(self.layout)

        self.brightness_boost = 1.5  # amplify simulated brightness

        # map of strip index -> list of (hue,value)
        self.strips = {}
        self.base_height = 64
        self.simCountSpinbox = QSpinBox()
        self.simCountSpinbox.setRange(-1, 10000)
        self.simCountSpinbox.setValue(1)

        # hidden checkbox used for menu-driven simulation toggle
        self.simulate_checkbox = QCheckBox('Simulate')
        self.simulate_checkbox.setChecked(False)
        self.simulate_checkbox.stateChanged.connect(self._sim_state_changed)
        self.simulate_checkbox.setVisible(False)
        self.scene = QGraphicsScene(self)
        self.view = QGraphicsView(self.scene)
        self.view.setRenderHint(QPainter.Antialiasing)
        self.view.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.view.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        # initial height for a single strip
        self.setMaximumHeight(self.base_height)
        self.view.setFixedHeight(self.base_height)
        # checkbox isn't added to layout so users toggle via menu
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
            # message may be sim:data or sim:stripIndex:data
            data = string.split("sim:", 1)[1].strip()
            strip_index = 0
            if ':' in data and data.split(':', 1)[0].isdigit():
                prefix, data = data.split(':', 1)
                strip_index = int(prefix)
            self.update_leds(data, strip_index)
            return True
        if string.startswith("state:"):
            print(string)
            return True
        return False

    def update_leds(self, compressed_data, strip_index=0):
        leds = self.parse_rle(compressed_data)
        self.strips[strip_index] = leds
        self.draw_leds()

    def parse_rle(self, data):
        leds = []
        for segment in data.split(";"):
            if segment:
                color, count = segment.split(":")
                hc, vc = color.split(",")
                hue_byte = int(hc)
                # Incoming hue from the ESP is in the 0-255 range like FastLED's
                # CHSV. Convert it to degrees (0-359) so our Qt display matches
                # the actual LED colour. Using 359 in the scaling ensures that
                # a hue of 255 maps to 359 rather than wrapping back to 0.
                hue = int(hue_byte * 359 / 255)
                value = int(vc)
                leds.extend([(hue, value)] * int(count))
        return leds

    def draw_leds(self):
        self.scene.clear()
        if not self.strips:
            return

        num_strips = len(self.strips)
        total_height = self.base_height * num_strips
        self.setMaximumHeight(total_height)
        self.view.setFixedHeight(total_height)

        view_size: QSize = self.view.viewport().size()
        width = view_size.width()
        row_height = self.base_height

        for row, strip_index in enumerate(sorted(self.strips.keys())):
            leds = self.strips[strip_index]
            led_width = width / max(1, len(leds))
            for i, (hue, value) in enumerate(leds):
                display_value = min(255, int(value * self.brightness_boost))
                color = QColor.fromHsv(hue, 255, display_value)
                rect = QRectF(i * led_width, row * row_height, led_width, row_height)
                item = LedRectItem(rect, color, value)
                self.scene.addItem(item)

        self.scene.setSceneRect(0, 0, width, total_height)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.draw_leds()  # Redraw on resize
