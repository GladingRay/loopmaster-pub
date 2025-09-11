#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testSchemeInfoStr()
    {

        std::vector<Dim> kernel_order { Dim::M, Dim::N, Dim::K };
        std::vector<Dim> a_order { Dim::M, Dim::K };
        std::vector<Dim> b_order { Dim::N, Dim::K };

        SchemeInfo si(0, 4, 4, 4, PackType::NoPack, PackType::ReadContinuous,
            PackType::WriteContinuous, kernel_order, a_order, b_order, 0);

        std::cout << si.toString() << std::endl;
    }

    void testGemmFamilyGeneratorPrintSchemeInfo()
    {
        std::vector<Dim> kernel_order { Dim::M, Dim::N, Dim::K };
        std::vector<Dim> a_order { Dim::M, Dim::K };
        std::vector<Dim> b_order { Dim::N, Dim::K };
        for (int kernel_id = 0; kernel_id <= 2; kernel_id++)
        {
            std::string filename = "space" + std::to_string(kernel_id) + ".log";
            std::ofstream of(filename);

            of << "============== begin =============\n";
            const size_t max_loop_perdim = 4;
            GemmFamilyGenerator gfg(max_loop_perdim);
            KernelConfig kernel_config(kernel_id);
            gfg.buildSchemeFamily(kernel_config);
            gfg.printFamily(of);

            of << "=============== end ==============\n";

            of.close();
        }
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testGemmFamilyGeneratorPrintSchemeInfo();
    return 0;
}