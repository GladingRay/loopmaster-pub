## SkylakeX arch info
num_threads=18
| cache | KB    | double  | copy bandwidth(GB/s) | load bandwidth(GB/s) |
|-------|-------|---------|----------------------|----------------------|
| L1    | 32    | 2^12    |       346.69         |      436.57          |
| L2    | 1024  | 2^17    |       99.58          |      165.22          |
| L3    | 25344 | 99*2^15 |       24.70          |      26.82           |

| info            | value  | unit    |
|-----------------|--------|---------|
| copy bandwidth  | 9.98   | GB/s    |
| load bandwidth  | 16.48  | GB/s    |
| peak flops      | 110.64 | GFLOPS  |
## Compare with
- OpenBLASv0.3.29
- MKLv2025.2
- TVM(auto_sched)v0.19.0
## Multi-threading settings
```bash
export OMP_NUM_THREADS=18
export OPENBLAS_NUM_THREADS=18
export OMP_PROC_BIND=close
export OMP_PLACES=sockets
numactl --cpunodebind=0 --membind=0 ${test}
```