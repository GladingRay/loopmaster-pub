#include "runtime.h"

template <>
inline void VectorCopyAdd<float>(
    float *dst_ptr, const float *src_ptr, const float beta)
{
    __asm__("vmovups    0(%[src_ptr]), %%zmm0;\n\t"
            "vmovups    0(%[dst_ptr]), %%zmm1;\n\t"
            "vbroadcastss   0(%[pbeta]), %%zmm2;\n\t"
            "vfmadd231ps    %%zmm1, %%zmm2, %%zmm0;\n\t"
            "vmovups        %%zmm0, 0(%[dst_ptr]);\n\t"
        : // output
        : // input
        [dst_ptr] "r"(dst_ptr), [src_ptr] "r"(src_ptr),
        [pbeta] "r"(&beta)
        : // clobber
        "memory", "zmm0", "zmm1", "zmm2");
}

template <>
inline void VectorCopyAdd<double>(
    double *dst_ptr, const double *src_ptr, const double beta)
{
    // printf("[VectorCopyAdd] before asm beta: %.2f, %p: %.2f, %p: %.2f\n",
    // beta, dst_ptr, *dst_ptr, src_ptr, *src_ptr);
    __asm__("vmovupd    0(%[src_ptr]), %%zmm0;\n\t"
            "vmovupd    0(%[dst_ptr]), %%zmm1;\n\t"
            "vbroadcastsd   0(%[pbeta]), %%zmm2;\n\t"
            "vfmadd231pd    %%zmm1, %%zmm2, %%zmm0;\n\t"
            "vmovupd        %%zmm0, 0(%[dst_ptr]);\n\t"
        : // output
        : // input
        [dst_ptr] "r"(dst_ptr), [src_ptr] "r"(src_ptr),
        [pbeta] "r"(&beta)
        : // clobber
        "memory", "zmm0", "zmm1", "zmm2");
    // printf("[VectorCopyAdd] after asm beta: %.2f, %p: %.2f, %p: %.2f\n",
    // beta, dst_ptr, *dst_ptr, src_ptr, *src_ptr);
}
