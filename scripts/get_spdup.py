import pandas as pd
import os
from paint_util import *

def avg_without_zero(val):
    count = 0
    sum = 0.0
    for num in val:
        if num > 0.0:
            sum += num
            count += 1
    return sum / count

def get_spdup(spdup_dir, arch, shapes, tn):
    datas = {}
    spdup_name = "{}/{}_{}.log".format(spdup_dir, arch, tn)
    spdup_file = open(spdup_name, "w")
    datas["type"] = shapes.copy()
    datas["type"].append("{}-avg".format(tn))
    for shape in shapes:
        csv_name = "../docs/csv/{}/{}_{}_spdup.csv".format(arch, shape, tn)
        data = pd.read_csv(csv_name)
        cols = data.columns.to_list()[3:]
        for col in cols:
            avg = avg_without_zero(data[col])
            spdup_file.write("{}_{}_spdup:{:.2f}\n".format(shape, col, avg))
            if col in datas:
                datas[col].append(avg)
            else:
                datas[col] = [avg]
    for key, val in datas.items():
        if key != "type":
            avg = sum(val) / len(val)
            val.append(avg)
            spdup_file.write("{}_avg_spdup:{:.2f}\n".format(key, avg))
    spdup_file.close()
    df = pd.DataFrame(datas)
    df.to_csv("{}/{}_{}.csv".format(spdup_dir, arch, tn))

def get_all_spdup(spdup_dir):
    if not os.path.exists(spdup_dir):
        os.mkdir(spdup_dir)
    arch_tn = {"sw6a":16, "sw8a":64, "skx":18}
    shapes = get_shape_list()
    for arch, max_tn in arch_tn.items():
        for tn in [1, max_tn]:
            get_spdup(spdup_dir, arch, shapes, tn)

def main():
    get_all_spdup("../docs/spdup")

if __name__ == "__main__":
    main()
