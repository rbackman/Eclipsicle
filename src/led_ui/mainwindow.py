# mainwindow.py

import sys
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QVBoxLayout, QWidget,
    QPushButton, QLabel, QSlider, QCheckBox, QColorDialog, QHBoxLayout, QSpinBox, QComboBox
)
from PyQt5.QtCore import Qt, pyqtSignal
from PyQt5.QtGui import QColor

from parameter_menu import ParameterMenuWidget
from serial_console import SerialConsole
from device_selector import DeviceSelector
from ledsimwidget import LEDSimWidget


class MainWindow(QMainWindow):

    def __init__(self, port):
        super().__init__()

        self.console = SerialConsole(port)

        self.parameter_menu = ParameterMenuWidget(self.console)

        self.console.add_json_listener(self.parameter_menu.json_received)
        self.led_sim_widget = LEDSimWidget(self.console)
        self.console.add_string_listener(self.led_sim_widget.process_string)
        central_widget = QWidget()
        main_layout = QVBoxLayout()
        main_layout.addWidget(self.led_sim_widget)
        main_layout.addWidget(self.parameter_menu)

        self.console.setVisible(False)
        self.toggle_console_btn = QPushButton("Toggle Console")
        self.toggle_console_btn.clicked.connect(self.toggle_console)

        main_layout.addWidget(self.toggle_console_btn)
        main_layout.addWidget(self.console)
        main_layout.addWidget(DeviceSelector(
            lambda port: self.console.connect(port), port))
        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)
        self.setWindowTitle('ESP32 LED Controller')
        self.resize(800, 600)

    def update_brightness(self, label, value):
        label.setText(f'Brightness: {value}')
        self.send_parameter("PARAM_BRIGHTNESS", value)

    def open_color_dialog(self):
        color = QColorDialog.getColor()
        if color.isValid():
            hex_color = color.name()
            self.color_label.setText(f'Selected Color: {hex_color}')
            # Convert color to HSV or RGB as needed and send
            self.send_parameter("PARAM_HUE", color.hue())

    def send_cmd(self, cmd):

        self.console.send_cmd(cmd)

    def toggle_console(self):
        self.console.setVisible(not self.console.isVisible())

    def send_parameter(self, param, value, boolValue=False):
        data = {

            "param": param,
            "value": value,

        }
        self.console.send_json(data)

    # def string_received(self, string):
    #     # print(f"s: {string}")
    #     # Handle incoming strings if needed

    # def json_received(self, data):
    #     # print(f"j: {data}")
    #     # Handle incoming JSON if needed
