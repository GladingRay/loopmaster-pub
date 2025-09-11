#include <iostream>
#if 0
#include <chrono>

#include "Scheme_2_kernel2_PA_PB_PC_M0K0N0K1N1M1N2K2M2K3N3_timing.hpp"
#include "ds_blas.h"

#include "test_util.hpp"

namespace loopmaster
{
    namespace unittest
    {

#define INIT_CMEM(id_)                                                         \
    double *gencodeC##id_                                                      \
        = static_cast<double *>(aligned_alloc(128, m * n * sizeof(double)));   \
    copyMatrix(m, n, gencodeC##id_, ldc, uniblasC, ldc);

#define TEST_SCHEME(id_, a_, b_, c_)                                             \
    using gen_gemm_func_t##id_ = gencode::                                       \
        Scheme_##id_##_kernel2_##a_##_##b_##_##c_##_M0K0N0K1N1M1N2K2M2K3N3_func< \
            double, 16, 512, 4, 256, 4096, 4, 128, 256>;                         \
    gen_gemm_func_t##id_ gemm_func##id_;                                         \
    gemm_func##id_(notrans, notrans, m, n, k, alpha, A, lda, B, ldb, beta,       \
        gencodeC##id_, ldc);                                                     \
    if (!compareMatrix(m, n, uniblasC, ldc, gencodeC##id_, ldc))                 \
    {                                                                            \
        std::cout << "Test Scheme" << #id_ #a_ #b_ #c_                           \
                  << " correctness FAIL.\n";                                     \
    }                                                                            \
    else                                                                         \
    {                                                                            \
        std::cout << "Test Scheme" << #id_ #a_ #b_ #c_                           \
                  << " correctness PASS.\n";                                     \
    }

#define PERF_SCHEME(id_, a_, b_, c_)                                           \
    flushAllCache();                                                           \
    start = std::chrono::high_resolution_clock::now();                         \
    gemm_func##id_(notrans, notrans, m, n, k, alpha, A, lda, B, ldb, beta,     \
        gencodeC##id_, ldc);                                                   \
    end = std::chrono::high_resolution_clock::now();                           \
    duration = end - start;                                                    \
    printf("gencode GEMM Scheme%s %dx%dx%d done in %.4f ms, GFLOPS = %.2f\n",  \
        #id_ #a_ #b_ #c_, m, n, k, duration.count(),                           \
        (ops / 1e6) / duration.count());

        void testGencodeRuntime(int m, int n, int k)
        {
            int lda = m;
            int ldb = k;
            int ldc = m;
            double ops = 2.0 * m * n * k;

            double *A = static_cast<double *>(aligned_alloc(128, m * k * sizeof(double)));
            initRandomMatrix(m, k, A, lda);

            double *B = static_cast<double *>(aligned_alloc(128, n * k * sizeof(double)));
            initRandomMatrix(k, n, B, ldb);

            double *uniblasC = static_cast<double *>(aligned_alloc(128, m * n * sizeof(double)));
            initRandomMatrix(m, n, uniblasC, ldc);

            INIT_CMEM(2)

            double alpha = 3.14;
            double beta = 1.732;

            flushAllCache();
            ds_blas::ds_op_t notrans = ds_blas::ds_op_t::NoTrans;
            auto uni_start = std::chrono::high_resolution_clock::now();
            ds_blas::xgemm<double, double, double>(ds_blas::ds_layout_t::ColMajor, notrans, notrans, m, n, k,
                                                    alpha, A, lda, B, ldb, beta, uniblasC, ldc);
            auto uni_end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> uni_dura = uni_end - uni_start;
            printf("uniblas GEMM %dx%dx%d done in %.4f ms, GFLOPS = %.2f\n", m, n, k, uni_dura.count(), (ops / 1e6) / uni_dura.count());

            TEST_SCHEME(2, PA, PB, PC)

            std::cout << "======= Testing Performence =======\n";
            std::chrono::time_point<std::chrono::high_resolution_clock> start;
            std::chrono::time_point<std::chrono::high_resolution_clock> end;
            std::chrono::duration<double, std::milli> duration;

            PERF_SCHEME(2, PA, PB, PC)
            gemm_func2.printTimingLog();

            free(A);
            free(B);
            free(uniblasC);
            free(gencodeC2);
        }
#undef INIT_CMEM
#undef TEST_SCHEME
#undef PERF_SCHEME
    }
}
#endif

int main(int argc, char *argv[])
{
#if 0
    if (argc < 4)
    {
        printf("Usage: ./%s m n k\n", argv[0]);
        exit(1);
    }
    loopmaster::unittest::testGencodeRuntime(std::atoi(argv[1]), std::atoi(argv[2]), std::atoi(argv[3]));
#endif
    std::cout << "test timing packing had been deprecated.\n";
    return 0;
}