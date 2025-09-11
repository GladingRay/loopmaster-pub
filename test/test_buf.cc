#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testBufAllocMemsetFree()
    {
        ModuleAST root;
        Expr *alloca_expr = new AlignedAllocExpr(
            new ConstNumExpr(128), new ConstNumExpr(1024));
        VarDeclWithRVStmt *alloca
            = new VarDeclWithRVStmt(DataType::TPtr, "buf", alloca_expr);

        Stmt *zero_buf = new ZeroBufStmt(
            new VarRef(alloca->getDecledVar()), new ConstNumExpr(1024));

        Stmt *free_buf = new FreeBufStmt(new VarRef(alloca->getDecledVar()));

        root.addChild(alloca);
        root.addChild(zero_buf);
        root.addChild(free_buf);

        root.codeGen(0, std::cout);
        BaseNode::destoryDeclVars();
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testBufAllocMemsetFree();

    return 0;
}