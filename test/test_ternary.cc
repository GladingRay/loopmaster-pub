#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testTernaryExpr()
    {
        ModuleAST root;
        Expr *bool_expr = new ConstNumExpr(1);
        Expr *first_expr = new ConstNumExpr(2);
        Expr *second_expr = new ConstNumExpr(3);

        Expr *ternary = new TernaryExpr(bool_expr, first_expr, second_expr);

        root.addChild(ternary);
        root.codeGen(0, std::cout);
        BaseNode::destoryDeclVars();
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testTernaryExpr();

    return 0;
}