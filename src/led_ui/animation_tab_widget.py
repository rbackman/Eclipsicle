from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QListWidget, QPushButton,
    QLineEdit, QTextEdit, QMessageBox
)
import os
import json
import re

class AnimationTabWidget(QWidget):
    """Simple editor for *.led animation scripts."""

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
        self.name_edit.setPlaceholderText("Animation name")
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
        self.save_btn = QPushButton("Save")
        self.format_btn = QPushButton("Format")
        self.send_btn = QPushButton("Send")
        save_row.addWidget(self.save_btn)
        save_row.addWidget(self.format_btn)
        save_row.addWidget(self.send_btn)
        layout.addLayout(save_row)

        self.add_btn.clicked.connect(self._start_new)
        self.name_edit.returnPressed.connect(self._finish_new)
        self.file_list.itemDoubleClicked.connect(
            lambda item: self.load_file(item.text()))
        self.save_btn.clicked.connect(self.save_file)
        self.format_btn.clicked.connect(self.format_script)
        self.send_btn.clicked.connect(self.send_script)

    def _load_param_names(self):
        """Return a set of known parameter short names."""
        path = os.path.join(os.path.dirname(__file__), "parameter_map.json")
        try:
            with open(path, "r") as f:
                data = json.load(f)
            names = {info.get("name", "").lower() for info in data.values()}
            names.update({k.replace("PARAM_", "").lower() for k in data})
            return names
        except Exception:
            return set()

    def refresh_files(self, files):
        self.file_list.clear()
        for fname in files:
            self.file_list.addItem(fname)

    def _start_new(self):
        self.editor.clear()
        self.current_file = None
        self.name_edit.setVisible(True)
        self.name_edit.setFocus()

    def _finish_new(self):
        name = self.name_edit.text().strip()
        if name:
            if not name.endswith('.led'):
                name += '.led'
            self.current_file = name
        self.name_edit.clear()
        self.name_edit.setVisible(False)

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
            self._start_new()
            return
        path = os.path.join(self.directory, self.current_file)
        with open(path, 'w') as f:
            f.write(self.editor.toPlainText())
        self.refresh_files(sorted(f for f in os.listdir(self.directory)
                                  if f.endswith('.led')))

    def send_script(self):
        if not self.console:
            return
        script = self.editor.toPlainText()
        if not script.strip():
            return
        encoded = script.replace('\n', '|')
        self.console.send_cmd(f"script:{encoded}")

    def format_script(self):
        """Format the current script and warn about unknown parameters."""
        text = self.editor.toPlainText()
        lines = text.splitlines()
        formatted = []
        unknown = set()
        known = self._load_param_names()
        section = None
        for raw in lines:
            line = raw.strip()
            if not line:
                continue
            lower = line.lower()
            if lower in ("animations:", "parameters:", "variables:"):
                section = lower[:-1]
                formatted.append(section.capitalize() + ":")
                continue
            line = re.sub(r"\s*:\s*", ":", line)
            line = re.sub(r"\s+", " ", line)
            if section in ("animations", "parameters"):
                for match in re.finditer(r"(\w+):", line):
                    name = match.group(1).lower()
                    if name not in known:
                        unknown.add(name)
            indent = "    " if section else ""
            formatted.append(indent + line)
        self.editor.setPlainText("\n".join(formatted))
        if unknown:
            QMessageBox.warning(
                self,
                "Unknown Parameters",
                "Unknown parameters found: " + ", ".join(sorted(unknown))
            )
