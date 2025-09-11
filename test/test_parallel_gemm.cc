#include <chrono>

#if defined(LM_SW6A)

#elif defined(LM_SW8A)

#elif defined(LM_SKX)
#include "ParallelScheme_NN_17_kernel3_NA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"
#include "ParallelScheme_NN_1_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
#include "ParallelScheme_NN_49_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
#include "ParallelScheme_NN_5_kernel3_NA_NB_RC_M0N0K0N1M1N2M2K1N3.hpp"
#include "ParallelScheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"
#include "ParallelScheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3.hpp"

#include "ParallelScheme_NN_255_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3M3.hpp"
#include "ParallelScheme_NN_251_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3M3.hpp"

#include "ParallelScheme_NN_250_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1M3N3.hpp"
#include "ParallelScheme_NN_254_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1M3N3.hpp"
//===----------------------------------------------------------------------===//
// mNK
//===----------------------------------------------------------------------===//
// #include "../build/ParallelScheme_NN_1681928192_kernel3_NA_NB_NC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_1681928192_kernel3_NA_WB_NC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_1681928192_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_1681928192_kernel3_NA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"

// #include "../build/ParallelScheme_NN_1681928192_kernel3_WA_NB_NC_M0N0K0N1M1N2N3K1K2M2.hpp"
// #include "../build/ParallelScheme_NN_1681928192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1K2M2.hpp"
// #include "../build/ParallelScheme_NN_1681928192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2.hpp"
//===----------------------------------------------------------------------===//
// MnK
//===----------------------------------------------------------------------===//
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_NB_RC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"

// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1K2N3.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1K2N3.hpp"
//===----------------------------------------------------------------------===//
// MNk
//===----------------------------------------------------------------------===//
// #include "../build/ParallelScheme_NN_8192168192_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_NA_NB_NC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_NB_NC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_WB_RC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_8192168192_kernel3_WA_NB_RC_M0N0K0N1M1N2N3K1M2.hpp"
//===----------------------------------------------------------------------===//
// mNk
//===----------------------------------------------------------------------===//
// #include "../build/ParallelScheme_NN_16819216_kernel3_NA_NB_NC_M0N0K0N1M1N2K1N3.hpp"
// #include "../build/ParallelScheme_NN_16819216_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_16819216_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_16819216_kernel3_NA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_16819216_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
//===----------------------------------------------------------------------===//
// Mnk
//===----------------------------------------------------------------------===//
// #include "../build/ParallelScheme_NN_81921616_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_81921616_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_81921616_kernel3_NA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"
// #include "../build/ParallelScheme_NN_81921616_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"

// #include "../build/ParallelScheme_NN_81921616_kernel3_NA_NB_NC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_81921616_kernel3_WA_NB_NC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_81921616_kernel3_NA_WB_NC_M0N0K0N1M1N2N3K1M2.hpp"
// #include "../build/ParallelScheme_NN_81921616_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2.hpp"
#endif
#include "test_util.hpp"

#define TEST_PSCHEME_IF(cond_, scheme_, ...)                                   \
    if (cond_)                                                                 \
    {                                                                          \
        printf("============= "#cond_" =============\n");                      \
        printf("Testing " #scheme_ "<double, " #__VA_ARGS__ ">\n");            \
        testAndPerfGemm<scheme_<double, __VA_ARGS__>>(                         \
            m, n, k, alpha, beta, a, lda, b, ldb, uniblasc, c_test, c_init,    \
            ldc, uni_gflops);                                                  \
        printf("---------------------------------------------------------\n"); \
    }

namespace loopmaster
{
namespace unittest
{
    void testParallelGemm(int m, int n, int k)
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

        double *c_init
            = static_cast<double *>(aligned_alloc(128, m * n * sizeof(double)));
        initRandomMatrix(m, n, c_init, ldc);

        double *uniblasc
            = static_cast<double *>(aligned_alloc(128, m * n * sizeof(double)));
        copyMatrix(m, n, uniblasc, ldc, c_init, ldc);
        double *c_test
            = static_cast<double *>(aligned_alloc(128, m * n * sizeof(double)));
        char notrans = 'N';
        double alpha = 3.141;
        double beta = 1.732;
        if (!SkipOpenBLAS)
            repeatAndTiming(WarmUp, dgemm_,
                &notrans, &notrans, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta,
                c_test, &ldc);
        // flushAllCache();
        double uni_gflops = 1.0;
        if (!SkipOpenBLAS)
        {
            double uni_ms = repeatAndTiming(RepIter, dgemm_,
                &notrans, &notrans, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta,
                uniblasc, &ldc);
            uni_gflops = (ops * RepIter / 1e6) / uni_ms;
            printf("BLASLib GEMM %dx%dx%d done in %.4f ms, GFLOPS = %.2f\n", m, n,
                k, uni_ms, uni_gflops);
            printf("-------------------------------------------------------\n");
        }

#if defined(LM_SW6A)

#elif defined(LM_SW8A)

#elif defined(LM_SKX)
        constexpr int TestMNK = 8192;
        // TEST_PSCHEME_IF(m == n && n == k && m == TestMNK,
        //     gencode::
        //         ParallelScheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 192, 2, 12, TestMNK, 384, 18, 1, 1)
        TEST_PSCHEME_IF(m == n && n == k && false,
            gencode::
                ParallelScheme_NN_1_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, TestMNK, 384, 18, 1, 1)
        TEST_PSCHEME_IF(m == n && n == k && false,
            gencode::
                ParallelScheme_NN_17_kernel3_NA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, TestMNK, 384, 18, 1, 1)
        TEST_PSCHEME_IF(m == n && n == k && false,
            gencode::
                ParallelScheme_NN_49_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, TestMNK, 384, 18, 1, 1)
        TEST_PSCHEME_IF(m == n && n == k,
            gencode::
                ParallelScheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
            16, 192, 2, 12, 8192, 384, 18, 1, 1)
        // TEST_PSCHEME_IF(m == n && n == k && m == 8192,
        //     gencode::
        //         ParallelScheme_NN_5_kernel3_NA_NB_RC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 192, 2, 12, 8192, 384, 18, 1, 1)

        // TEST_PSCHEME_IF(m == n && n == k && m == 8192,
        //     gencode::ParallelScheme_NN_251_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3M3_func, 
        //     16, 192, 192, 2, 12, 132, 384, 1,1,1,18)
        
        // TEST_PSCHEME_IF(m == n && n == k && m == 8192,
        //     gencode::ParallelScheme_NN_255_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3M3_func, 
        //     16, 192, 192, 2, 12, 132, 384, 1,1,1,18)

        // TEST_PSCHEME_IF(m == n && n == k && m == 8192,
        //     gencode::ParallelScheme_NN_250_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1M3N3_func, 
        //     16, 192, 192, 2, 12, 132, 384, 1,1,18,1)
        
        // TEST_PSCHEME_IF(m == n && n == k && m == 8192,
        //     gencode::ParallelScheme_NN_254_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1M3N3_func, 
        //     16, 192, 192, 2, 12, 132, 384, 1,1,18,1)
//===----------------------------------------------------------------------===//
// mNK
//===----------------------------------------------------------------------===//
        // constexpr int TestM = 16;
        // TEST_PSCHEME_IF(m == TestM && n == k && n == 8192,
        //     gencode::ParallelScheme_NN_1681928192_kernel3_NA_NB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, TestM, 2, 12, 256, 384, 18, 1, 1)
        // TEST_PSCHEME_IF(m == TestM && n == k && n == 8192,
        //     gencode::ParallelScheme_NN_1681928192_kernel3_NA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, TestM, 2, 12, 256, 384, 18, 1, 1)
        // TEST_PSCHEME_IF(m == TestM && n == k && n == 8192,
        //     gencode::ParallelScheme_NN_1681928192_kernel3_WA_NB_NC_M0N0K0N1M1N2N3K1K2M2_func,
        //     16, TestM, 2, 12, 256, 384, 8192, 18, 1, 1, 1)
        // TEST_PSCHEME_IF(m == TestM && n == k && n == 8192,
        //     gencode::ParallelScheme_NN_1681928192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1K2M2_func,
        //     16, TestM, 2, 12, 256, 384, 8192, 18, 1, 1, 1)
        // TEST_PSCHEME_IF(m == TestM && n == k && n == 8192,
        //     gencode::ParallelScheme_NN_1681928192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, TestM, 2, 12, 256, 384, 18, 1, 1)
//===----------------------------------------------------------------------===//
// MnK
//===----------------------------------------------------------------------===//
        // constexpr int TestN = 16;
        // TEST_PSCHEME_IF(m == k && m == 8192 && n == TestN,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, TestN, 384, 18, 1, 1)
        // TEST_PSCHEME_IF(m == k && m == 8192 && n == TestN,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_NB_RC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, TestN, 384, 18, 1, 1)
        // TEST_PSCHEME_IF(m == k && m == 8192 && n == TestN,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, TestN, 384, 18, 1, 1)
        // TEST_PSCHEME_IF(m == k && m == 8192 && n == TestN,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, TestN, 384, 18, 1, 1)

        // TEST_PSCHEME_IF(m == k && m == 8192 && n == TestN,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1K2N3_func,
        //     16, 256, 2, 12, TestN, 384, 8192, 18, 1, 1, 1)
        // TEST_PSCHEME_IF(m == k && m == 8192 && n == TestN,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1K2N3_func,
        //     16, 256, 2, 12, TestN, 384, 8192, 18, 1, 1, 1)
//===----------------------------------------------------------------------===//
// MNk
//===----------------------------------------------------------------------===//
        // constexpr int TestK = 128;
        // TEST_PSCHEME_IF(m == n && m == 8192 && k == TestK,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, 256, TestK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == n && m == 8192 && k == TestK,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_NB_RC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, 256, TestK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == n && m == 8192 && k == TestK,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, 256, TestK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == n && m == 8192 && k == TestK,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, 256, TestK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == n && m == 8192 && k == TestK,
        //     gencode::ParallelScheme_NN_8192168192_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, 256, TestK, 18, 1, 1)
//===----------------------------------------------------------------------===//
// mNk
//===----------------------------------------------------------------------===//
        constexpr int TestMK = 128;

        // TEST_PSCHEME_IF(m == k && m == TestMK && n == 8192,
        //     gencode::ParallelScheme_NN_16819216_kernel3_NA_NB_NC_M0N0K0N1M1N2K1N3_func,
        //     16, 2, 12, 256, TestMK, 1, 18)
        // TEST_PSCHEME_IF(m == k && m == TestMK && n == 8192,
        //     gencode::ParallelScheme_NN_1681928192_kernel3_NA_NB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, TestMK, 2, 12, 256, TestMK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == k && m == TestMK && n == 8192,
        //     gencode::ParallelScheme_NN_1681928192_kernel3_NA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, TestMK, 2, 12, 256, TestMK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == k && m == TestMK && n == 8192,
        //     gencode::ParallelScheme_NN_1681928192_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, TestMK, 2, 12, 256, TestMK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == k && m == TestMK && n == 8192,
        //     gencode::ParallelScheme_NN_16819216_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, TestMK, 2, 12, 256, TestMK, 1, 1, 18)
        // TEST_PSCHEME_IF(m == k && m == TestMK && n == 8192,
        //     gencode::ParallelScheme_NN_16819216_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, TestMK, 2, 12, 256, TestMK, 1, 1, 18)
        // TEST_PSCHEME_IF(m == k && m == TestMK && n == 8192,
        //     gencode::ParallelScheme_NN_16819216_kernel3_NA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, TestMK, 2, 12, 256, TestMK, 1, 1, 18)
        // TEST_PSCHEME_IF(m == k && m == TestMK && n == 8192,
        //     gencode::ParallelScheme_NN_16819216_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, TestMK, 2, 12, 256, TestMK, 1, 1, 18)
//===----------------------------------------------------------------------===//
// Mnk
//===----------------------------------------------------------------------===//
        // constexpr int TestNK = 16;
        // TEST_PSCHEME_IF(m == 8192 && n == k && n == TestNK,
        //     gencode::ParallelScheme_NN_81921616_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, TestNK, TestNK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == 8192 && n == k && n == TestNK,
        //     gencode::ParallelScheme_NN_81921616_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, TestNK, TestNK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == 8192 && n == k && n == TestNK,
        //     gencode::ParallelScheme_NN_81921616_kernel3_NA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, TestNK, TestNK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == 8192 && n == k && n == TestNK,
        //     gencode::ParallelScheme_NN_81921616_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3_func,
        //     16, 256, 2, 12, TestNK, TestNK, 18, 1, 1)
        // TEST_PSCHEME_IF(m == 8192 && n == k && n == TestNK,
        //     gencode::ParallelScheme_NN_81921616_kernel3_NA_NB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, 256, 2, 12, TestNK, TestNK, 1, 1, 18)
        // TEST_PSCHEME_IF(m == 8192 && n == k && n == TestNK,
        //     gencode::ParallelScheme_NN_81921616_kernel3_WA_NB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, 256, 2, 12, TestNK, TestNK, 1, 1, 18)
        // TEST_PSCHEME_IF(m == 8192 && n == k && n == TestNK,
        //     gencode::ParallelScheme_NN_81921616_kernel3_NA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, 256, 2, 12, TestNK, TestNK, 1, 1, 18)
        // TEST_PSCHEME_IF(m == 8192 && n == k && n == TestNK,
        //     gencode::ParallelScheme_NN_81921616_kernel3_WA_WB_NC_M0N0K0N1M1N2N3K1M2_func,
        //     16, 256, 2, 12, TestNK, TestNK, 1, 1, 18)
#endif

        free(a);
        free(b);
        free(uniblasc);
        free(c_test);
        free(c_init);
    }
}
}

#undef TEST_PSCHEME_IF

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s m n k\n", argv[0]);
        exit(1);
    }
    loopmaster::unittest::testParallelGemm(
        std::atoi(argv[1]), std::atoi(argv[2]), std::atoi(argv[3]));
    return 0;
}
