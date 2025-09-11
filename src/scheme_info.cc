#include "scheme_info.h"

namespace loopmaster
{

SchemeInfo::SchemeInfo(size_t id_, size_t m_loop_num_, size_t n_loop_num_,
    size_t k_loop_num_, PackType packa_, PackType packb_, PackType unpackc_,
    std::vector<Dim> &kernel_order_, std::vector<Dim> &a_order_in_kernel_,
    std::vector<Dim> &b_order_in_kernel_, size_t kernel_id_,
    bool vector_unpackc_enable_, bool pre_relayout_)
    : id(id_)
    , m_loop_num(m_loop_num_)
    , n_loop_num(n_loop_num_)
    , k_loop_num(k_loop_num_)
    , packa(packa_)
    , packb(packb_)
    , unpackc(unpackc_)
    , kernel_order(kernel_order_)
    , a_order_in_kernel(a_order_in_kernel_)
    , b_order_in_kernel(b_order_in_kernel_)
    , kernel_id(kernel_id_)
    , vector_unpackc_enable(vector_unpackc_enable_)
    , pre_relayout(pre_relayout_)
{
    this->total_loop_num
        = this->m_loop_num + this->n_loop_num + this->k_loop_num;

    loop_order.assign(this->total_loop_num, Dim::Void);

    this->kernel_m_loop_num = 0;
    this->kernel_n_loop_num = 0;
    this->kernel_k_loop_num = 0;

    for (size_t i = 0; i < this->kernel_order.size(); ++i)
    {
        if (this->kernel_order[i] == Dim::M)
            this->kernel_m_loop_num++;
        else if (this->kernel_order[i] == Dim::N)
            this->kernel_n_loop_num++;
        else if (this->kernel_order[i] == Dim::K)
            this->kernel_k_loop_num++;
    }

    this->out_m_loop_num = this->m_loop_num - this->kernel_m_loop_num;
    this->out_n_loop_num = this->n_loop_num - this->kernel_n_loop_num;
    this->out_k_loop_num = this->k_loop_num - this->kernel_k_loop_num;

    this->kernel_id = kernel_id_;

    this->a_type = TransType::N;
    this->b_type = TransType::N;
}

SchemeInfo::SchemeInfo(size_t id_, const std::vector<Dim> &loop_order_,
    size_t kernel_id_, PackType packa_, PackType packb_, PackType unpackc_,
    bool vector_unpackc_enable_, bool pre_relayout_)
    : id(id_)
    , loop_order(loop_order_)
    , kernel_id(kernel_id_)
    , packa(packa_)
    , packb(packb_)
    , unpackc(unpackc_)
    , m_loop_num(0)
    , n_loop_num(0)
    , k_loop_num(0)
    , kernel_m_loop_num(0)
    , kernel_n_loop_num(0)
    , kernel_k_loop_num(0)
    , out_m_loop_num(0)
    , out_n_loop_num(0)
    , out_k_loop_num(0)
    , total_loop_num(loop_order_.size())
    , vector_unpackc_enable(vector_unpackc_enable_)
    , pre_relayout(pre_relayout_)
{
    KernelConfig kernel_config(this->kernel_id);

    this->kernel_order = kernel_config.kernel_order;
    this->a_order_in_kernel = kernel_config.a_order;
    this->b_order_in_kernel = kernel_config.b_order;

    for (int i = 0; i < this->loop_order.size(); i++)
    {
        Dim cur_dim = this->loop_order[i];

        if (i < this->kernel_order.size())
        {
            LM_ASSERT(cur_dim == this->kernel_order[i],
                "loop order doesn't match kernel order.");
        }

        if (cur_dim == Dim::M)
        {
            this->m_loop_num++;
            if (i < this->kernel_order.size())
                this->kernel_m_loop_num++;
        }
        else if (cur_dim == Dim::N)
        {
            this->n_loop_num++;
            if (i < this->kernel_order.size())
                this->kernel_n_loop_num++;
        }
        else if (cur_dim == Dim::K)
        {
            this->k_loop_num++;
            if (i < this->kernel_order.size())
                this->kernel_k_loop_num++;
        }
    }

    this->out_m_loop_num = this->m_loop_num - this->kernel_m_loop_num;
    this->out_n_loop_num = this->n_loop_num - this->kernel_n_loop_num;
    this->out_k_loop_num = this->k_loop_num - this->kernel_k_loop_num;

    this->a_type = TransType::N;
    this->b_type = TransType::N;

    this->findPackAPos();
    this->findPackBPos();
    this->findUnpackCPos();
}

bool SchemeInfo::isIllegal()
{
    bool has_m = false;
    bool has_n = false;
    bool has_k = false;

    for (int i = this->loop_order.size() - 1;
        i >= 0 && i >= this->loop_order.size() - 3; i--)
    {
        if (this->loop_order[i] == Dim::M)
            has_m = true;
        else if (this->loop_order[i] == Dim::N)
            has_n = true;
        else if (this->loop_order[i] == Dim::K)
            has_k = true;
    }

    return packa != PackType::NoPack && packb != PackType::NoPack
        && !(has_m && has_n && has_k);
}

void SchemeInfo::findPackAPos()
{
    this->packa_pos = -1;

    if (this->packa == PackType::NoPack)
        return;

    bool has_m = false;
    bool has_k = false;

    this->packa_pos = this->loop_order.size();

    if (this->pre_relayout) return;

    for (int i = this->loop_order.size() - 1; i >= 0; i--)
    {
        if (this->loop_order[i] == Dim::M)
            has_m = true;
        if (this->loop_order[i] == Dim::K)
            has_k = true;

        if (has_m && has_k)
        {
            this->packa_pos = i;
            return;
        }
    }
}

void SchemeInfo::findPackBPos()
{
    this->packb_pos = -1;

    if (this->packb == PackType::NoPack)
        return;

    bool has_n = false;
    bool has_k = false;

    this->packb_pos = this->loop_order.size();

    if (this->pre_relayout) return;

    for (int i = this->loop_order.size() - 1; i >= 0; i--)
    {
        if (this->loop_order[i] == Dim::N)
            has_n = true;
        if (this->loop_order[i] == Dim::K)
            has_k = true;

        if (has_k && has_n)
        {
            this->packb_pos = i;
            return;
        }
    }
}

void SchemeInfo::findUnpackCPos()
{
    this->unpackc_pos = -1;

    if (this->unpackc == PackType::NoPack)
        return;

    this->unpackc_pos = this->loop_order.size();

    if (this->pre_relayout) return;

    for (int i = this->loop_order.size() - 1; i >= 0; i--)
    {
        if (this->loop_order[i] == Dim::K)
            return;
        this->unpackc_pos--;
    }
}

std::string SchemeInfo::toString()
{
    std::string ret = "Scheme_";
    ret.push_back(static_cast<char>(this->a_type));
    ret.push_back(static_cast<char>(this->b_type));
    ret += "_" + std::to_string(this->id) + "_kernel"
        + std::to_string(this->kernel_id) + "_";

    if (packa == PackType::ReadContinuous)
        ret += "RA_";
    else if (packa == PackType::WriteContinuous)
        ret += "WA_";
    else
        ret += "NA_";

    if (packb == PackType::ReadContinuous)
        ret += "RB_";
    else if (packb == PackType::WriteContinuous)
        ret += "WB_";
    else
        ret += "NB_";

    if (unpackc == PackType::ReadContinuous)
        ret += "RC_";
    else if (unpackc == PackType::WriteContinuous)
        ret += "WC_";
    else
        ret += "NC_";

    size_t m_idx = 0;
    size_t n_idx = 0;
    size_t k_idx = 0;

    for (size_t i = 0; i < loop_order.size(); i++)
    {
        if (loop_order[i] == Dim::M)
        {
            ret += "M";
            ret += std::to_string(m_idx);
            m_idx++;
        }
        else if (loop_order[i] == Dim::N)
        {
            ret += "N";
            ret += std::to_string(n_idx);
            n_idx++;
        }
        else if (loop_order[i] == Dim::K)
        {
            ret += "K";
            ret += std::to_string(k_idx);
            k_idx++;
        }
        else
        {
            ret += "V";
            ret += std::to_string(i);
        }
    }

    return ret;
}
} // namespace loopmaster