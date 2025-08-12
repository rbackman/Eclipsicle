# main.py
from PyQt5.QtWidgets import QApplication

from fake_console import FakeConsole
from mainwindow import MainWindow


def start_app():
    app = QApplication([])
    console = FakeConsole()
    win = MainWindow(None, console=console)
    win.show()
    app.win = win
    app.exec_()


if __name__ == '__main__':
    print("Starting ESP32 LED Controller UI...")
    start_app()
