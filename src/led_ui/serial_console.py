from PyQt5.QtWidgets import QTextEdit, QLineEdit, QVBoxLayout, QWidget, QMessageBox, QPushButton, QHBoxLayout, QCheckBox
from PyQt5.QtCore import pyqtSignal, QTimer, Qt

import serial
import threading
import json
import typing


class SerialConsole(QWidget):
    append_signal = pyqtSignal(str)
    motorToCheck = 0
    motors = ["dowel", "drill", "x_axis"]
    stringListeners: list[typing.Callable] = []
    jsonListeners: list[typing.Callable] = []

    def __init__(self, port, baud=115200, timeout=0.1):
        super().__init__()

        self.ser = None
        self.running = False  # Start off

        try:
            self.ser = serial.Serial(port, baudrate=baud, timeout=timeout)
        except serial.SerialException as e:
            self.show_error(f"Could not open serial port {port}:\n{e}")
            return
        except Exception as e:
            self.show_error(
                f"Unexpected error while connecting to {port} {baud} {timeout}:\n{e}")
            return

        self.init_ui()

        self.append_signal.connect(self.output.append)

        self.running = True
        self.reader_thread = threading.Thread(
            target=self.reader_loop, daemon=True)
        self.reader_thread.start()

    def closeEvent(self, event):
        self.running = False
        if self.reader_thread.is_alive():
            self.reader_thread.join(timeout=1)
        if self.ser and self.ser.is_open:
            self.ser.close()
        event.accept()

    def add_string_listener(self, listener):
        self.stringListeners.append(listener)

    def add_json_listener(self, listener):
        self.jsonListeners.append(listener)

    def init_ui(self):
        layout = QVBoxLayout()
        self.output = QTextEdit()
        self.output.setReadOnly(True)
        self.input = QLineEdit()
        self.echo_checkbox = QCheckBox("Echo")
        self.echo_checkbox.setChecked(False)

        self.input.returnPressed.connect(self.manual_send)

        layout.addWidget(self.output)
        hlayout = QHBoxLayout()

        self.verbose_checkbox = QCheckBox('Verbose')
        self.verbose_checkbox.setChecked(False)
        self.verbose_checkbox.stateChanged.connect(lambda state:  self.send_cmd(
            "verbose") if state == Qt.Checked else self.send_cmd("quiet"))
        self.echo_checkbox.stateChanged.connect(lambda state: self.send_cmd(
            "echo") if state == Qt.Checked else self.send_cmd("no_echo"))

        hlayout.addWidget(self.input)
        hlayout.addWidget(self.echo_checkbox)
        hlayout.addWidget(self.verbose_checkbox)
        layout.addLayout(hlayout)

        self.setLayout(layout)

    def request_states(self):
        motor = self.motors[self.motorToCheck]
        self.motorToCheck = (self.motorToCheck + 1) % len(self.motors)

        if self.ser and self.ser.is_open:
            self.send_json({"motor": motor, "cmd": "get_state"})

    def show_error(self, message):
        QMessageBox.critical(self, "Connection Error", message)

    def reader_loop(self):
        while self.running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting:
                    msg = self.ser.readline().decode(errors='ignore').strip()
                    if msg and self.running:
                        if msg.endswith(";"):
                            msg = msg[:-1]
                            print("parsing json")

                            try:
                                data = json.loads(msg)
                                print(
                                    f"Received JSON: send to listeners {len(self.jsonListeners)}  \n")
                                handled = False
                                for listener in self.jsonListeners:
                                    if (listener(data)):
                                        handled = True
                                        break
                                if not handled:
                                    self.log(f"Unhandled JSON: {data}")

                            except json.JSONDecodeError as e:
                                print("failed to parse json ", e)
                                print(msg)
                                pass
                        else:
                            for listener in self.stringListeners:
                                listener(msg)
                            self.log(msg)
            except Exception as e:
                print(f"Serial read error: {e}")
                break

    def log(self, msg):
        print(msg)
        if self.running:
            self.append_signal.emit(msg)

    def send_json(self, data):
        if not self.ser or not self.ser.is_open:
            self.log("⚠️ Serial port not open.")
            return
        msg = json.dumps(data)
        jsonstring = msg+";"
        self.ser.write(jsonstring.encode())
        if self.echo_checkbox.isChecked():
            self.log("→ " + jsonstring)

    def send_cmd(self, data):
        if not self.ser or not self.ser.is_open:
            self.log("⚠️ Serial port not open.")
            return

        self.ser.write((data + "\n").encode())
        if self.echo_checkbox.isChecked():
            self.log("→ " + data)

    def manual_send(self):
        msg = self.input.text()
        self.ser.write((msg + "\n").encode())
        self.log("→ " + msg)
        self.input.clear()

    def connect(self, port):
        if self.ser and self.ser.is_open:
            self.ser.close()
        try:
            self.ser = serial.Serial(port, 115200, timeout=0.1)
            self.log(f"Connected to {port}")
        except serial.SerialException as e:
            self.show_error(f"Could not open serial port {port}:\n{e}")
            return
        except Exception as e:
            self.show_error(
                f"Unexpected error while connecting to {port}:\n{e}")
            return
