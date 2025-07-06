import PyInstaller.__main__
import subprocess
from pathlib import Path

PyInstaller.__main__.run([
    "--onefile",
    "offline_main.py"
])

# Also build the C++ simulators used for testing when possible.
tools_dir = Path(__file__).resolve().parents[1] / "tools"
build_script = tools_dir / "build.py"
if build_script.exists():
    try:
        subprocess.check_call(["python", str(build_script)])
    except Exception as exc:
        print(f"Warning: failed to build simulators: {exc}")
