#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "base_node.h"
#include "enum_type.h"
#include "kernel_config.h"

namespace loopmaster
{
class SchemeInfo
{
public:
    size_t id;
    std::vector<Dim> loop_order;

    std::vector<Dim> kernel_order;

    std::vector<Dim> a_order_in_kernel;

    std::vector<Dim> b_order_in_kernel;

    std::vector<Dim> c_order_in_kernel;

    TransType a_type;
    TransType b_type;

    size_t kernel_id;

    size_t m_loop_num;
    size_t n_loop_num;
    size_t k_loop_num;

    size_t kernel_m_loop_num;
    size_t kernel_n_loop_num;
    size_t kernel_k_loop_num;

    size_t out_m_loop_num;
    size_t out_n_loop_num;
    size_t out_k_loop_num;

    size_t total_loop_num;

    PackType packa;
    PackType packb;
    PackType unpackc;

    int packa_pos;
    int packb_pos;
    int unpackc_pos;

    bool vector_unpackc_enable;

    bool pre_relayout;

    SchemeInfo() = default;

    SchemeInfo(size_t id, size_t m_loop_num_, size_t n_loop_num_,
        size_t k_loop_num_, PackType packa_, PackType packb_, PackType unpackc_,
        std::vector<Dim> &kernel_order_, std::vector<Dim> &a_order_in_kernel_,
        std::vector<Dim> &b_order_in_kernel_, size_t kernel_id_,
        bool vector_unpackc_enable = true, bool pre_relayout = false);

    SchemeInfo(size_t id, const std::vector<Dim> &loop_order, size_t kernel_id_,
        PackType packa_, PackType packb_, PackType unpackc_,
        bool vector_unpackc_enable = true, bool pre_relayout = false);

    /**
     * @brief Check last 3 dim in loop_order contains both M,N,K when packa and
     * packb
     *
     * @return true
     * @return false
     */
    bool isIllegal();

    void findPackAPos();

    void findPackBPos();

    void findUnpackCPos();

    void setLoopOrder(const std::vector<Dim> &loop_order_)
    {
        this->loop_order = loop_order_;
        findPackAPos();
        findPackBPos();
        findUnpackCPos();
    }

    std::string toString();
};
}