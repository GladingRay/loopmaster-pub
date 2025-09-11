import sys
import re

def to_inc(file : str):
    return "#include \"{}\"".format(file)

def to_func(file : str):
    return file.split('.')[0] + "_func"

def tab():
    return "    "

def to_bench(file : str):
    return "BENCH_SCHEME_IF(,\n{}{},\n{}\n)".format(tab(), to_func(file), tab())


def print_benchinc():
    lines = sys.stdin.read()
    line_list = lines.rstrip().split('\n')
    for line in line_list:
        print(to_inc(line))
    for line in line_list:
        print(to_bench(line))

if __name__ == "__main__":
    print_benchinc()