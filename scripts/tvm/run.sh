#!/usr/bin/bash
# args: pyname tn m n k epoch flag shape_type

export OPENBLAS_NUM_THREADS=$2
export OMP_NUM_THREADS=$2
export TVM_NUM_THREADS=$2
logfilename=$1_$2_$3_$4_$5

numactl --cpunodebind=1 --membind=1 python3 $1 $3 $4 $5 $logfilename.json $6 $7 $8 $9
