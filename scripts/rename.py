import os
from paint_util import *

def get_filename(arch_type, shape_type, tn):
    filename = "../docs/{}/{}_{}_{}.log".format(arch_type, arch_type, shape_type, tn)
    return filename

def rename_shape(arch, shape_type, tn):

    gen_old = get_filename(arch+"gen", convert_shape(shape_type), tn)
    blas_old = get_filename(arch+"blas", convert_shape(shape_type), tn)
    mkl_old = get_filename(arch+"mkl", convert_shape(shape_type), tn)
    gen_new = get_filename(arch+"gen", shape_type, tn)
    blas_new = get_filename(arch+"blas", shape_type, tn)
    mkl_new = get_filename(arch+"mkl", shape_type, tn)

    file_map = {gen_old:gen_new, blas_old:blas_new, mkl_old:mkl_new}

    for old, new in file_map.items():
        if os.path.exists(old):
            os.rename(old, new)

def main():
    travel_space(rename_shape)

if __name__ == "__main__":
    main()
