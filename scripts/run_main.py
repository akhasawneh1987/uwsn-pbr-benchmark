#!/usr/bin/env python3
import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

"""Main benchmark runner: 14 cases x 10 protocols x 20 seeds. Protocols as published (no framework add-ons). Resumable: skips jobs already in the output file.
Usage: run_main.py BUDGET_SECONDS [PARALLEL]"""
import json, os, subprocess, sys, time

NS3 = _os.environ.get('NS3_DIR', _os.path.join(REPO, 'ns-3.38'))
BIN = NS3 + '/build/scratch/ns3.38-pbrx-sim-optimized'
OUT = _os.path.join(REPO, 'results/main/main_faithful.jsonl')
MODES = [0, 1, 2, 4, 6, 7, 8, 9, 10, 11]
SEEDS = range(11, 31)
BASE = dict(nodes=400, sinks=5, lam=1.0, channel='acoustic', mobility='walk', speed=1.0)
CASES = [('base', {})]
CASES += [('nodes%d' % n, dict(nodes=n)) for n in (100, 200, 300)]
CASES += [('static', dict(mobility='static', speed=0.0)), ('walk3', dict(speed=3.0)),
          ('cur0.3', dict(mobility='meander', speed=0.3)), ('cur1', dict(mobility='meander', speed=1.0))]
CASES += [('load0.5', dict(lam=0.5)), ('load2', dict(lam=2.0))]
CASES += [('range', dict(channel='range'))]
CASES += [('sinks%d' % k, dict(sinks=k)) for k in (1, 3, 7)]


def jobs():
    for name, ov in CASES:
        c = dict(BASE, **ov)
        for seed in SEEDS:
            for m in MODES:
                yield name, c, seed, m


def done_keys():
    keys = set()
    if os.path.exists(OUT):
        for line in open(OUT):
            try:
                d = json.loads(line)
                keys.add((d['case'], d['mode'], d['seed']))
            except Exception:
                pass
    return keys


def cmd(c, seed, m):
    return [BIN, '--mode=%d' % m, '--seed=%d' % seed, '--nodes=%d' % c['nodes'], '--sinks=%d' % c['sinks'],
            '--lambda=%g' % c['lam'], '--channel=' + c['channel'], '--mobility=' + c['mobility'],
            '--speed=%g' % c['speed'], '--near=1', '--tmax=200000']


def main():
    budget = float(sys.argv[1]) if len(sys.argv) > 1 else 540
    par = int(sys.argv[2]) if len(sys.argv) > 2 else 2
    t0 = time.time()
    done = done_keys()
    todo = [j for j in jobs() if (j[0], j[3], j[2]) not in done]
    env = dict(os.environ, LD_LIBRARY_PATH=NS3 + '/build/lib')
    running, n_ok, n_fail = [], 0, 0
    with open(OUT, 'a') as out:
        while todo or running:
            while todo and len(running) < par and time.time() - t0 < budget - 20:
                name, c, seed, m = todo.pop(0)
                p = subprocess.Popen(cmd(c, seed, m), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, env=env, text=True)
                running.append((p, name, seed, m))
            if not running:
                break
            time.sleep(0.2)
            for r in list(running):
                p, name, seed, m = r
                if p.poll() is None:
                    continue
                running.remove(r)
                txt = p.stdout.read().strip()
                try:
                    d = json.loads(txt.splitlines()[-1])
                    d.update(case=name, mode=m)
                    out.write(json.dumps(d) + '\n'); out.flush(); n_ok += 1
                except Exception:
                    n_fail += 1
                    sys.stderr.write('FAIL %s mode=%d seed=%d rc=%s\n' % (name, m, seed, p.returncode))
    total = len(list(jobs()))
    print('chunk: %d ok, %d failed, %.0f s; done %d / %d' % (n_ok, n_fail, time.time() - t0, len(done_keys()), total))


if __name__ == '__main__':
    main()
