import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

import sys
sys.path.insert(0, _os.path.join(REPO, 'scripts'))
import run_main as rm
rule = sys.argv[3]                     # 'route' or 'sink'
rm.OUT = _os.path.join(REPO, 'results/sensitivity/sens_%s.jsonl' % rule)
rm.CASES = [c for c in rm.CASES if c[0] in ('base', 'cur1')]
rm.MODES = [1, 2, 6, 8, 10]            # the protocols the rules affect
# add-on set with one of the two features switched off (base case and 1 m/s current)
flag = ['--noRouteBit=1', '--noSinkShortcut=0'] if rule == 'route' else ['--noRouteBit=0', '--noSinkShortcut=1']
_cmd = rm.cmd
rm.cmd = lambda c, seed, m: _cmd(c, seed, m) + flag + (['--ns3::AquaSimPbrx::HcGamma=0.008'] if m == 8 else [])
sys.argv = sys.argv[:3]
rm.main()
