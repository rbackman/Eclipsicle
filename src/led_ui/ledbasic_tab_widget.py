from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QListWidget, QPushButton,
    QLineEdit, QTextEdit
)
import os


class LEDBasicTabWidget(QWidget):
    """Simple editor for LED Basic (*.bas) scripts."""

    def __init__(self, directory: str, console=None):
        super().__init__()
        self.directory = directory
        os.makedirs(self.directory, exist_ok=True)
        self.console = console
        self.current_file = None

        layout = QVBoxLayout(self)

        header = QHBoxLayout()
        self.add_btn = QPushButton("+")
        self.name_edit = QLineEdit()
        self.name_edit.setPlaceholderText("Script name")
        self.name_edit.setVisible(False)
        header.addWidget(self.add_btn)
        header.addWidget(self.name_edit)
        layout.addLayout(header)

        self.file_list = QListWidget()
        layout.addWidget(self.file_list)

        self.editor = QTextEdit()
        layout.addWidget(self.editor)

        save_row = QHBoxLayout()
        save_row.addStretch(1)
        self.copy_btn = QPushButton("Copy")
        self.save_btn = QPushButton("Save")
        self.send_btn = QPushButton("Send")
        save_row.addWidget(self.copy_btn)
        save_row.addWidget(self.save_btn)
        save_row.addWidget(self.send_btn)
        layout.addLayout(save_row)

        self.add_btn.clicked.connect(self._create_default_file)
        self.name_edit.returnPressed.connect(self._finish_rename)
        self.file_list.itemClicked.connect(
            lambda item: self.load_file(item.text()))
        self.file_list.itemDoubleClicked.connect(self._start_rename)
        self.save_btn.clicked.connect(self.save_file)
        self.copy_btn.clicked.connect(self.copy_script)
        self.send_btn.clicked.connect(self.send_script)

    def refresh_files(self, files):
        self.file_list.clear()
        for fname in files:
            self.file_list.addItem(fname)

    def _generate_name(self, base="new"):
        idx = 1
        name = f"{base}{idx}.bas"
        while os.path.exists(os.path.join(self.directory, name)):
            idx += 1
            name = f"{base}{idx}.bas"
        return name

    def _create_default_file(self):
        name = self._generate_name()
        path = os.path.join(self.directory, name)
        with open(path, "w") as f:
            f.write("setup\n  brightness(128)\n  clear()\nend\n\nloop(time)\n  // your code here\n  show()\nend\n")
        self.refresh_files(sorted(f for f in os.listdir(self.directory)
                                  if f.endswith('.bas')))
        self.load_file(name)

    def _start_rename(self, item):
        self._rename_old = item.text()
        self.name_edit.setText(self._rename_old)
        self.name_edit.setVisible(True)
        self.name_edit.setFocus()

    def _finish_rename(self):
        new_name = self.name_edit.text().strip()
        old_name = getattr(self, "_rename_old", None)
        self.name_edit.clear()
        self.name_edit.setVisible(False)
        if not old_name or not new_name:
            return
        if not new_name.endswith('.bas'):
            new_name += '.bas'
        old_path = os.path.join(self.directory, old_name)
        new_path = os.path.join(self.directory, new_name)
        if os.path.exists(new_path):
            return
        try:
            os.rename(old_path, new_path)
        except OSError:
            return
        if self.current_file == old_name:
            self.current_file = new_name
        self.refresh_files(sorted(f for f in os.listdir(self.directory)
                                  if f.endswith('.bas')))

    def load_file(self, name: str):
        path = os.path.join(self.directory, name)
        try:
            with open(path, 'r') as f:
                self.editor.setPlainText(f.read())
            self.current_file = name
        except OSError:
            pass

    def save_file(self):
        if not self.current_file:
            self.current_file = self._generate_name()
        path = os.path.join(self.directory, self.current_file)
        with open(path, 'w') as f:
            f.write(self.editor.toPlainText())
        self.refresh_files(sorted(f for f in os.listdir(self.directory)
                                  if f.endswith('.bas')))

    def copy_script(self):
        text = self.editor.toPlainText()
        if not text.strip():
            return
        base = os.path.splitext(self.current_file or "untitled.bas")[0]
        name = f"{base}_copy.bas"
        idx = 1
        while os.path.exists(os.path.join(self.directory, name)):
            name = f"{base}_copy{idx}.bas"
            idx += 1
        with open(os.path.join(self.directory, name), "w") as f:
            f.write(text)
        self.refresh_files(sorted(f for f in os.listdir(self.directory)
                                  if f.endswith('.bas')))
        self.load_file(name)

    def send_script(self):
        if not self.console:
            return
        script = self.editor.toPlainText()
        if not script.strip():
            return
        encoded = script.replace('\n', '|')
        self.console.send_cmd(f"basic:{encoded}")

