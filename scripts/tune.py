import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

"""Retune open values on training seeds 101-103 (static + walk 1 m/s, acoustic), faithful defaults.
usage: tune.py MODE OUT 'attr=v1,v2;attr2=v1,v2' [budget]"""
import itertools, json, os, subprocess, sys, time
NS3 = _os.environ.get('NS3_DIR', _os.path.join(REPO, 'ns-3.38')); BIN = NS3 + '/build/scratch/ns3.38-pbrx-sim-optimized'
mode, out, grid = int(sys.argv[1]), sys.argv[2], sys.argv[3]
budget = float(sys.argv[4]) if len(sys.argv) > 4 else 560
axes = [(a.split('=')[0], a.split('=')[1].split(',')) for a in grid.split(';')]
done = set()
if os.path.exists(out):
    for l in open(out):
        d = json.loads(l); done.add((tuple(d['cfg']), d['seed'], d['mobility']))
jobs = []
for vals in itertools.product(*[v for _, v in axes]):
    for seed in (101, 102, 103):
        for mob in (['static'], ['walk', '--speed=1']):
            if (tuple(vals), seed, mob[0]) in done: continue
            args = []
            for (a, _), v in zip(axes, vals):
                args.append(('--%s=%s' % (a[1:], v)) if a.startswith('@') else ('--ns3::AquaSimPbrx::%s=%s' % (a, v)))
            jobs.append((vals, seed, mob, [BIN, '--mode=%d' % mode, '--seed=%d' % seed, '--channel=acoustic', '--mobility=' + mob[0]] + mob[1:] + args))
env = dict(os.environ, LD_LIBRARY_PATH=NS3 + '/build/lib'); t0 = time.time(); run = []
with open(out, 'a') as f:
    while jobs or run:
        while jobs and len(run) < 2 and time.time() - t0 < budget - 30:
            vals, seed, mob, c = jobs.pop(0)
            run.append((subprocess.Popen(c, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, env=env, text=True), vals))
        if not run: break
        time.sleep(0.2)
        for r in list(run):
            if r[0].poll() is None: continue
            run.remove(r); d = json.loads(r[0].stdout.read().strip().splitlines()[-1]); d['cfg'] = list(r[1]); f.write(json.dumps(d) + '\n'); f.flush()
print('left', len(jobs))
# summary with the tie rule
import collections
R = collections.defaultdict(list)
for l in open(out):
    d = json.loads(l); R[tuple(d['cfg'])].append(d)
rows = []
for k, v in R.items():
    if len(v) < 6: continue
    st = sum(x['delivered'] for x in v if x['mobility'] == 'static') / 3; wk = sum(x['delivered'] for x in v if x['mobility'] == 'walk') / 3
    rows.append((k, st + wk, sum(x['pdr'] for x in v) / 6, sum(x['delay'] for x in v) / 6))
rows.sort(key=lambda r: -r[1])
for r in rows: print([a for a, _ in axes], r[0], 'tot=%.0f pdr=%.3f delay=%.1f' % r[1:])
best = rows[0][1]; near = [r for r in rows if r[1] >= 0.98 * best]
near.sort(key=lambda r: -r[2]); top_pdr = near[0][2]; near2 = [r for r in near if r[2] >= top_pdr - 0.01]
near2.sort(key=lambda r: r[3])
print('CHOSEN', near2[0][0])
