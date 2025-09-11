#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testWalkAST()
    {
        std::vector<Dim> loop_order { Dim::M, Dim::K, Dim::N, Dim::K, Dim::N,
            Dim::M, Dim::N, Dim::K };
        SchemeInfo scheme(0, loop_order, 2, PackType::ReadContinuous,
            PackType::WriteContinuous, PackType::ReadContinuous);

        GemmGenerator gemm_generator(scheme);

        gemm_generator.buildGemmAST();

        PASS pass(gemm_generator);

        ParallelScheme ps(scheme);

        pass.parallelizeGemmAST(ps);

        BaseNode &root = gemm_generator.getGemmASTRoot();

        NodeInfo target { NodeType::VectorCopyStmtKind, "" };
        std::vector<BaseNode *> results;
        root.walkFindAllNodes(target, results);

        for (BaseNode *bn : results)
        {
            bn->codeGen(0, std::cout);
        }
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testWalkAST();
    return 0;
}