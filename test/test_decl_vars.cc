#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testDeclVars()
    {
        BaseNode root;

        ArgDeclStmt ads(DataType::I, "BM0", true);
        ArgDeclStmt ads2(DataType::TPtr, "a_offset", false);

        ads.codeGen(0, std::cout);
        std::cout << std::endl;
        ads2.codeGen(0, std::cout);

        std::cout << std::endl
                  << BaseNode::getVar(DataType::I, "BM0")->toStr() << std::endl
                  << BaseNode::getVar(DataType::TPtr, "a_offset")->toStr()
                  << std::endl;

        BaseNode::destoryDeclVars();
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testDeclVars();

    return 0;
}