from PyQt5.QtWidgets import (QTextEdit, QLineEdit, QVBoxLayout, QWidget,
                             QMessageBox, QPushButton, QHBoxLayout, QCheckBox,
                             QSpinBox, QLabel)
from PyQt5.QtCore import pyqtSignal, QTimer, Qt

import serial
import threading
import json
import typing


class SerialConsole(QWidget):
    append_signal = pyqtSignal(str)
    string_signal = pyqtSignal(str)
    json_signal = pyqtSignal(dict)

    motorToCheck = 0
    motors = ["dowel", "drill", "x_axis"]
    stringListeners: list[typing.Callable] = []
    jsonListeners: list[typing.Callable] = []

    def __init__(self, port, baud=115200, timeout=0.1):
        super().__init__()

        self.ser = None
        self.running = False  # Start off
        self.last_error = ""
        self.string_signal.connect(self.broadcast_string)
        self.json_signal.connect(self.broadcast_json)
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
        self.compact = False
        self.full_visible = True
        if self.last_error:
            self.error_label.setText(self.last_error)

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

    def clear(self):
        """Clear the output console."""
        self.output.clear()
        self.error_label.setText("")
        self.last_error = ""

    def init_ui(self):
        layout = QVBoxLayout()
        self.output = QTextEdit()
        self.output.setReadOnly(True)
        self.error_label = QLabel()
        self.error_label.setStyleSheet("color: red")
        self.error_label.setText("")
        self.input = QLineEdit()
        self.echo_checkbox = QCheckBox("Echo")
        self.echo_checkbox.setChecked(False)
        # make the clear button a small trash icon
        self.clear_button = QPushButton("Clear")
        self.clear_button.setIcon(self.style().standardIcon(
            self.style().SP_DialogResetButton))

        self.clear_button.clicked.connect(self.clear)
        self.clear_button.setToolTip("Clear the output console")
        self.clear_button.setShortcut("Ctrl+L")
    # set size policy to fixed width

        self.clear_button.setMaximumWidth(50)

        self.input.returnPressed.connect(self.manual_send)
        hlayout = QHBoxLayout()

        hlayout.addWidget(self.error_label)
        hlayout.addStretch(1)
        hlayout.addWidget(self.clear_button)
        layout.addLayout(hlayout)
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

    def showCompact(self, compact: bool):
        """Show only the error label when compact is True."""
        self.compact = compact
        for w in (self.output, self.input, self.echo_checkbox,
                  self.verbose_checkbox):
            w.setVisible(not compact)
        self.error_label.setVisible(compact)

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
                        if msg.startswith("sim:"):
                            self.string_signal.emit(msg)

                        elif msg.endswith(";"):
                            msg = msg[:-1]

                            try:
                                data = json.loads(msg)
                                if self.verbose_checkbox.isChecked():
                                    self.log("Received JSON: " + msg)
                                self.json_signal.emit(data)

                            except json.JSONDecodeError as e:
                                self.log("failed to parse json " + str(e))
                                self.log(msg)
                                pass
                        else:
                            self.string_signal.emit(msg)
                            self.log(msg)
            except Exception as e:
                self.log(f"Serial read error: {e}")
                break

    def log(self, msg):
        print(msg)
        if self.running:
            self.append_signal.emit(msg)

    def logError(self, msg: str):
        """Log an error message in red and remember it."""
        self.last_error = msg
        print(f"ERROR: {msg}")
        if self.running:
            self.append_signal.emit(f"<span style='color:red'>{msg}</span>")
        if hasattr(self, 'error_label'):
            self.error_label.setText(msg)

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

    def broadcast_string(self, msg: str):
        for listener in self.stringListeners:
            try:
                listener(msg)
            except Exception as e:
                print(f"Error for [{msg}] in string listener: {e} ")

    def broadcast_json(self, data: dict):
        for listener in self.jsonListeners:
            try:
                if listener(data):
                    break
            except Exception as e:
                print(f"Error in JSON listener: {e}")

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
