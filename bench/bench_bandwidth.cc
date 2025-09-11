#include "runtime.h"
#include "test_util.hpp"
#include <cassert>

namespace loopmaster
{
namespace bench
{
    constexpr int L1SizeInDouble = CacheInfo::L1SizeInBytes / sizeof(double);
    constexpr int L2SizeInDouble = CacheInfo::L2SizeInBytes / sizeof(double);
    constexpr int L3SizeInDouble = CacheInfo::L3SizeInBytes / sizeof(double);

    // clang-format off
    void vcopyAsm16(double *dst, double *src) {
        double *src1 = src + 16;
        double *src2 = src1 + 16;
        double *src3 = src2 + 16;
        double *dst1 = dst + 16;
        double *dst2 = dst1 + 16;
        double *dst3 = dst2 + 16;
        #if defined(LM_SW6A) || defined(LM_SW8A)

        #elif defined(LM_SKX)
        __asm__(
            "vmovupd    0(%[src]), %%zmm0;\n\t"
            "vmovupd    64(%[src]), %%zmm1;\n\t"
            "vmovupd    0(%[src1]), %%zmm2;\n\t"
            "vmovupd    64(%[src1]), %%zmm3;\n\t"
            "vmovupd    0(%[src2]), %%zmm4;\n\t"
            "vmovupd    64(%[src2]), %%zmm5;\n\t"
            "vmovupd    0(%[src3]), %%zmm6;\n\t"
            "vmovupd    64(%[src3]), %%zmm7;\n\t"

            "vmovupd    %%zmm0, 0(%[dst]);\n\t"
            "vmovupd    %%zmm1, 64(%[dst]);\n\t"
            "vmovupd    %%zmm2, 0(%[dst1]);\n\t"
            "vmovupd    %%zmm3, 64(%[dst1]);\n\t"
            "vmovupd    %%zmm4, 0(%[dst2]);\n\t"
            "vmovupd    %%zmm5, 64(%[dst2]);\n\t"
            "vmovupd    %%zmm6, 0(%[dst3]);\n\t"
            "vmovupd    %%zmm7, 64(%[dst3]);\n\t"
            :
            :
            [src] "r"(src), [src1] "r"(src1), [src2] "r"(src2),
            [src3] "r"(src3), [dst] "r"(dst), [dst1] "r"(dst1),
            [dst2] "r"(dst2), [dst3] "r"(dst3)
            :
            "memory", "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6",
            "zmm7"
        );
        #endif
    }

    void vloadAsm32(double *src) {
        double *src1 = src + 16;
        double *src2 = src1 + 16;
        double *src3 = src2 + 16;
        double *src4 = src3 + 16;
        double *src5 = src4 + 16;
        double *src6 = src5 + 16;
        double *src7 = src6 + 16;
        #if defined(LM_SW6A) || defined(LM_SW8A)

        #elif defined(LM_SKX)
        __asm__(
            "vmovupd    0(%[src]), %%zmm0;\n\t"
            "vmovupd    64(%[src]), %%zmm1;\n\t"
            "vmovupd    0(%[src1]), %%zmm2;\n\t"
            "vmovupd    64(%[src1]), %%zmm3;\n\t"
            "vmovupd    0(%[src2]), %%zmm4;\n\t"
            "vmovupd    64(%[src]), %%zmm5;\n\t"
            "vmovupd    0(%[src3]), %%zmm6;\n\t"
            "vmovupd    64(%[src3]), %%zmm7;\n\t"
            "vmovupd    0(%[src4]), %%zmm8;\n\t"
            "vmovupd    64(%[src4]), %%zmm9;\n\t"
            "vmovupd    0(%[src5]), %%zmm10;\n\t"
            "vmovupd    64(%[src5]), %%zmm11;\n\t"
            "vmovupd    0(%[src6]), %%zmm12;\n\t"
            "vmovupd    64(%[src6]), %%zmm13;\n\t"
            "vmovupd    0(%[src7]), %%zmm14;\n\t"
            "vmovupd    64(%[src7]), %%zmm15;\n\t"
            :
            :
            [src] "r"(src), [src1] "r"(src1), [src2] "r"(src2),
            [src3] "r"(src3), [src4] "r"(src4), [src5] "r"(src5),
            [src6] "r"(src6), [src7] "r"(src7)
            :
            "memory", "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6",
            "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14",
            "zmm15"
        );
        #endif
    }
    // clang-format on

    template <typename T>
    void vectorCopyData(T *dst, T *src, int n)
    {
        T *dst_ptr = dst;
        T *src_ptr = src;
        constexpr int step = 4 * 16;
        for (int i = 0; i < n; i += step)
        {
            vcopyAsm16(dst_ptr, src_ptr);
            dst_ptr += step;
            src_ptr += step;
        }
    }

    template <typename T>
    void scalarCopyData(T *dst, T *src, int n)
    {
        // T *dst_ptr = dst;
        // T *src_ptr = src;
        constexpr int step = 8;
        for (int i = 0; i < n; i += step)
        {
            dst[i + 0] = src[i + 0];
            dst[i + 1] = src[i + 1];
            dst[i + 2] = src[i + 2];
            dst[i + 3] = src[i + 3];
            dst[i + 4] = src[i + 4];
            dst[i + 5] = src[i + 5];
            dst[i + 6] = src[i + 6];
            dst[i + 7] = src[i + 7];
            // dst_ptr += step;
            // src_ptr += step;
        }
    }

    template <typename T>
    void vectorLoadData(T *data, int n)
    {
        constexpr int step = 8 * 16;
        for (int i = 0; i < n; i += step)
        {
            vloadAsm32(data);
            data += step;
        }
    }

    template <typename T>
    void scalarLoadData(T *data, int n)
    {
        volatile register double dst;
        constexpr int step = 4;
        for (int i = 0; i < n; i += step)
        {
            dst = data[i + 0];
            dst = data[i + 1];
            dst = data[i + 2];
            dst = data[i + 3];
        }
    }

    void benchVCopyBandwidth(
        int num_in_double, int iter, const std::string &info)
    {
        double *src = static_cast<double *>(
            aligned_alloc(128, num_in_double * sizeof(double)));
        double *dst = static_cast<double *>(
            aligned_alloc(128, num_in_double * sizeof(double)));

        for (int i = 0; i < num_in_double; i++)
        {
            src[i] = 3.141;
            dst[i] = 1.732;
        }

        double ms = unittest::repeatAndTiming(
            iter, vectorCopyData<double>, dst, src, num_in_double);
        size_t access_size_kb
            = (double)num_in_double * iter * sizeof(double) * 2 / 1024;
        double bw = access_size_kb / ms / 1e3; // GB/s
        printf("[%s] access size = %ld KB, time = %.2f ms, "
               "bandwidth = %.2f GB/s\n",
            info.c_str(), access_size_kb / iter, ms, bw);
        free(src);
        free(dst);
    }

    void benchSCopyBandwidth(
        int num_in_double, int iter, const std::string &info)
    {
        double *src = static_cast<double *>(
            aligned_alloc(128, num_in_double * sizeof(double)));
        double *dst = static_cast<double *>(
            aligned_alloc(128, num_in_double * sizeof(double)));

        for (int i = 0; i < num_in_double; i++)
        {
            src[i] = 3.141;
            dst[i] = 1.732;
        }

        double ms = unittest::repeatAndTiming(
            iter, scalarCopyData<double>, dst, src, num_in_double);
        size_t access_size_kb
            = (double)num_in_double * iter * sizeof(double) * 2 / 1024;
        double bw = access_size_kb / ms / 1e3; // GB/s
        printf("[%s] access size = %ld KB, time = %.2f ms, "
               "bandwidth = %.2f GB/s\n",
            info.c_str(), access_size_kb / iter, ms, bw);
        free(src);
        free(dst);
    }

    void benchVLoadBandwidth(
        int num_in_double, int iter, const std::string &info)
    {
        double *data = static_cast<double *>(
            aligned_alloc(128, num_in_double * sizeof(double)));

        for (int i = 0; i < num_in_double; i++)
        {
            data[i] = 3.141;
        }

        double ms = unittest::repeatAndTiming(
            iter, vectorLoadData<double>, data, num_in_double);
        size_t access_size_kb = (double)num_in_double * iter * sizeof(double) / 1024;
        double bw = access_size_kb / ms / 1e3; // GB/s
        printf("[%s] access size = %ld KB, time = %.2f ms, "
            "bandwidth = %.2f GB/s\n",
         info.c_str(), access_size_kb / iter, ms, bw);
        free(data);
    }

    void benchSLoadBandwidth(
        int num_in_double, int iter, const std::string &info)
    {
        double *data = static_cast<double *>(
            aligned_alloc(128, num_in_double * sizeof(double)));

        for (int i = 0; i < num_in_double; i++)
        {
            data[i] = 3.141;
        }

        double ms = unittest::repeatAndTiming(
            iter, scalarLoadData<double>, data, num_in_double);
        size_t access_size_kb = (double)num_in_double * iter * sizeof(double) / 1024;
        double bw = access_size_kb / ms / 1e3; // GB/s
        printf("[%s] access size = %ld KB, time = %.2f ms, "
            "bandwidth = %.2f GB/s\n",
         info.c_str(), access_size_kb / iter, ms, bw);
        free(data);
    }

    void benchVCopyBandwidth()
    {
        benchVCopyBandwidth(L1SizeInDouble / 4, 100000, "Vector Copy L1 Cache / 4");

        benchVCopyBandwidth(L2SizeInDouble / 8, 100000, "Vector Copy L2 Cache / 8");

        benchVCopyBandwidth(L2SizeInDouble / 4, 100000, "Vector Copy L2 Cache / 4");

        benchVCopyBandwidth(L3SizeInDouble / 4, 1000, "Vector Copy L3 Cache / 4");

        benchVCopyBandwidth(L3SizeInDouble * 8, 100, "Vector Copy Memory");
    }

    void benchSCopyBandwidth()
    {
        benchSCopyBandwidth(L1SizeInDouble / 4, 100000, "Scalar Copy L1 Cache / 4");

        benchSCopyBandwidth(L2SizeInDouble / 8, 100000, "Scalar Copy L2 Cache / 8");

        benchSCopyBandwidth(L2SizeInDouble / 4, 100000, "Scalar Copy L2 Cache / 4");

        benchSCopyBandwidth(L3SizeInDouble / 4, 1000, "Scalar Copy L3 Cache / 4");

        benchSCopyBandwidth(L3SizeInDouble * 4, 100, "Scalar Copy Memory");
    }

    void benchVLoadBandwidth()
    {
        benchVLoadBandwidth(L1SizeInDouble / 2, 100000, "Vector Load L1 Cache / 2");

        benchVLoadBandwidth(L2SizeInDouble / 4, 100000, "Vector Load L2 Cache / 4");

        benchVLoadBandwidth(L2SizeInDouble / 2, 100000, "Vector Load L2 Cache / 2");

        benchVLoadBandwidth(L3SizeInDouble / 2, 1000, "Vector Load L3 Cache / 2");

        benchVLoadBandwidth(L3SizeInDouble * 4, 100, "Vector Load Memory");
    }

    void guessCacheSizeByVLoad()
    {
        constexpr int ElemNum = L1SizeInDouble / 2;
        constexpr int End = L3SizeInDouble * 2;
        for (int elemnum = ElemNum; elemnum < End; elemnum += ElemNum)
            benchVLoadBandwidth(elemnum, ElemNum * 10000 / elemnum, "...");
    }

    void benchSLoadBandwidth()
    {
        benchSLoadBandwidth(L1SizeInDouble / 2, 100000, "Scalar Load L1 Cache / 2");

        benchSLoadBandwidth(L2SizeInDouble / 4, 100000, "Scalar Load L2 Cache / 4");

        benchSLoadBandwidth(L2SizeInDouble / 2, 100000, "Scalar Load L2 Cache / 2");

        benchSLoadBandwidth(L3SizeInDouble / 2, 1000, "Scalar Load L3 Cache / 2");

        benchSLoadBandwidth(L3SizeInDouble * 4, 100, "Scalar Load Memory");
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::bench::benchVCopyBandwidth();
    loopmaster::bench::benchVLoadBandwidth();
    return 0;
}
