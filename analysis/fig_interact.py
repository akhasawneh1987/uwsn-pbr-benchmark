import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
plt.rcParams.update({'font.family':'serif','font.serif':['Times New Roman','Times','DejaVu Serif'],'font.size':8})
COL={'HFS-PBR':'#2a78d6','QLFR':'#eb6834','SPRE-PBR':'#1baf7a'}
# rows: nodes 400,300,200 (top to bottom); cols: static, walk 1, current 1
G={(200,'s'):('HFS-PBR','QLFR',24.3,'<0.001'),(200,'w'):('HFS-PBR','EEDBR',22.3,'<0.001'),(200,'c'):('QLFR','HFS-PBR',10.7,'0.006'),
   (300,'s'):('HFS-PBR','SPRE-PBR',17.2,'<0.001'),(300,'w'):('HFS-PBR','SPRE-PBR',15.8,'0.001'),(300,'c'):('QLFR','HFS-PBR',9.5,'0.001'),
   (400,'s'):('SPRE-PBR','RE-PBR',6.9,'<0.001'),(400,'w'):('SPRE-PBR','RE-PBR',6.0,'<0.001'),(400,'c'):('QLFR','HFS-PBR',3.5,'0.05')}
fig,ax=plt.subplots(figsize=(3.45,2.5))
cols=['s','w','c']; rows=[400,300,200]
for i,n in enumerate(rows):
  for j,m in enumerate(cols):
    w,r,g,p=G[(n,m)]
    ax.add_patch(plt.Rectangle((j+0.04,2-i+0.04),0.92,0.92,color=COL[w],alpha=0.18,lw=0))
    ax.add_patch(plt.Rectangle((j+0.04,2-i+0.04),0.06,0.92,color=COL[w],lw=0))
    ax.text(j+0.55,2-i+0.72,w,ha='center',va='center',fontsize=7.6,fontweight='bold',color='#0b0b0b')
    ax.text(j+0.55,2-i+0.49,'+%.1f%% vs %s'%(g,r) if len(r)<8 else '+%.1f%% vs'%g,ha='center',va='center',fontsize=5.9,color='#52514e')
    if len(r)>=8: ax.text(j+0.55,2-i+0.33,r,ha='center',va='center',fontsize=5.9,color='#52514e')
    ax.text(j+0.55,2-i+(0.17 if len(r)>=8 else 0.3),('p %s'%p) if p.startswith('<') else 'p = %s'%p,ha='center',va='center',fontsize=5.9,color='#52514e')
ax.set_xlim(0,3); ax.set_ylim(0,3)
ax.set_xticks([0.5,1.5,2.5]); ax.set_xticklabels(['Static','Random walk 1 m/s','Current 1 m/s'],fontsize=7)
ax.set_yticks([0.5,1.5,2.5]); ax.set_yticklabels(['200 nodes','300 nodes','400 nodes'],fontsize=7)
for s in ax.spines.values(): s.set_visible(False)
ax.tick_params(length=0)
fig.tight_layout(pad=0.3); fig.savefig(_os.path.join(REPO, 'figures/fig_interact.pdf')); fig.savefig(_os.path.join(REPO, 'figures/fig_interact.png'),dpi=220)
