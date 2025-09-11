#pragma once

#include <fstream>
#include <ostream>
#include <string>
#include <vector>

#include "gemm_generator.h"
#include "kernel_config.h"
#include "scheme_info.h"
#include "stmt.h"
#include "tile.h"

namespace loopmaster
{
class GemmFamilyGenerator
{
private:
    size_t total_scheme_num;
    size_t cur_id;

    std::vector<SchemeInfo> scheme_vec;

    size_t max_loop_perdim;

public:
    GemmFamilyGenerator(size_t max_loop_perdim);

    void buildSchemeFamily(KernelConfig &kernel_config,
        bool vector_unpackc_enable=false, bool pre_relayout=false);

    void dfsLoopOrder(SchemeInfo &temp_scheme, Dim pre_dim, size_t depth,
        size_t m_last, size_t n_last, size_t k_last);

    void generateAllSchemes(
        const std::string &place_dir, bool parallel_pass_enable);

    void printFamily(std::ostream &os);

    ~GemmFamilyGenerator();
};

} // namespace loopmaster
