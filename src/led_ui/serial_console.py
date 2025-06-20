from PyQt5.QtWidgets import QTextEdit, QLineEdit, QVBoxLayout, QWidget, QMessageBox, QPushButton
from PyQt5.QtCore import pyqtSignal, QTimer
import serial
import threading
import json


class SerialConsole(QWidget):
    append_signal = pyqtSignal(str)
    motorToCheck = 0
    motors = ["dowel", "drill", "x_axis"]
    stringListeners = []
    jsonListeners = []

    def __init__(self, port):
        super().__init__()

        self.ser = None
        self.running = False  # Start off

        try:
            self.ser = serial.Serial(port, 115200, timeout=0.1)
        except serial.SerialException as e:
            self.show_error(f"Could not open serial port {port}:\n{e}")
            return
        except Exception as e:
            self.show_error(
                f"Unexpected error while connecting to {port}:\n{e}")
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

        self.input.returnPressed.connect(self.manual_send)

        layout.addWidget(self.output)
        layout.addWidget(self.input)

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

                        # first check if msg is json
                        try:
                            data = json.loads(msg)
                            for listener in self.jsonListeners:
                                listener(data)
                            self.log(msg)
                        except:

                            for listener in self.stringListeners:
                                listener(msg)
                            self.log(msg)
                            pass
            except Exception as e:
                print(f"Serial read error: {e}")
                break

    def log(self, msg):
        if self.running:
            self.append_signal.emit(msg)

    def send_json(self, data):
        if not self.ser or not self.ser.is_open:
            self.log("⚠️ Serial port not open.")
            return
        msg = json.dumps(data)
        self.ser.write((msg + ";").encode())
        self.log("→ " + msg)

    def send_cmd(self, data):
        if not self.ser or not self.ser.is_open:
            self.log("⚠️ Serial port not open.")
            return
        msg = json.dumps(data)
        self.ser.write((msg + "\n").encode())
        self.log("→ " + msg)

    def manual_send(self):
        msg = self.input.text()
        self.ser.write((msg + "\n").encode())
        self.log("→ " + msg)
        self.input.clear()

    def closeEvent(self, event):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()
        event.accept()
