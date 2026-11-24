import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

import json, numpy as np, matplotlib, collections
matplotlib.use('Agg')
import matplotlib.pyplot as plt
plt.rcParams.update({'font.family':'serif','font.serif':['Times New Roman','Times','DejaVu Serif'],'font.size':8,
 'axes.spines.top':False,'axes.spines.right':False,'axes.edgecolor':'#52514e','axes.linewidth':0.6,
 'xtick.color':'#52514e','ytick.color':'#52514e','xtick.labelsize':7,'ytick.labelsize':7,'legend.fontsize':7,'savefig.dpi':600})
A=json.load(open(_os.path.join(REPO, 'results/robustness/group_a.json')))
# ---- CD diagram
mr=A['meanrank_main']; CD=3.6207
order=sorted(mr,key=mr.get); k=len(order)
fig,ax=plt.subplots(figsize=(7.1,2.1)); ax.set_xlim(0.5,10.5); ax.set_ylim(-0.2,1.25); ax.axis('off')
y0=1.0
ax.plot([1,10],[y0,y0],color='#52514e',lw=0.8)
for t in range(1,11):
    ax.plot([t,t],[y0,y0+0.04],color='#52514e',lw=0.8); ax.text(t,y0+0.07,str(t),ha='center',va='bottom',fontsize=7,color='#52514e')
# CD bar
ax.plot([1,1+CD],[y0+0.22,y0+0.22],color='#0b0b0b',lw=1.2)
for x in (1,1+CD): ax.plot([x,x],[y0+0.19,y0+0.25],color='#0b0b0b',lw=1.2)
ax.text(1+CD/2,y0+0.27,'CD = 3.62',ha='center',va='bottom',fontsize=7)
left=order[:5]; right=order[5:]
for i,p in enumerate(left):
    yy=y0-0.13-0.13*i; x=mr[p]
    ax.plot([x,x,0.9],[y0,yy,yy],color='#52514e',lw=0.7); ax.text(0.85,yy,'%s (%.2f)'%(p,x),ha='right',va='center',fontsize=7)
for i,p in enumerate(right[::-1]):
    yy=y0-0.13-0.13*i; x=mr[p]
    ax.plot([x,x,10.1],[y0,yy,yy],color='#52514e',lw=0.7); ax.text(10.15,yy,'(%.2f) %s'%(x,p),ha='left',va='center',fontsize=7)
# cliques
vals=[mr[p] for p in order]; cl=[]
for i in range(k):
    j=max(jj for jj in range(k) if vals[jj]-vals[i]<CD)
    if j>i and not any(a<=i and j<=b for a,b in cl): cl.append((i,j))
for n,(a,b) in enumerate(cl):
    yy=y0-0.06-0.05*n
    ax.plot([vals[a]-0.03,vals[b]+0.03],[yy,yy],color='#2a78d6',lw=2.4,solid_capstyle='round')
fig.tight_layout(pad=0.2); fig.savefig(_os.path.join(REPO, 'figures/fig_cd.pdf')); fig.savefig(_os.path.join(REPO, 'figures/fig_cd.png'),dpi=200)
print('cliques',[(order[a],order[b]) for a,b in cl])
# ---- overhead stacked
ov=A['overhead_base']; P=sorted(ov,key=lambda p:sum(ov[p].values()))
cats=[('data','Data','#2a78d6'),('hello','Hello or beacon','#eb6834'),('ack','ACK','#1baf7a'),('ctrl','Other control','#eda100')]
fig,ax=plt.subplots(figsize=(3.45,2.7))
left=np.zeros(len(P))
for key,lab,col in cats:
    v=np.array([ov[p][key] for p in P])
    ax.barh(range(len(P)),v,left=left,height=0.62,color=col,edgecolor='white',linewidth=0.8,label=lab); left+=v
for i,p in enumerate(P): ax.text(left[i]+1,i,'%.0f'%left[i],va='center',fontsize=6.5)
ax.set_yticks(range(len(P))); ax.set_yticklabels(P); ax.set_xlabel('Transmissions per delivered packet'); ax.set_xlim(0,max(left)*1.12)
ax.grid(axis='x',color='#e4e3df',lw=0.5); ax.set_axisbelow(True)
ax.legend(frameon=False,loc='lower center',bbox_to_anchor=(0.4,1.0),ncol=4,fontsize=6.3,handlelength=1.2,columnspacing=0.8)
fig.tight_layout(pad=0.3); fig.savefig(_os.path.join(REPO, 'figures/fig_overhead.pdf')); fig.savefig(_os.path.join(REPO, 'figures/fig_overhead.png'),dpi=200)
