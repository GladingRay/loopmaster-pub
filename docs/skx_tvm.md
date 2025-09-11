# TVM gencode on skx
## MNk-18-8192x8192x128
### Transform B
ax6->K(8)  
ax4->K(16)  
ax7->N(8)  
ax5->N(4)  
ax012->N(256)
continous: ax7 ax6 ax5 ax4 ax012
tile rule: N(8) K(8) N(4x8) K(16x8) N(256x4x8, parallel)
### C_tmp
src: M(1024) N(32)  
dst: M(1024) N(32)
### Compute
o2i  
N: 8192=256x4x8  
M: 8192=4x2x64x16  
K: 128=16x8  
loop body: M(16) N(8) | K(8) M(64x16=1024) N(4x8=32) K(16x8=128) (C_tmp init) M(2x1024=2048) MN_fused(256x4, parallel)

