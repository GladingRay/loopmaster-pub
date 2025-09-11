#include "common.h"

namespace loopmaster
{

#define CHECK_TARGET_END()                                                     \
    if (this->istarget(node_info))                                             \
        return this;                                                           \
    if (this->isend())                                                         \
        return nullptr;

#define CHECK_WALK_RET(_var)                                                   \
    if (_var)                                                                  \
        if (BaseNode *_var##_ret = _var->walkFindNode(node_info))              \
            return _var##_ret;

BaseNode *BaseNode::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *ModuleAST::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    for (BaseNode *child : this->children)
    {
        CHECK_WALK_RET(child)
    }
    return nullptr;
}

BaseNode *Expr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()
    LM_ASSERT(false, "Virtual Expr class shouldn't be used.")
    return nullptr;
}

BaseNode *ConstNumExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()
    return nullptr;
}

BaseNode *VarRef::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()
    return nullptr;
}

BaseNode *MinExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(first)
    CHECK_WALK_RET(second)
    return nullptr;
}

BaseNode *UnaryOpExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(e)
    return nullptr;
}

BaseNode *BinaryOpExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(left)
    CHECK_WALK_RET(right)
    return nullptr;
}

BaseNode *AlignedAllocExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(align_size)
    CHECK_WALK_RET(bytes)
    return nullptr;
}

BaseNode *SizeofTypeExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *MultiCalcExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    for (BaseNode *opd : this->operands)
    {
        CHECK_WALK_RET(opd)
    }

    return nullptr;
}

BaseNode *TernaryExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(bool_expr)
    CHECK_WALK_RET(first_expr)
    CHECK_WALK_RET(second_expr)

    return nullptr;
}

BaseNode *SubscriptExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(ptr_expr)
    CHECK_WALK_RET(offset_expr)

    return nullptr;
}

BaseNode *OmpGetTidExpr::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *Stmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()
    LM_ASSERT(false, "Virtual Stmt class shouldn't be used.")
    return nullptr;
}

BaseNode *VoidLineStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *CommentStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *PragmaStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *IncludeStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *NameSpaceStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    for (BaseNode *child : this->body)
    {
        CHECK_WALK_RET(child)
    }

    return nullptr;
}

BaseNode *VarDeclStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *VarDeclWithRVStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(rv)

    return nullptr;
}

BaseNode *LoopIndexDeclStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(rv)

    return nullptr;
}

BaseNode *ArgDeclStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *VarAssignStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(left)
    CHECK_WALK_RET(right)

    return nullptr;
}

BaseNode *TemplateStructFuncStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    for (BaseNode *stmt : this->targ_list)
    {
        CHECK_WALK_RET(stmt)
    }

    for (BaseNode *stmt : this->farg_list)
    {
        CHECK_WALK_RET(stmt)
    }

    for (BaseNode *stmt : this->body)
    {
        CHECK_WALK_RET(stmt)
    }

    return nullptr;
}

BaseNode *LoopStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(init)
    CHECK_WALK_RET(end_flag)
    CHECK_WALK_RET(update)

    for (BaseNode *stmt : this->loop_body)
    {
        CHECK_WALK_RET(stmt)
    }

    return nullptr;
}

BaseNode *KernelCallStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    for (BaseNode *e : this->func_args)
    {
        CHECK_WALK_RET(e)
    }

    return nullptr;
}

BaseNode *ZeroBufStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(buf_ptr)
    CHECK_WALK_RET(buf_size)

    return nullptr;
}

BaseNode *FreeBufStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(buf_ptr)

    return nullptr;
}

BaseNode *CodeBlockStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    for (BaseNode *stmt : this->stmts_in_block)
    {
        CHECK_WALK_RET(stmt)
    }

    return nullptr;
}

BaseNode *VectorCopyStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(src_ptr)
    CHECK_WALK_RET(dst_ptr)

    return nullptr;
}

BaseNode *VectorCopyAddStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(src_ptr)
    CHECK_WALK_RET(dst_ptr)
    CHECK_WALK_RET(beta_ref)

    return nullptr;
}

BaseNode *CbetaOpStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(m_len)
    CHECK_WALK_RET(n_len)
    CHECK_WALK_RET(c_ptr)
    CHECK_WALK_RET(ldc)
    CHECK_WALK_RET(beta_ref)
    CHECK_WALK_RET(tid)
    CHECK_WALK_RET(tn)

    return nullptr;
}

BaseNode *ExprToStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(expr)

    return nullptr;
}

BaseNode *OmpParallelStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(parallel_body)
    CHECK_WALK_RET(num_threads)

    return nullptr;
}

BaseNode *OmpBarrierStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *OmpParallelForStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(loop)
    CHECK_WALK_RET(num_threads)

    return nullptr;
}

BaseNode *OmpSetNestedStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    return nullptr;
}

BaseNode *OmpAtomicStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(body);

    return nullptr;
}

BaseNode *SyncVarDeclInitStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(num);

    return nullptr;
}

BaseNode *SyncOnStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(tid)
    CHECK_WALK_RET(local_tid)

    return nullptr;
}

BaseNode *ReductionCStmt::walkFindNode(NodeInfo &node_info)
{
    CHECK_TARGET_END()

    CHECK_WALK_RET(type)
    CHECK_WALK_RET(tn)
    CHECK_WALK_RET(m)
    CHECK_WALK_RET(n)
    CHECK_WALK_RET(ldc)
    CHECK_WALK_RET(cptr)
    CHECK_WALK_RET(bufcptr)
    CHECK_WALK_RET(betaref)

    return nullptr;
}

#undef CHECK_TARGET_END
#undef CHECK_WALK_RET
}