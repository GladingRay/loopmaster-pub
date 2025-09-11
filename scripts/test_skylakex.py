import argparse
import subprocess

class TestArgs:
    def __init__(self, m, n, k, tn=1):
        self.m = m
        self.n = n
        self.k = k
        self.tn = tn
        pass

def run(arg: TestArgs):
    subprocess.run("cd ../tuner && python3 ./tune_flow.py skylakex 3 ../gencode/ {} {}\
                   {} --timeout 240 --thread_num {} --search_type 1".format(
                       arg.m, arg.n, arg.k, arg.tn), shell=True, check=True)
def run_list(args):
    for arg in args:
        run(arg)

def prepare_square_cases_single():
    ret = []
    ret.append(TestArgs(8192, 8192, 8192))
    ret.append(TestArgs(4096, 4096, 4096))
    ret.append(TestArgs(64, 64, 64))
    ret.append(TestArgs(128, 128, 128))
    ret.append(TestArgs(256, 256, 256))
    
    return ret

def prepare_small_m_cases_single():
    ret = []

    for size in range(1024, 4097, 1024):
        ret.append(TestArgs(128, size, size))
    return ret

def prepare_small_n_cases_single():
    ret = []
    for size in range(1024, 4097, 1024):
        ret.append(TestArgs(size, 128, size))
    return ret

def prepare_small_k_cases_single():
    ret = []
    for size in range(1024, 4097, 1024):
        ret.append(TestArgs(size, size, 128))
    return ret

def prepare_paper_data():
    ret = []
    # ret.append(TestArgs(128, 1024, 1024))
    # ret.append(TestArgs(1024, 128, 1024))
    ret.append(TestArgs(1024, 1024, 128))
    ret.append(TestArgs(1024, 128, 128))
    ret.append(TestArgs(128, 1024, 128))
    ret.append(TestArgs(128, 128, 1024))
    return ret

def main():

    # run_list(prepare_square_cases_single())

    # run_list(prepare_small_m_cases_single())

    # run_list(prepare_small_n_cases_single())

    # run_list(prepare_small_k_cases_single())

    run_list(prepare_paper_data())

if __name__ == '__main__':
    main()