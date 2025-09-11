#include "common.h"

namespace loopmaster
{

#define CHECK_TARGET_END()                                                     \
    if (this->istarget(node_info))                                             \
        return results.push_back(this);                                        \
    if (this->isend())                                                         \
        return;

#define CHECK_WALK_ALL(_var)                                                   \
    if (_var)                                                                  \
        _var->walkFindAllNodes(node_info, results);

void BaseNode::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()
}

void ModuleAST::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    for (BaseNode *child : this->children)
    {
        CHECK_WALK_ALL(child)
    }
}

void Expr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()
    LM_ASSERT(false, "Virtual Expr class shouldn't be used.")
    return;
}

void ConstNumExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()
    return;
}

void VarRef::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()
    return;
}

void MinExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(first)
    CHECK_WALK_ALL(second)
    return;
}

void UnaryOpExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(e)
    return;
}

void BinaryOpExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(left)
    CHECK_WALK_ALL(right)
    return;
}

void AlignedAllocExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(align_size)
    CHECK_WALK_ALL(bytes)
    return;
}

void SizeofTypeExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void MultiCalcExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    for (BaseNode *opd : this->operands)
    {
        CHECK_WALK_ALL(opd)
    }

    return;
}

void TernaryExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(bool_expr)
    CHECK_WALK_ALL(first_expr)
    CHECK_WALK_ALL(second_expr)

    return;
}

void SubscriptExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(ptr_expr)
    CHECK_WALK_ALL(offset_expr)

    return;
}

void OmpGetTidExpr::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void Stmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()
    LM_ASSERT(false, "Virtual Stmt class shouldn't be used.")
    return;
}

void VoidLineStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void CommentStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void PragmaStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void IncludeStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void NameSpaceStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    for (BaseNode *child : this->body)
    {
        CHECK_WALK_ALL(child)
    }

    return;
}

void VarDeclStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void VarDeclWithRVStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(rv)

    return;
}

void LoopIndexDeclStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(rv)

    return;
}

void ArgDeclStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void VarAssignStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(left)
    CHECK_WALK_ALL(right)

    return;
}

void TemplateStructFuncStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    for (BaseNode *stmt : this->targ_list)
    {
        CHECK_WALK_ALL(stmt)
    }

    for (BaseNode *stmt : this->farg_list)
    {
        CHECK_WALK_ALL(stmt)
    }

    for (BaseNode *stmt : this->body)
    {
        CHECK_WALK_ALL(stmt)
    }

    return;
}

void LoopStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(init)
    CHECK_WALK_ALL(end_flag)
    CHECK_WALK_ALL(update)

    for (BaseNode *stmt : this->loop_body)
    {
        CHECK_WALK_ALL(stmt)
    }

    return;
}

void KernelCallStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    for (BaseNode *e : this->func_args)
    {
        CHECK_WALK_ALL(e)
    }

    return;
}

void ZeroBufStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(buf_ptr)
    CHECK_WALK_ALL(buf_size)

    return;
}

void FreeBufStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(buf_ptr)

    return;
}

void CodeBlockStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    for (BaseNode *stmt : this->stmts_in_block)
    {
        CHECK_WALK_ALL(stmt)
    }

    return;
}

void VectorCopyStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(src_ptr)
    CHECK_WALK_ALL(dst_ptr)

    return;
}

void VectorCopyAddStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(src_ptr)
    CHECK_WALK_ALL(dst_ptr)
    CHECK_WALK_ALL(beta_ref)

    return;
}

void CbetaOpStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(m_len)
    CHECK_WALK_ALL(n_len)
    CHECK_WALK_ALL(c_ptr)
    CHECK_WALK_ALL(ldc)
    CHECK_WALK_ALL(beta_ref)
    CHECK_WALK_ALL(tid)
    CHECK_WALK_ALL(tn)

    return;
}

void ExprToStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(expr)

    return;
}

void OmpParallelStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(parallel_body)
    CHECK_WALK_ALL(num_threads)

    return;
}

void OmpBarrierStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void OmpParallelForStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(loop)
    CHECK_WALK_ALL(num_threads)

    return;
}

void OmpSetNestedStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    return;
}

void OmpAtomicStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(body)

    return;
}

void SyncVarDeclInitStmt::walkFindAllNodes(
    NodeInfo &node_info, std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(num)

    return;
}

void SyncOnStmt::walkFindAllNodes(NodeInfo &node_info,
    std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(tid)
    CHECK_WALK_ALL(local_tid)

    return;
}

void ReductionCStmt::walkFindAllNodes(NodeInfo &node_info,
    std::vector<BaseNode *> &results)
{
    CHECK_TARGET_END()

    CHECK_WALK_ALL(type)
    CHECK_WALK_ALL(tn)
    CHECK_WALK_ALL(m)
    CHECK_WALK_ALL(n)
    CHECK_WALK_ALL(ldc)
    CHECK_WALK_ALL(cptr)
    CHECK_WALK_ALL(bufcptr)
    CHECK_WALK_ALL(betaref)
}

#undef CHECK_TARGET_END
#undef CHECK_WALK_ALL
}