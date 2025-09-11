#include "common.h"

#include <fstream>

std::unique_ptr<loopmaster::SchemeInfo> args2scheme(int argc, char *argv[])
{
    loopmaster::PackType atype(static_cast<loopmaster::PackType>(argv[1][0]));
    loopmaster::PackType btype(static_cast<loopmaster::PackType>(argv[2][0]));
    loopmaster::PackType ctype(static_cast<loopmaster::PackType>(argv[3][0]));

    LM_ASSERT(ctype != loopmaster::PackType::WriteContinuous, "Write continuous"
            " unpackc is not support.")
    
    size_t kernel_id = std::atol(argv[4]);
    loopmaster::KernelConfig kernel(kernel_id);
    std::vector<loopmaster::Dim> loops(kernel.kernel_order);
    std::string loops_out_kernel(argv[5]);
    size_t scheme_id = std::atol(argv[6]);

    for (char loop : loops_out_kernel)
    {
        loops.push_back(static_cast<loopmaster::Dim>(loop));
    }

    return std::make_unique<loopmaster::SchemeInfo>(scheme_id, loops,
        kernel_id, atype, btype, ctype, false);
}

void logcmd(int argc, char *argv[])
{
    if (argc <= 9) return;
    std::string filename(argv[9]);
    std::ofstream logf(filename, std::ios::app);
    std::string cmd;
    for (int i = 0; i < 9; i++)
    {
        cmd += argv[i];
        cmd += (i == 9 - 1 ? "\n" : " ");
    }
    logf << cmd;
    logf.close();
}

int main(int argc, char *argv[])
{
    if (argc < 9)
    {
        printf(
            "Usage: %s atype btype ctype kernelid loops schemeid\n"
            "                 parallel_policy[0/1/2]:\n"
            "                     0: no parallel\n"
            "                     1: mn parallel\n"
            "                     2: k parallel\n"
            "                 path\n"
            "                 logfile\n",
            argv[0]);
        exit(1);
    }

    std::unique_ptr<loopmaster::SchemeInfo> scheme = args2scheme(argc, argv);
    int parallel_policy = std::atoi(argv[7]);
    bool parallel_on_k = (parallel_policy == 2 ? true : false);
    loopmaster::GemmGenerator gemm(*scheme);
    gemm.buildGemmAST();
    loopmaster::PASS pass(gemm);
    pass.refineVectorLoop();

    if (parallel_policy > 0)
    {
        loopmaster::ParallelScheme ps(*scheme, parallel_on_k);
        pass.parallelizeGemmAST(ps);
        gemm.codeGen(argv[8]);
    }
    else
    {
        gemm.codeGen(argv[8]);
    }

    logcmd(argc, argv);
    return 0;
}