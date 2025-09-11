#pragma once

#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <omp.h>

extern "C"
{
    void dgemm_(char const *transA, char const *transB, int const *m,
        int const *n, int const *k, double const *alpha, double const *A,
        int const *lda, double const *B, int const *ldb, double const *beta,
        double *C, int const *ldc);
}

namespace loopmaster
{
namespace unittest
{
#if defined(LM_SW6A)
    static constexpr int MaxTestThreads = 16;
#elif defined(LM_SW8A)
    static constexpr int MaxTestThreads = 64;
#elif defined(LM_SKX)
    static constexpr int MaxTestThreads = 18;
#endif
    static constexpr int WarmUp = 0;
    static constexpr int RepIter = 1;

    static constexpr bool SkipOpenBLAS = false;

    using ClockType = std::chrono::high_resolution_clock;
    using TimePointType
        = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using DuraType = std::chrono::duration<double, std::milli>;

    template <typename T>
    void printMatrix(int m, int n, T *a, int lda)
    {
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < m; i++)
            {
                printf("%.2f \t", a[i + j * lda]);
            }
            printf("\n");
        }
    }

    template <typename T>
    void initRandomMatrix(int m, int n, T *a, int lda)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<T> dis(-10, 10);
#pragma omp parallel for collapse(2) num_threads(MaxTestThreads)
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < m; i++)
            {
                a[i + j * lda] = dis(gen);
            }
        }
    }

    template <typename T>
    void initOneMatrix(int m, int n, T *a, int lda)
    {
#pragma omp parallel for collapse(2) num_threads(MaxTestThreads)
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < m; i++)
            {
                a[i + j * lda] = 1.0;
            }
        }
    }

    template <typename T>
    void copyMatrix(int m, int n, T *dst, int lddst, T *src, int ldsrc)
    {
#pragma omp parallel for collapse(2) num_threads(MaxTestThreads)
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < m; i++)
            {
                dst[i + j * lddst] = src[i + j * ldsrc];
            }
        }
    }

    template <typename T>
    T normMatrix(int m, int n, T *a, int lda)
    {
        T ans = 0.0f;
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < m; i++)
            {
                ans += a[i + j * lda] * a[i + j * lda];
            }
        }
        return ans;
    }

    template <typename T>
    bool compareMatrix(int m, int n, T *a, int lda, T *b, int ldb)
    {
        constexpr double EPS = 1e-5;
        auto fpeq = [](T a, T b) -> bool { return std::abs(a - b) <= EPS; };

        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < m; i++)
            {
                if (!fpeq(a[i + j * lda], b[i + j * ldb]))
                {
                    printf("(%d, %d) %.5f != %.5f\n", i, j, a[i + j * lda],
                        b[i + j * ldb]);
                    return false;
                }
            }
        }
        return true;
    }

    inline double getGflops(double ops, double ms)
    {
        return (ops / 1e6) / ms;
    }

    const char *getTNEnv(const std::string &name)
    {
        if (name == "OpenBLAS") return "OPENBLAS_NUM_THREADS";
        else if (name == "blis") return "BLIS_NUM_THREADS";
        else if (name == "swblas") return "SWBLAS_NUM_THREADS";
        else return "OMP_NUM_THREADS";
    }

    int getTNFromEnv(const char *env)
    {
        char *thread_num = getenv(env);
        int tn = 0;
        if (thread_num == NULL) tn = 0;
        else tn = std::atoi(thread_num);
        return tn;
    }

    void logPerf(
        const std::string &name, int m, int n, int k, double ms, double gflops)
    {
        int tn = getTNFromEnv(getTNEnv(name));
        printf("name:%s,tn:%d,m:%d,n:%d,k:%d,time(ms):%.4f,gflops:%.4f\n",
            name.c_str(), tn, m, n, k, ms, gflops);
    }

    volatile void flushAllCache()
    {
        // 32M bytes = 2^25 bytes = 2^22 double
#if defined(LM_SW6A) || defined(LM_SKX)
        constexpr int CacheSizeInDouble = 1 << 22;
#elif defined(LM_SW8A)
        constexpr int CacheSizeInDouble = 1 << 24;
#endif
        volatile double *cache = new double[CacheSizeInDouble];
        volatile double sum = 0;
#pragma omp parallel for
        for (int i = 0; i < CacheSizeInDouble; i++)
        {
            sum += cache[i];
            cache[i] = sum;
        }
        delete[] cache;
    }

    template <typename TFunc, typename... Args>
    double repeatAndTiming(int iter, TFunc func, Args... args)
    {
        TimePointType start = ClockType::now();

        for (int i = 0; i < iter; i ++)
        {
            func(args...);
        }

        TimePointType end = ClockType::now();

        DuraType dura = end - start;

        return dura.count(); // ms
    }

    template <typename GemmType>
    void testAndPerfGemm(int m, int n, int k, double alpha, double beta,
        double *a, int lda, double *b, int ldb, double *c_ref, double *c_test,
        double *c_init, int ldc, double base_gflops)
    {
        GemmType gemm_func;
        repeatAndTiming(WarmUp, gemm_func,
            m, n, k, alpha, a, lda, b, ldb, beta, c_test, ldc);
        copyMatrix(m, n, c_test, ldc, c_init, ldc);

        double ms = repeatAndTiming(RepIter,
            gemm_func, m, n, k, alpha, a, lda, b, ldb, beta, c_test, ldc);
        double ops = 2.0 * m * n * k * RepIter;
        double gflops = (ops / 1e6) / ms;
        double spdup = gflops / base_gflops;
        if (SkipOpenBLAS)
        {
            printf("Test done in %.4f ms, GFLOPS = %.2f\n",
                ms, gflops);
        }
        else if (!compareMatrix(m, n, c_ref, ldc, c_test, ldc))
        {
            printf("Test FAIL done in %.4f ms, GFLOPS = %.2f, spdup = %.2f\n",
                ms, gflops, spdup);
        }
        else
        {
            printf("Test PASS done in %.4f ms, GFLOPS = %.2f, spdup = %.2f\n",
                ms, gflops, spdup);
        }
    }

    template <typename GemmType>
    bool checkGenGemm(
        const std::string &name,
        int m, int n, int k, double alpha, double beta,
        double *a, int lda, double *b, int ldb,
        double *c_ref, double *c_test, int ldc)
    {
        char notrans = 'N';
        dgemm_(&notrans, &notrans, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta,
            c_ref, &ldc);
        GemmType gemm;
        gemm(m, n, k, alpha, a, lda, b, ldb, beta, c_test, ldc);
        if (compareMatrix<double>(m, n, c_ref, ldc, c_test, ldc))
        {
            std::cout << name << " PASS\n";
            return true;
        }
        else
        {
            std::cout << name << " FAIL\n";
            return false;
        }
    }

    template <typename GemmType>
    void benchAndLogGenGemm(
        const std::string &name,
        int m, int n, int k, double alpha, double beta,
        double *a, int lda, double *b, int ldb, double *c, int ldc,
        double ops, int warms, int iters)
    {
        GemmType gemm;
        repeatAndTiming(warms, gemm,
            m, n, k, alpha, a, lda, b, ldb, beta, c, ldc);
        double ms = repeatAndTiming(iters, gemm,
            m, n, k, alpha, a, lda, b, ldb, beta, c, ldc);
        double gflops = getGflops(ops * iters, ms);
        logPerf(name, m, n, k, ms, gflops);
    }

#define BENCH_SCHEME_IF(cond_, scheme_, ...)                                   \
    if (cond_)                                                                 \
    {                                                                          \
        if (iters > 0)                                                         \
        {                                                                      \
            benchAndLogGenGemm<scheme_<double, __VA_ARGS__>>(                  \
                #scheme_ "<double, " #__VA_ARGS__ ">",                         \
                m, n, k, alpha, beta, a, lda, b, ldb, c, ldc,                  \
                ops, warms, iters);                                            \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            checkGenGemm<scheme_<double, __VA_ARGS__>>(                        \
                #scheme_ "<double, " #__VA_ARGS__ ">",                         \
                m, n, k, alpha, beta, a, lda, b, ldb,                          \
                c_ref, c, ldc);                                                \
        }                                                                      \
    }

}
}
