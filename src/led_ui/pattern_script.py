import re
import os
from typing import List, Dict

BASIC_DIR = os.path.join(os.path.dirname(__file__), "data", "ledbasic")

class PatternStep:
    def __init__(self, cmd:str, args:Dict[str,str]):
        self.cmd = cmd
        self.args = args

def parse_script(text:str) -> List[PatternStep]:
    steps = []
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        parts = line.split()
        cmd = parts[0]
        args = {}
        for p in parts[1:]:
            if '=' in p:
                k,v = p.split('=',1)
                args[k]=v
            else:
                args[p]=''
        steps.append(PatternStep(cmd,args))
    return steps


def run_script(steps:List[PatternStep], console):
    import time
    for st in steps:
        if st.cmd == 'delay':
            ms = int(st.args.get('ms', list(st.args.values())[0])) if st.args else 0
            time.sleep(ms/1000.0)
        elif st.cmd == 'animation':
            anim = st.args.get('name') or (list(st.args.keys())[0] if st.args else '')
            if anim:
                console.send_cmd(f"setanimation:{anim}")
                for k,v in st.args.items():
                    if k == 'name':
                        continue
                    console.send_cmd(f"param:{k}={v}")
        elif st.cmd == 'basic':
            name = st.args.get('name') or (list(st.args.keys())[0] if st.args else '')
            if name:
                if not name.endswith('.bas'):
                    name += '.bas'
                path = os.path.join(BASIC_DIR, name)
                try:
                    with open(path, 'r') as f:
                        txt = f.read()
                    encoded = txt.replace('\n', '|')
                    console.send_cmd(f"basic:{encoded}")
                except OSError:
                    pass
        else:
            console.send_cmd(st.cmd)

