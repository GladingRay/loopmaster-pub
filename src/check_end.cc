#include "common.h"

namespace loopmaster
{
bool BaseNode::isend() { return true; }

bool ModuleAST::isend() { return this->children.empty(); }

bool Expr::isend() { return true; }

bool ConstNumExpr::isend() { return true; }

bool VarRef::isend() { return true; }

bool MinExpr::isend()
{
    return this->first == nullptr && this->second == nullptr;
}

bool UnaryOpExpr::isend() { return this->e == nullptr; }

bool BinaryOpExpr::isend()
{
    return this->left == nullptr && this->right == nullptr;
}

bool AlignedAllocExpr::isend()
{
    return this->align_size == nullptr && this->bytes == nullptr;
}

bool SizeofTypeExpr::isend() { return true; }

bool MultiCalcExpr::isend() { return this->operands.empty(); }

bool TernaryExpr::isend()
{
    return this->bool_expr == nullptr && this->first_expr == nullptr
        && this->second_expr == nullptr;
}

bool SubscriptExpr::isend()
{
    return this->ptr_expr == nullptr && this->offset_expr == nullptr;
}

bool OmpGetTidExpr::isend() { return true; }

bool Stmt::isend() { return true; }

bool VoidLineStmt::isend() { return true; }

bool CommentStmt::isend() { return true; }

bool PragmaStmt::isend() { return true; }

bool IncludeStmt::isend() { return true; }

bool NameSpaceStmt::isend() { return this->body.empty(); }

bool VarDeclStmt::isend() { return true; }

bool VarDeclWithRVStmt::isend() { return this->rv == nullptr; }

bool LoopIndexDeclStmt::isend() { return this->rv == nullptr; }

bool ArgDeclStmt::isend() { return true; }

bool VarAssignStmt::isend()
{
    return this->left == nullptr && this->right == nullptr;
}

bool TemplateStructFuncStmt::isend()
{
    return this->targ_list.empty() && this->farg_list.empty()
        && this->body.empty();
}

bool LoopStmt::isend()
{
    return this->init == nullptr && this->end_flag == nullptr
        && this->update == nullptr && this->loop_body.empty();
}

bool KernelCallStmt::isend()
{
    return this->func_args.empty();
}

bool ZeroBufStmt::isend()
{
    return this->buf_ptr == nullptr && this->buf_size == nullptr;
}

bool FreeBufStmt::isend()
{
    return this->buf_ptr == nullptr;
}

bool CodeBlockStmt::isend()
{
    return this->stmts_in_block.empty();
}

bool VectorCopyStmt::isend()
{
    return this->src_ptr == nullptr && this->dst_ptr == nullptr;
}

bool VectorCopyAddStmt::isend()
{
    return this->src_ptr == nullptr && this->dst_ptr == nullptr
        && this->beta_ref == nullptr;
}

bool CbetaOpStmt::isend()
{
    return this->m_len == nullptr && this->n_len == nullptr
        && this->c_ptr == nullptr && this->ldc == nullptr
        && this->beta_ref == nullptr && this->tid == nullptr
        && this->tn == nullptr;
}

bool ExprToStmt::isend()
{
    return this->expr == nullptr;
}

bool OmpParallelStmt::isend()
{
    return this->parallel_body == nullptr && this->num_threads == nullptr;
}

bool OmpBarrierStmt::isend()
{
    return true;
}

bool OmpParallelForStmt::isend()
{
    return this->loop == nullptr && this->num_threads == nullptr;
}

bool OmpSetNestedStmt::isend()
{
    return true;
}

bool OmpAtomicStmt::isend()
{
    return this->body == nullptr;
}

bool SyncVarDeclInitStmt::isend()
{
    return this->num == nullptr;
}

bool SyncOnStmt::isend()
{
    return this->tid == nullptr && this->local_tid == nullptr;
}

bool ReductionCStmt::isend()
{
    return this->type == nullptr && this->tn == nullptr &&
        this->m == nullptr && this->n == nullptr && this->ldc == nullptr &&
        this->cptr == nullptr && this->bufcptr == nullptr &&
        this->betaref == nullptr;
}
}