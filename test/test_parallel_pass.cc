#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testParallelPASS(const std::string &place_dir)
    {
        std::vector<Dim> kernel = { Dim::M, Dim::K, Dim::N, Dim::K, Dim::N };

        std::vector<Dim> loop_order = { Dim::M, Dim::K, Dim::N, Dim::K, Dim::N,
            Dim::M, Dim::K, Dim::N, Dim::K };
        PackType packtypes[2] = { PackType::NoPack,
            PackType::WriteContinuous };

        std::vector<SchemeInfo> generate_cases;
        int scheme_id = 2;
        for (PackType &pa : packtypes)
        {
            for (PackType &pb : packtypes)
            {
                for (PackType pc : { PackType::NoPack, PackType::ReadContinuous })
                {
                    generate_cases.push_back(
                        SchemeInfo(scheme_id++, loop_order, 2, pa, pb, pc));
                }
            }
        }

        for (SchemeInfo &scheme : generate_cases)
        {
            scheme.loop_order = loop_order;

            GemmGenerator gemm_generator(scheme);
            gemm_generator.buildGemmAST();

            PASS pass(gemm_generator);

            pass.refineVectorLoop();

            ParallelScheme ps(scheme);

            pass.parallelizeGemmAST(ps);
            gemm_generator.codeGen(place_dir);
        }
    }
}
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s place_dir\n", argv[0]);
        exit(1);
    }

    std::string place_dir(argv[1]);
    loopmaster::unittest::testParallelPASS(place_dir);

    return 0;
}