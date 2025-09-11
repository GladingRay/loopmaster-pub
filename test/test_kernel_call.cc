#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testKernelCall()
    {
        VarDeclWithRVStmt m(DataType::I, "m", new ConstNumExpr(32));
        VarDeclWithRVStmt n(DataType::I, "n", new ConstNumExpr(8));
        VarDeclWithRVStmt k(DataType::I, "k", new ConstNumExpr(128));

        VarDeclWithRVStmt lda(DataType::I, "lda", new ConstNumExpr(16));
        VarDeclWithRVStmt ldb(DataType::I, "ldb", new ConstNumExpr(4));
        VarDeclWithRVStmt ldc(DataType::I, "ldc", new ConstNumExpr(16));

        VarDeclStmt a_ptr(DataType::TPtr, "a");
        VarDeclStmt b_ptr(DataType::TPtr, "b");
        VarDeclStmt c_ptr(DataType::TPtr, "c");

        VarDeclStmt alpha(DataType::T, "alpha");

        std::vector<BaseNode *> func_args { new VarRef(DataType::I, "m"),
            new VarRef(DataType::I, "n"), new VarRef(DataType::I, "k"),
            new VarRef(DataType::T, "alpha"), new VarRef(DataType::TPtr, "a"),
            new VarRef(DataType::I, "lda"), new VarRef(DataType::TPtr, "b"),
            new VarRef(DataType::I, "ldb"), new VarRef(DataType::TPtr, "c"),
            new VarRef(DataType::I, "ldc") };

        KernelCallStmt ker_stmt(
            "gemm_micro_kernel", 0, false, true, false, func_args);

        ker_stmt.codeGen(0, std::cout);
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testKernelCall();
    return 0;
}