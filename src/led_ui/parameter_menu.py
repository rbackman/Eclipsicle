
import json

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTreeWidget, QTreeWidgetItem,
    QStackedWidget, QLabel, QSlider, QCheckBox, QColorDialog, QPushButton,
    QHeaderView, QSpinBox, QFileDialog, QListWidget, QTabWidget, QTextEdit
)
from data_tab_widget import DataTabWidget
from animation_tab_widget import AnimationTabWidget

from PyQt5.QtCore import Qt, QTimer, pyqtSignal
from PyQt5.QtGui import QColor
import qtawesome as qta           # pip install qtawesome
from debounceSlider import DebouncedSlider
import os
import time

from serial_console import SerialConsole
from shared import get_param_name

#  map from parameter name to full parameter info
ParameterMap: dict[str, dict] = {}
ParameterIDMap: dict[int, str] = {}
ParameterDefaults: dict[str, dict] = {}

PARAM_MAP_FILE = os.path.join(os.path.dirname(__file__), "parameter_map.json")
CONFIG_DIR = os.path.join("data", "configurations")
ANIM_DIR = os.path.join("data", "animations")


def _load_anim_names() -> set:
    """Return set of animation names from animation_map.json."""
    path = os.path.join(os.path.dirname(__file__), "animation_map.json")
    try:
        with open(path, "r") as f:
            data = json.load(f)
        return {k.lower() for k in data.keys()}
    except Exception:
        return set()


def save_parameter_map():
    """Persist the current ParameterMap to disk."""
    with open(PARAM_MAP_FILE, "w") as f:
        json.dump(ParameterMap, f, indent=2)


# ───────────────────────────── MENU + PARAM ------------------------------------------------------------------
# (unchanged skeleton – plug your full MENU_TREE and PARAM_MAP here)
MENU_TREE = {
    "Main": {
        "Patterns": {
            "Particles": {
                "Color": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_WIDTH", "PARAM_BRIGHTNESS"],
                "Speed": ["PARAM_TIME_SCALE", "PARAM_ACCELERATION", "PARAM_VELOCITY"],
                "Life": ["PARAM_LIFE", "PARAM_FADE", "PARAM_SPAWN_RATE"]
            },
            "Rainbow": ["PARAM_REPEAT", "PARAM_OFFSET",
                        "PARAM_TIME_SCALE", "PARAM_SCROLL_SPEED", "PARAM_BRIGHTNESS"],
            "Double Rainbow": ["PARAM_TIME_SCALE"],
            "Slider":  ["PARAM_WIDTH", "PARAM_GRAVITY",
                        "PARAM_REPEAT", "PARAM_POSITION",
                        "PARAM_MULTIPLIER", "PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS"],
            "Random": ["PARAM_RANDOM_MIN", "PARAM_RANDOM_MAX",
                       "PARAM_RANDOM_ON", "PARAM_RANDOM_OFF", "PARAM_BRIGHTNESS"

                       ],
            "Nebula": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS",
                       "PARAM_NOISE_SPEED", "PARAM_TIME_SCALE", "PARAM_NOISE_SCALE"],
            "Falling Bricks": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS",
                               "PARAM_WIDTH", "PARAM_SPEED", "PARAM_TIME_SCALE",
                               "PARAM_HUE_VARIANCE"],
            "Sphere": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS", "PARAM_RADIUS", "PARAM_THICKNESS", "PARAM_POS_X", "PARAM_POS_Y", "PARAM_POS_Z"],
            "Plane": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS", "PARAM_POS_Z"],
            "Point Control": ["PARAM_CURRENT_STRIP", "PARAM_CURRENT_LED", "PARAM_HUE", "PARAM_BRIGHTNESS",]

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


        },
    }
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
        ParameterMap[name] = prm
        ParameterIDMap[prm["id"]] = name
    print("parameters added to map:")
    save_parameter_map()


def loadParameters():
    """Load the parameters from a file."""
    try:
        with open(PARAM_MAP_FILE, "r") as f:
            data = f.read()
            if data:
                global ParameterMap, ParameterIDMap, ParameterDefaults
                ParameterMap = json.loads(data)
                if not ParameterDefaults:
                    ParameterDefaults = json.loads(data)
                ParameterIDMap = {v["id"]: k for k, v in ParameterMap.items()}
                print("Loaded parameters from file:")
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
        self.widgets = {}
        # print(f"Creating parameter page with { params} parameters\n")
        for prmname in params:
            prm = params[prmname]
            kind = prm["type"]
            shortname = get_param_name(prmname)
            print(
                f"Adding parameter:{shortname}  id:{prm['id']} of type {kind}\n")
            if kind == "int":
                sld = DebouncedSlider(prmname, prm)
                sld.sendInt.connect(
                    lambda v,   p=prm:  self._send(p['id'], v))
                lay.addWidget(sld)
                self.widgets[prm['id']] = sld
            elif kind == "bool":
                cb = QCheckBox(shortname)
                cb.stateChanged.connect(
                    lambda s, p=prm: self._send(p['id'], bool(s)))
                cb.setChecked(prm.get("value", False))
                lay.addWidget(cb)
                self.widgets[prm['id']] = cb
            elif kind == "color":
                color_box = QLabel()
                color_box.setFixedSize(24, 24)
                color_box.setStyleSheet(
                    "background: black; border: 1px solid #aaa;")
                btn = QPushButton(qta.icon("fa5s.paint-brush"),
                                  shortname)
                btn.clicked.connect(
                    lambda _, p=prm, b=color_box: self._pick(p['id'], b))
                btn.setToolTip("Pick a color")
                btn.setStyleSheet("text-align: left;")
                hue = prm.get("value", 0)
                color_box.setStyleSheet(
                    f"background: hsl({hue}, 100%, 50%); border: 1px solid #aaa;")
                color_box.setToolTip(f"Current hue: {hue}")
                hbox = QHBoxLayout()
                hbox.addWidget(btn)
                hbox.addWidget(color_box)
                lay.addLayout(hbox)
                self.widgets[prm['id']] = color_box
            elif kind == "float":

                sld = DebouncedSlider(prmname, prm, "float")

                sld.sendFloat.connect(
                    lambda v, p=prm: self._send(p['id'], v))
                lay.addWidget(sld)
                self.widgets[prm['id']] = sld

    def _upd_int(self, lbl, prm, v):
        lbl.setText(f"{prm['id']}: {v}")
        self._send(prm['id'], v)

    def _upd_float(self, lbl, prm, v):
        lbl.setText(f"{prm['id']}: {v / 1000:.2f}")
        self._send(prm['id'], v / 1000)

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

    def set_param_value(self, pid, val):
        w = self.widgets.get(pid)
        if w is None:
            return
        if isinstance(w, DebouncedSlider):
            w.setValue(val)
        elif isinstance(w, QCheckBox):
            w.blockSignals(True)
            w.setChecked(bool(val))
            w.blockSignals(False)
        elif isinstance(w, QLabel):
            hue = int(val)
            w.setStyleSheet(
                f"background: hsl({hue}, 100%, 50%); border: 1px solid #aaa;")

    def _send(self, pid, val):
        if pid in ParameterIDMap:
            ParameterMap[ParameterIDMap[pid]]["value"] = val
            save_parameter_map()
        if (len(ParameterMap)):
            # send short version if available
            cmd = "p:" + str(pid) + ":" + str(val)
            self.console.send_cmd(cmd)
        else:
            self.console.send_json({"param": pid, "value": val})


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

        self.startSSpinbox = QSpinBox()
        self.startSSpinbox.setRange(0, 255)
        self.endSSpinbox = QSpinBox()
        self.endSSpinbox.setRange(0, 255)
        self.startSSpinbox.setValue(0)
        self.endSSpinbox.setValue(255)

        self.led_selectLayout.addWidget(QLabel("Start LED:"))
        self.led_selectLayout.addWidget(self.startSSpinbox)
        self.led_selectLayout.addWidget(QLabel("End LED:"))
        self.led_selectLayout.addWidget(self.endSSpinbox)
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
        if (animation == "Point Control"):
            # special case for Point Control, send current strip and led

            self.console.send_cmd(
                f"setanimation:POINT_CONTROL")
            return
        if overwrite:
            if self.partial_animation_toggle.isChecked():
                start_led = self.startSSpinbox.value()
                end_led = self.endSSpinbox.value()

                self.console.send_cmd(
                    f"setanimation:{animation}:{start_led}:{end_led}")

            else:
                self.console.send_cmd(f"setanimation:{animation}")
        else:
            if self.partial_animation_toggle.isChecked():
                start_led = self.startSSpinbox.value()
                end_led = self.endSSpinbox.value()

                self.console.send_cmd(
                    f"addanimation:{animation}:{start_led}:{end_led}")
            else:
                self.console.send_cmd(f"replaceanimation:all:{animation}")


class ParameterMenuWidget(QWidget):
    profile_changed = pyqtSignal(str)

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

        self.tabs = QTabWidget()
        root = QVBoxLayout(self)
        root.addWidget(self.tabs)
        self.current_profile = ""
        os.makedirs(CONFIG_DIR, exist_ok=True)
        os.makedirs(ANIM_DIR, exist_ok=True)

        # Parameter tab ---------------------------------------------------
        paramTab = QWidget()
        pRoot = QVBoxLayout(paramTab)
        pRoot.setAlignment(Qt.AlignTop)
        paramHLayout = QHBoxLayout()
        pRoot.addLayout(paramHLayout)
        self.animationSender = AnimationSendWidget(console)

        pRoot.addWidget(self.animationSender)
        paramHLayout.addWidget(self.tree, 1)
        paramHLayout.addWidget(self.pages, 1)
        # confirm and save buttons are now in the main menu
        self.tabs.addTab(paramTab, "Parameters")

        # Data tab -------------------------------------------------------
        self.data_tab = DataTabWidget()
        self.data_tab.save_requested.connect(self.save_profile_as)
        self.data_tab.load_requested.connect(
            lambda name: self.load_profile_file(os.path.join(CONFIG_DIR, name)))
        self.tabs.addTab(self.data_tab, "Data")

        # Animation script tab -----------------------------------------
        self.animation_tab = AnimationTabWidget(ANIM_DIR, console)
        self.tabs.addTab(self.animation_tab, "Animations")
        self.console.add_json_listener(self.animation_tab.json_received)
        self.setWindowTitle("ESP32 Pattern Controller")
        self.resize(760, 500)
        loadParameters()
        self._validate_menu_tree()
        self.update_widgets_from_map()
        self.refresh_data_files()

        stateTab = QWidget()
        sRoot = QVBoxLayout(stateTab)
        self.stateText = QTextEdit()
        self.stateText.setReadOnly(True)
        sRoot.addWidget(self.stateText)
        self.tabs.addTab(stateTab, "State")

    # helper to build icon safely
    def _qta_icon(self, key):
        try:
            return qta.icon(key) if key else qta.icon("fa5s.square")
        except Exception:
            return qta.icon("fa5s.square")

    def _build_tree(self):
        def add(node, branch):
            if not isinstance(branch, dict):
                return
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

    def _validate_menu_tree(self):
        names = _load_anim_names()
        if not names:
            return

        def collect(branch):
            out = []
            if isinstance(branch, list):
                return out
            for key, sub in branch.items():
                if isinstance(sub, (list, dict)):
                    out.append(key)
                if isinstance(sub, dict):
                    out.extend(collect(sub))
            return out

        patterns = collect(MENU_TREE.get("Main", {}).get("Patterns", {}))
        for p in patterns:
            if p.lower().replace(" ", "") not in {n.replace(" ", "") for n in names}:
                print(f"Warning: pattern '{p}' missing from animation_map.json")

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
        treePath = self.get_tree_path(cur)
        isPatternType = "Patterns" in treePath

        # use treepath to look for an array in MENU_TREE
        treedata = MENU_TREE
        for part in treePath:
            if part in treedata:
                treedata = treedata[part]
            else:
                if self.console:
                    self.console.logError(f"Path {treePath} not found in MENU_TREE")
                else:
                    print(f"Path {treePath} not found in MENU_TREE")
                return
        def _gather_params(branch):
            if isinstance(branch, list):
                return list(branch)
            if isinstance(branch, dict):
                out = []
                for v in branch.values():
                    out.extend(_gather_params(v))
                return out
            return []

        param_list = []
        if isinstance(treedata, list) or isinstance(treedata, dict):
            param_list = _gather_params(treedata)

        if param_list:
            if name not in self.cache:
                data = {}

                for prm in param_list:
                    if prm in ParameterMap:
                        pdata = ParameterMap[prm]
                        dtype = pdata.get("type", "int")
                        if dtype == "int":
                            data[prm] = {
                                "id": pdata.get("id", 0),
                                "type":  'int',
                                "value": pdata.get("value", 0),
                                "min":  pdata.get("min", 0),
                                "max":  pdata.get("max", 255)
                            }
                        elif dtype == "bool":
                            data[prm] = {
                                "id": pdata.get("id", 0),
                                "type": 'bool',
                                "value": pdata.get("value", False)
                            }
                        elif dtype == "color":
                            data[prm] = {
                                "id": pdata.get("id", 0),
                                "type": 'color',
                                "value": pdata.get("value", 0)
                            }
                        elif dtype == "float":
                            data[prm] = {
                                "id": pdata.get("id", 0),
                                "type": 'float',
                                "value": pdata.get("value", 0.0),
                                "min": pdata.get("min", 0.0),
                                "max": pdata.get("max", 1.0)
                            }
                    else:
                        if self.console:
                            self.console.logError(
                                f"Parameter {prm} not found in ParameterMap")
                        else:
                            print(f"Parameter {prm} not found in ParameterMap")
                        continue

                print(
                    f"Loading parameters for {name} with map: {param_list}  \n {data}   \n")
                page = ParamPage(data,
                                 self.console) if data else QWidget()
                self.cache[name] = page
                self.pages.addWidget(page)

        if isPatternType:
            # if it has the word patterns in it, show the animation sender

            self.animationSender.setVisible(True)
            self.animationSender.set_animation(treePath[-1])

        else:
            self.animationSender.setVisible(False)

        pathname = "/".join(treePath)

        self.console.send_cmd(f"menu:{pathname}")
        if name in self.cache:
            # if we have a page for this name, show it
            print(f"Showing cached page for {name}")
            self.pages.setCurrentWidget(self.cache[name])
            self.pages.show()
        else:
            # otherwise show an empty page
            print(f"No cached page for {name}, showing empty page")
            self.pages.hide()

    def json_received(self, obj: dict):
        """Handle incoming JSON data."""
        if obj["type"] == "parameters" and "data" in obj:

            data = obj["data"]
            checkParameters(data)
            self.cache.clear()
            while self.pages.count():
                w = self.pages.widget(0)
                self.pages.removeWidget(w)
                w.deleteLater()
            return True
        if obj.get("type") == "stripState":
            self.stateText.setPlainText(json.dumps(obj, indent=2))
            for anim in obj.get("animations", []):
                for pname, val in anim.get("params", {}).items():
                    if pname in ParameterMap:
                        ParameterMap[pname]["value"] = val
            self.update_widgets_from_map()
            return True
        return False

    def save_profile(self):
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        self.save_profile_as(f"profile_{timestamp}")

    def save_profile_as(self, name: str):
        os.makedirs(CONFIG_DIR, exist_ok=True)
        if not name.endswith(".json"):
            name += ".json"
        path = os.path.join(CONFIG_DIR, name)
        diff = {
            k: prm["value"]
            for k, prm in ParameterMap.items()
            if ParameterDefaults.get(k, {}).get("value") != prm.get("value")
        }
        with open(path, "w") as f:
            json.dump(diff, f, indent=2)
        self.current_profile = name
        self.refresh_data_files()
        self.profile_changed.emit(name)

    def set_default(self):
        self.console.send_cmd("saveDefaults")

    def load_profile(self):
        path, _ = QFileDialog.getOpenFileName(
            self, "Load Parameter Profile", "", "JSON Files (*.json)")
        if path:
            self.load_profile_file(path)

    # ------------------------------------------------------------------
    def load_profile_file(self, path: str):
        with open(path, "r") as f:
            data = json.load(f)
        ParameterMap.clear()
        ParameterMap.update(json.loads(json.dumps(ParameterDefaults)))
        for k, v in data.items():
            if k in ParameterMap:
                ParameterMap[k]["value"] = v
        global ParameterIDMap
        ParameterIDMap = {v["id"]: k for k, v in ParameterMap.items()}
        save_parameter_map()
        self.cache.clear()
        while self.pages.count():
            w = self.pages.widget(0)
            self.pages.removeWidget(w)
            w.deleteLater()
        self.current_profile = os.path.basename(path)
        print(f"Loaded profile {path}")
        self.refresh_data_files()
        self.apply_parameter_values()
        self.profile_changed.emit(self.current_profile)

    def refresh_data_files(self):
        os.makedirs(CONFIG_DIR, exist_ok=True)
        files = [f for f in sorted(os.listdir(
            CONFIG_DIR)) if f.endswith(".json")]
        self.data_tab.refresh_files(files)
        if hasattr(self, 'animation_tab'):
            os.makedirs(ANIM_DIR, exist_ok=True)
            anims = [f for f in sorted(
                os.listdir(ANIM_DIR)) if f.endswith('.led')]
            self.animation_tab.refresh_files(anims)

    def apply_parameter_values(self):
        for prm in ParameterMap.values():
            pid = prm.get("id")
            val = prm.get("value")
            cmd = f"p:{pid}:{val}"
            self.console.send_cmd(cmd)
        self.set_default()

    def update_widgets_from_map(self):
        for page in self.cache.values():
            for prm in ParameterMap.values():
                pid = prm.get("id")
                val = prm.get("value")
                page.set_param_value(pid, val)
        save_parameter_map()
