import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

import json, numpy as np, matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
plt.rcParams.update({'font.family':'serif','font.serif':['Times New Roman','Times','DejaVu Serif'],
  'font.size':8,'axes.labelsize':8,'xtick.labelsize':7,'ytick.labelsize':7,'legend.fontsize':7,
  'axes.spines.top':False,'axes.spines.right':False,'axes.edgecolor':'#52514e','axes.linewidth':0.6,
  'xtick.color':'#52514e','ytick.color':'#52514e','axes.labelcolor':'#0b0b0b','savefig.dpi':600,
  'axes.grid':True,'grid.color':'#e4e3df','grid.linewidth':0.5,'axes.axisbelow':True})
F=json.load(open(_os.path.join(REPO, 'results/main/main_tables.json')))
A=json.load(open(_os.path.join(REPO, 'results/addon/addon_on_tables.json')))
P=['DBR','EEDBR','WDFAD-DBR','HydroCast','DVOR','QLFR','APCDBRP','HFS-PBR','RE-PBR','SPRE-PBR']
CASES=['base','nodes100','nodes200','nodes300','static','walk3','cur0.3','cur1','load0.5','load2','range','sinks1','sinks3','sinks7']
LAB={'base':'Base','nodes100':'100 nodes','nodes200':'200 nodes','nodes300':'300 nodes','static':'Static','walk3':'Walk 3 m/s',
 'cur0.3':'Current 0.3','cur1':'Current 1','load0.5':'0.5 pkt/s','load2':'2 pkt/s','range':'Range ch.','sinks1':'1 sink','sinks3':'3 sinks','sinks7':'7 sinks'}
HL={'HFS-PBR':('#2a78d6','o'),'QLFR':('#eb6834','s'),'SPRE-PBR':('#1baf7a','^'),'DBR':('#eda100','D')}
W=3.45
import collections
from scipy.stats import t as tdist
RAW=collections.defaultdict(list)
for l in open(_os.path.join(REPO, 'results/main/main_faithful.jsonl')):
  d=json.loads(l); RAW[(d['case'],d['proto'])].append(d)
def ci(c,p,k):
  v=np.array([d[k] for d in RAW[(c,p)]]); return tdist.ppf(0.975,len(v)-1)*v.std(ddof=1)/np.sqrt(len(v))
# Fig: heatmap of normalised main metric + rank
order=sorted(P,key=lambda p:np.mean([F[c][p]['rank'] for c in CASES]))
M=np.array([[F[c][p]['delivered']/max(F[c][q]['delivered'] for q in P) for c in CASES] for p in order])
from matplotlib.colors import LinearSegmentedColormap
cm=LinearSegmentedColormap.from_list('b',['#f4f8fe','#cde2fb','#86b6ef','#3987e5','#1c5cab','#0d366b'])
fig,ax=plt.subplots(figsize=(7.1,3.0))
im=ax.imshow(M,cmap=cm,vmin=0.2,vmax=1,aspect='auto')
for i,p in enumerate(order):
  for j,c in enumerate(CASES):
    r=F[c][p]['rank']; ax.text(j,i,str(r),ha='center',va='center',fontsize=7,color='white' if M[i,j]>0.68 else '#0b0b0b',fontweight='bold' if r==1 else 'normal')
ax.set_xticks(range(len(CASES)));ax.set_xticklabels([LAB[c] for c in CASES],rotation=35,ha='right')
ax.set_yticks(range(len(order)));ax.set_yticklabels(order);ax.grid(False)
for s in ax.spines.values(): s.set_visible(False)
ax.set_xticks(np.arange(-.5,len(CASES)),minor=True);ax.set_yticks(np.arange(-.5,len(order)),minor=True)
ax.grid(which='minor',color='white',linewidth=1.5);ax.tick_params(which='minor',length=0)
cb=fig.colorbar(im,ax=ax,fraction=0.025,pad=0.01);cb.set_label('Delivered before T10 / best in case');cb.outline.set_visible(False)
fig.tight_layout();fig.savefig(_os.path.join(REPO, 'figures/fig_heatmap.pdf'));fig.savefig(_os.path.join(REPO, 'figures/fig_heatmap.png'),dpi=200)
def sweep(cases,xs,xlabel,fname,key='delivered',ylabel='Packets delivered before T10'):
  fig,ax=plt.subplots(figsize=(W,2.7))
  for p in P:
    y=[F[c][p][key] for c in cases]
    if p in HL: continue
    ax.plot(xs,y,color='#c3c2b7',lw=1,zorder=1)
  ax.plot([],[],color='#c3c2b7',lw=1,label='Other protocols')
  for p,(col,mk) in HL.items():
    y=[F[c][p][key] for c in cases]
    e=[ci(c,p,key) for c in cases]
    ax.fill_between(xs,np.array(y)-e,np.array(y)+e,color=col,alpha=0.15,lw=0,zorder=2)
    ax.plot(xs,y,color=col,lw=2,marker=mk,ms=4.5,mec='white',mew=0.8,label=p,zorder=3)
  ax.set_xticks(xs);ax.set_xlabel(xlabel);ax.set_ylabel(ylabel)
  ax.legend(frameon=False,ncol=3,loc='lower center',bbox_to_anchor=(0.5,1.0),handlelength=1.8,columnspacing=1.0)
  fig.tight_layout();fig.savefig(fname+'.pdf');fig.savefig(fname+'.png',dpi=200)
sweep(['nodes100','nodes200','nodes300','base'],[100,200,300,400],'Number of sensor nodes',_os.path.join(REPO, 'figures/fig_nodes'))
sweep(['sinks1','sinks3','base','sinks7'],[1,3,5,7],'Number of surface sinks',_os.path.join(REPO, 'figures/fig_sinks'))
sweep(['nodes100','nodes200','nodes300','base'],[100,200,300,400],'Number of sensor nodes',_os.path.join(REPO, 'figures/fig_nodes_pdr'),key='pdr',ylabel='Packet delivery ratio')
# scatter PDR vs main metric, base
fig,ax=plt.subplots(figsize=(W,2.6))
off={'SPRE-PBR':(6,4),'RE-PBR':(6,-9),'QLFR':(6,6),'HFS-PBR':(6,-9),'WDFAD-DBR':(6,0),'DVOR':(6,0),'EEDBR':(6,0),'APCDBRP':(-6,-9),'HydroCast':(6,0),'DBR':(-6,0)}
for p in P:
  x=F['base'][p]['pdr'];y=F['base'][p]['delivered'];col='#2a78d6'
  ax.scatter(x,y,s=34,color=col,edgecolor='white',linewidth=1,zorder=3)
  dx,dy=off[p]
  if p=='HFS-PBR': continue
  ax.annotate('QLFR, HFS-PBR' if p=='QLFR' else p,(x,y),xytext=(dx,dy),textcoords='offset points',fontsize=7,ha='left' if dx>0 else 'right',va='center',color='#0b0b0b')
ax.set_xlabel('Packet delivery ratio');ax.set_ylabel('Packets delivered before T10');ax.set_xlim(0.35,0.8)
fig.tight_layout();fig.savefig(_os.path.join(REPO, 'figures/fig_tradeoff.pdf'));fig.savefig(_os.path.join(REPO, 'figures/fig_tradeoff.png'),dpi=200)
# add-on gain
AP=['RE-PBR','SPRE-PBR','HydroCast','EEDBR','QLFR','HFS-PBR']
g={p:100*np.mean([A[c][p]['delivered']/F[c][p]['delivered']-1 for c in CASES]) for p in AP}
AP=sorted(AP,key=lambda p:g[p])
fig,ax=plt.subplots(figsize=(W,2.0))
ax.barh(range(len(AP)),[g[p] for p in AP],height=0.6,color='#2a78d6',edgecolor='none')
for i,p in enumerate(AP): ax.text(g[p]+1,i,('%+.0f%%'%g[p]) if abs(g[p])>=0.5 else '0%',va='center',fontsize=7)
ax.set_yticks(range(len(AP)));ax.set_yticklabels(AP);ax.set_xlabel('Mean change in packets delivered before T10 (%)')
ax.grid(axis='y',visible=False);ax.set_xlim(-2,68)
fig.tight_layout();fig.savefig(_os.path.join(REPO, 'figures/fig_addon.pdf'));fig.savefig(_os.path.join(REPO, 'figures/fig_addon.png'),dpi=200)
print({p:round(g[p],1) for p in g})
# energy & delay base: two panels
fig,axs=plt.subplots(1,2,figsize=(7.1,2.3))
for ax,key,lab in [(axs[0],'J_per_deliv','Energy per delivered packet (J)'),(axs[1],'delay','Mean end-to-end delay (s)')]:
  o=sorted(P,key=lambda p:F['base'][p][key])
  v=[F['base'][p][key] for p in o]
  ax.barh(range(len(o)),v,height=0.6,color='#2a78d6')
  for i,x in enumerate(v): ax.text(x*1.01+0.1,i,'%.1f'%x,va='center',fontsize=6.5)
  ax.set_yticks(range(len(o)));ax.set_yticklabels(o);ax.set_xlabel(lab);ax.grid(axis='y',visible=False);ax.invert_yaxis()
  ax.set_xlim(0,max(v)*1.15)
fig.tight_layout();fig.savefig(_os.path.join(REPO, 'figures/fig_energy_delay.pdf'));fig.savefig(_os.path.join(REPO, 'figures/fig_energy_delay.png'),dpi=200)
