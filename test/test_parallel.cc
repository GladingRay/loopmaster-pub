#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testParallelStmt()
    {
        std::vector<BaseNode *> loop_body { new VarDeclWithRVStmt(DataType::I,
                                            "tid", new OmpGetTidExpr()),
            new OmpBarrierStmt() };
        LoopStmt *loop = new LoopStmt(
            new LoopIndexDeclStmt(DataType::I, "idx", new VarRef(DataType::I, "0")),
            new BinaryOpExpr(OpType::Less, new VarRef(DataType::I, "idx"),
                new ConstNumExpr(16)),
            new BinaryOpExpr(OpType::AddAssign, new VarRef(DataType::I, "idx"),
                new ConstNumExpr(1)),
            loop_body);

        std::vector<BaseNode *> stmts { new OmpSetNestedStmt(true),
            new OmpParallelForStmt(loop, new ConstNumExpr(2)) };
        CodeBlockStmt codeblock(stmts);

        codeblock.codeGen(0, std::cout);

        BaseNode::destoryDeclVars();
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testParallelStmt();
    return 0;
}