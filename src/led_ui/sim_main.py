from PyQt5.QtWidgets import QApplication
from led3dwidget import LED3DWidget
from fake_console import FakeConsole


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
