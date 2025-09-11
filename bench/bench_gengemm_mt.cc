/**
 * Bench generated multi-threaded gemm performance
 */
#include "../test/test_util.hpp"

#ifdef LM_SW6A
#include "./sw6a_mt_inc.h"
#elif defined(LM_SW8A)
#include "./sw8a_mt_inc.h"
#elif defined(LM_SKX)
#include "./skx_mt_inc.h"
#endif

namespace loopmaster
{
namespace bench
{
    using namespace unittest;
    using namespace gencode;

    void benchGenGemm(
        int m, int n, int k, int warms, int iters)
    {
        int lda = m;
        int ldb = k;
        int ldc = m;
        double ops = 2.0 * m * n * k;

        double *a
            = static_cast<double *>(aligned_alloc(128, m * k * sizeof(double)));
        initRandomMatrix(m, k, a, lda);

        double *b
            = static_cast<double *>(aligned_alloc(128, n * k * sizeof(double)));
        initRandomMatrix(k, n, b, ldb);

        double *c
            = static_cast<double *>(aligned_alloc(128, m * n * sizeof(double)));
        initRandomMatrix(m, n, c, ldc);

        double *c_ref
            = static_cast<double *>(aligned_alloc(128, m * n * sizeof(double)));
        copyMatrix(m, n, c_ref, ldc, c, ldc);

        double alpha = 3.141;
        double beta = 1.732;

        constexpr int MaxSize = 8192;
        constexpr int TestSize = 128;

#ifdef LM_SW6A

#elif defined(LM_SW8A)

#elif defined(LM_SKX)
//===----------------------------------------------------------------------===//
// Square
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == n && m == k && m == 1024,
            ParallelScheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 128, 2, 12, 512, 384, 9, 1, 2
        ) // 1024 1024 1024
        BENCH_SCHEME_IF(m == n && m == k && m == 2048,
            ParallelScheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 128, 2, 12, 2048, 384, 18, 1, 1
        ) // 2048
        BENCH_SCHEME_IF(m == n && m == k && m == 3072,
            ParallelScheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, MaxSize, 384, 18, 1, 1
        ) // 3072
        BENCH_SCHEME_IF(m == n && m == k && m == 4096,
            ParallelScheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 256, 2, 12, MaxSize, 384, 18, 1, 1
        ) // 4096
        BENCH_SCHEME_IF(m == n && m == k && m == 5120,
            ParallelScheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 144, 2, 12, MaxSize, 384, 18, 1, 1
        ) // 5120
        BENCH_SCHEME_IF(m == n && m == k && m == 6144,
            ParallelScheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, MaxSize, 384, 18, 1, 1
        ) // 6144
        BENCH_SCHEME_IF(m == n && m == k && m == 7168,
            ParallelScheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 160, 2, 12, MaxSize, 384, 18, 1, 1
        ) // 7168
        BENCH_SCHEME_IF(m == n && m == k && m == 8192,
            ParallelScheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 160, 2, 12, MaxSize, 384, 18, 1, 1
        ) // 8192
//===----------------------------------------------------------------------===//
// mNK
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(n == k && n == MaxSize && m >= 16 && m <= 32,
            ParallelScheme_NN_1681928192_kernel3_NA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
            16, TestSize, 2, 12, 120, 384, 18, 1, 1
        )
        BENCH_SCHEME_IF(n == k && n == MaxSize && m >= 48 && m <= 128,
            ParallelScheme_NN_1681928192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1K2M2_func,
            16, TestSize, 2, 12, 120, 384, MaxSize, 18, 1, 1, 1
        )
//===----------------------------------------------------------------------===//
// MnK
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == k && m == MaxSize && n >= 16 && n <= 128,
            ParallelScheme_NN_8192168192_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1K2N3_func,
            16, 160, 2, 12, TestSize, 384, MaxSize, 18, 1, 1, 1
        )
//===----------------------------------------------------------------------===//
// MNk
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == n && m == MaxSize && k >= 16 && k <= 48,
            ParallelScheme_NN_5_kernel3_NA_NB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 32, 2, 12, 256, 128, 18, 1, 1
        )
        BENCH_SCHEME_IF(m == n && m == MaxSize && k >= 64 && k <= 80,
            ParallelScheme_NN_21_kernel3_NA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 32, 2, 12, 256, 128, 18, 1, 1
        )
        BENCH_SCHEME_IF(m == n && m == MaxSize && k >= 96 && k <= 128,
            ParallelScheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 32, 2, 12, 256, 128, 18, 1, 1
        )
//===----------------------------------------------------------------------===//
// mnK
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == n && k == MaxSize && m >= 16 && m <= 128,
            ReductionScheme_NN_16168192_kernel3_NA_NB_NC_M0N0K0N1M1N2M2N3K1_func,
            16, TestSize, 2, 12, TestSize, 256, 18
        )
//===----------------------------------------------------------------------===//
// mNk
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == k && n == MaxSize && m >= 16 && m <= 32,
            ParallelScheme_NN_49_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, TestSize, 2, 12, 256, TestSize, 1, 1, 18
        )
        BENCH_SCHEME_IF(m == k && n == MaxSize && m >= 48 && m <= 128,
            ParallelScheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, TestSize, 2, 12, 256, TestSize, 1, 1, 18
        )
//===----------------------------------------------------------------------===//
// Mnk
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(n == k && m == MaxSize && n >= 64 && n <= 128,
            ParallelScheme_NN_81921616_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
            16, 256, 2, 12, TestSize, TestSize, 1, 1, 18
        )
        BENCH_SCHEME_IF(n == k && m == MaxSize && n >= 16 && n <= 48,
            ParallelScheme_NN_1_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 256, 2, 12, TestSize, TestSize, 18, 1, 1
        )
#endif
        free(a);
        free(b);
        free(c);
        free(c_ref);
    }
}
}

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        printf("Usage: %s m n k warms iters\n"
               "Example: %s 2048 2048 2048 2 5\n", argv[0], argv[0]);
        exit(1);
    }

    int m = std::atoi(argv[1]);
    int n = std::atoi(argv[2]);
    int k = std::atoi(argv[3]);
    int warms = std::atoi(argv[4]);
    int iters = std::atoi(argv[5]);
    loopmaster::bench::benchGenGemm(m, n, k, warms, iters);
    return 0;
}
