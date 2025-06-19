# main.py

import sys
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QVBoxLayout, QWidget,
    QPushButton, QLabel, QSlider, QCheckBox, QColorDialog, QHBoxLayout
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor
from parameter_menu import ParameterMenuWidget
from serial_console import SerialConsole
from device_selector import DeviceSelector


def launch_main(app, port):
    win = MainWindow(port)
    win.show()
    app.win = win


def start_app():
    app = QApplication([])
    selector = DeviceSelector(lambda port: launch_main(app, port))
    selector.show()
    app.selector = selector
    app.exec_()


class MainWindow(QMainWindow):

    def __init__(self, port):
        super().__init__()

        self.console = SerialConsole(port)
        # self.console.add_string_listener(self.string_received)
        # self.console.add_json_listener(self.json_received)

        self.parameter_menu = ParameterMenuWidget(self.console)
        central_widget = QWidget()
        main_layout = QVBoxLayout()

        self.verbose_checkbox = QCheckBox('Verbose Mode')
        self.verbose_checkbox.setChecked(True)
        self.verbose_checkbox.stateChanged.connect(lambda state:  self.send_cmd(
            "verbose") if state == Qt.Checked else self.send_cmd("quiet"))

        main_layout.addWidget(self.verbose_checkbox)
        main_layout.addWidget(self.parameter_menu)
        # # Brightness Slider
        # brightness_layout = QHBoxLayout()
        # brightness_label = QLabel('Brightness: 255')
        # self.brightness_slider = QSlider(Qt.Horizontal)
        # self.brightness_slider.setRange(0, 255)
        # self.brightness_slider.setValue(255)
        # self.brightness_slider.valueChanged.connect(
        #     lambda value: self.update_brightness(brightness_label, value))

        # brightness_layout.addWidget(brightness_label)
        # brightness_layout.addWidget(self.brightness_slider)

        # # Invert Checkbox
        # self.invert_checkbox = QCheckBox('Invert Colors')
        # self.invert_checkbox.stateChanged.connect(
        #     lambda state: self.send_parameter("PARAM_INVERT", 0, state == Qt.Checked))

        # # Color Picker
        # color_layout = QHBoxLayout()
        # self.color_label = QLabel('Selected Color: #FFFFFF')
        # self.color_button = QPushButton('Pick Color')
        # self.color_button.clicked.connect(self.open_color_dialog)

        # color_layout.addWidget(self.color_label)
        # color_layout.addWidget(self.color_button)

        # Toggle Serial Console
        self.console.setVisible(False)
        self.toggle_console_btn = QPushButton("Toggle Console")
        self.toggle_console_btn.clicked.connect(self.toggle_console)

        # Assemble layouts
        # main_layout.addLayout(brightness_layout)
        # main_layout.addWidget(self.invert_checkbox)
        # main_layout.addLayout(color_layout)
        main_layout.addWidget(self.toggle_console_btn)
        main_layout.addWidget(self.console)

        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)
        self.setWindowTitle('ESP32 LED Controller')
        self.resize(400, 200)

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


if __name__ == '__main__':
    print("Starting ESP32 LED Controller UI...")
    start_app()
