# mainwindow.py

import sys
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QVBoxLayout, QWidget, QSplitter,
    QPushButton, QLabel, QSlider, QCheckBox, QColorDialog,
    QMenuBar, QAction, QLineEdit
)
from PyQt5.QtCore import Qt, pyqtSignal
from PyQt5.QtGui import QColor

from make_strips import makeTess
from parameter_menu import ParameterMenuWidget
from serial_console import SerialConsole, DEFAULT_BAUD
from device_selector import DeviceSelector
from ledsimwidget import LEDSimWidget
from led3dwidget import LED3DWidget


class MainWindow(QMainWindow):

    def __init__(self, port):
        super().__init__()
        makeTess()
        self.console = SerialConsole(port, baud=DEFAULT_BAUD)

        self.parameter_menu = ParameterMenuWidget(self.console)

        self.console.add_json_listener(self.parameter_menu.json_received)
        self.led_sim_widget = LEDSimWidget(self.console)
        self.led_sim_widget.setVisible(False)
        self.console.add_string_listener(self.led_sim_widget.process_string)

        square_nodes = [
            (0, 0.0, 0.0, 0.0),
            (10, 1.0, 0.0, 0.0),
            (20, 1.0, 1.0, 0.0),
            (30, 0.0, 1.0, 0.0),
        ]
        self.led_3d_widget = LED3DWidget(self.console, square_nodes, 40)
        self.console.add_string_listener(self.led_3d_widget.process_string)
        self.parameter_menu.parameter_sent.connect(
            self.led_3d_widget.parameter_changed)
        self.parameter_menu.animation_sent.connect(
            self.led_3d_widget.animation_changed)
        self.led_3d_widget.setVisible(False)

        central_widget = QWidget()
        main_layout = QVBoxLayout()
        main_layout.addWidget(self.led_sim_widget)

        self.splitter = QSplitter(Qt.Horizontal)
        self.splitter.addWidget(self.parameter_menu)
        self.splitter.addWidget(self.led_3d_widget)
        self.splitter.setStretchFactor(0, 0)
        self.splitter.setStretchFactor(1, 1)
        main_layout.addWidget(self.splitter, 1)

        self.console.showCompact(True)
        self.console.full_visible = False
        main_layout.addWidget(self.console)
        main_layout.addWidget(DeviceSelector(
            lambda port: self.console.connect(port), port))
        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)
        self._create_menu_bar()
        self.setWindowTitle('ESP32 LED Controller')
        self.resize(800, 600)

    def _create_menu_bar(self):
        menu_bar = self.menuBar()

        file_menu = menu_bar.addMenu('File')

        default_action = QAction('Set Default', self)
        default_action.triggered.connect(self.parameter_menu.set_default)
        file_menu.addAction(default_action)

        load_action = QAction('Load From Disk', self)
        load_action.triggered.connect(self.parameter_menu.load_profile)
        file_menu.addAction(load_action)

        reset_action = QAction('Reset Defaults', self)
        reset_action.triggered.connect(
            lambda: self.console.send_cmd('resetDefaults'))
        file_menu.addAction(reset_action)

        confirm_action = QAction('Confirm Parameters', self)
        confirm_action.triggered.connect(
            lambda: self.console.send_cmd('confirmParameters'))

        confirm_animation_action = QAction('Confirm Animation', self)
        confirm_animation_action.triggered.connect(
            lambda: self.console.send_cmd('confirmAnimations'))
        file_menu.addAction(confirm_animation_action)
        file_menu.addAction(confirm_action)

        state_action = QAction('Get Strip State', self)
        state_action.triggered.connect(
            lambda: self.console.send_cmd('getStripState'))
        file_menu.addAction(state_action)

        view_menu = menu_bar.addMenu('View')
        self.sim_action = QAction('Show Simulation', self, checkable=True)
        self.sim_action.setChecked(False)
        self.sim_action.triggered.connect(self.toggle_simulation)
        view_menu.addAction(self.sim_action)

        self.sim3d_action = QAction('Show 3D View', self, checkable=True)
        self.sim3d_action.setChecked(False)
        self.sim3d_action.triggered.connect(self.toggle_3d)
        view_menu.addAction(self.sim3d_action)

        self.console_action = QAction('Show Console', self, checkable=True)
        self.console_action.setChecked(False)
        self.console_action.triggered.connect(self.toggle_console)
        view_menu.addAction(self.console_action)

        options_menu = menu_bar.addMenu('Options')
        self.verbose_action = QAction('Verbose', self, checkable=True)
        self.verbose_action.setChecked(False)
        self.verbose_action.triggered.connect(
            lambda checked: self.console.verbose_checkbox.setChecked(checked))
        options_menu.addAction(self.verbose_action)

        self.echo_action = QAction('Echo', self, checkable=True)
        self.echo_action.setChecked(False)
        self.echo_action.triggered.connect(
            lambda checked: self.console.echo_checkbox.setChecked(checked))
        options_menu.addAction(self.echo_action)

        # display current preset on the menu bar
        self.preset_field = QLineEdit()
        self.preset_field.setReadOnly(True)
        self.preset_field.setPlaceholderText("Current Preset")
        menu_bar.setCornerWidget(self.preset_field, Qt.TopRightCorner)
        self.parameter_menu.profile_changed.connect(self.preset_field.setText)

        # keep menu actions in sync with console widgets
        self.console.verbose_checkbox.stateChanged.connect(
            lambda state: self.verbose_action.setChecked(bool(state)))
        self.console.echo_checkbox.stateChanged.connect(
            lambda state: self.echo_action.setChecked(bool(state)))

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
        if isinstance(self.sender(), QAction):
            visible = self.sender().isChecked()
        else:
            visible = not self.console.full_visible
        self.console.showCompact(not visible)
        if hasattr(self, 'console_action'):
            self.console_action.setChecked(visible)

    def toggle_simulation(self):
        if isinstance(self.sender(), QAction):
            enabled = self.sender().isChecked()
        else:
            enabled = not self.led_sim_widget.simulate_checkbox.isChecked()
        if enabled:
            self.console.send_cmd(
                f"simulate:1"
            )
        else:
            self.console.send_cmd("simulate:-1")
        self.led_sim_widget.setVisible(enabled)

    def toggle_3d(self):
        if isinstance(self.sender(), QAction):
            enabled = self.sender().isChecked()
        else:
            enabled = not self.led_3d_widget.simulate_checkbox.isChecked()

        self.led_3d_widget.setVisible(enabled)
        if enabled:
            self.console.send_cmd(
                f"simulate:1"
            )
        else:
            self.console.send_cmd("simulate:-1")

        if enabled and self.splitter.sizes()[1] == 0:
            self.splitter.setSizes([200, max(100, self.width() - 200)])

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
