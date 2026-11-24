import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

import json, numpy as np, collections
from scipy.stats import kendalltau, friedmanchisquare, rankdata
R=collections.defaultdict(list)
for l in open(_os.path.join(REPO, 'results/main/main_faithful.jsonl')):
    d=json.loads(l); R[(d['case'],d['proto'])].append(d)
CASES=['base','nodes100','nodes200','nodes300','static','walk3','cur0.3','cur1','load0.5','load2','range','sinks1','sinks3','sinks7']
P=['DBR','EEDBR','WDFAD-DBR','HydroCast','DVOR','QLFR','APCDBRP','HFS-PBR','RE-PBR','SPRE-PBR']
def m(c,p,k): return float(np.mean([d[k] for d in R[(c,p)]]))
out={}
# ranks by main metric
rk={c:dict(zip(P,rankdata([-m(c,p,'delivered') for p in P]))) for c in CASES}
out['meanrank_main']={p:float(np.mean([rk[c][p] for c in CASES])) for p in P}
# FND ranking
rf={c:dict(zip(P,rankdata([-m(c,p,'FND') for p in P]))) for c in CASES}
out['meanrank_fnd']={p:float(np.mean([rf[c][p] for c in CASES])) for p in P}
taus=[kendalltau([rk[c][p] for p in P],[rf[c][p] for p in P])[0] for c in CASES]
out['tau_main_fnd']=dict(mean=float(np.mean(taus)),min=float(np.min(taus)),max=float(np.max(taus)))
out['fnd_wins']=collections.Counter(min(P,key=lambda p:rf[c][p]) for c in CASES)
# TOPSIS: criteria PDR(+), J/deliv(-), delay(-), T10(+)
def topsis(c,w):
    X=np.array([[m(c,p,'pdr'),m(c,p,'J_per_deliv'),m(c,p,'delay'),m(c,p,'T10')] for p in P])
    N=X/np.sqrt((X**2).sum(0)); V=N*w
    ben=np.array([1,0,0,1],bool)
    best=np.where(ben,V.max(0),V.min(0)); worst=np.where(ben,V.min(0),V.max(0))
    dp=np.sqrt(((V-best)**2).sum(1)); dn=np.sqrt(((V-worst)**2).sum(1))
    return dn/(dp+dn)
W={'equal':np.array([.25,.25,.25,.25]),'reliability':np.array([.4,.2,.2,.2]),'lifetime':np.array([.2,.2,.2,.4]),'energy':np.array([.2,.4,.2,.2]),'delay':np.array([.2,.2,.4,.2])}
out['topsis']={}
for name,w in W.items():
    rr={c:dict(zip(P,rankdata(-topsis(c,w)))) for c in CASES}
    out['topsis'][name]={p:float(np.mean([rr[c][p] for c in CASES])) for p in P}
    out['topsis'][name+'_wins']=dict(collections.Counter(min(P,key=lambda p:rr[c][p]) for c in CASES))
X=np.array([[rr[c][p] for c in CASES] for p in P])
out['topsis_equal_friedman']=float(friedmanchisquare(*[[dict(zip(P,rankdata(-topsis(c,W['equal']))))[p] for c in CASES] for p in P]).pvalue)
# overhead base: per delivered packet by type
ov={}
for p in P:
    rs=R[('base',p)]
    dl=sum(d['delivered'] for d in rs)
    ov[p]=dict(data=sum(d['data_tx'] for d in rs)/dl, hello=sum(d['hello'] for d in rs)/dl,
               ack=sum(d['acks'] for d in rs)/dl, ctrl=sum(d['ctrl']+d['probes'] for d in rs)/dl)
out['overhead_base']=ov
json.dump(out,open(_os.path.join(REPO, 'results/robustness/group_a.json'),'w'),indent=1,default=dict)
for k,v in out.items(): print(k, json.dumps(v,default=dict)[:600])
