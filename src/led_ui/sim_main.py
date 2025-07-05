from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QTimer
from led3dwidget import LED3DWidget

class FakeConsole:
    def __init__(self):
        self.string_listeners = []
        self.json_listeners = []
        self.timer = QTimer()
        self.timer.timeout.connect(self._tick)
        self.hue = 0
        self.running = False
        self.led_count = 40

    def add_string_listener(self, cb):
        self.string_listeners.append(cb)

    def add_json_listener(self, cb):
        self.json_listeners.append(cb)

    def send_cmd(self, cmd):
        if cmd.startswith("simulate:"):
            n = int(cmd.split(":")[1])
            if n >= 0:
                self.running = True
                self.timer.start(100)
            else:
                self.running = False
                self.timer.stop()
        # print to console so user sees the command
        print(f"fake send: {cmd}")

    def send_json(self, data):
        print(f"fake json: {data}")

    def _tick(self):
        if not self.running:
            return
        rle = f"{self.hue},255:{self.led_count};"
        msg = f"sim:{rle}"
        for cb in self.string_listeners:
            cb(msg)
        self.hue = (self.hue + 5) % 256


def main():
    app = QApplication([])
    console = FakeConsole()
    nodes = [
        (0, 0.0, 0.0, 0.0),
        (10, 1.0, 0.0, 0.0),
        (20, 1.0, 1.0, 0.0),
        (30, 0.0, 1.0, 0.0),
    ]
    widget = LED3DWidget(console, nodes, 40)
    widget.show()
    widget.simulate_checkbox.setChecked(True)
    app.exec_()


if __name__ == "__main__":
    main()
