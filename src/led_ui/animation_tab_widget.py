from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QListWidget, QPushButton,
    QLineEdit, QTextEdit, QMessageBox
)
import os
import json
import re

from shared import PARAM_MAP_FILE, ANIM_MAP_FILE


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
        self.format_btn = QPushButton("Format")
        save_row.addWidget(self.format_btn)
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
        self.format_btn.clicked.connect(self.format_script)
        self.send_btn.clicked.connect(self.send_script)

    def _load_param_names(self):
        """Return a set of known parameter short names."""
        path = PARAM_MAP_FILE
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

    def _generate_name(self, base="new"):
        """Return a unique filename in the directory."""
        idx = 1
        name = f"{base}{idx}.led"
        while os.path.exists(os.path.join(self.directory, name)):
            idx += 1
            name = f"{base}{idx}.led"
        return name

    def _create_default_file(self):
        """Create a new script file with default content and load it."""
        name = self._generate_name()
        path = os.path.join(self.directory, name)
        with open(path, "w") as f:
            f.write("ConfigFile:\nParameters:\nAnimations:\n")
        self.refresh_files(sorted(f for f in os.listdir(self.directory)
                                  if f.endswith('.led')))
        self.load_file(name)

    def _start_rename(self, item):
        """Begin renaming the given list item."""
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
        if not new_name.endswith('.led'):
            new_name += '.led'
        old_path = os.path.join(self.directory, old_name)
        new_path = os.path.join(self.directory, new_name)
        if os.path.exists(new_path):
            QMessageBox.warning(self, "Rename Error",
                                "File already exists")
            return
        try:
            os.rename(old_path, new_path)
        except OSError:
            return
        if self.current_file == old_name:
            self.current_file = new_name
        self.refresh_files(sorted(f for f in os.listdir(self.directory)
                                  if f.endswith('.led')))

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
                                  if f.endswith('.led')))

    def send_script(self):
        if not self.console:
            return
        script = self.editor.toPlainText()
        if not script.strip():
            return
        compressed = self._compress_script(script)
        encoded = compressed.replace('\n', '|')
        self.console.send_cmd(f"script:{encoded}")

    def _load_param_map(self):
        """Return mapping of parameter names to IDs."""
        path = PARAM_MAP_FILE
        try:
            with open(path, "r") as f:
                data = json.load(f)
            mapping = {}
            for key, info in data.items():
                pid = info.get("id")
                name = info.get("name", "")
                mapping[key.lower()] = pid
                mapping[key.replace("PARAM_", "").lower()] = pid
                if name:
                    mapping[name.lower()] = pid
            return mapping
        except Exception:
            return {}

    def _load_anim_map(self):
        """Return mapping of animation names to IDs."""
        path = ANIM_MAP_FILE
        try:
            with open(path, "r") as f:
                data = json.load(f)
            mapping = {}
            for key, info in data.items():
                aid = info.get("id")
                name = info.get("name", "")
                mapping[key.lower()] = aid
                if name:
                    mapping[name.lower()] = aid
            return mapping
        except Exception:
            return {}
        self.console.add_json_listener(self.animation_tab.json_received)

    def json_received(self, data):
        """Handle incoming JSON data from the console."""
        if not isinstance(data, dict):
            return
        if data.get("type") == "animations":
            animations = data.get("data", {})
            if animations:
                # save to animation_map.json
                path = ANIM_MAP_FILE
                try:
                    with open(path, "w") as f:
                        json.dump(animations, f, indent=2)
                except Exception as e:
                    if self.console:
                        self.console.logError(
                            f"Error saving animation map: {e}")
                    else:
                        print(f"Error saving animation map: {e}")

    def _eval_value(self, expr: str, variables: dict) -> str:
        """Evaluate an expression using the provided variables."""
        expr = expr.strip()
        if expr in variables:
            return str(variables[expr])
        try:
            val = eval(expr, {"__builtins__": None}, variables)
            return str(val)
        except Exception:
            return expr

    def _compress_script(self, text: str) -> str:
        """Replace names with numeric IDs and resolve variables."""
        param_map = self._load_param_map()
        anim_map = self._load_anim_map()
        if not param_map:
            return text
        lines = []
        section = None
        variables = {}
        for raw in text.splitlines():
            line = raw.strip()
            if not line:
                continue
            lower = line.lower()
            if lower in ("animations:", "parameters:", "variables:", "slideshow:"):
                section = lower[:-1]
                if section == "animations":
                    lines.append("a:")
                elif section == "parameters":
                    lines.append("p:")
                elif section == "slideshow":
                    lines.append("s:")
                else:
                    lines.append("v:")
                continue
            if section == "variables":
                if ':' in line:
                    name, expr = [p.strip() for p in line.split(':', 1)]
                    try:
                        variables[name] = float(
                            self._eval_value(expr, variables))
                    except Exception:
                        pass
                continue
            if section == "parameters":
                tokens = line.split()
                if not tokens:
                    continue
                if ':' in tokens[0]:
                    k, v = [p.strip() for p in tokens[0].split(':', 1)]
                    v = self._eval_value(v, variables)
                    pid = param_map.get(k.lower())
                    if pid is not None:
                        lines.append(f"{pid}:{v}")
                        continue
                else:
                    k = tokens[0]
                    pid = param_map.get(k.lower())
                    if pid is not None:
                        out = [str(pid)]
                        for t in tokens[1:]:
                            if ':' in t:
                                key, val = t.split(':', 1)
                                val = self._eval_value(val, variables)
                                out.append(f"{key}:{val}")
                        lines.append(' '.join(out))
                        continue
                lines.append(line)
            elif section == "animations":
                tokens = line.split()
                if not tokens:
                    continue
                name = tokens[0]
                aid = anim_map.get(name.lower())
                out = [str(aid) if aid is not None else name]
                for t in tokens[1:]:
                    if ':' in t:
                        k, v = t.split(':', 1)
                        kl = k.lower()
                        v = self._eval_value(v, variables)
                        pid = param_map.get(kl)
                        if pid is not None and kl not in ("start", "end"):
                            out.append(f"{pid}:{v}")
                        else:
                            out.append(f"{k}:{v}")
                    else:
                        out.append(t)
                lines.append(' '.join(out))
            elif section == "slideshow":
                parts = line.split()
                if parts:
                    name = parts[0]
                    dur = parts[1] if len(parts) > 1 else "0"
                    lines.append(f"{name}:{dur}")
            else:
                lines.append(line)
        return '\n'.join(lines)

    def format_script(self):
        """Format the script and warn about unknown names or syntax errors."""
        text = self.editor.toPlainText()
        lines = text.splitlines()
        formatted = []
        unknown_params = set()
        unknown_anims = set()
        bad_syntax = False
        known_params = self._load_param_names()
        anim_map = self._load_anim_map()
        known_anims = {k.lower() for k in anim_map}
        section = None
        for raw in lines:
            line = raw.strip()
            if not line:
                continue
            lower = line.lower()
            if lower in ("animations:", "parameters:", "variables:", "slideshow:"):
                section = lower[:-1]
                formatted.append(section.capitalize() + ":")
                continue
            line = re.sub(r"\s*:\s*", ":", line)
            line = re.sub(r"\s+", " ", line)
            if section == "animations":
                tokens = line.split()
                if tokens:
                    name = tokens[0].lower()
                    if name not in known_anims:
                        unknown_anims.add(tokens[0])
                    extras = tokens[1:]
                    for t in extras:
                        if ":" not in t:
                            bad_syntax = True
                            continue
                        key = t.split(":", 1)[0].lower()
                        if key in ("start", "end", "cycle"):
                            continue
                        if key not in known_params:
                            unknown_params.add(key)
            elif section == "parameters":
                tokens = line.split()
                if tokens:
                    first = tokens[0]
                    extras = tokens[1:]
                    if ":" in first:
                        pkey = first.split(":", 1)[0].lower()
                    else:
                        pkey = first.lower()
                    if pkey not in known_params:
                        unknown_params.add(pkey)
                    for t in extras:
                        if ":" not in t:
                            bad_syntax = True
                            continue
                        key = t.split(":", 1)[0].lower()
                        if key in ("start", "end", "cycle"):
                            continue
                        if key not in known_params:
                            unknown_params.add(key)
            indent = "    " if section else ""
            formatted.append(indent + line)
        self.editor.setPlainText("\n".join(formatted))
        messages = []
        if unknown_params:
            messages.append(
                "Unknown parameters: " + ", ".join(sorted(unknown_params))
            )
        if unknown_anims:
            messages.append(
                "Unknown animations: " + ", ".join(sorted(unknown_anims))
            )
        if bad_syntax:
            messages.append("Some tokens are missing ':' separators")
        if messages:
            QMessageBox.warning(self, "Format Issues", "\n".join(messages))

    def copy_script(self):
        """Duplicate the current script to a new file."""
        text = self.editor.toPlainText()
        if not text.strip():
            return
        base = os.path.splitext(self.current_file or "untitled.led")[0]
        name = f"{base}_copy.led"
        idx = 1
        while os.path.exists(os.path.join(self.directory, name)):
            name = f"{base}_copy{idx}.led"
            idx += 1
        with open(os.path.join(self.directory, name), "w") as f:
            f.write(text)
        self.refresh_files(sorted(f for f in os.listdir(self.directory)
                                  if f.endswith('.led')))
        self.load_file(name)
