import sys
import json
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QListWidget, QStackedWidget,
    QLabel, QSlider, QCheckBox, QColorDialog, QPushButton, QHBoxLayout
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor
from serial_console import SerialConsole

# Example simplified data structure for menus
MENU_STRUCTURE = {
    "Main": ["Patterns", "LED", "Audio", "Debug"],
    "Patterns": ["Particles", "Rainbow", "Slider", "Random"],
    "Particles": ["Color", "Speed", "Life"],
    "LED": ["Master LED"],
}

# Example parameter mapping (use your real parameter IDs here)
PARAMETERS = {
    "Particles": [
        {"id": "PARAM_HUE", "type": "color"},
        {"id": "PARAM_PARTICLE_WIDTH", "type": "int", "min": 1, "max": 60},
    ],
    "Master LED": [
        {"id": "PARAM_MASTER_LED_HUE", "type": "color"},
        {"id": "PARAM_MASTER_LED_BRIGHTNESS", "type": "int", "min": 0, "max": 255},
        {"id": "PARAM_INVERT", "type": "bool"},
    ]
}


class ParameterMenuWidget(QWidget):
    def __init__(self, serial_console):
        super().__init__()

        self.serial_console = serial_console

        self.layout = QHBoxLayout()

        # Menu List
        self.menu_list = QListWidget()
        self.menu_list.addItems(MENU_STRUCTURE.keys())
        self.menu_list.currentItemChanged.connect(self.menu_selected)

        # Stacked widget to hold parameter UIs
        self.stack = QStackedWidget()
        self.parameter_pages = {}

        for menu in MENU_STRUCTURE.keys():
            page = QWidget()
            page.setLayout(QVBoxLayout())
            self.stack.addWidget(page)
            self.parameter_pages[menu] = page

        self.layout.addWidget(self.menu_list, 1)
        self.layout.addWidget(self.stack, 3)

        self.setLayout(self.layout)

    def menu_selected(self, current, previous):
        if current:
            menu_name = current.text()
            page = self.parameter_pages[menu_name]
            page.layout().setAlignment(Qt.AlignTop)

            # Clear current widgets
            for i in reversed(range(page.layout().count())):
                page.layout().itemAt(i).widget().setParent(None)

            # Populate parameters
            params = PARAMETERS.get(menu_name, [])
            for param in params:
                widget = self.create_param_widget(param)
                page.layout().addWidget(widget)

            self.stack.setCurrentWidget(page)

    def create_param_widget(self, param):
        param_type = param["type"]

        if param_type == "int":
            slider = QSlider(Qt.Horizontal)
            slider.setMinimum(param["min"])
            slider.setMaximum(param["max"])
            slider.valueChanged.connect(
                lambda v, p=param: self.send_param(p["id"], v))

            label = QLabel(f'{param["id"]}: {slider.value()}')
            slider.valueChanged.connect(
                lambda v, l=label, p=param: l.setText(f'{p["id"]}: {v}'))

            layout = QVBoxLayout()
            layout.addWidget(label)
            layout.addWidget(slider)

            widget = QWidget()
            widget.setLayout(layout)

        elif param_type == "bool":
            checkbox = QCheckBox(param["id"])
            checkbox.stateChanged.connect(
                lambda s, p=param: self.send_param(p["id"], bool(s)))
            widget = checkbox

        elif param_type == "color":
            button = QPushButton(f'{param["id"]}: Select Color')
            button.clicked.connect(lambda _, p=param: self.pick_color(p["id"]))
            widget = button

        return widget

    def pick_color(self, param_id):
        color = QColorDialog.getColor()
        if color.isValid():
            hsv = color.getHsv()
            hue = int(hsv[0] * 360 / 255)
            self.send_param(param_id, hue)

    def send_param(self, param_id, value):
        data = {"type": "parameter", "paramID": param_id, "value": value}
        print(f"Sending: {json.dumps(data)}")
        self.serial_console.send_json(data)
