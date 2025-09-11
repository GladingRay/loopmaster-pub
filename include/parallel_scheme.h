#pragma once
#include "scheme_info.h"
#include <cctype>

namespace loopmaster
{

class ParallelUnit
{
public:
    Dim dim;
    uint32_t num_threads;
    std::string idx_name;
    std::string tid_name;
    std::string tn_name;

    ParallelUnit() = default;
    ~ParallelUnit() = default;
};

class ParallelScheme
{
public:
    SchemeInfo scheme;
    std::vector<ParallelUnit> parallel_policy;

    ParallelUnit packa_policy;
    ParallelUnit packb_policy;
    ParallelUnit unpackc_policy;

    std::vector<std::string> packa_tn_names;
    std::vector<std::string> packb_tn_names;
    std::vector<std::string> unpackc_tn_names;

    int packa_mid = 0;
    int packa_kid = 0;
    int packb_nid = 0;
    int packb_kid = 0;
    int unpackc_mid = 0;
    int unpackc_nid = 0;

    std::string packa_pre_len;
    std::string packb_pre_len;
    std::string unpackc_pre_len;

    bool parallel_on_k;

    ParallelScheme(SchemeInfo &scheme_info, bool paral_on_k = false)
        : scheme(scheme_info)
        , parallel_on_k(paral_on_k)
        , parallel_policy()
    {
        if (!paral_on_k)
            this->initParallelPolicy();
        else
        {
            this->initReductionPolicy();
        }
    }

    /**
     * @brief Set the threads num of loops that outside the kernel based on
     * schemeinfo
     *
     */
    void initParallelPolicy();

    /**
     * @brief Set the threads num of loop that outside the kernel based on
     * schemeinfo
     * 
     */
    void initReductionPolicy();

    /**
     * @brief Convert x_idxn to TN_X_IDXn
     *
     * @param idx_name
     * @return std::string
     */
    std::string getTNTemplateArgName(const std::string &idx_name) const;
};

}