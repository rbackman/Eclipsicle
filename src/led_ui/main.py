# main.py

import sys
from PyQt5.QtWidgets import (
    QApplication
)

from device_selector import DeviceSelector

from mainwindow import MainWindow


def launch_main(app, port, selector=None):
    if selector:
        print("Closing device selector...")
        selector.close()
    win = MainWindow(port)
    win.show()
    app.win = win


def start_app():
    app = QApplication([])
    selector = DeviceSelector(lambda port: launch_main(app, port, selector))
    selector.show()
    app.selector = selector
    app.exec_()


if __name__ == '__main__':
    print("Starting ESP32 LED Controller UI...")
    start_app()
