def get_shape_list():
    ret = []
    for m in ['m', 'mb']:
        for n in ['n', 'nb']:
            for k in ['k', 'kb']:
                if not (len(m) == 1 and len(n) == 1 and len(k) == 1):
                    ret.append(m+'_'+n+'_'+k)
    return ret

def convert_shape(shape_type : str):
    shape = shape_type.split('_')
    ret = ''
    for size in shape:
        if len(size) == 2:
            ret += size[0].upper()
        elif len(size) == 1:
            ret += size
    return ret

def travel_space(do):
    arch_list = {"sw6a":16, "sw8a":64, "skx":18}
    shape_list = get_shape_list()
    for arch, max_tn in arch_list.items():
        for shape in shape_list:
            for tn in [1, max_tn]:
                do(arch, shape, tn)
                print("done {} {} {}".format(arch, shape, tn))
