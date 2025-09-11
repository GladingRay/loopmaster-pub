#include <chrono>
#include <fstream>
#include <iostream>

#include "common.h"

#include "test_util.hpp"

namespace loopmaster
{
namespace unittest
{

    void scalarCopy(int m, int n, double *a, int lda, double *buffer)
    {
        for (int n_idx0 = 0; n_idx0 < n; n_idx0++)
        {
            for (int m_idx0 = 0; m_idx0 < m; m_idx0++)
            {
                *buffer++ = a[m_idx0 + n_idx0 * lda];
            }
        }
    }

    void vectorCopy(int m, int n, double *a, int lda, double *buffer)
    {
        for (int n_idx0 = 0; n_idx0 < n; n_idx0++)
        {
            for (int m_idx0 = 0; m_idx0 < m; m_idx0 += 4)
            {
                double *a_ptr = a + m_idx0 + n_idx0 * lda;
                memcpy(buffer, a_ptr, 4 * sizeof(double));
                buffer += 4;
            }
        }
    }

    template <int MR, int KB>
    void packUniblas(int m, int k, double *a, int lda, double *buffer)
    {
        double *buffer_ptr = buffer;
        for (int k_idx0 = 0; k_idx0 < k; k_idx0 += KB)
        {
            int k_len0 = std::min(k - k_idx0, KB);
            for (int k_idx1 = 0; k_idx1 < k_len0; k_idx1++)
            {
                for (int m_idx0 = 0; m_idx0 < m; m_idx0 += MR)
                {
                    double *buffer_v0
                        = buffer_ptr + m_idx0 * k_len0 + k_idx1 * MR;
                    double *buffer_v1 = buffer_v0 + 4;
                    double *buffer_v2 = buffer_v1 + 4;
                    double *buffer_v3 = buffer_v2 + 4;

                    double *a_v0 = a + m_idx0 + (k_idx0 + k_idx1) * lda;
                    double *a_v1 = a_v0 + 4;
                    double *a_v2 = a_v1 + 4;
                    double *a_v3 = a_v2 + 4;

                    memcpy(buffer_v0, a_v0, MR * sizeof(double));
                }
            }
            buffer_ptr += m * k_len0;
        }
    }

    template <int MR, int KB>
    void packUniblasScalar(int m, int k, double *a, int lda, double *buffer)
    {
        double *buffer_ptr = buffer;
        for (int k_idx0 = 0; k_idx0 < k; k_idx0 += KB)
        {
            int k_len0 = std::min(k - k_idx0, KB);
            for (int k_idx1 = 0; k_idx1 < k_len0; k_idx1++)
            {
                for (int m_idx0 = 0; m_idx0 < m; m_idx0 += MR)
                {
                    for (int m_idx1 = 0; m_idx1 < MR; m_idx1++)
                    {
                        buffer_ptr[m_idx1 + m_idx0 * k_len0 + k_idx1 * MR]
                            = a[m_idx0 + m_idx1 + (k_idx0 + k_idx1) * lda];
                    }
                }
            }
            buffer_ptr += m * k_len0;
        }
    }

    template <int MR, int KB>
    void packGencode(int m, int n, double *a, int lda, double *buffer)
    {
        constexpr int BM0 = MR;
        constexpr int BK0 = 4;
        constexpr int BK1 = KB;
        int bk2 = n;
        int bm1 = m;
        for (int srck_idx0 = 0; (srck_idx0 < bk2); (srck_idx0 += 1))
        {
            int tiledk_idx0 = ((srck_idx0 % BK0) - 0);
            int dstk_len0
                = (srck_idx0 >= (bk2 - (bk2 % BK0))) ? (bk2 % BK0) : BK0;
            int tiledk_idx1 = ((srck_idx0 % BK1) - (srck_idx0 % BK0));
            int dstk_len1
                = (srck_idx0 >= (bk2 - (bk2 % BK1))) ? (bk2 % BK1) : BK1;
            int tiledk_idx2 = ((srck_idx0 % bk2) - (srck_idx0 % BK1));
            int dstk_len2
                = (srck_idx0 >= (bk2 - (bk2 % bk2))) ? (bk2 % bk2) : bk2;
            for (int srcm_idx0 = 0; (srcm_idx0 < bm1); (srcm_idx0 += 16))
            {
                double *src_ptr = (a + srcm_idx0 + (srck_idx0 * lda));
                int tiledm_idx0 = ((srcm_idx0 % BM0) - 0);
                int dstm_len0
                    = (srcm_idx0 >= (bm1 - (bm1 % BM0))) ? (bm1 % BM0) : BM0;
                int tiledm_idx1 = ((srcm_idx0 % bm1) - (srcm_idx0 % BM0));
                int dstm_len1
                    = (srcm_idx0 >= (bm1 - (bm1 % bm1))) ? (bm1 % bm1) : bm1;
                double *dst_ptr = (buffer + (tiledm_idx0 * 1)
                    + (tiledk_idx0 * dstm_len0) + (tiledk_idx1 * dstm_len0)
                    + (tiledm_idx1 * dstk_len1) + (tiledk_idx2 * dstm_len1));
                memcpy(dst_ptr, src_ptr, 16 * sizeof(double));
            }
        }
    }

    template <int MR, int KB>
    void packGencodeScalar(int m, int n, double *a, int lda, double *buffer)
    {
        constexpr int BM0 = MR;
        constexpr int BK0 = 4;
        constexpr int BK1 = KB;
        int bk2 = n;
        int bm1 = m;
        for (int srck_idx0 = 0; (srck_idx0 < bk2); (srck_idx0 += 1))
        {
            int tiledk_idx0 = ((srck_idx0 % BK0) - 0);
            int dstk_len0
                = (srck_idx0 >= (bk2 - (bk2 % BK0))) ? (bk2 % BK0) : BK0;
            int tiledk_idx1 = ((srck_idx0 % BK1) - (srck_idx0 % BK0));
            int dstk_len1
                = (srck_idx0 >= (bk2 - (bk2 % BK1))) ? (bk2 % BK1) : BK1;
            int tiledk_idx2 = ((srck_idx0 % bk2) - (srck_idx0 % BK1));
            int dstk_len2
                = (srck_idx0 >= (bk2 - (bk2 % bk2))) ? (bk2 % bk2) : bk2;
            for (int srcm_idx0 = 0; (srcm_idx0 < bm1); (srcm_idx0++))
            {
                int tiledm_idx0 = ((srcm_idx0 % BM0) - 0);
                int dstm_len0
                    = (srcm_idx0 >= (bm1 - (bm1 % BM0))) ? (bm1 % BM0) : BM0;
                int tiledm_idx1 = ((srcm_idx0 % bm1) - (srcm_idx0 % BM0));
                int dstm_len1
                    = (srcm_idx0 >= (bm1 - (bm1 % bm1))) ? (bm1 % bm1) : bm1;

                buffer[(tiledm_idx0 * 1) + (tiledk_idx0 * dstm_len0)
                    + (tiledk_idx1 * dstm_len0) + (tiledm_idx1 * dstk_len1)
                    + (tiledk_idx2 * dstm_len1)]
                    = a[srcm_idx0 + (srck_idx0 * lda)];
            }
        }
    }

    using TimePointType
        = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using DuraType = std::chrono::duration<double, std::milli>;

    template <typename TFUNC>
    void timingFunc(TFUNC func, int m, int n, double *a, int lda)
    {
        flushAllCache();

        double *buffer = (double *)aligned_alloc(128, m * n * sizeof(double));

        TimePointType start = std::chrono::high_resolution_clock::now();
        func(m, n, a, lda, buffer);
        TimePointType end = std::chrono::high_resolution_clock::now();

        DuraType dura = end - start;

        printf("%.4f ms\n", dura.count());
        free(buffer);
    }

    void testDiffPacking(int m, int n)
    {
        double *a = (double *)aligned_alloc(128, m * n * sizeof(double));
        printf("Testing scalarCopy         : ");
        timingFunc(scalarCopy, m, n, a, m);
        printf("Testing vectorCopy         : ");
        timingFunc(vectorCopy, m, n, a, m);
        printf("Testing packUniblas        : ");
        timingFunc(packUniblas<16, 128>, m, n, a, m);
        printf("Testing packGencode        : ");
        timingFunc(packGencode<16, 128>, m, n, a, m);
        printf("Testing packUniblasScalar  : ");
        timingFunc(packUniblasScalar<16, 160>, m, n, a, m);
        printf("Testing packGencodeScalar  : ");
        timingFunc(packGencodeScalar<16, 160>, m, n, a, m);
        free(a);
    }
}
}

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("Usage: %s m n\n", argv[0]);
        exit(1);
    }
    int m = std::atoi(argv[1]);
    int n = std::atoi(argv[2]);
    loopmaster::unittest::testDiffPacking(m, n);

    return 0;
}