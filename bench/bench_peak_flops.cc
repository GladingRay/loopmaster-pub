#include "runtime.h"
#include "sw_asm_macro.h"
#include "test_util.hpp"

namespace loopmaster
{
namespace bench
{
    // clang-format off
    void vmad16(uint64_t iter)
    {
        #if defined(LM_SW6A) || defined(LM_SW8A)

        #elif defined(LM_SKX)
        __asm__(
            "vxorpd %%zmm0, %%zmm0, %%zmm0;\n\t"
            "vxorpd %%zmm1, %%zmm1, %%zmm1;\n\t"
            "vxorpd %%zmm2, %%zmm2, %%zmm2;\n\t"
            "vxorpd %%zmm3, %%zmm3, %%zmm3;\n\t"
            "vxorpd %%zmm4, %%zmm4, %%zmm4;\n\t"
            "vxorpd %%zmm5, %%zmm5, %%zmm5;\n\t"
            "vxorpd %%zmm6, %%zmm6, %%zmm6;\n\t"
            "vxorpd %%zmm7, %%zmm7, %%zmm7;\n\t"
            "vxorpd %%zmm8, %%zmm8, %%zmm8;\n\t"
            "vxorpd %%zmm9, %%zmm9, %%zmm9;\n\t"
            "vxorpd %%zmm11, %%zmm11, %%zmm11;\n\t"
            "vxorpd %%zmm12, %%zmm12, %%zmm12;\n\t"
            "vxorpd %%zmm13, %%zmm13, %%zmm13;\n\t"
            "vxorpd %%zmm14, %%zmm14, %%zmm14;\n\t"
            "vxorpd %%zmm15, %%zmm15, %%zmm15;\n\t"
            "cmpq           $0, %[iter];\n\t"
            "je             2f;\n\t"
            "1:\n\t"
                "vfmadd231pd    %%zmm0, %%zmm0, %%zmm0;\n\t"
                "vfmadd231pd    %%zmm1, %%zmm1, %%zmm1;\n\t"
                "vfmadd231pd    %%zmm2, %%zmm2, %%zmm2;\n\t"
                "vfmadd231pd    %%zmm3, %%zmm3, %%zmm3;\n\t"
                "vfmadd231pd    %%zmm4, %%zmm4, %%zmm4;\n\t"
                "vfmadd231pd    %%zmm5, %%zmm5, %%zmm5;\n\t"
                "vfmadd231pd    %%zmm6, %%zmm6, %%zmm6;\n\t"
                "vfmadd231pd    %%zmm7, %%zmm7, %%zmm7;\n\t"
                "vfmadd231pd    %%zmm8, %%zmm8, %%zmm8;\n\t"
                "vfmadd231pd    %%zmm9, %%zmm9, %%zmm9;\n\t"
                "vfmadd231pd    %%zmm10, %%zmm10, %%zmm10;\n\t"
                "vfmadd231pd    %%zmm11, %%zmm11, %%zmm11;\n\t"
                "vfmadd231pd    %%zmm12, %%zmm12, %%zmm12;\n\t"
                "vfmadd231pd    %%zmm13, %%zmm13, %%zmm13;\n\t"
                "vfmadd231pd    %%zmm14, %%zmm14, %%zmm14;\n\t"
                "vfmadd231pd    %%zmm15, %%zmm15, %%zmm15;\n\t"
                "subq           $1, %[iter];\n\t"
                "jne            1b;\n\t"
            "2:\n\t"
            :
            :
            [iter] "r"(iter)
            :
            "memory", "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7",
            "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15"
        );
        #endif
    }
    // clang-format on

    void benchPeakFlops()
    {
        constexpr uint64_t iter = 100000000;
        double ms = unittest::repeatAndTiming(1, vmad16, iter);
        double ops = 2.0 * VecLen<double>::value * 16 * iter;
        double gflops = (ops / 1e9) / (ms / 1e3);
        printf("[vmad peak flops] time = %.2f ms, peak flops = %.2f "
               "GFLOPS/s\n",
            ms, gflops);
    }
}
}
int main(int argc, char *argv[])
{
    loopmaster::bench::benchPeakFlops();
    return 0;
}
