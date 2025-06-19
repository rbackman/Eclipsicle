import sys
import json
from collections import deque
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QTreeWidget, QTreeWidgetItem,
    QStackedWidget, QLabel, QSlider, QCheckBox, QColorDialog, QPushButton,
    QHeaderView
)
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QColor
import qtawesome as qta           # pip install qtawesome
from serial_console import SerialConsole

# ───────────────────────────── MENU + PARAM ------------------------------------------------------------------
# (unchanged skeleton – plug your full MENU_TREE and PARAM_MAP here)
MENU_TREE = {
    "Main": {
        "Patterns": {
            "Particles": {"Color": {}, "Speed": {}, "Life": {}},
            "Rainbow": {},
            "Double Rainbow": {},
            "Slider": {"Color": {}, "Settings": {}},
            "Random": {},
        },
        "LED": {"Master LED": {}},
        "Audio": {},
        "Debug": {"Display": {}, "Settings": {}, "Misc": {}, "LEDDbg": {}},
    }
}


# #define PARAMETER_LIST             \
#     X(PARAM_HUE)                   \
#     X(PARAM_HUE_END)               \
#     X(PARAM_BRIGHTNESS)            \
#     X(PARAM_PARTICLE_WIDTH)        \
#     X(PARAM_PARTICLE_FADE)         \
#     X(PARAM_PARTICLE_UPDATE_ALL)   \
#     X(PARAM_SLIDER_WIDTH)          \
#     X(PARAM_SLIDER_GRAVITY)        \
#     X(PARAM_VELOCITY)              \
#     X(PARAM_MAX_SPEED)             \
#     X(PARAM_ACCELERATION)          \
#     X(PARAM_RANDOM_DRIFT)          \
#     X(PARAM_TIME_SCALE)            \
#     X(PARAM_SLIDER_REPEAT)         \
#     X(PARAM_RAINBOW_REPEAT)        \
#     X(PARAM_RAINBOW_OFFSET)        \
#     X(PARAM_SLIDER_POSITION)       \
#     X(PARAM_SLIDER_HUE)            \
#     X(PARAM_SPAWN_RATE)            \
#     X(PARAM_PARTICLE_LIFE)         \
#     X(PARAM_SOUND_SCALE)           \
#     X(PARAM_SCROLL_SPEED)          \
#     X(PARAM_SLIDER_MULTIPLIER)     \
#     X(PARAM_RANDOM_ON)             \
#     X(PARAM_RANDOM_OFF)            \
#     X(PARAM_RANDOM_MIN)            \
#     X(PARAM_RANDOM_MAX)            \
#     X(PARAM_INVERT)                \
#     X(PARAM_CENTERED)              \
#     X(PARAM_BLACK_AND_WHITE)       \
#     X(PARAM_LOOP_ANIM)             \
#     X(PARAM_CYCLE)                 \
#     X(PARAM_SEQUENCE)              \
#     X(PARAM_SHOW_FPS)              \
#     X(PARAM_DISPLAY_ACCEL)         \
#     X(PARAM_RECORD_AUDIO)          \
#     X(PARAM_CURRENT_STRIP)         \
#     X(PARAM_CURRENT_LED)           \
#     X(PARAM_MASTER_LED_HUE)        \
#     X(PARAM_MASTER_LED_BRIGHTNESS) \
#     X(PARAM_MASTER_LED_SATURATION) \
#     X(PARAM_MASTER_VOLUME)         \
#     X(PARAM_MOTOR_SPEED)           \
#     X(PARAM_BEAT)                  \
#     X(PARAM_BEAT_MAX_SIZE)         \
#     X(PARAM_BEAT_FADE)             \
#     X(PARAM_ANIMATION_TYPE)        \
#     X(PARAM_UNKNOWN)

PARAM_MAP = {
    "Color": [
        {"id": "PARAM_HUE", "type": "color"},
        {"id": "PARAM_HUE_END", "type": "int", "min": 0, "max": 360},
        {"id": "PARAM_PARTICLE_WIDTH", "type": "int", "min": 1, "max": 60},
    ],
    "Random": [
        {"id": "PARAM_RANDOM_MIN", "type": "int", "min": 0, "max": 255},
        {"id": "PARAM_RANDOM_MAX", "type": "int", "min": 0, "max": 255},
        {"id": "PARAM_RANDOM_ON", "type": "int", "min": 0, "max": 100},
        {"id": "PARAM_RANDOM_OFF", "type": "int", "min": 0, "max": 100},
    ],
    "Speed": [
        {"id": "PARAM_TIME_SCALE", "type": "float", "min": 0, "max": 2},
        {"id": "PARAM_ACCELERATION", "type": "float", "min": 0, "max": 10},
    ],
    "Life": [
        {"id": "PARAM_PARTICLE_LIFE", "type": "int", "min": 0, "max": 100},
        {"id": "PARAM_PARTICLE_FADE", "type": "int", "min": 0, "max": 100},
        {"id": "PARAM_SPAWN_RATE", "type": "int", "min": 0, "max": 100},
    ],
    "Slider": [
        {"id": "PARAM_SLIDER_WIDTH", "type": "int", "min": 1, "max": 100},
        {"id": "PARAM_SLIDER_GRAVITY", "type": "int", "min": 0, "max": 100},


    ],
    "Master LED": [
        {"id": "PARAM_MASTER_LED_HUE", "type": "color"},
        {"id": "PARAM_MASTER_LED_BRIGHTNESS", "type": "int", "min": 0, "max": 255},
        {"id": "PARAM_MASTER_LED_SATURATION", "type": "int", "min": 0, "max": 100},
        {"id": "PARAM_MASTER_VOLUME", "type": "int", "min": 0, "max": 100},
    ],
    "Display": [
        {"id": "PARAM_SHOW_FPS", "type": "bool"},
        {"id": "PARAM_DISPLAY_ACCEL", "type": "bool"},
        {"id": "PARAM_RECORD_AUDIO", "type": "bool"},
    ],
    "Misc": [
        {"id": "PARAM_INVERT", "type": "bool"},
        {"id": "PARAM_CENTERED", "type": "bool"},
        {"id": "PARAM_BLACK_AND_WHITE", "type": "bool"},
        {"id": "PARAM_LOOP_ANIM", "type": "bool"},
        {"id": "PARAM_CYCLE", "type": "bool"},
        {"id": "PARAM_SEQUENCE", "type": "bool"},
    ],
    "Settings": [
        {"id": "PARAM_CURRENT_STRIP", "type": "int", "min": 0, "max": 100},
        {"id": "PARAM_CURRENT_LED", "type": "int", "min": 0, "max": 100},
        {"id": "PARAM_MASTER_LED_HUE", "type": "int", "min": 0, "max": 360},
        {"id": "PARAM_MASTER_LED_BRIGHTNESS", "type": "int", "min": 0, "max": 255},
        {"id": "PARAM_MASTER_LED_SATURATION", "type": "int", "min": 0, "max": 100},
    ],
}

# ───────────────────────────── Font‑Awesome / MDI icon table --------------------------------------------------
# Use Font‑Awesome 5 solid (fa5s) by default to avoid “Invalid font prefix” errors.
ICON = {
    "Main": "fa5s.home",
    "Patterns": "mdi.texture",
    "Particles": "mdi.atom",
    "Rainbow": "fa5s.rainbow",
    "Double Rainbow": "fa5s.rainbow",
    "Slider": "fa5s.sliders-h",
    "Random": "fa5s.random",
    "LED": "fa5s.lightbulb",
    "Master LED": "fa5s.lightbulb",
    "Audio": "fa5s.music",
    "Debug": "fa5s.bug",
    "Display": "fa5s.desktop",
    "Settings": "fa5s.cog",
    "Misc": "fa5s.ellipsis-h",
    "LEDDbg": "fa5s.eye",
}

# ───────────────────────────── Serial throttler --------------------------------------------------------------


class DebouncedSlider(QWidget):
    def __init__(self, param, console: SerialConsole):
        super().__init__()
        self.param = param
        self.console = console

        self.slider = QSlider(Qt.Horizontal)
        self.slider.setRange(param['min'], param['max'])
        self.label = QLabel(f"{param['id']}: {self.slider.value()}")

        self.timer = QTimer(self)
        self.timer.setSingleShot(True)
        self.timer.setInterval(100)    # wait 100 ms after last move
        self.timer.timeout.connect(self._flush)

        self.slider.valueChanged.connect(self._on_change)

        layout = QVBoxLayout(self)
        layout.addWidget(self.label)
        layout.addWidget(self.slider)

    def _on_change(self, v):
        self.label.setText(f"{self.param['id']}: {v}")
        self.timer.start()             # reset/start the 100 ms timer

    def _flush(self):
        v = self.slider.value()
        self.console.send_json(
            {"param": self.param['id'], "value": v})


class ThrottledSerial:
    def __init__(self, console: SerialConsole, interval_ms: int = 30):
        self.console = console
        self.q: deque[dict] = deque()
        self.timer = QTimer()
        self.timer.setInterval(interval_ms)
        self.timer.timeout.connect(self._tick)
        self.timer.start()

    def send_json(self, obj: dict):
        self.q.append(obj)

    def _tick(self):
        if self.q:
            self.console.send_json(self.q.popleft())

# ───────────────────────────── Parameter page ----------------------------------------------------------------


class ParamPage(QWidget):
    def __init__(self, params, console:  SerialConsole):
        super().__init__()
        self.console = console
        lay = QVBoxLayout(self)
        lay.setAlignment(Qt.AlignTop)
        for prm in params:
            kind = prm["type"]
            if kind == "int":
                lbl = QLabel(f"{prm['id']}: 0")
                sld = DebouncedSlider(prm, self.console)
                lay.addWidget(lbl)
                lay.addWidget(sld)
            elif kind == "bool":
                cb = QCheckBox(prm["id"])
                cb.stateChanged.connect(
                    lambda s, p=prm: self._send(p['id'], bool(s)))
                lay.addWidget(cb)
            elif kind == "color":
                btn = QPushButton(qta.icon("fa5s.paint-brush"),
                                  f"{prm['id']}: pick …")
                btn.clicked.connect(lambda _, p=prm: self._pick(p['id']))
                lay.addWidget(btn)
            elif kind == "float":
                lbl = QLabel(f"{prm['id']}: 0.0")
                sld = QSlider(Qt.Horizontal)
                sld.setRange(int(prm['min'] * 100), int(prm['max'] * 100))
                sld.valueChanged.connect(
                    lambda v, l=lbl, p=prm: self._upd_float(l, p, v))
                lay.addWidget(lbl)
                lay.addWidget(sld)

    def _upd_int(self, lbl, prm, v):
        lbl.setText(f"{prm['id']}: {v}")
        self._send(prm['id'], v)

    def _upd_float(self, lbl, prm, v):
        lbl.setText(f"{prm['id']}: {v / 100:.2f}")
        self._send(prm['id'], v / 100)

    def _upd_bool(self, cb, prm, s):
        cb.setText(f"{prm['id']}: {bool(s)}")
        self._send(prm['id'], bool(s))

    def _pick(self, pid):
        c = QColorDialog.getColor()
        if c.isValid():
            self._send(pid, int(c.hueF() * 360))

    def _send(self, name, val): self.console.send_json(
        {"param": name, "value": val})

# ───────────────────────────── Main widget -------------------------------------------------------------------


class ParameterMenuWidget(QWidget):
    def __init__(self, console: SerialConsole):
        super().__init__()
        self.console = console
        self.cache: dict[str, ParamPage] = {}
        self.tree = QTreeWidget()
        self.tree.setHeaderHidden(True)
        self.tree.header().setSectionResizeMode(QHeaderView.ResizeToContents)
        self.tree.setIndentation(14)
        self.pages = QStackedWidget()

        self._build_tree()
        self.tree.currentItemChanged.connect(self._sel_changed)
        root = QHBoxLayout(self)
        root.addWidget(self.tree, 1)
        root.addWidget(self.pages, 3)
        self.setWindowTitle("ESP32 Pattern Controller")
        self.resize(760, 500)

    # helper to build icon safely
    def _qta_icon(self, key):
        try:
            return qta.icon(key) if key else qta.icon("fa5s.square")
        except Exception:
            return qta.icon("fa5s.square")

    def _build_tree(self):
        def add(node, branch):
            for name, sub in branch.items():
                itm = QTreeWidgetItem([name])
                itm.setIcon(0, self._qta_icon(ICON.get(name)))
                node.addChild(itm)
                add(itm, sub)
        root = QTreeWidgetItem(["Main"])
        root.setIcon(0, self._qta_icon(ICON.get("Main")))
        self.tree.addTopLevelItem(root)
        add(root, MENU_TREE["Main"])
        self.tree.expandAll()
        # focus first leaf
        leaf = root.child(0)
        while leaf and leaf.childCount():
            leaf = leaf.child(0)
        self.tree.setCurrentItem(leaf)

    def _sel_changed(self, cur, _prev):
        if not cur:
            return
        name = cur.text(0)
        if name not in self.cache:
            page = ParamPage(PARAM_MAP.get(name, []),
                             self.console) if name in PARAM_MAP else QWidget()
            self.cache[name] = page
            self.pages.addWidget(page)
            isMenu = name not in PARAM_MAP
            if (isMenu):
                self.console.send_cmd(f"menu:{name}")
        self.pages.setCurrentWidget(self.cache[name])
