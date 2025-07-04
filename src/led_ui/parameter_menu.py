
import json

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTreeWidget, QTreeWidgetItem,
    QStackedWidget, QLabel, QSlider, QCheckBox, QColorDialog, QPushButton,
    QHeaderView, QSpinBox, QFileDialog, QListWidget, QTabWidget
)

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


# ───────────────────────────── MENU + PARAM ------------------------------------------------------------------
# (unchanged skeleton – plug your full MENU_TREE and PARAM_MAP here)
MENU_TREE = {
    "Main": {
        "Patterns": {
            "Particles": {
                "Color": ["PARAM_HUE", "PARAM_HUE_END", "PARAM_PARTICLE_WIDTH", "PARAM_BRIGHTNESS"],
                "Speed": ["PARAM_TIME_SCALE", "PARAM_ACCELERATION", "PARAM_VELOCITY"],
                "Life": ["PARAM_PARTICLE_LIFE", "PARAM_PARTICLE_FADE", "PARAM_SPAWN_RATE"]
            },
            "Rainbow": ["PARAM_RAINBOW_REPEAT", "PARAM_RAINBOW_OFFSET",
                        "PARAM_TIME_SCALE", "PARAM_SCROLL_SPEED", "PARAM_BRIGHTNESS"],
            "Double Rainbow": ["PARAM_TIME_SCALE"],
            "Slider":  ["PARAM_SLIDER_WIDTH", "PARAM_SLIDER_GRAVITY",
                        "PARAM_SLIDER_REPEAT", "PARAM_SLIDER_POSITION",
                        "PARAM_SLIDER_MULTIPLIER", "PARAM_HUE", "PARAM_HUE_END", "PARAM_BRIGHTNESS"],
            "Random": ["PARAM_RANDOM_MIN", "PARAM_RANDOM_MAX",
                       "PARAM_RANDOM_ON", "PARAM_RANDOM_OFF", "PARAM_BRIGHTNESS"

                       ],
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

            "Point Control": ["PARAM_CURRENT_STRIP", "PARAM_CURRENT_LED"]
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
    mapAsString = json.dumps(params, indent=2)

    # save the map to a file
    with open("parameter_map.json", "w") as f:
        f.write(mapAsString)


def loadParameters():
    """Load the parameters from a file."""
    try:
        with open("parameter_map.json", "r") as f:
            data = f.read()
            if data:
                global ParameterMap, ParameterIDMap
                ParameterMap = json.loads(data)
                ParameterIDMap = {v["id"]: k for k, v in ParameterMap.items()}
                print("Loaded parameters from file:")
                # print(ParameterMap)
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
        # print(f"Creating parameter page with { params} parameters\n")
        for prmname in params:
            prm = params[prmname]
            kind = prm["type"]
            shortname = get_param_name(prmname)
            print(
                f"Adding parameter:{shortname}  id:{prm['id']} of type {kind}\n")
            if kind == "int":
                # lbl = QLabel(f"{shortname}: 0")
                sld = DebouncedSlider(prmname, prm)

                sld.sendInt.connect(
                    lambda v,   p=prm:  self._send(p['id'], v))

                lay.addWidget(sld)
            elif kind == "bool":
                cb = QCheckBox(shortname)
                cb.stateChanged.connect(
                    lambda s, p=prm: self._send(p['id'], bool(s)))
                cb.setChecked(prm.get("value", False))
                lay.addWidget(cb)
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
            elif kind == "float":

                sld = DebouncedSlider(prmname, prm, "float")

                sld.sendFloat.connect(
                    lambda v, p=prm: self._send(p['id'], v))
                lay.addWidget(sld)

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

    def _send(self, pid, val):
        if pid in ParameterIDMap:
            ParameterMap[ParameterIDMap[pid]]["value"] = val
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

        self.tabs = QTabWidget()
        root = QVBoxLayout(self)
        root.addWidget(self.tabs)

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
        dataTab = QWidget()
        dRoot = QVBoxLayout(dataTab)
        self.dataList = QListWidget()
        self.dataList.itemDoubleClicked.connect(self._data_double_clicked)
        dRoot.addWidget(self.dataList)
        # load from disk action moved to the File menu
        self.tabs.addTab(dataTab, "Data")

        self.setWindowTitle("ESP32 Pattern Controller")
        self.resize(760, 500)
        loadParameters()
        self.refresh_data_files()

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
        isPatternType = treePath[-2] == "Patterns"

        # use treepath to look for an array in MENU_TREE
        treedata = MENU_TREE
        for part in treePath:
            if part in treedata:
                treedata = treedata[part]
            else:
                print(f"Path {treePath} not found in MENU_TREE")
                return
        # if the last part is a list, it is a pattern type
        if isinstance(treedata, list):
            # has parameters so make a page
            if name not in self.cache:
                # pmap = PARAM_MAP.get(name, [])
                data = {}

                for prm in treedata:
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
                        print(f"Parameter {prm} not found in ParameterMap")
                        continue

                print(
                    f"Loading parameters for {name} with map: {treedata}  \n {data}   \n")
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
            return True
        return False

    def save_profile(self):
        os.makedirs("data", exist_ok=True)
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        path = os.path.join("data", f"profile_{timestamp}.json")
        with open(path, "w") as f:
            json.dump(ParameterMap, f, indent=2)
        self.console.send_cmd("saveDefaults")
        self.refresh_data_files()

    def load_profile(self):
        path, _ = QFileDialog.getOpenFileName(self, "Load Parameter Profile", "", "JSON Files (*.json)")
        if path:
            self.load_profile_file(path)

    # ------------------------------------------------------------------
    def load_profile_file(self, path:str):
        with open(path, "r") as f:
            data = json.load(f)
        ParameterMap.clear()
        ParameterMap.update(data)
        global ParameterIDMap
        ParameterIDMap = {v["id"]: k for k, v in ParameterMap.items()}
        self.cache.clear()
        while self.pages.count():
            w = self.pages.widget(0)
            self.pages.removeWidget(w)
            w.deleteLater()
        print(f"Loaded profile {path}")
        self.refresh_data_files()
        self.apply_parameter_values()

    def refresh_data_files(self):
        os.makedirs("data", exist_ok=True)
        self.dataList.clear()
        for fname in sorted(os.listdir("data")):
            if fname.endswith(".json"):
                self.dataList.addItem(fname)

    def _data_double_clicked(self, item):
        path = os.path.join("data", item.text())
        self.load_profile_file(path)

    def apply_parameter_values(self):
        for prm in ParameterMap.values():
            pid = prm.get("id")
            val = prm.get("value")
            cmd = f"p:{pid}:{val}"
            self.console.send_cmd(cmd)
        self.console.send_cmd("saveDefaults")
