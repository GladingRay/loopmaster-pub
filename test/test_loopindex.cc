#include "common.h"

namespace loopmaster
{
namespace unittest
{
    struct TestParam
    {
        size_t m_loops;
        size_t n_loops;
        size_t k_loops;

        std::vector<Dim> loops;
    };

    void testLoopIndex()
    {
        std::vector<TestParam> test_cases
            = { { 3, 3, 3,
                    { Dim::M, Dim::N, Dim::K, Dim::M, Dim::N, Dim::K, Dim::M,
                        Dim::K, Dim::N } },
                  { 3, 3, 3,
                      { Dim::M, Dim::N, Dim::K, Dim::N, Dim::M, Dim::K, Dim::N,
                          Dim::K, Dim::M } },

                  { 3, 2, 3,
                      { Dim::M, Dim::N, Dim::K, Dim::M, Dim::K, Dim::N, Dim::M,
                          Dim::K } },
                  { 2, 3, 3,
                      { Dim::M, Dim::N, Dim::K, Dim::N, Dim::K, Dim::M, Dim::N,
                          Dim::K } } };

        std::vector<Dim> kernel_order = { Dim::M, Dim::N, Dim::K };

        std::vector<Dim> a_order = { Dim::M, Dim::K };

        std::vector<Dim> b_order = { Dim::N, Dim::K };

        for (const auto &test_case : test_cases)
        {
            SchemeInfo scheme(1, test_case.m_loops, test_case.n_loops,
                test_case.k_loops, PackType::WriteContinuous,
                PackType::WriteContinuous, PackType::WriteContinuous,
                kernel_order, a_order, b_order, 0);
            scheme.loop_order = test_case.loops;

            GemmGenerator gg(scheme);

            gg.printABCLoopIndex();
        }
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testLoopIndex();
    return 0;
}