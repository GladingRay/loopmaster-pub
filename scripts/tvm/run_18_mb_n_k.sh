#!/usr/bin/bash
tn=18
m=8192
# k=8192
for ((n=16; n<=128; n+=16)); do
    sh ./run.sh tvm_autosched_gemm_colmajor.py $tn $m $n $n 1000 N mb_n_k 1000
done