#!/bin/bash
tn=$1
m=$2
n=$3
k=$4
export OMP_NUM_THREADS=$tn
export OPENBLAS_NUM_THREADS=$tn
export OMP_PROC_BIND=close
export OMP_PLACES=sockets

numactl --cpunodebind=1 --membind=1 ../build/benchblas OpenBLAS $m $n $k 2 $5
