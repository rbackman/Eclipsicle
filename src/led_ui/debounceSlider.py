from PyQt5.QtWidgets import QSpinBox

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QSlider
)

from PyQt5.QtCore import Qt, QTimer, pyqtSignal

from shared import get_param_name


class DebouncedSlider(QWidget):
    sendSignal = pyqtSignal(int)

    def __init__(self, param):
        super().__init__()
        self.param = param

        self.slider = QSlider(Qt.Horizontal)
        self.slider.setRange(param['min'], param['max'])

        self.spin = QSpinBox()
        self.spin.setRange(param['min'], param['max'])

        self.label = QLabel(get_param_name(param['id']))

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

    def _on_change(self, v):
        if self.slider.value() != v:
            self.slider.setValue(v)
        if self.spin.value() != v:
            self.spin.setValue(v)
        self.timer.start()

    def _flush(self):
        self.sendSignal.emit(self.slider.value())
