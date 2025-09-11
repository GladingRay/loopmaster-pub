#include "common.h"

namespace loopmaster
{
GemmFamilyGenerator::GemmFamilyGenerator(size_t max_loop_perdim_)
{
    this->cur_id = 0;
    this->total_scheme_num = 0;
    this->max_loop_perdim = max_loop_perdim_;
}

void GemmFamilyGenerator::dfsLoopOrder(SchemeInfo &temp_scheme, Dim pre_dim,
    size_t depth, size_t m_last, size_t n_last, size_t k_last)
{
    if (depth == temp_scheme.loop_order.size() && m_last == 0 && n_last == 0
        && k_last == 0)
    {
        if (temp_scheme.isIllegal())
            return;

        temp_scheme.id = this->cur_id;
        temp_scheme.findPackAPos();
        temp_scheme.findPackBPos();
        temp_scheme.findUnpackCPos();

        this->scheme_vec.push_back(temp_scheme);
        this->total_scheme_num++;
        this->cur_id++;
        return;
    }

    if (depth > temp_scheme.loop_order.size() || m_last < 0 || n_last < 0
        || k_last < 0)
        return;

    if (pre_dim != Dim::M && m_last > 0)
    {
        temp_scheme.loop_order[depth] = Dim::M;
        dfsLoopOrder(
            temp_scheme, Dim::M, depth + 1, m_last - 1, n_last, k_last);
    }

    if (pre_dim != Dim::N && n_last > 0)
    {
        temp_scheme.loop_order[depth] = Dim::N;
        dfsLoopOrder(
            temp_scheme, Dim::N, depth + 1, m_last, n_last - 1, k_last);
    }

    if (pre_dim != Dim::K && k_last > 0)
    {
        temp_scheme.loop_order[depth] = Dim::K;
        dfsLoopOrder(
            temp_scheme, Dim::K, depth + 1, m_last, n_last, k_last - 1);
    }
}

void GemmFamilyGenerator::buildSchemeFamily(
    KernelConfig &kernel_config, bool vector_unpackc_enable,
    bool pre_relayout)
{
    std::vector<Dim> &kernel_loop_vec = kernel_config.kernel_order;

    for (size_t m_loop_num = kernel_config.kernel_m_num + 1;
        m_loop_num <= max_loop_perdim; ++m_loop_num)
    {
        for (size_t n_loop_num = kernel_config.kernel_n_num + 1;
            n_loop_num <= max_loop_perdim; ++n_loop_num)
        {
            for (size_t k_loop_num = kernel_config.kernel_k_num + 1;
                k_loop_num <= max_loop_perdim; ++k_loop_num)
            {
                SchemeInfo temp_scheme(0, m_loop_num, n_loop_num, k_loop_num,
                    PackType::NoPack, PackType::NoPack, PackType::NoPack,
                    kernel_loop_vec, kernel_config.a_order,
                    kernel_config.b_order, kernel_config.kernel_id, vector_unpackc_enable, pre_relayout);
                size_t m_last = m_loop_num;
                size_t n_last = n_loop_num;
                size_t k_last = k_loop_num;

                for (size_t i = 0; i < kernel_loop_vec.size(); ++i)
                {
                    temp_scheme.loop_order[i] = kernel_loop_vec[i];

                    if (kernel_loop_vec[i] == Dim::M)
                        m_last--;
                    else if (kernel_loop_vec[i] == Dim::N)
                        n_last--;
                    else if (kernel_loop_vec[i] == Dim::K)
                        k_last--;
                }
                for (PackType pa : { PackType::NoPack, PackType::ReadContinuous,
                         PackType::WriteContinuous })
                {
                    for (PackType pb :
                        { PackType::NoPack, PackType::ReadContinuous,
                            PackType::WriteContinuous })
                    {
                        for (PackType pc :
                            { PackType::NoPack, PackType::ReadContinuous })
                        {
                            temp_scheme.packa = pa;
                            temp_scheme.packb = pb;
                            temp_scheme.unpackc = pc;

                            dfsLoopOrder(temp_scheme, kernel_loop_vec.back(),
                                kernel_loop_vec.size(), m_last, n_last, k_last);
                        } // pc
                    } // pb
                } // pa
            } // k_loop_num
        } // n_loop_num
    } // m_loop_num
}

void GemmFamilyGenerator::generateAllSchemes(
    const std::string &place_dir, bool parallel_pass_enable)
{
    for (int i = 0; i < scheme_vec.size(); i++)
    {
        printf("Generating %s\n", scheme_vec[i].toString().c_str());
        GemmGenerator gemm_generator(scheme_vec[i]);
        gemm_generator.buildGemmAST();
        PASS pass(gemm_generator);
        pass.refineVectorLoop();
        gemm_generator.codeGen(place_dir);
        if (parallel_pass_enable)
        {
            ParallelScheme ps(gemm_generator.getSchemeInfo());
            pass.parallelizeGemmAST(ps);
            gemm_generator.codeGen(place_dir);
        }
    }
}

void GemmFamilyGenerator::printFamily(std::ostream &os)
{
    for (size_t i = 0; i < this->scheme_vec.size(); ++i)
        os << '[' << std::to_string(i) << ']' << this->scheme_vec[i].toString()
           << std::endl;
}

GemmFamilyGenerator::~GemmFamilyGenerator() { }
} // namespace loopmaster