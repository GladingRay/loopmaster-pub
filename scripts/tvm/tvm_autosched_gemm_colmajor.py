import numpy as np
import tvm
from tvm import te, auto_scheduler
import os

import tvm.auto_scheduler

@auto_scheduler.register_workload
def gemm_with_ab(M, N, K, alpha, beta, dtype):

    A = te.placeholder((M, K), name="A", dtype=dtype)
    B = te.placeholder((K, N), name="B", dtype=dtype)
    C = te.placeholder((M, N), name="C", dtype=dtype)

    k = te.reduce_axis((0, K), name="k")

    matmul = te.compute(
        (M, N),
        lambda i, j: te.sum(A[i, k] * B[k, j], axis=k),
        name="matmul",
        attrs={"layout_free_placeholders" : [B]},)

    alpha_matmul = te.compute(
        (M, N),
        lambda i, j: alpha * matmul[i, j],
        name="alpha_matmul",
        attrs={"layout_free_placeholders" : []},
    )

    beta_c = te.compute(
        (M, N),
        lambda i, j: beta * C[i, j],
        name="beta_c",
        attrs={"layout_free_placeholders" : []},
    )

    out = te.compute((M, N), lambda i, j: alpha_matmul[i, j] + beta_c[i, j],
                     name="out")

    return [A, B, C, out]

@auto_scheduler.register_workload
def gemm_with_ab_colmajor(M, N, K, alpha, beta, dtype):

    A = te.placeholder((K, M), name="A", dtype=dtype)
    B = te.placeholder((N, K), name="B", dtype=dtype)
    C = te.placeholder((N, M), name="C", dtype=dtype)

    k = te.reduce_axis((0, K), name="k")

    matmul = te.compute(
        (N, M),
        lambda j, i: te.sum(A[k, i] * B[j, k], axis=k),
        name="matmul",
        attrs={"layout_free_placeholders" : [B]},)

    alpha_matmul = te.compute(
        (N, M),
        lambda j, i: alpha * matmul[j, i],
        name="alpha_matmul",
        attrs={"layout_free_placeholders" : []},
    )

    beta_c = te.compute(
        (N, M),
        lambda j, i: beta * C[j, i],
        name="beta_c",
        attrs={"layout_free_placeholders" : []},
    )

    out = te.compute((N, M), lambda j, i: alpha_matmul[j, i] + beta_c[j, i],
                     name="out")

    return [A, B, C, out]

alpha = 3.14
beta = 1.732
target = tvm.target.Target("llvm -mcpu=skylake-avx512")

def tune(M, N, K, dtype, epoch, logfile):

    task = tvm.auto_scheduler.SearchTask(
        func=gemm_with_ab_colmajor,
        args=(M, N, K, alpha, beta, dtype),
        target=target)
    
    runner = tvm.auto_scheduler.LocalRunner(timeout=100)

    print(task.compute_dag)

    tune_option = auto_scheduler.TuningOptions(
        num_measure_trials=epoch,
        measure_callbacks=[auto_scheduler.RecordToFile(logfile)],
        verbose=2,
        runner=runner
    )

    task.tune(tune_option)
    print("Tune done")

def getfunc(M, N, K, dtype, logfile):
    task = tvm.auto_scheduler.SearchTask(
        func=gemm_with_ab_colmajor,
        args=(M, N, K, alpha, beta, dtype),
        target=target)

    sch, args = task.apply_best(logfile)
    tirfile = os.path.splitext(logfile)[0] + ".tir"
    print(logfile + " " + tirfile)
    with open(tirfile, 'w') as f:
        f.write(str(tvm.lower(sch, args, simple_mode=True)))
    from tvm import transform
    with transform.PassContext(opt_level=3):
        func = tvm.build(sch, args, target=target, name="gemm_with_ab_colmajor")
        llfile = os.path.splitext(logfile)[0] + ".ll"
        asmfile = os.path.splitext(logfile)[0] + ".s"
        with open(llfile, 'w') as f:
            f.write(str(func.get_source("ll")))
        with open(asmfile, 'w') as f:
            f.write(str(func.get_source("asm")))
    print("Build done")
    return func

def check(func, M, N, K, shape_type, its):
    anp = np.random.rand(K, M).astype(np.float64)
    bnp = np.random.rand(N, K).astype(np.float64)
    cinit = np.random.rand(N, M).astype(np.float64)
    cnp = cinit.copy()
    cnp = alpha * bnp.dot(anp) + beta * cnp

    dev = tvm.cpu()
    atvm = tvm.nd.array(anp, device=dev)
    btvm = tvm.nd.array(bnp, device=dev)
    ctvm = tvm.nd.array(cinit, device=dev)

    func(atvm, btvm, ctvm, ctvm)
    np.testing.assert_allclose(cnp, ctvm.numpy(), rtol=1e-3)
    print("Check done.")
    evaluator = func.time_evaluator(func.entry_name, dev, number=its)
    latency = evaluator(atvm, btvm, ctvm, ctvm).mean
    tn = os.environ['TVM_NUM_THREADS']
    perflog = 'gemmperf_{}_{}.log'.format(tn, shape_type)
    with open(perflog, 'a') as f:
        f.write('M=%d, N=%d, K=%d, time: %f ms, GFLOPS: %f\n' % (M, N, K, latency * 1000, 2 * M * N * K / latency / 1e9))
    print('M=%d, N=%d, K=%d, time: %f ms, GFLOPS: %f' % (M, N, K, latency * 1000, 2 * M * N * K / latency / 1e9))

import sys

def main():
    if (len(sys.argv) < 8):
        print(f"Usage: {sys.argv[0]} M N K logfile epoch tuneflag type iters")
        exit(1)
    M = int(sys.argv[1])
    N = int(sys.argv[2])
    K = int(sys.argv[3])
    logfile = sys.argv[4]
    epoch = int(sys.argv[5])
    tuneflag = sys.argv[6]
    shape_type = sys.argv[7]
    its = int(sys.argv[8]) if len(sys.argv) >= 9 else 10
    print("tuneflag: {}, iters: {}".format(tuneflag, its))
    if tuneflag == "Y":
        print(f"Start tune epoch = {epoch}.")
        tune(M, N, K, "float64", epoch, logfile)
    
    func = getfunc(M, N, K, "float64", logfile)
    check(func, M, N, K, shape_type, its)

if __name__ == "__main__":
    main()
