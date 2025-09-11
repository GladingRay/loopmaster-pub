#include "common.h"

namespace loopmaster
{
namespace unittest
{
    void testVcopyCodeGen()
    {
        ModuleAST root;
        Stmt *dst_decl = new VarDeclStmt(DataType::TPtr, "dst_ptr");
        Stmt *src_decl = new VarDeclStmt(DataType::TPtr, "src_ptr");

        Expr *dst_ptr
            = new VarRef(dynamic_cast<VarDeclStmt *>(dst_decl)->decled_var);
        Expr *src_ptr
            = new VarRef(dynamic_cast<VarDeclStmt *>(src_decl)->decled_var);
        Stmt *vcopy_stmt = new VectorCopyStmt(dst_ptr, src_ptr);

        root.addChild(dst_decl);
        root.addChild(src_decl);
        root.addChild(vcopy_stmt);

        root.codeGen(0, std::cout);
        BaseNode::destoryDeclVars();
    }
}
}

int main(int argc, char *argv[])
{
    loopmaster::unittest::testVcopyCodeGen();

    return 0;
}