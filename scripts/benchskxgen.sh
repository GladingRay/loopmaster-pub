#!/bin/bash
m=$1
n=$2
k=$3
export OMP_NUM_THREADS=1
export OMP_PROC_BIND=close
export OMP_PLACES=cores

numactl --cpunodebind=1 --membind=1 ../build/benchgen $m $n $k 2 $4
