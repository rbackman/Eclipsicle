# main.py

import sys
from PyQt5.QtWidgets import (
    QApplication
)

from device_selector import DeviceSelector

from mainwindow import MainWindow


 

def start_app():
    app = QApplication([])
    win = MainWindow(None)
    win.show()
    app.win = win
    app.exec_()


if __name__ == '__main__':
    print("Starting ESP32 LED Controller UI...")
    start_app()
