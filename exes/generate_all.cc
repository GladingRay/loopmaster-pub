#include "common.h"

namespace loopmaster
{
void generateAllSchemes(size_t kernel_id, const std::string &place_dir,
    bool parallel_pass_enable, bool vector_unpackc_enable, bool pre_relayout)
{
    GemmFamilyGenerator gfg(4);
    KernelConfig kernel_config(kernel_id);

    std::ofstream space_ofs("SchemeFamily.log");

    gfg.buildSchemeFamily(kernel_config, vector_unpackc_enable, pre_relayout);
    gfg.printFamily(space_ofs);
    space_ofs.close();
    gfg.generateAllSchemes(place_dir, parallel_pass_enable);
}
}

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        printf("Usage: %s kernel_id place_dir parallel_pass_enable "
               "vector_unpackc_enable pre_relayout\n",
            argv[0]);
        exit(1);
    }
    size_t kernel_id = std::atol(argv[1]);
    std::string place_dir(argv[2]);
    bool parallel_pass_enable = std::atoi(argv[3]) ? true : false;
    bool vector_unpackc_enable = std::atoi(argv[4]) ? true : false;
    bool pre_relayout = std::atoi(argv[5]) ? true : false;

    loopmaster::generateAllSchemes(
        kernel_id, place_dir, parallel_pass_enable, vector_unpackc_enable, pre_relayout);
    return 0;
}