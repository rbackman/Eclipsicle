import sys
import subprocess
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QTimer
from ledsimwidget import LEDSimWidget


def main():
    app = QApplication(sys.argv)
    proc = subprocess.Popen(["../../tools/simulate_rle"], stdout=subprocess.PIPE, text=True)
    widget = LEDSimWidget(console=None)
    widget.show()

    def read_output():
        line = proc.stdout.readline()
        if line:
            if line.startswith("sim:"):
                data = line.split("sim:",1)[1].strip()
                if ':' in data and data.split(':',1)[0].isdigit():
                    _, data = data.split(':',1)
                widget.update_leds(data)
        elif proc.poll() is not None:
            timer.stop()
            app.quit()

    timer = QTimer()
    timer.timeout.connect(read_output)
    timer.start(10)
    app.exec_()
    proc.terminate()

if __name__ == "__main__":
    main()
