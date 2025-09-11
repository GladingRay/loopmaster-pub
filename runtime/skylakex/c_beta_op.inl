#include "runtime.h"

template <>
inline void CbetaOp<double>(
    int m, int n, double *c, int ldc, double beta,
    int tid, int tn)
{
    int elem_num = m * n;
    int vec_num = (elem_num + VecLen<double>::value - 1) / VecLen<double>::value;
    int elem_tail = elem_num % VecLen<double>::value;
    int vec_num_pt = vec_num / tn;
    int elem_num_pt = vec_num_pt * VecLen<double>::value;
    int pre_elem_num = tid * elem_num_pt;
    int local_elem_num = (tid == tn - 1 ? elem_num - pre_elem_num : elem_num_pt);

    int local_start = pre_elem_num;
    int local_end = local_start + local_elem_num;
    double *local_c = c + local_start;

    for (int idx = local_start; idx < local_end; idx += VecLen<double>::value)
    {
        __asm__(
            "vbroadcastsd   0(%[pbeta]), %%zmm0;\n\t"
            "vmovupd        0(%[local_c]), %%zmm1;\n\t"
            "vmulpd         %%zmm0, %%zmm1, %%zmm2;\n\t"
            "vmovupd        %%zmm2, 0(%[local_c]);\n\t"
            : // output
            : // input
            [local_c]"r"(local_c),
            [pbeta]"r"(&beta)
            : // clobber
            "memory", "zmm0", "zmm1", "zmm2"
        );
        local_c += VecLen<double>::value;
    }
}