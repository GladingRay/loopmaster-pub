#include "runtime.h"

template <>
inline void VectorCopy<float>(float *dst_ptr, const float *src_ptr)
{
    __asm__("vmovups 0(%[src_ptr]), %%zmm0;\n\t"
            "vmovups %%zmm0, 0(%[dst_ptr]);\n\t"
        : // output
        : // input
        [dst_ptr] "r"(dst_ptr),
        [src_ptr] "r"(src_ptr)
        : // clobber
        "memory", "zmm0");
}

template <>
inline void VectorCopy<double>(double *dst_ptr, const double *src_ptr)
{
    __asm__("vmovupd 0(%[src_ptr]), %%zmm0;\n\t"
            "vmovupd %%zmm0, 0(%[dst_ptr]);\n\t"
        : // output
        : // input
        [dst_ptr] "r"(dst_ptr),
        [src_ptr] "r"(src_ptr)
        : // clobber
        "memory", "zmm0");
}
