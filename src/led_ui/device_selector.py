from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QComboBox, QHBoxLayout
from serial.tools import list_ports
import serial
import json
import time
import os

CONFIG_PATH = "last_device.txt"


class DeviceSelector(QWidget):
    def __init__(self, on_device_ready, current_port=None):
        super().__init__()
        self.on_device_ready = on_device_ready
        self.setWindowTitle("Select Serial Device")

        layout = QVBoxLayout()
        self.label = QLabel("Select a device to connect to:")
        self.dropdown = QComboBox()
        self.scan_btn = QPushButton("Scan for compatible devices")
        self.connect_btn = QPushButton("Connect")

        self.scan_btn.clicked.connect(self.scan_and_label_devices)
        self.connect_btn.clicked.connect(self.try_connect)
        connectLayout = QVBoxLayout()
        connectLayout.addWidget(self.label)
        connectLayout.addWidget(self.dropdown)
        connectLayout.addWidget(self.scan_btn)
        connectLayout.addWidget(self.connect_btn)

        self.connectWidget = QWidget()
        self.connectWidget.setLayout(connectLayout)
        if current_port:
            self.connectWidget.setVisible(False)
        self.connectedPortLabel = QLabel(
            f"Connected Port: {current_port}" if current_port else "")

        hlayout = QHBoxLayout()
        hlayout.addWidget(self.connectedPortLabel)
        self.showConnectLayout = QPushButton("Show Connection")
        self.showConnectLayout.clicked.connect(
            lambda: self.connectWidget.setVisible(not self.connectWidget.isVisible()))

        if not current_port:
            self.connectedPortLabel.setVisible(False)
        layout.addWidget(self.connectWidget)
        layout.addLayout(hlayout)
        self.setLayout(layout)

        self.device_info = {}  # port -> description
        self.populate_device_list()
        self.auto_select_last_port()

    def populate_device_list(self):
        """Add all serial ports initially."""
        self.dropdown.clear()
        self.device_info.clear()
        ports = list_ports.comports()
        for port in ports:
            desc = f"{port.device} - {port.description}"
            self.device_info[port.device] = desc
            self.dropdown.addItem(desc, port.device)

    def scan_and_label_devices(self):
        """Try to identify known devices and update labels."""
        ports = list_ports.comports()
        for port in ports:
            try:
                ser = serial.Serial(port.device, 115200, timeout=1)
                ser.write(b'"identify"\n')
                time.sleep(0.2)
                resp = ser.readline().decode(errors='ignore').strip()
                ser.close()

                if resp.startswith("{") and "dowel" in resp:
                    parsed = json.loads(resp)
                    label = f"{port.device} - {parsed.get('device', 'Unknown')} v{parsed.get('version', '?')}"
                    idx = self.dropdown.findData(port.device)
                    if idx != -1:
                        self.dropdown.setItemText(idx, label)
            except Exception as e:
                print(f"Error checking {port.device}: {e}")

    def auto_select_last_port(self):
        if os.path.exists(CONFIG_PATH):
            with open(CONFIG_PATH, "r") as f:
                last_port = f.read().strip()
                idx = self.dropdown.findData(last_port)
                if idx != -1:
                    self.dropdown.setCurrentIndex(idx)

    def try_connect(self):
        selected = self.dropdown.currentData()
        if selected:
            with open(CONFIG_PATH, "w") as f:
                f.write(selected)
            self.on_device_ready(selected)
            self.connectedPortLabel.setText(f"Connected Port: {selected}")
            self.connectedPortLabel.setVisible(True)
            self.connectWidget.setVisible(False)
            # self.close()
