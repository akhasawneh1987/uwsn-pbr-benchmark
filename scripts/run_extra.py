import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

import sys
sys.path.insert(0, _os.path.join(REPO, 'scripts'))
import run_main as rm
rm.OUT = _os.path.join(REPO, 'results/robustness/extra.jsonl')
B = rm.BASE
rm.CASES = [
  ('untuned', {}),                                             # tuning sensitivity (5 tuned protocols only)
  ('idle_base', dict(idle=0.008)), ('idle_static', dict(mobility='static', speed=0.0, idle=0.008)),
  ('n200_static', dict(nodes=200, mobility='static', speed=0.0)), ('n300_static', dict(nodes=300, mobility='static', speed=0.0)),
  ('n200_cur1', dict(nodes=200, mobility='meander', speed=1.0)), ('n300_cur1', dict(nodes=300, mobility='meander', speed=1.0)),
]
UNTUNED = {6: ['--ns3::AquaSimPbrx::EedbrMaxHold=1', '--ns3::AquaSimPbrx::EedbrP0=0.42'],
           7: ['--ns3::AquaSimPbrx::WdfadP=0.999'],
           8: ['--ns3::AquaSimPbrx::HcGamma=0.016'],
           10: ['--ns3::AquaSimPbrx::QlfrH=2', '--qlfrPth=0.7'],
           11: ['--ns3::AquaSimPbrx::HfsBeta=0.5', '--ns3::AquaSimPbrx::HfsAlpha=0.5']}
_jobs = rm.jobs
def jobs():
    for name, c, seed, m in _jobs():
        if name == 'untuned' and m not in UNTUNED: continue
        yield name, c, seed, m
rm.jobs = jobs
_cmd = rm.cmd
def cmd(c, seed, m, _name=[None]):
    a = _cmd(c, seed, m) + ['--tmax=200000']
    if c.get('idle'): a.append('--idle=%g' % c['idle'])
    if c.get('_untuned') and m in UNTUNED: a += UNTUNED[m]
    return a
rm.cmd = cmd
# mark untuned case
rm.CASES = [(n, dict(ov, _untuned=True) if n == 'untuned' else ov) for n, ov in rm.CASES]
rm.main()
