#!/usr/bin/env python3
import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

"""Tables for the main benchmark (main.jsonl -> main_tables.md + summary.json)."""
import json, collections, math, sys
import numpy as np
from scipy.stats import wilcoxon, t as tdist

IN = sys.argv[1] if len(sys.argv) > 1 else _os.path.join(REPO, 'results/addon/addon_on.jsonl')
OUT = sys.argv[2] if len(sys.argv) > 2 else _os.path.join(REPO, 'results/addon/addon_on_tables.md')
NAME = {0: 'DBR', 1: 'RE-PBR', 2: 'SPRE-PBR', 4: 'APCDBRP', 6: 'EEDBR', 7: 'WDFAD-DBR', 8: 'HydroCast',
        9: 'DVOR', 10: 'QLFR', 11: 'HFS-PBR'}
CASES = ['base', 'nodes100', 'nodes200', 'nodes300', 'static', 'walk3', 'cur0.3', 'cur1', 'load0.5', 'load2',
         'range', 'sinks1', 'sinks3', 'sinks7']
DESC = {'base': '400 nodes, 5 sinks, 1 pkt/s, acoustic, walk 1 m/s', 'nodes100': '100 nodes', 'nodes200': '200 nodes',
        'nodes300': '300 nodes', 'static': 'static', 'walk3': 'walk 3 m/s', 'cur0.3': 'current 0.3 m/s',
        'cur1': 'current 1 m/s', 'load0.5': '0.5 pkt/s', 'load2': '2 pkt/s', 'range': 'ideal range channel',
        'sinks1': '1 sink', 'sinks3': '3 sinks', 'sinks7': '7 sinks'}
METRICS = [('delivered', 'Delivered before T10', '%.0f'), ('pdr', 'PDR', '%.3f'), ('J_per_deliv', 'J / delivered', '%.1f'),
           ('delay', 'Delay (s)', '%.2f'), ('T10', 'T10 (s)', '%.0f'), ('tx_per_pkt', 'Data tx / generated', '%.1f')]

R = collections.defaultdict(dict)   # (case, mode) -> seed -> record
for line in open(IN):
    d = json.loads(line)
    R[(d['case'], d['mode'])][d['seed']] = d


def ci(x):
    x = np.asarray(x, float)
    if len(x) < 2:
        return 0.0
    return tdist.ppf(0.975, len(x) - 1) * x.std(ddof=1) / math.sqrt(len(x))


out, summary = [], {}
out.append('# Main benchmark results\n')
out.append('Mean over 20 topologies (seeds 11–30) ± 95% CI for the main metric. Rank 1 = most packets delivered before 10% of sensors die.\n')
out.append('"vs next" = paired two-sided Wilcoxon signed-rank test between rank 1 and rank 2 (same topologies).\n')
ranks = collections.defaultdict(list)
wins = collections.Counter()
pdr_ranks = collections.defaultdict(list)
for case in CASES:
    rows = []
    for m in NAME:
        recs = R.get((case, m), {})
        if not recs:
            continue
        seeds = sorted(recs)
        vals = {k: [recs[s][k] for s in seeds] for k, _, _ in METRICS}
        rows.append((m, seeds, vals))
    rows.sort(key=lambda r: -np.mean(r[2]['delivered']))
    summary[case] = {}
    out.append('\n## %s — %s\n' % (case, DESC[case]))
    out.append('| rank | protocol | n | ' + ' | '.join(h for _, h, _ in METRICS) + ' |')
    out.append('|' + '---|' * (3 + len(METRICS)))
    for i, (m, seeds, vals) in enumerate(rows):
        cells = []
        for k, _, fmt in METRICS:
            c = fmt % np.mean(vals[k])
            if k == 'delivered':
                c += ' ± %.0f' % ci(vals[k])
            cells.append(c)
        out.append('| %d | %s | %d | %s |' % (i + 1, NAME[m], len(seeds), ' | '.join(cells)))
        ranks[m].append(i + 1)
        summary[case][NAME[m]] = {k: float(np.mean(vals[k])) for k, _, _ in METRICS}
        summary[case][NAME[m]]['delivered_ci'] = float(ci(vals['delivered']))
        summary[case][NAME[m]]['rank'] = i + 1
    wins[rows[0][0]] += 1
    a, b = rows[0], rows[1]
    common = sorted(set(a[1]) & set(b[1]))
    xa = [R[(case, a[0])][s]['delivered'] for s in common]
    xb = [R[(case, b[0])][s]['delivered'] for s in common]
    nwin = sum(1 for u, v in zip(xa, xb) if u > v)
    try:
        p = wilcoxon(xa, xb).pvalue
    except ValueError:
        p = float('nan')
    gain = 100 * (np.mean(xa) / np.mean(xb) - 1)
    out.append('\n%s vs %s: %+.1f%%, better on %d/%d topologies, p = %.2g.' % (NAME[a[0]], NAME[b[0]], gain, nwin, len(common), p))
    summary[case]['_top2'] = dict(first=NAME[a[0]], second=NAME[b[0]], gain_pct=gain, wins=nwin, n=len(common), p=p)
    # minimum-PDR view: rank only protocols with mean PDR >= 0.5
    ok = [r for r in rows if np.mean(r[2]['pdr']) >= 0.5]
    out.append('Among protocols with mean PDR ≥ 0.5: ' + (', '.join('%d. %s' % (i + 1, NAME[r[0]]) for i, r in enumerate(ok)) or 'none') + '.')
    for i, r in enumerate(ok):
        pdr_ranks[r[0]].append(i + 1)

out.append('\n## Overall (14 cases)\n')
out.append('| protocol | mean rank | best rank | worst rank | cases ranked 1st | mean rank among PDR ≥ 0.5 (cases) |')
out.append('|---|---|---|---|---|---|')
for m in sorted(NAME, key=lambda m: np.mean(ranks[m])):
    pr = pdr_ranks.get(m, [])
    out.append('| %s | %.1f | %d | %d | %d | %s |' % (NAME[m], np.mean(ranks[m]), min(ranks[m]), max(ranks[m]), wins[m],
                                                     ('%.1f (%d)' % (np.mean(pr), len(pr))) if pr else '— (0)'))
open(OUT, 'w').write('\n'.join(out) + '\n')
json.dump(summary, open(OUT.replace('.md', '.json'), 'w'), indent=1)
print('\n'.join(out[-14:]))
