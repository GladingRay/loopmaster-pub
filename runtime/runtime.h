#pragma once

#include <complex>
#include <cstdio>
#include <iostream>
#include <string>
#include <omp.h>

extern "C"
{
    // Pack A, Pack B, Pack C
    void dgemm_kernel_ge(size_t block_m, size_t block_n, size_t block_k,
        const double alpha, const double *ker_a, const double *ker_b,
        double *ker_c, size_t ldc, size_t lda, size_t ldb);

    // NO pack A, pack B, pack C
    void dgemm_kernel_ge_a_pb_pc(size_t block_m, size_t block_n, size_t block_k,
        const double alpha, const double *ker_a, const double *ker_b,
        double *ker_c, size_t ldc, size_t lda, size_t ldb);
    // Pack A, NO pack B, pack C
    void dgemm_kernel_ge_pa_b_pc(size_t block_m, size_t block_n, size_t block_k,
        const double alpha, const double *ker_a, const double *ker_b,
        double *ker_c, size_t ldc, size_t lda, size_t ldb);
    // NO pack A, NO pack B, pack C
    void dgemm_kernel_ge_a_b_pc(size_t block_m, size_t block_n, size_t block_k,
        const double alpha, const double *ker_a, const double *ker_b,
        double *ker_c, size_t ldc, size_t lda, size_t ldb);

#ifdef LM_SKX
    void dgemm_kernel_ge_pa_pb_c(size_t block_m, size_t block_n, size_t block_k,
        const double alpha, const double *ker_a, const double *ker_b,
        double *ker_c, size_t ldc, size_t lda, size_t ldb);
    void dgemm_kernel_ge_pa_pb_pc(size_t block_m, size_t block_n,
        size_t block_k, const double alpha, const double *ker_a,
        const double *ker_b, double *ker_c, size_t ldc, size_t lda, size_t ldb);
    void dgemm_kernel_ge_a_pb_c(size_t block_m, size_t block_n, size_t block_k,
        const double alpha, const double *ker_a, const double *ker_b,
        double *ker_c, size_t ldc, size_t lda, size_t ldb);
    void dgemm_kernel_ge_pa_b_c(size_t block_m, size_t block_n, size_t block_k,
        const double alpha, const double *ker_a, const double *ker_b,
        double *ker_c, size_t ldc, size_t lda, size_t ldb);
    void dgemm_kernel_ge_a_b_c(size_t block_m, size_t block_n, size_t block_k,
        const double alpha, const double *ker_a, const double *ker_b,
        double *ker_c, size_t ldc, size_t lda, size_t ldb);
#endif
}

template <typename T>
static inline std::string dataTypeToStr();

#if defined(LM_SW6A) || defined(LM_SW8A)
#include "sw6a/util.inl"
#elif defined(LM_SKX)
#include "skylakex/util.inl"
#endif

template <typename T>
struct VecLen
{
    static constexpr int value = 0;
};

#if defined(LM_SW6A) || defined(LM_SW8A)
#include "sw6a/vec_len.inl"
#elif defined(LM_SKX)
#include "skylakex/vec_len.inl"
#endif

// Cache info
#if defined(LM_SW6A) || defined(LM_SW8A)
#include "sw6a/cache_info.inl"
#elif defined(LM_SKX)
#include "skylakex/cache_info.inl"
#endif

template <typename T>
static inline void VectorCopy(T *dst_ptr, const T *src_ptr)
{
    std::cerr << "Vector copy unsupport data type: " << dataTypeToStr<T>();
    std::abort();
}

#if defined(LM_SW6A) || defined(LM_SW8A)
#include "sw6a/vector_copy.inl"
#elif defined(LM_SKX)
#include "skylakex/vector_copy.inl"
#endif

template <typename T>
static inline void VectorCopyAdd(T *dst_ptr, const T *src_ptr, const T beta)
{
    std::cerr << "Vector copy add unsupport data type: " << dataTypeToStr<T>();
    std::abort();
}

#if defined(LM_SW6A) || defined(LM_SW8A)
#include "sw6a/vector_copy_add.inl"
#elif defined(LM_SKX)
#include "skylakex/vector_copy_add.inl"
#endif

/**
 * @brief
 *
 * @tparam T [float/double]
 * @tparam KernelID
 *   0: mr(16)xnr(4)xk, a in col-major (mr, k), b in row-major (nr, k)
 *   1: mr(16)xkr(4)xnr(4)xk, a in col-major (mr, k), b in col-major (kr, nr, k)
 *   2: mr(16)xkr(4)xnr(4)xkxbn, a in col-major, b in col-major
 * @tparam PackAFlag [false/true]
 * @tparam PackBFlag [false/true]
 * @tparam UnpackCFlag [false/true]
 */

template <typename T, int KernelID, bool PackAFlag, bool PackBFlag,
    bool UnpackCFlag>
static inline void GemmKernel(size_t m, size_t n, size_t k, const T alpha,
    const T *a, size_t lda, const T *b, size_t ldb, T *c, size_t ldc)
{
    fprintf(stderr, "GemmKernel unsupport template args: %s, %d, %s, %s, %s\n",
        dataTypeToStr<T>(), KernelID, PackAFlag ? "true" : "false",
        PackBFlag ? "true" : "false", UnpackCFlag ? "true" : "false");
    std::abort();
}
#if defined(LM_SW6A) || defined(LM_SW8A)
#include "sw6a/gemm_kernel.inl"
#elif defined(LM_SKX)
#include "skylakex/gemm_kernel.inl"
#endif

template <typename T>
static inline void CbetaOp(
    int m, int n, T *c, int ldc, T beta, int tid, int tn)
{
    std::cerr << "C mul beta unsupport data type: " << dataTypeToStr<T>();
    std::abort();
}

#if defined(LM_SW6A) || defined(LM_SW8A)
#include "sw6a/c_beta_op.inl"
#elif defined(LM_SKX)
#include "skylakex/c_beta_op.inl"
#endif

template <int GN, int TN>
static inline void InitSync(volatile int sync[GN][TN][TN])
{
    for (int gid = 0; gid < GN; gid++)
    {
        for (int tid0 = 0; tid0 < TN; tid0++)
        {
            for (int tid1 = 0; tid1 < TN; tid1++)
            {
                sync[gid][tid0][tid1] = 0;
            }
        }
    }
}

template <int GN, int TN>
static inline void SyncOn(int tid, int local_tid, volatile int sync[GN][TN][TN])
{
    int gid = tid / TN;
    for (int other = 0; other < TN; other++)
    {
        sync[gid][local_tid][other] = 1;
#pragma omp flush(sync)
    }

    for (int other = 0; other < TN; other++)
    {
        do
        {
#pragma omp flush(sync)
            sched_yield();
        } while (!(sync[gid][other][local_tid]));
        sync[gid][other][local_tid] = 0;
    }

    for (int other = 0; other < TN; other++)
    {
        do
        {
#pragma omp flush(sync)
            sched_yield();
        } while (sync[gid][local_tid][other]);
    }
}

template <typename T, int RE_TN>
static inline void ReductionC(int m, int n, int ldc, T *c, T *buf_c, T beta)
{
#pragma omp for
    for (int fused_idx = 0; fused_idx < m * n; fused_idx++)
    {
        int nidx = (fused_idx / m) % n;
        int midx = fused_idx % m;
        int coff = midx + nidx * ldc;
        c[coff] *= beta;
        for (int other = 0; other < RE_TN; other++)
        {
            c[coff] += buf_c[other * m * n + coff];
        }
    }
}
