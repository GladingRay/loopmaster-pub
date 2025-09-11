#pragma once
#include <cassert>
#include <cstddef>
#include <vector>

#include "enum_type.h"
#include "lm_assert.h"

namespace loopmaster
{
class KernelConfig
{
public:
    size_t kernel_id;
    std::vector<Dim> kernel_order;
    std::vector<Dim> a_order;
    std::vector<Dim> b_order;
    size_t kernel_m_num;
    size_t kernel_n_num;
    size_t kernel_k_num;
    KernelConfig(size_t kernel_id_)
        : kernel_id(kernel_id_)
        , kernel_m_num(0)
        , kernel_n_num(0)
        , kernel_k_num(0)
    {
        LM_ASSERT(kernel_id >= 0 && kernel_id <= 3,
            ("Unsupport kernel id " + std::to_string(kernel_id)));
        if (kernel_id == 0)
        {
            kernel_order = { Dim::M, Dim::N, Dim::K };
        }
        else if (kernel_id == 1)
        {
            kernel_order = { Dim::M, Dim::K, Dim::N, Dim::K };
        }
        else if (kernel_id == 2)
        {
            kernel_order = { Dim::M, Dim::K, Dim::N, Dim::K, Dim::N };
        }
        else if (kernel_id == 3)
        {
            kernel_order = { Dim::M, Dim::N, Dim::K, Dim::N, Dim::M, Dim::N};
        }

        for (Dim &dim : kernel_order)
        {
            if (dim == Dim::M)
            {
                a_order.push_back(Dim::M);
                kernel_m_num++;
            }
            else if (dim == Dim::K)
            {
                a_order.push_back(Dim::K);
                b_order.push_back(Dim::K);
                kernel_k_num++;
            }
            else if (dim == Dim::N)
            {
                b_order.push_back(Dim::N);
                kernel_n_num++;
            }
        }
    }

    ~KernelConfig() { }
};
}