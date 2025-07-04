from PyQt5.QtWidgets import QSpinBox

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QSlider,   QDoubleSpinBox
)

from PyQt5.QtCore import Qt, QTimer, pyqtSignal
from shared import get_param_name


class DebouncedSlider(QWidget):
    sendInt = pyqtSignal(int)
    sendFloat = pyqtSignal(float)

    def __init__(self, paramname, param, type="int"):
        super().__init__()
        self.param = param
        self.paramname = paramname
        self.shortname = get_param_name(paramname)
        self.type = type
        self.scale = 1000 if type == "float" else 1
        if type == "float":
            self.slider = QSlider(Qt.Horizontal)
            self.scale = 1000
            self.slider.setRange(
                int(param['min'] * self.scale), int(param['max'] * self.scale))
            self.slider.setSingleStep(1)
            self.spin = QDoubleSpinBox()
            self.spin.setRange(param['min'], param['max'])
            self.spin.setSingleStep(0.1)
            self.spin.setDecimals(2)
            self.spin.setValue(param['value'])
            self.slider.setValue(int(param['value'] * self.scale))
        elif type == "int":
            print(
                f"Creating int slider for {self.shortname} with value {param['value']} range {param['min']} to {param['max']}")
            self.slider = QSlider(Qt.Horizontal)
            self.slider.setTickPosition(QSlider.TicksBelow)
            self.slider.setTickInterval(1)
            self.slider.setRange(param['min'], param['max'])
            self.slider.setSingleStep(1)
            self.slider.setValue(param['value'])

            self.spin = QSpinBox()
            self.spin.setRange(param['min'], param['max'])
            self.spin.setSingleStep(1)
            self.spin.setValue(param['value'])

        self.label = QLabel(self.shortname)

        self.timer = QTimer(self)
        self.timer.setSingleShot(True)
        # Lower interval for more responsive UI updates
        self.timer.setInterval(50)
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
        if self.type == "float":
            self.slider.setValue(int(v * self.scale))
            self.spin.setValue(v)
        else:
            self.slider.setValue(v)
            self.spin.setValue(v)

    def setRange(self, min_val, max_val):
        """Set the range of the slider and spin box."""
        if self.type == "float":
            self.slider.setRange(int(min_val * self.scale),
                                 int(max_val * self.scale))
            self.spin.setRange(min_val, max_val)
        else:
            self.slider.setRange(min_val, max_val)
            self.spin.setRange(min_val, max_val)

    def _on_change(self, v):
        sender = self.sender()

        if self.type == "float":
            if sender == self.slider:
                float_val = self.slider.value() / self.scale
                self.spin.blockSignals(True)
                self.spin.setValue(float_val)
                self.spin.blockSignals(False)
            elif sender == self.spin:
                int_val = int(self.spin.value() * self.scale)
                self.slider.blockSignals(True)
                self.slider.setValue(int_val)
                self.slider.blockSignals(False)
        else:
            if sender == self.slider:
                self.spin.blockSignals(True)
                self.spin.setValue(v)
                self.spin.blockSignals(False)
            elif sender == self.spin:
                self.slider.blockSignals(True)
                self.slider.setValue(v)
                self.slider.blockSignals(False)

        self.timer.start()

    def _flush(self):
        if self.type == "float":
            self.sendFloat.emit(self.slider.value() / self.scale)
        else:
            self.sendInt.emit(self.slider.value())
