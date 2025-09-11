#!/usr/bin/bash
tn=18
n=8192
k=8192
for ((m=16; m<=128; m+=16)); do
    sh ./run.sh tvm_autosched_gemm_colmajor.py $tn $m $n $k 1000 Y m_nb_kb
done