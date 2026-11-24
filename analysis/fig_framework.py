import os as _os
REPO = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch
plt.rcParams.update({'font.family':'serif','font.serif':['Times New Roman','Times','DejaVu Serif']})
fig,ax=plt.subplots(figsize=(7.1,2.55)); ax.set_xlim(-1.5,101.5); ax.set_ylim(0,36); ax.axis('off')
INK='#0b0b0b'; SUB='#52514e'; EDGE='#2a78d6'; FILL='#eef4fd'
steps=[('1. Implementation','10 protocols from\ntheir published\ndescriptions in one\nAqua-Sim NG module'),
('2. Unified models','Same channel,\nenergy, MAC,\nmobility and traffic\nfor all protocols'),
('3. Tuning','Open parameters\ntuned on training\nseeds 101–103 with\none common rule'),
('4. Test runs','14 scenarios\n× 20 topologies\n× 10 protocols\n= 2,800 runs'),
('5. Analysis','Main metric, PDR,\nenergy, delay, T10\nWilcoxon, Friedman\nand Nemenyi tests')]
w,h,y=17.6,19,13; gap=(100-5*w)/4
xs=[i*(w+gap) for i in range(5)]
for x,(t,b) in zip(xs,steps):
  ax.add_patch(FancyBboxPatch((x,y),w,h,boxstyle='round,pad=0.25,rounding_size=1.2',fc=FILL,ec=EDGE,lw=1.0))
  ax.text(x+w/2,y+h-3.2,t,ha='center',va='center',fontsize=7.4,fontweight='bold',color=INK)
  ax.text(x+w/2,y+h/2-2.2,b,ha='center',va='center',fontsize=6.3,color=SUB,linespacing=1.3)
for i in range(4):
  ax.add_patch(FancyArrowPatch((xs[i]+w+0.4,y+h/2),(xs[i+1]-0.4,y+h/2),arrowstyle='-|>',mutation_scale=9,color=SUB,lw=1))
# add-on box
bx=xs[3]; bw=xs[4]+w-bx
ax.add_patch(FancyBboxPatch((bx,1.2),bw,7.6,boxstyle='round,pad=0.25,rounding_size=1.2',fc='white',ec=SUB,lw=0.9,ls=(0,(3,2))))
ax.text(bx+bw/2,5.0,'Separate experiment: one-bit route flag\nand nearest-sink shortcut (not ranked)',ha='center',va='center',fontsize=6.3,color=SUB,linespacing=1.3)
ax.add_patch(FancyArrowPatch((xs[3]+w/2,y-0.4),(xs[3]+w/2,9.3),arrowstyle='-|>',mutation_scale=8,color=SUB,lw=0.9))
# rules band
ax.add_patch(FancyBboxPatch((0,1.2),xs[2]+w,7.6,boxstyle='round,pad=0.25,rounding_size=1.2',fc='white',ec=SUB,lw=0.9))
ax.text((xs[2]+w)/2,5.0,'Fairness rules: faithful protocols, common tuning, exact information\nfor missing mechanisms, shared test topologies and disclosure',ha='center',va='center',fontsize=6.3,color=SUB,linespacing=1.3)
fig.tight_layout(pad=0.2); fig.savefig(_os.path.join(REPO, 'figures/fig_framework.pdf')); fig.savefig(_os.path.join(REPO, 'figures/fig_framework.png'),dpi=220)
