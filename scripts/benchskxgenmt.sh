#!/bin/bash
tn=$1
m=$2
n=$3
k=$4
export OMP_NUM_THREADS=$tn
export OMP_PROC_BIND=close
export OMP_PLACES=cores

numactl --cpunodebind=1 --membind=1 ../build/benchgenmt $m $n $k 2 $5
