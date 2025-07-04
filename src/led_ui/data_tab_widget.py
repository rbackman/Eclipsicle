from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QListWidget, QPushButton, QLineEdit
from PyQt5.QtCore import pyqtSignal


class DataTabWidget(QWidget):
    """Widget to manage preset files with add/save functionality."""

    save_requested = pyqtSignal(str)
    load_requested = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        layout = QVBoxLayout(self)

        header = QHBoxLayout()
        self.add_btn = QPushButton("+")
        self.name_edit = QLineEdit()
        self.name_edit.setPlaceholderText("Preset name")
        self.name_edit.setVisible(False)
        header.addWidget(self.add_btn)
        header.addWidget(self.name_edit)
        layout.addLayout(header)

        self.data_list = QListWidget()
        layout.addWidget(self.data_list)

        self.add_btn.clicked.connect(self._start_save)
        self.name_edit.returnPressed.connect(self._finish_save)
        self.data_list.itemDoubleClicked.connect(
            lambda item: self.load_requested.emit(item.text()))

    def _start_save(self):
        self.name_edit.setVisible(True)
        self.name_edit.setFocus()

    def _finish_save(self):
        name = self.name_edit.text().strip()
        if name:
            self.save_requested.emit(name)
        self.name_edit.clear()
        self.name_edit.setVisible(False)

    def refresh_files(self, files):
        self.data_list.clear()
        for fname in files:
            self.data_list.addItem(fname)
