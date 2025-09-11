#include <chrono>
#include "test_util.hpp"

#if defined(LM_SW6A)

#elif defined(LM_SW8A)

#elif defined(LM_SKX)
#include "Scheme_NN_65_kernel3_WA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"
#include "Scheme_NN_69_kernel3_WA_WB_RC_M0N0K0N1M1N2M2K1N3.hpp"
#include "Scheme_NN_21_kernel3_NA_WB_RC_M0N0K0N1M1N2M2K1N3.hpp"
#include "Scheme_NN_17_kernel3_NA_WB_NC_M0N0K0N1M1N2M2K1N3.hpp"
#include "Scheme_NN_1_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
#include "Scheme_NN_49_kernel3_WA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
#include "Scheme_NN_53_kernel3_WA_NB_RC_M0N0K0N1M1N2M2K1N3.hpp"
#include "Scheme_NN_5_kernel3_NA_NB_RC_M0N0K0N1M1N2M2K1N3.hpp"

// #include "../build/Scheme_NN_512512512_kernel3_WA_WB_NC_M0N0K0N1M1N2M2N3K1M3.hpp"
// #include "../build/Scheme_NN_256256256_kernel3_NA_WB_NC_M0N0K0N1M1N2M2N3K1.hpp"
// #include "../build/Scheme_NN_646464_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3.hpp"
//===----------------------------------------------------------------------===//
// mNK
//===----------------------------------------------------------------------===//
// #include "../build/Scheme_NN_1681928192_kernel3_NA_NB_NC_M0N0K0N1M1N2K1N3M2.hpp"
// #include "../build/Scheme_NN_1681928192_kernel3_NA_WB_NC_M0N0K0N1M1N2K1N3M2.hpp"
#endif

#define TEST_SCHEME_IF(cond_, scheme_, ...)                                    \
    if (cond_)                                                                 \
    {                                                                          \
        printf("============= "#cond_" =============\n");                      \
        printf("Testing " #scheme_ "<double, " #__VA_ARGS__ ">\n");            \
        testAndPerfGemm<scheme_<double, __VA_ARGS__>>(                         \
            m, n, k, alpha, beta, a, lda, b, ldb, uniblasc,                    \
            c_test, c_init, ldc,                                               \
            uni_gflops);                                                       \
        printf("---------------------------------------------------------\n"); \
    }

namespace loopmaster
{
namespace unittest
{
    void testGencodeRuntime(int m, int n, int k)
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
        double alpha = 3.14;
        double beta = 1.732;
        repeatAndTiming(10, dgemm_,
            &notrans, &notrans, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta,
            c_test, &ldc);
        flushAllCache();

        double uni_ms = repeatAndTiming(RepIter, dgemm_,
            &notrans, &notrans, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta,
            uniblasc, &ldc);
        double uni_gflops = (ops * RepIter / 1e6) / uni_ms;
        printf("BLASLib GEMM %dx%dx%d done in %.4f ms, GFLOPS = %.2f\n", m, n,
            k, uni_ms, uni_gflops);
        printf("-----------------------------------------------------------\n");
#if defined(LM_SW6A)

#elif defined(LM_SW8A)

#elif defined(LM_SKX)
        constexpr int TestMNK = 64;
        TEST_SCHEME_IF(m == n && n == k && m == TestMNK,
            gencode::Scheme_NN_646464_kernel3_NA_NB_NC_M0N0K0N1M1N2M2K1N3_func,
            16, TestMNK, 2, 12, TestMNK, 4)
        TEST_SCHEME_IF(m == n && n == k && m == TestMNK,
            gencode::Scheme_NN_512512512_kernel3_WA_WB_NC_M0N0K0N1M1N2M2N3K1M3_func,
            16, 64, 64, 2, 12, 64, 64)
        TEST_SCHEME_IF(m == n && n == k && m == TestMNK,
            gencode::Scheme_NN_256256256_kernel3_NA_WB_NC_M0N0K0N1M1N2M2N3K1_func,
            16, 64, 2, 12, 64, 64)
//===----------------------------------------------------------------------===//
// mNK
//===----------------------------------------------------------------------===//
        constexpr int TestM = 16;
        TEST_SCHEME_IF(m == TestM && n == k && n == 8192,
            gencode::Scheme_NN_1681928192_kernel3_NA_NB_NC_M0N0K0N1M1N2K1N3M2_func,
            16, TestM, 2, 12, 32, 384)
        TEST_SCHEME_IF(m == TestM && n == k && n == 8192,
            gencode::Scheme_NN_1681928192_kernel3_NA_WB_NC_M0N0K0N1M1N2K1N3M2_func,
            16, TestM, 2, 12, 32, 384)
#endif
        free(a);
        free(b);
        free(uniblasc);
        free(c_test);
        free(c_init);
    }
}
}

#undef TEST_SCHEME

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s m n k\n", argv[0]);
        exit(1);
    }
    loopmaster::unittest::testGencodeRuntime(
        std::atoi(argv[1]), std::atoi(argv[2]), std::atoi(argv[3]));
    return 0;
}
