#!/bin/bash
tn=$1
export OMP_NUM_THREADS=$tn
export OPENBLAS_NUM_THREADS=$tn
export OMP_PROC_BIND=close
export OMP_PLACES=cores