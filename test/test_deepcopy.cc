#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testDeepCopy()
    {
        std::vector<Dim> loop_order = { Dim::M, Dim::K, Dim::N, Dim::K, Dim::N,
            Dim::M, Dim::N, Dim::K };
        PackType packtypes[3] = { PackType::NoPack, PackType::ReadContinuous,
            PackType::WriteContinuous };

        std::vector<SchemeInfo> generate_cases;
        int scheme_id = 2;
        for (PackType &pa : packtypes)
        {
            for (PackType &pb : packtypes)
            {
                for (PackType pc : { PackType::ReadContinuous })
                {
                    generate_cases.push_back(
                        SchemeInfo(scheme_id++, loop_order, 2, pa, pb, pc));
                }
            }
        }

        for (SchemeInfo &scheme : generate_cases)
        {
            GemmGenerator gemm_generator(scheme);
            gemm_generator.buildGemmAST();

            PASS pass(gemm_generator);

            ParallelScheme ps(scheme);

            pass.parallelizeGemmAST(ps);

            BaseNode *copy_root = gemm_generator.getGemmASTRoot().deepcopy();
            copy_root->codeGen(0, std::cout);
            delete copy_root;
        }
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testDeepCopy();
    return 0;
}