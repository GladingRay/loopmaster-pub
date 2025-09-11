"""
benchmark for blaslib on skx
contains {mM}{nN}{kK} shape x {single multi}
"""
import subprocess
from test_util import *


def runbench(arg : TestArgs, logname):
    subprocess.run("sh ./benchskxblas.sh {} {} {} {} {} >> {}".format(arg.tn, arg.m, arg.n, arg.k, arg.rep_iter(), logname), shell=True, check=True)

if __name__ == '__main__':
    test_name = "skxblas"
    check_and_create_dir("../docs/"+test_name+"/")
    bench_funcs = [get_mnK, get_mNk, get_Mnk]
    benchsome(bench_funcs, test_name, 1, runbench)
    benchsome(bench_funcs, test_name, 18, runbench)