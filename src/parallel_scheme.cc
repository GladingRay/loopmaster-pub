#include "common.h"

namespace loopmaster
{
void ParallelScheme::initParallelPolicy()
{
    uint64_t kernel_loop_num = this->scheme.kernel_order.size();
    std::vector<Dim> &loop_order = this->scheme.loop_order;
    uint64_t m_idx = this->scheme.kernel_m_loop_num;
    uint64_t n_idx = this->scheme.kernel_n_loop_num;
    uint64_t k_idx = this->scheme.kernel_k_loop_num;

    for (int32_t i = kernel_loop_num; i < this->scheme.total_loop_num; i++)
    {
        Dim &cur_dim = loop_order[i];
        if (cur_dim == Dim::M)
        {
            this->parallel_policy.push_back(
                { cur_dim, 1, "m_idx" + std::to_string(m_idx),
                    "tid_m_idx" + std::to_string(m_idx),
                    "TN_M_IDX" + std::to_string(m_idx) });
            m_idx++;
        }
        else if (cur_dim == Dim::N)
        {
            this->parallel_policy.push_back(
                { cur_dim, 1, "n_idx" + std::to_string(n_idx),
                    "tid_n_idx" + std::to_string(n_idx),
                    "TN_N_IDX" + std::to_string(n_idx) });
            n_idx++;
        }
        else if (cur_dim == Dim::K)
        {
            this->parallel_policy.push_back(
                { cur_dim, 1, "k_idx" + std::to_string(k_idx),
                    "tid_k_idx" + std::to_string(k_idx),
                    "TN_K_IDX" + std::to_string(k_idx) });
            k_idx++;
        }
        if (i < this->scheme.packa_pos)
        {
            packa_tn_names.push_back(this->parallel_policy.back().tn_name);
        }
        if (i < this->scheme.packb_pos)
        {
            packb_tn_names.push_back(this->parallel_policy.back().tn_name);
        }
        if (i < this->scheme.unpackc_pos)
        {
            unpackc_tn_names.push_back(this->parallel_policy.back().tn_name);
        }
    }

    int pack_m_id = 0;
    int pack_n_id = 0;
    int pack_k_id = 0;
    for (int i = 0; i < this->scheme.total_loop_num; i++)
    {
        Dim &cur_dim = this->scheme.loop_order[i];
        if (cur_dim == Dim::M)
        {
            if (i < this->scheme.packa_pos)
            {
                packa_policy.dim = Dim::M;
                packa_policy.idx_name = "packam_idx" + std::to_string(pack_m_id);
                packa_mid = pack_m_id;
            }
            if (i < this->scheme.unpackc_pos)
            {
                unpackc_policy.dim = Dim::M;
                unpackc_policy.idx_name = "unpackcm_idx" + std::to_string(pack_m_id);
                unpackc_mid = pack_m_id;
            }
            pack_m_id++;
        }
        else if (cur_dim == Dim::N)
        {
            if (i < this->scheme.packb_pos)
            {
                packb_policy.dim = Dim::N;
                packb_policy.idx_name = "packbn_idx" + std::to_string(pack_n_id);
                packb_nid = pack_n_id;
            }
            if (i < this->scheme.unpackc_pos)
            {
                unpackc_policy.dim = Dim::N;
                unpackc_policy.idx_name = "unpackcn_idx" + std::to_string(pack_n_id);
                unpackc_nid = pack_n_id;
            }
            pack_n_id++;
        }
        else if (cur_dim == Dim::K)
        {
            if (i < this->scheme.packa_pos)
            {
                packa_policy.dim = Dim::K;
                packa_policy.idx_name = "packak_idx" + std::to_string(pack_k_id);
                packa_kid = pack_k_id;
            }
            if (i < this->scheme.packb_pos)
            {
                packb_policy.dim = Dim::K;
                packb_policy.idx_name = "packbk_idx" + std::to_string(pack_k_id);
                packb_kid = pack_k_id;
            }
            pack_k_id++;
        }
    }

    packa_policy.tn_name = "PACKA_TN";
    packb_policy.tn_name = "PACKB_TN";
    unpackc_policy.tn_name = "UNPACKC_TN";

    packa_policy.tid_name = "packa_tid";
    packb_policy.tid_name = "packb_tid";
    unpackc_policy.tid_name = "unpackc_tid";

    if (this->packa_policy.dim == Dim::M)
        this->packa_pre_len = "packak_len" + std::to_string(packa_kid+1);
    else if (this->packa_policy.dim == Dim::K)
        this->packa_pre_len = "packam_len" + std::to_string(packa_mid+1);
    
    if (this->packb_policy.dim == Dim::N)
        this->packb_pre_len = "packbk_len" + std::to_string(packb_kid+1);
    else if (this->packb_policy.dim == Dim::K)
        this->packb_pre_len = "packbn_len" + std::to_string(packb_nid+1);

    if (this->unpackc_policy.dim == Dim::M)
        this->unpackc_pre_len = "unpackcn_len" + std::to_string(unpackc_nid+1);
    else if (this->unpackc_policy.dim == Dim::N)
        this->unpackc_pre_len = "unpackcm_len" + std::to_string(unpackc_mid+1);
}

void ParallelScheme::initReductionPolicy()
{
    this->parallel_policy.push_back(
        { Dim::K, 1, "k_idx" + std::to_string(this->scheme.k_loop_num-1),
            "tid_k_reduction", "RE_TN" });
}

std::string ParallelScheme::getTNTemplateArgName(
    const std::string &idx_name) const
{
    std::string ret = idx_name;

    for (char &ch : ret)
        ch = std::toupper(ch);

    return "TN_" + ret;
}

}