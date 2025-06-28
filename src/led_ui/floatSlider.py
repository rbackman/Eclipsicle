
from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QSlider, QDoubleSpinBox
)

from PyQt5.QtCore import Qt, pyqtSignal

from shared import get_param_name


class FloatSliderWithSpinBox(QWidget):
    sendSignal = pyqtSignal(float)

    def __init__(self, param):
        super().__init__()
        self.param = param

        self.slider = QSlider(Qt.Horizontal)
        self.slider.setRange(int(param['min'] * 100), int(param['max'] * 100))

        self.spin = QDoubleSpinBox()
        self.spin.setDecimals(2)
        self.spin.setRange(param['min'], param['max'])
        self.spin.setSingleStep(0.01)

        self.label = QLabel(get_param_name(param['id']))

        self.slider.valueChanged.connect(self._on_slider_change)
        self.spin.valueChanged.connect(self._on_spin_change)

        layout = QVBoxLayout(self)
        layout.addWidget(self.label)
        hbox = QHBoxLayout()
        hbox.addWidget(self.slider)
        hbox.addWidget(self.spin)
        layout.addLayout(hbox)

    def _on_slider_change(self, v):
        f = v / 100.0
        if self.spin.value() != f:
            self.spin.setValue(f)
        self.sendSignal.emit(f)

    def _on_spin_change(self, v):
        i = int(v * 100)
        if self.slider.value() != i:
            self.slider.setValue(i)
        self.sendSignal.emit(v)
