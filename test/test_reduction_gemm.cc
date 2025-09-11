#include <chrono>

#if defined(LM_SW6A)

#elif defined(LM_SW8A)

#elif defined(LM_SKX)
#include "../build/ReductionScheme_NN_16168192_kernel3_NA_NB_NC_M0N0K0N1M1N2M2N3K1.hpp"
#endif
#include "test_util.hpp"

#define TEST_RSCHEME(scheme_, ...)                                             \
    printf("Testing " #scheme_ "<double, " #__VA_ARGS__ ">\n");                \
    testAndPerfGemm<scheme_<double, __VA_ARGS__>>(                             \
        m, n, k, alpha, beta, a, lda, b, ldb, uniblasc, c_test, c_init, ldc,   \
        uni_gflops);                                                           \
    printf("----------------------------------------------------------\n");

namespace loopmaster
{
namespace unittest
{
    void testReductionGemm(int m, int n, int k)
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

        double uni_gflops = 1;
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
        constexpr int TestMN = 128;
        TEST_RSCHEME(
            gencode::ReductionScheme_NN_16168192_kernel3_NA_NB_NC_M0N0K0N1M1N2M2N3K1_func,
            16, TestMN, 2, 12, TestMN, 256, 18)
#endif

        free(a);
        free(b);
        free(uniblasc);
        free(c_test);
        free(c_init);
    }
}
}

#undef TEST_PSCHEME

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s m n k\n", argv[0]);
        exit(1);
    }
    loopmaster::unittest::testReductionGemm(
        std::atoi(argv[1]), std::atoi(argv[2]), std::atoi(argv[3]));
    return 0;
}
