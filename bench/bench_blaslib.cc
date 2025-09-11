/**
 * Bench BLASLib gemm performance
 */
#include "../test/test_util.hpp"

namespace loopmaster
{
namespace bench
{
    using namespace unittest;
    void benchBlasGemm(
        const std::string &name, int m, int n, int k, int warms, int iters)
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

        char notrans = 'N';
        double alpha = 3.141;
        double beta = 1.732;

        repeatAndTiming(warms, dgemm_,
            &notrans, &notrans, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta,
            c, &ldc);

        double ms = repeatAndTiming(iters, dgemm_,
            &notrans, &notrans, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta,
            c, &ldc);
        double gflops = getGflops(ops * iters, ms);
        logPerf(name, m, n, k, ms, gflops);
        free(a);
        free(b);
        free(c);
    }
}
}

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        printf("Usage: %s name m n k warms iters\n"
               "Example: %s OpenBLAS 2048 2048 2048 2 10\n", argv[0], argv[0]);
        exit(1);
    }

    const std::string name(argv[1]);
    int m = std::atoi(argv[2]);
    int n = std::atoi(argv[3]);
    int k = std::atoi(argv[4]);
    int warms = std::atoi(argv[5]);
    int iters = std::atoi(argv[6]);
    loopmaster::bench::benchBlasGemm(name, m, n, k, warms, iters);
    return 0;
}