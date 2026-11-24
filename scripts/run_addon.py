import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))
"""Add-on experiment: one-bit route flag in the hello + nearest-sink shortcut switched on for all protocols.
HydroCast uses gamma = 0.008 s/m, the value tuned with the add-ons on. Usage: run_addon.py BUDGET_SECONDS [PARALLEL]"""
import sys
sys.path.insert(0, _os.path.join(REPO, 'scripts'))
import run_main as rm
rm.OUT = _os.path.join(REPO, 'results/addon/addon_on.jsonl')
_cmd = rm.cmd
rm.cmd = lambda c, seed, m: _cmd(c, seed, m) + ['--noRouteBit=0', '--noSinkShortcut=0'] + (['--ns3::AquaSimPbrx::HcGamma=0.008'] if m == 8 else [])
rm.main()
