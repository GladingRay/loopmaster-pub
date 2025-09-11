#include "runtime.h"

/**
 * Gemm kernel wrapper, m must aligned with 16
 */

template <>
inline void GemmKernel<double, 3, true, true, true>(size_t m, size_t n,
    size_t k, const double alpha, const double *a, size_t lda, const double *b,
    size_t ldb, double *c, size_t ldc)
{
    dgemm_kernel_ge_pa_pb_pc(m, n, k, alpha, a, b, c, ldc, lda, ldb);

}

template <>
inline void GemmKernel<double, 3, false, false, true>(size_t m, size_t n,
    size_t k, const double alpha, const double *a, size_t lda, const double *b,
    size_t ldb, double *c, size_t ldc)
{
    dgemm_kernel_ge_a_b_pc(m, n, k, alpha, a, b, c, ldc, lda, ldb);
}

template <>
inline void GemmKernel<double, 3, false, true, true>(size_t m, size_t n,
    size_t k, const double alpha, const double *a, size_t lda, const double *b,
    size_t ldb, double *c, size_t ldc)
{
    dgemm_kernel_ge_a_pb_pc(m, n, k, alpha, a, b, c, ldc, lda, ldb);
}

template <>
inline void GemmKernel<double, 3, true, false, true>(size_t m, size_t n,
    size_t k, const double alpha, const double *a, size_t lda, const double *b,
    size_t ldb, double *c, size_t ldc)
{
    dgemm_kernel_ge_pa_b_pc(m, n, k, alpha, a, b, c, ldc, lda, ldb);
}

template <>
inline void GemmKernel<double, 3, true, true, false>(size_t m, size_t n,
    size_t k, const double alpha, const double *a, size_t lda, const double *b,
    size_t ldb, double *c, size_t ldc)
{
    dgemm_kernel_ge_pa_pb_c(m, n, k, alpha, a, b, c, ldc, lda, ldb);
}

template <>
inline void GemmKernel<double, 3, false, false, false>(size_t m, size_t n,
    size_t k, const double alpha, const double *a, size_t lda, const double *b,
    size_t ldb, double *c, size_t ldc)
{
    dgemm_kernel_ge_a_b_c(m, n, k, alpha, a, b, c, ldc, lda, ldb);
}

template <>
inline void GemmKernel<double, 3, false, true, false>(size_t m, size_t n,
    size_t k, const double alpha, const double *a, size_t lda, const double *b,
    size_t ldb, double *c, size_t ldc)
{
    dgemm_kernel_ge_a_pb_c(m, n, k, alpha, a, b, c, ldc, lda, ldb);
}

template <>
inline void GemmKernel<double, 3, true, false, false>(size_t m, size_t n,
    size_t k, const double alpha, const double *a, size_t lda, const double *b,
    size_t ldb, double *c, size_t ldc)
{
    dgemm_kernel_ge_pa_b_c(m, n, k, alpha, a, b, c, ldc, lda, ldb);
}
