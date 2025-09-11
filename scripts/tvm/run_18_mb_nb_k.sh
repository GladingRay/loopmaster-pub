#!/usr/bin/bash
tn=18
m=8192
n=8192
for ((k=16; k<=128; k+=16)); do
    sh ./run.sh tvm_autosched_gemm_colmajor.py $tn $m $n $k 1000 Y mb_nb_k
done