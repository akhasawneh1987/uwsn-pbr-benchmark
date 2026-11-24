import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

import json,collections,numpy as np
from scipy.stats import wilcoxon, kendalltau, rankdata
R=collections.defaultdict(list)
for f in ['results/main/main_faithful.jsonl','results/robustness/extra.jsonl']:
  for l in open(_os.path.join(REPO, f)):
    d=json.loads(l); R[(d['case'],d['proto'])].append(d)
P=['DBR','EEDBR','WDFAD-DBR','HydroCast','DVOR','QLFR','APCDBRP','HFS-PBR','RE-PBR','SPRE-PBR']
def M(c,p,k='delivered'): return np.mean([d[k] for d in R[(c,p)]])
def table(c):
  ok=[p for p in P if len(R[(c,p)])==20]
  o=sorted(ok,key=lambda p:-M(c,p))
  rows=[(i+1,p,M(c,p),M(c,p,'pdr'),M(c,p,'J_per_deliv'),M(c,p,'delay'),M(c,p,'T10')) for i,p in enumerate(o)]
  a,b=o[0],o[1]
  sa={d['seed']:d['delivered'] for d in R[(c,a)]}; sb={d['seed']:d['delivered'] for d in R[(c,b)]}
  s=sorted(sa); x=np.array([sa[i] for i in s]); y=np.array([sb[i] for i in s])
  return rows,(a,b,100*(x.mean()/y.mean()-1),int((x>y).sum()),wilcoxon(x,y).pvalue)
import sys
for c in sys.argv[1:]:
  rows,t=table(c); print('==',c, 'n=%d'%len(rows)); 
  for r in rows: print('%d %-10s %6.0f %.3f %5.1f %5.2f %6.0f'%r)
  print('top2 %s vs %s %+.1f%% better %d/20 p=%.2g'%t)
  base={'idle_base':'base','idle_static':'static'}.get(c)
  if base:
    r1=[rankdata([-M(base,p) for p in P])[i] for i in range(10)]; r2=[rankdata([-M(c,p) for p in P])[i] for i in range(10)]
    print('tau vs',base,kendalltau(r1,r2)[0])
    for p in P: print('  %-10s %+.0f%%'%(p,100*(M(c,p)/M(base,p)-1)), 'T10 %+.0f%%'%(100*(M(c,p,'T10')/M(base,p,'T10')-1)))
