"""
Test util
"""
import os
class TestArgs:
    def __init__(self, m, n, k, tn=1):
        self.m = m
        self.n = n
        self.k = k
        self.tn = tn
        pass
    def rep_iter(self):
        ret = 10
        count = 0
        if self.m >= 8192:
            count += 1
        if self.n >= 8192:
            count += 1
        if self.k >= 8192:
            count += 1
        if self.tn == 1 and count == 1:
            ret = 100
        if self.tn >= 16 and count == 1:
            ret = 1000
        return ret

def check_and_create_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def get_singleMNK(arch, tn=1):
    ret = []
    size = [64, 128, 256, 512, 1024, 2048, 4096, 8192]
    for m in size:
        ret.append(TestArgs(m, m, m))
    return ret, "../docs/{}/{}_mb_nb_kb_1.log".format(arch, arch)

def get_multiMNK(arch, tn):
    ret = []
    for size in range(1024, 8193, 1024):
        ret.append(TestArgs(size, size, size, tn))
    return ret, "../docs/{}/{}_mb_nb_kb_{}.log".format(arch, arch, tn)

def get_MNK(arch, tn):
    if tn == 1:
        return get_singleMNK(arch, tn)
    else:
        return get_multiMNK(arch, tn)

def get_mNK(arch, tn, bigdim=8192):
    ret = []
    for m in range(16, 129, 16):
        ret.append(TestArgs(m, bigdim, bigdim, tn))
    return ret, "../docs/{}/{}_m_nb_kb_{}.log".format(arch, arch, tn)

def get_MnK(arch, tn, bigdim=8192):
    ret = []
    for n in range(16, 129, 16):
        ret.append(TestArgs(bigdim, n, bigdim, tn))
    return ret, "../docs/{}/{}_mb_n_kb_{}.log".format(arch, arch, tn)

def get_MNk(arch, tn, bigdim=8192):
    ret = []
    for k in range(16, 129, 16):
        ret.append(TestArgs(bigdim, bigdim, k, tn))
    return ret, "../docs/{}/{}_mb_nb_k_{}.log".format(arch, arch, tn)

def get_mnK(arch, tn, bigdim=8192):
    ret = []
    for mn in range(16, 129, 16):
        ret.append(TestArgs(mn, mn, bigdim, tn))
    return ret, "../docs/{}/{}_m_n_kb_{}.log".format(arch, arch, tn)

def get_Mnk(arch, tn, bigdim=8192):
    ret = []
    for nk in range(16, 129, 16):
        ret.append(TestArgs(bigdim, nk, nk, tn))
    return ret, "../docs/{}/{}_mb_n_k_{}.log".format(arch, arch, tn)

def get_mNk(arch, tn, bigdim=8192):
    ret = []
    for mk in range(16, 129, 16):
        ret.append(TestArgs(mk, bigdim, mk, tn))
    return ret, "../docs/{}/{}_m_nb_k_{}.log".format(arch, arch, tn)

def benchargs(f, arch, tn, run):
    args, logname = f(arch, tn)
    for arg in args:
        run(arg, logname)

def benchall(arch, tn, run):
    benchargs(get_MNK, arch, tn, run)
    benchargs(get_mNK, arch, tn, run)
    benchargs(get_MNk, arch, tn, run)
    benchargs(get_MnK, arch, tn, run)
    benchargs(get_mnK, arch, tn, run)
    benchargs(get_mNk, arch, tn, run)
    benchargs(get_Mnk, arch, tn, run)

def benchsome(shape_funcs, arch, tn, run):
    for shape_func in shape_funcs:
        benchargs(shape_func, arch, tn, run)