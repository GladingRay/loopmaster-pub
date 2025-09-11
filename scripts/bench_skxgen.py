"""
benchmark for gen gemm on skx
contains {mM}{nN}{kK} shape x {single multi}
"""
import subprocess
from test_util import *


def runbench(arg : TestArgs, logname):
    subprocess.run("sh ./benchskxgen.sh {} {} {} {} >> {}".format(arg.m, arg.n, arg.k, arg.rep_iter(), logname), shell=True, check=True)

def runbenchmt(arg : TestArgs, logname):
    subprocess.run("sh ./benchskxgenmt.sh {} {} {} {} {} >> {}".format(arg.tn, arg.m, arg.n, arg.k, arg.rep_iter(), logname), shell=True, check=True)


if __name__ == '__main__':
    test_name = "skxgen"
    check_and_create_dir("../docs/"+test_name+"/")
    bench_funcs = [get_mnK, get_mNk, get_Mnk, get_mNK, get_MnK, get_MNk, get_MNK]
    benchsome(bench_funcs, test_name, 1, runbench)
    benchsome(bench_funcs, test_name, 18, runbenchmt)
