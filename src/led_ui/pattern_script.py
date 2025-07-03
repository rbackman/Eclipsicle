import re
from typing import List, Dict

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
        else:
            console.send_cmd(st.cmd)

