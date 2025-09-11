/**
 * Bench generated single-threaded gemm performance
 */
#include "../test/test_util.hpp"

#ifdef LM_SW6A
#include "./sw6a_inc.h"
#elif defined(LM_SW8A)
#include "./sw8a_inc.h"
#elif defined(LM_SKX)
#include "./skx_inc.h"
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
        BENCH_SCHEME_IF(m == n && m == k && m == 64,
            Scheme_NN_49_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 64, 2, 12, 64, 64
        )
        BENCH_SCHEME_IF(m == n && m == k && m >= 128 && m < 8192,
            Scheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, 8640, 384
        )
        BENCH_SCHEME_IF(m == n && m == k && m == 8192,
            Scheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, 8640, 384
        )
//===----------------------------------------------------------------------===//
// mNK
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(n == k && n == MaxSize && m == 16,
            Scheme_NN_1_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, TestSize, 2, 12, 12, 1024
        )
        BENCH_SCHEME_IF(n == k && n == MaxSize && m >= 32 && m <= 64,
            Scheme_NN_3281928192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
            16, TestSize, 2, 12, 12, 1024
        )
        BENCH_SCHEME_IF(n == k && n == MaxSize && m >= 80 && m <= 128,
            Scheme_NN_3281928192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
            16, TestSize, 2, 12, 12, 384
        )
//===----------------------------------------------------------------------===//
// MnK
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == k && m == MaxSize && n >= 16 && n <= 128,
            Scheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 16, 2, 12, TestSize, 384
        )
//===----------------------------------------------------------------------===//
// MNk
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == n && m == MaxSize && k >= 16 && k <= 128,
            Scheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 512, 2, 12, MaxSize, TestSize
        )
//===----------------------------------------------------------------------===//
// mnK
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == n && k == MaxSize && m >= 16 && m <= 48,
            Scheme_NN_1_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 16, 2, 12, TestSize, 512
        )
        BENCH_SCHEME_IF(m == n && k == MaxSize && m >= 64 && m <= 128,
            Scheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 16, 2, 12, TestSize, 384
        )
//===----------------------------------------------------------------------===//
// mNk
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(m == k && n == MaxSize && m == 16,
            Scheme_NN_1_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 16, 2, 12, MaxSize, TestSize
        )
        BENCH_SCHEME_IF(m == k && n == MaxSize && m >= 32 && m <= 48,
            Scheme_NN_49_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, TestSize, 2, 12, MaxSize, TestSize
        )
        BENCH_SCHEME_IF(m == k && n == MaxSize && m >= 64 && m <= 128,
            Scheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, TestSize, 2, 12, MaxSize, TestSize
        )
//===----------------------------------------------------------------------===//
// Mnk
//===----------------------------------------------------------------------===//
        BENCH_SCHEME_IF(n == k && m == MaxSize && n >= 16 && n <= 128,
            Scheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, TestSize, TestSize
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
