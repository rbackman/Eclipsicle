
import json

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTreeWidget, QTreeWidgetItem,
    QStackedWidget, QLabel, QSlider, QCheckBox, QColorDialog, QPushButton,
    QHeaderView, QSpinBox
)

from PyQt5.QtCore import Qt, QTimer, pyqtSignal
from PyQt5.QtGui import QColor
import qtawesome as qta           # pip install qtawesome
from debounceSlider import DebouncedSlider
from floatSlider import FloatSliderWithSpinBox
from serial_console import SerialConsole
from shared import get_param_name

#  this maps from parameter name to their id

ParameterMap: dict[str, int] = {}


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
        "LED": {"Master LED": {}, "Animation Type": {}},
        "Audio": {},
        "Debug": {"Display": {}, "Settings": {}, "Misc": {}, },
        "Animation Type": {
            "Single Animation": {},
            "Multi Animation": {
                "Current Strip": {},
                "All Strips": {},
            },

            "Point Control": {}
        },
    }
}


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
        {"id": "PARAM_TIME_SCALE", "type": "float", "min": 0, "max": 5},
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

        {"id": "PARAM_SLIDER_REPEAT", "type": "float", "min": 0, "max": 5},

        {"id": "PARAM_SLIDER_POSITION", "type": "int", "min": 0, "max": 100},
        {"id": "PARAM_HUE", "type": "color"},
        {"id": "PARAM_SLIDER_MULTIPLIER", "type": "float", "min": 0, "max": 10},

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
    "Point Control": [
        {"id": "PARAM_CURRENT_STRIP", "type": "int", "min": 0, "max": 100},
        {"id": "PARAM_CURRENT_LED", "type": "int", "min": 0, "max": 500},
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
    "Point Control": "fa5s.eye",
    "Single Animation": "fa5s.play",
    "Multi Animation": "fa5s.play-circle",
}


def checkParameters(params):
    """Check if the parameters are valid."""

    for name, prm in params.items():
        # print(f"  {name} : {prm}\n")
        ParameterMap[name] = prm
    print("parameters added to map:")
    mapAsString = json.dumps(ParameterMap, indent=2)
    print(mapAsString)
    # save the map to a file
    with open("parameter_map.json", "w") as f:
        f.write(mapAsString)


def loadParameters():
    """Load the parameters from a file."""
    try:
        with open("parameter_map.json", "r") as f:
            data = f.read()
            if data:
                global ParameterMap
                ParameterMap = json.loads(data)
                print("Loaded parameters from file:")
                print(ParameterMap)
    except FileNotFoundError:
        print("No parameter map file found, using empty map.")
    except json.JSONDecodeError as e:
        print(f"Error decoding parameter map: {e}")
# ───────────────────────────── Serial throttler --------------------------------------------------------------


class ParamPage(QWidget):
    def __init__(self, params, console:  SerialConsole):
        super().__init__()
        self.console = console
        lay = QVBoxLayout(self)
        lay.setAlignment(Qt.AlignTop)
        for prm in params:
            kind = prm["type"]
            if kind == "int":
                lbl = QLabel(f"{get_param_name(prm['id'])}: 0")
                sld = DebouncedSlider(prm)
                sld.sendSignal.connect(
                    lambda v,   p=prm: self._send(p['id'], v))
                lay.addWidget(lbl)
                lay.addWidget(sld)
            elif kind == "bool":
                cb = QCheckBox(prm["id"])
                cb.stateChanged.connect(
                    lambda s, p=prm: self._send(p['id'], bool(s)))
                lay.addWidget(cb)
            elif kind == "color":
                color_box = QLabel()
                color_box.setFixedSize(24, 24)
                color_box.setStyleSheet(
                    "background: black; border: 1px solid #aaa;")
                btn = QPushButton(qta.icon("fa5s.paint-brush"),
                                  get_param_name(prm['id']))
                btn.clicked.connect(
                    lambda _, p=prm, b=color_box: self._pick(p['id'], b))

                hbox = QHBoxLayout()
                hbox.addWidget(btn)
                hbox.addWidget(color_box)
                lay.addLayout(hbox)
            elif kind == "float":
                widget = FloatSliderWithSpinBox(prm)
                widget.sendSignal.connect(
                    lambda v, p=prm: self._send(p['id'], v))
                lay.addWidget(widget)

        loadParameters()

    def _upd_int(self, lbl, prm, v):
        lbl.setText(f"{prm['id']}: {v}")
        self._send(prm['id'], v)

    def _upd_float(self, lbl, prm, v):
        lbl.setText(f"{prm['id']}: {v / 100:.2f}")
        self._send(prm['id'], v / 100)

    def _upd_bool(self, cb, prm, s):
        cb.setText(f"{prm['id']}: {bool(s)}")
        self._send(prm['id'], bool(s))

    def _pick(self, pid, color_box):
        c = QColorDialog.getColor()
        if c.isValid():
            hue = int(c.hueF() * 360)
            color_box.setStyleSheet(
                f"background: {c.name()}; border: 1px solid #aaa;")
            self._send(pid, hue)

    def _send(self, name, val):
        if (len(ParameterMap)):
            # send short version if available
            cmd = "p:" + str(ParameterMap[name]) + ":" + str(val)

            self.console.send_cmd(cmd)
        else:
            self.console.send_json({"param": name, "value": val})


# ───────────────────────────── Main widget -------------------------------------------------------------------


class AnimationSendWidget(QWidget):
    # a widget that has a button and options to send single animation, overwrite, start led,end led or full strip
    def __init__(self, console):
        super().__init__()
        self.console = console
        self.init_ui()

    def init_ui(self):
        self.layout = QHBoxLayout()
        self.setLayout(self.layout)
        self.led_selectLayout = QHBoxLayout()
        self.led_selectWidget = QWidget()
        self.led_selectWidget.setLayout(self.led_selectLayout)
        self.layout.addWidget(self.led_selectWidget)

        self.startLEDSSpinbox = QSpinBox()
        self.startLEDSSpinbox.setRange(0, 255)
        self.endLEDSSpinbox = QSpinBox()
        self.endLEDSSpinbox.setRange(0, 255)
        self.startLEDSSpinbox.setValue(0)
        self.endLEDSSpinbox.setValue(255)

        self.led_selectLayout.addWidget(QLabel("Start LED:"))
        self.led_selectLayout.addWidget(self.startLEDSSpinbox)
        self.led_selectLayout.addWidget(QLabel("End LED:"))
        self.led_selectLayout.addWidget(self.endLEDSSpinbox)
        self.led_selectWidget.setVisible(False)
        self.overwrite_toggle = QCheckBox("Overwrite Animation")
        self.overwrite_toggle.setChecked(False)
        self.layout.addWidget(self.overwrite_toggle)
        self.partial_animation_toggle = QCheckBox("Partial Animation")
        self.partial_animation_toggle.setChecked(False)
        self.partial_animation_toggle.stateChanged.connect(
            lambda state: self.led_selectWidget.setVisible(state == Qt.Checked))
        self.layout.addWidget(self.partial_animation_toggle)
        self.animation_label = QLabel("Animation: None")
        self.layout.addWidget(self.animation_label)
        self.send_animation_btn = QPushButton("Send Animation Command")
        self.send_animation_btn.clicked.connect(self.send_animation_cmd)
        self.layout.addWidget(self.send_animation_btn)
        self.setLayout(self.layout)
        self.setWindowTitle("Animation Sender")
        self.resize(300, 100)
        self.setStyleSheet("background-color: #f0f0f0;")

    def set_animation(self, animation):
        self.animation_label.setText(f"Animation: {animation}")

    def send_animation_cmd(self):

        animation = self.animation_label.text().split(": ")[1]
        overwrite = self.overwrite_toggle.isChecked()
        if overwrite:
            if self.partial_animation_toggle.isChecked():
                start_led = self.startLEDSSpinbox.value()
                end_led = self.endLEDSSpinbox.value()

                self.console.send_cmd(
                    f"setanimation:{animation}:{start_led}:{end_led}")

            else:
                self.console.send_cmd(f"setanimation:{animation}")
        else:
            if self.partial_animation_toggle.isChecked():
                start_led = self.startLEDSSpinbox.value()
                end_led = self.endLEDSSpinbox.value()

                self.console.send_cmd(
                    f"addanimation:{animation}:{start_led}:{end_led}")
            else:
                self.console.send_cmd(f"replaceanimation:all:{animation}")


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
        root = QVBoxLayout(self)
        root.setAlignment(Qt.AlignTop)
        paramHLayout = QHBoxLayout()
        root.addLayout(paramHLayout)
        self.animationSender = AnimationSendWidget(console)
        root.addWidget(self.animationSender)
        paramHLayout.addWidget(self.tree, 1)
        paramHLayout.addWidget(self.pages, 1)
        self.confirm = QPushButton("Confirm")
        self.confirm.setIcon(qta.icon("fa5s.check"))
        self.confirm.clicked.connect(
            lambda: self.console.send_cmd("confirmParameters"))
        paramHLayout.addWidget(self.confirm)
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
        # self.tree.setCurrentItem(leaf)

    def get_tree_path(self, item: QTreeWidgetItem):
        """Get the full path of the item in the tree."""
        path = []
        while item:
            path.append(item.text(0))
            item = item.parent()
        return list(reversed(path))

    def _sel_changed(self, cur, _prev):
        if not cur:
            return
        name = cur.text(0)

        if name not in self.cache:
            page = ParamPage(PARAM_MAP.get(name, []),
                             self.console) if name in PARAM_MAP else QWidget()
            self.cache[name] = page
            self.pages.addWidget(page)

        treePath = self.get_tree_path(cur)
        isPatternType = treePath[-2] == "Patterns"

        if isPatternType:
            # if it has the word patterns in it, show the animation sender

            self.animationSender.setVisible(True)
            self.animationSender.set_animation(treePath[-1])

        else:
            self.animationSender.setVisible(False)

        pathname = "/".join(treePath)

        self.console.send_cmd(f"menu:{pathname}")
        self.pages.setCurrentWidget(self.cache[name])

    def json_received(self, obj: dict):
        """Handle incoming JSON data."""
        if obj["type"] == "parameters" and "data" in obj:

            data = obj["data"]
            checkParameters(data)
            return True
        return False
