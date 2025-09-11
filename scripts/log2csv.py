import os
import pandas as pd
from paint_util import *
def parse_line(line : str):
    line = line.split(">,")[-1]
    str_spilt = line.split(',')
    shape = []
    gflops = 0.0
    for s in str_spilt:
        s_split = s.split(':')
        key = s_split[0]
        val = s_split[1]
        if key == 'm' or key == 'n' or key == 'k':
            shape.append(val)
        elif key == "gflops":
            gflops = float(val)
    shape = '*'.join(shape)
    return shape, gflops

def parse_tvm_line(line : str):
    str_split = line.split(", ")
    shape = []
    gflops = 0.0
    for s in str_split:
        seq = '='
        if ':' in s:
            seq = ": "
        s_split = s.split(seq)
        key = s_split[0]
        val = s_split[1]
        if key == 'M' or key == 'N' or key == 'K':
            shape.append(val)
        elif key == "GFLOPS":
            gflops = float(val)
    shape = 'x'.join(shape)
    return shape, gflops

def parse_data(filename, perf_name, parse_func):
    ret = {
        "shape" : [],
        perf_name : []
    }
    with open(filename, "r") as f:
        lines = f.readlines()
        for line in lines:
            shape, gflops = parse_func(line.rstrip())
            if shape in ret["shape"]:
                ret[perf_name][ret["shape"].index(shape)] = gflops
            else:
                ret["shape"].append(shape)
                ret[perf_name].append(gflops)
    return ret

def get_tvm_shape_type(shape_type: str):
    ret = []
    for ch in shape_type:
        if ch.isupper():
            ret.append(ch.lower()+'b')
        else:
            ret.append(ch)
    return '_'.join(ret)

def get_blas_name(arch):
    if "sw" in arch:
        return "swblas"
    if "skx" in arch:
        return "OpenBLAS"

def get_parse_func(file):
    if "tvm" in file:
        return parse_tvm_line
    else:
        return parse_line

def get_csv(arch, shape_type, tn):
    gen_file = "../docs/{}gen/{}gen_{}_{}.log".format(arch, arch, shape_type, tn)
    blas_file = "../docs/{}blas/{}blas_{}_{}.log".format(arch, arch, shape_type, tn)
    mkl_file = "../docs/{}mkl/{}mkl_{}_{}.log".format(arch, arch, shape_type, tn)
    tvm_file = "../docs/{}tvm/gemmperf_{}_{}.log".format(arch, tn, shape_type)

    file_list = [gen_file, blas_file, mkl_file, tvm_file]
    name_list = ["gencode", get_blas_name(arch), "mkl", "tvm(auto_sched)"]
    data_list = []

    for i in range(len(file_list)):
        file = file_list[i]
        name = name_list[i]
        if os.path.exists(file) and not (arch == "sw8a" and tn == 1 and "tvm" in file):
            data_list.append(parse_data(file, name, get_parse_func(file)))
    csv_data = {}

    for data in data_list:
        for key, val in data.items():
            if key not in csv_data:
                while len(val) < 8:
                    val.append(0.0)
                csv_data[key] = val

    spdup_data = {}
    for key, val in csv_data.items():
        if key == "shape":
            spdup_data["shape"] = csv_data["shape"]
        else:
            spdup_data[key] = [0.0 if val[i] == 0.0 else csv_data["gencode"][i] / val[i] for i in range(len(val))]

    df = pd.DataFrame(csv_data)
    spdup_df = pd.DataFrame(spdup_data)
    csv_dir = "../docs/csv/{}/".format(arch)
    if not os.path.exists(csv_dir):
        os.mkdir(csv_dir)
    df.to_csv(csv_dir+"{}_{}.csv".format(shape_type, tn))
    spdup_df.to_csv(csv_dir+"{}_{}_spdup.csv".format(shape_type, tn))

def main():
    travel_space(get_csv)

if __name__ == "__main__":
    main()