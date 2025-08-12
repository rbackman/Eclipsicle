from PyQt5.QtCore import QTimer

from strip_sim import StripSim
from shared import PARAM_MAP_FILE, load_json


class FakeConsole:
    """Minimal stand-in for SerialConsole used in offline mode."""

    def __init__(self, led_count: int = 40):
        self.string_listeners = []
        self.json_listeners = []
        self.timer = QTimer()
        self.timer.timeout.connect(self._tick)
        self.running = False
        self.sim = StripSim(led_count)
        self.param_id_map = self._load_param_map()

    @staticmethod
    def _load_param_map():
        data = load_json(PARAM_MAP_FILE)
        return {info["id"]: name for name, info in data.items()}

    def add_string_listener(self, cb):
        self.string_listeners.append(cb)

    def add_json_listener(self, cb):
        self.json_listeners.append(cb)

    def send_cmd(self, cmd: str):
        if cmd.startswith("simulate:"):
            count = int(cmd.split(":", 1)[1])
            if count >= 0:
                self.running = True
                self.timer.start(100)
            else:
                self.running = False
                self.timer.stop()
        elif cmd.startswith("p:"):
            parts = cmd.split(":")
            if len(parts) >= 3:
                pid = int(parts[1])
                val = parts[2]
                name = self.param_id_map.get(pid)
                if name:
                    self.sim.handle_cmd(f"{name}:{val}")
                    return
        self.sim.handle_cmd(cmd)

    def send_json(self, data: dict):
        param = data.get("param")
        value = data.get("value")
        if param is not None and value is not None:
            self.sim.handle_cmd(f"{param}:{value}")

    def _tick(self):
        if not self.running:
            return
        self.sim.update()
        rle = self.sim.get_rle()
        msg = f"sim:{rle}"
        for cb in self.string_listeners:
            cb(msg)
