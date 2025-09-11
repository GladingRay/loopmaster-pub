#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testLoopCodegen()
    {
        VarDeclStmt end_var(DataType::I, "m");

        VarRef end(end_var.getDecledVar());

        ConstNumExpr zero(0);
        ConstNumExpr one(1);
        ConstNumExpr mc(64);

        LoopIndexDeclStmt *init
            = new LoopIndexDeclStmt(DataType::I, "i", new VarRef(DataType::I, "0"));

        BinaryOpExpr *end_expr
            = new BinaryOpExpr(OpType::Less, new VarRef(init->getDecledVar()),
                new VarRef(end_var.getDecledVar()));

        BinaryOpExpr *inc_expr = new BinaryOpExpr(OpType::AddAssign,
            new VarRef(init->getDecledVar()), new ConstNumExpr(64));

        LoopStmt loop(init, end_expr, inc_expr, { new Stmt() });

        loop.codeGen(0, std::cout);

        BaseNode::destoryDeclVars();
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testLoopCodegen();
    return 0;
}