#!/usr/bin/bash
tn=18
# n=8192
# k=8192
for ((m=1024; m<=8192; m+=1024)); do
    sh ./run.sh tvm_autosched_gemm_colmajor.py $tn $m $m $m 1000 Y mb_nb_kb
done