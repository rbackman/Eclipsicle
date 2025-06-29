from PyQt5.QtWidgets import QSpinBox

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QSlider
)

from PyQt5.QtCore import Qt, QTimer, pyqtSignal
from shared import get_param_name


class DebouncedSlider(QWidget):
    sendSignal = pyqtSignal(int)

    def __init__(self, paramname, param):
        super().__init__()
        self.param = param
        self.paramname = paramname
        self.shortname = get_param_name(paramname)
        self.slider = QSlider(Qt.Horizontal)
        self.slider.setRange(param['min'], param['max'])

        self.spin = QSpinBox()
        self.spin.setRange(param['min'], param['max'])

        self.label = QLabel(self.shortname)

        self.timer = QTimer(self)
        self.timer.setSingleShot(True)
        self.timer.setInterval(100)
        self.timer.timeout.connect(self._flush)

        self.slider.valueChanged.connect(self._on_change)
        self.spin.valueChanged.connect(self._on_change)

        lay = QVBoxLayout(self)
        lay.addWidget(self.label)

        hbox = QHBoxLayout()
        hbox.addWidget(self.slider)
        hbox.addWidget(self.spin)
        lay.addLayout(hbox)

    def setValue(self, v):
        """Set the value of the slider and spin box."""
        if self.slider.value() != v:
            self.slider.setValue(v)
        if self.spin.value() != v:
            self.spin.setValue(v)
        # self.label.setText(f"{self.shortname}: {v}")

    def setRange(self, min_val, max_val):
        """Set the range of the slider and spin box."""
        self.slider.setRange(min_val, max_val)
        self.spin.setRange(min_val, max_val)
        self.param['min'] = min_val
        self.param['max'] = max_val

    def _on_change(self, v):
        if self.slider.value() != v:
            self.slider.setValue(v)
        if self.spin.value() != v:
            self.spin.setValue(v)
        self.timer.start()

    def _flush(self):
        self.sendSignal.emit(self.slider.value())
