#include "common.h"

namespace loopmaster
{
BaseNode *BaseNode::deepcopy()
{
    return new BaseNode();
}

BaseNode *ModuleAST::deepcopy()
{
    std::vector<BaseNode *> new_ch;

    for (BaseNode *child : this->children)
    {
        new_ch.push_back(child->deepcopy());
    }

    return new ModuleAST(this->module_name, new_ch);
}

BaseNode *Expr::deepcopy() { return new Expr(*this); }

BaseNode *ConstNumExpr::deepcopy() { return new ConstNumExpr(*this); }

BaseNode *VarRef::deepcopy() { return new VarRef(*this); }

BaseNode *MinExpr::deepcopy()
{
    return new MinExpr(this->first->deepcopy(), this->second->deepcopy());
}

BaseNode *UnaryOpExpr::deepcopy()
{
    return new UnaryOpExpr(this->op, this->e->deepcopy());
}

BaseNode *BinaryOpExpr::deepcopy()
{
    return new BinaryOpExpr(
        this->op, this->left->deepcopy(), this->right->deepcopy());
}

BaseNode *AlignedAllocExpr::deepcopy()
{
    return new AlignedAllocExpr(
        this->align_size->deepcopy(), this->bytes->deepcopy());
}

BaseNode *SizeofTypeExpr::deepcopy() { return new SizeofTypeExpr(*this); }

BaseNode *MultiCalcExpr::deepcopy()
{
    std::vector<BaseNode *> new_operands;

    for (BaseNode *operand : this->operands)
    {
        new_operands.push_back(operand->deepcopy());
    }

    return new MultiCalcExpr(this->op, new_operands);
}

BaseNode *TernaryExpr::deepcopy()
{
    return new TernaryExpr(this->bool_expr->deepcopy(),
        this->first_expr->deepcopy(), this->second_expr->deepcopy());
}

BaseNode *SubscriptExpr::deepcopy()
{
    return new SubscriptExpr(
        this->ptr_expr->deepcopy(), this->offset_expr->deepcopy());
}

BaseNode *OmpGetTidExpr::deepcopy() { return new OmpGetTidExpr(*this); }

BaseNode *Stmt::deepcopy() { return new Stmt(*this); }

BaseNode *VoidLineStmt::deepcopy() { return new VoidLineStmt(*this); }

BaseNode *CommentStmt::deepcopy() { return new CommentStmt(*this); }

BaseNode *PragmaStmt::deepcopy() { return new PragmaStmt(*this); }

BaseNode *IncludeStmt::deepcopy() { return new IncludeStmt(*this); }

BaseNode *NameSpaceStmt::deepcopy()
{
    std::vector<BaseNode *> new_body;
    for (BaseNode *stmt : this->body)
    {
        new_body.push_back(stmt->deepcopy());
    }
    return new NameSpaceStmt(this->namespace_name, new_body);
}

BaseNode *VarDeclStmt::deepcopy()
{
    return new VarDeclStmt(this->type, this->var_name, this->is_constant);
}

BaseNode *VarDeclWithRVStmt::deepcopy()
{
    return new VarDeclWithRVStmt(
        this->type, this->var_name, this->rv->deepcopy(), this->is_constant);
}

BaseNode *LoopIndexDeclStmt::deepcopy()
{
    return new LoopIndexDeclStmt(
        this->type, this->var_name, this->rv->deepcopy(), this->is_constant);
}

BaseNode *ArgDeclStmt::deepcopy()
{
    return new ArgDeclStmt(this->type, this->var_name, this->is_constant);
}

BaseNode *VarAssignStmt::deepcopy()
{
    return new VarAssignStmt(this->left->deepcopy(), this->right->deepcopy());
}

BaseNode *TemplateStructFuncStmt::deepcopy()
{
    std::vector<BaseNode *> new_targs;
    std::vector<BaseNode *> new_fargs;
    std::vector<BaseNode *> new_body;

    for (BaseNode *targ : this->targ_list)
    {
        new_targs.push_back(targ->deepcopy());
    }

    for (BaseNode *farg : this->farg_list)
    {
        new_fargs.push_back(farg->deepcopy());
    }

    for (BaseNode *stmt : this->body)
    {
        new_body.push_back(stmt->deepcopy());
    }

    return new TemplateStructFuncStmt(
        this->func_name, new_targs, new_fargs, new_body);
}

BaseNode *LoopStmt::deepcopy()
{
    std::vector<BaseNode *> new_loop_body;
    for (BaseNode *stmt : this->loop_body)
    {
        new_loop_body.push_back(stmt->deepcopy());
    }

    return new LoopStmt(this->init->deepcopy(), this->end_flag->deepcopy(),
        this->update->deepcopy(), new_loop_body);
}

BaseNode *KernelCallStmt::deepcopy()
{
    std::vector<BaseNode *> new_func_args;
    for (BaseNode *arg : this->func_args)
    {
        new_func_args.push_back(arg->deepcopy());
    }
    return new KernelCallStmt(this->kernel_name, this->kernel_id,
        this->packa_flag, this->packb_flag, this->unpackc_flag, new_func_args);
}

BaseNode *ZeroBufStmt::deepcopy()
{
    return new ZeroBufStmt(
        this->buf_ptr->deepcopy(), this->buf_size->deepcopy());
}

BaseNode *FreeBufStmt::deepcopy()
{
    return new FreeBufStmt(this->buf_ptr->deepcopy());
}

BaseNode *CodeBlockStmt::deepcopy()
{
    std::vector<BaseNode *> new_body;

    for (BaseNode *stmt : this->stmts_in_block)
    {
        new_body.push_back(stmt->deepcopy());
    }
    return new CodeBlockStmt(new_body);
}

BaseNode *VectorCopyStmt::deepcopy()
{
    return new VectorCopyStmt(
        this->dst_ptr->deepcopy(), this->src_ptr->deepcopy());
}

BaseNode *VectorCopyAddStmt::deepcopy()
{
    return new VectorCopyAddStmt(this->dst_ptr->deepcopy(),
        this->src_ptr->deepcopy(), this->beta_ref->deepcopy());
}

BaseNode *CbetaOpStmt::deepcopy()
{
    return new CbetaOpStmt(this->m_len->deepcopy(), this->n_len->deepcopy(),
        this->c_ptr->deepcopy(), this->ldc->deepcopy(),
        this->beta_ref->deepcopy(), this->tid->deepcopy(),
        this->tn->deepcopy());
}

BaseNode *ExprToStmt::deepcopy()
{
    return new ExprToStmt(this->expr->deepcopy());
}

BaseNode *OmpParallelStmt::deepcopy()
{
    return new OmpParallelStmt(
        this->parallel_body->deepcopy(), this->num_threads->deepcopy());
}

BaseNode *OmpBarrierStmt::deepcopy() { return new OmpBarrierStmt(*this); }

BaseNode *OmpParallelForStmt::deepcopy()
{
    return new OmpParallelForStmt(this->loop->deepcopy(),
        this->num_threads ? this->num_threads->deepcopy() : nullptr);
}

BaseNode *OmpSetNestedStmt::deepcopy() { return new OmpSetNestedStmt(*this); }

BaseNode *OmpAtomicStmt::deepcopy()
{
    return new OmpAtomicStmt(this->body->deepcopy());
}

BaseNode *SyncVarDeclInitStmt::deepcopy()
{
    return new SyncVarDeclInitStmt(this->name, this->gn_name, this->num->deepcopy());
}

BaseNode *SyncOnStmt::deepcopy()
{
    return new SyncOnStmt(this->tid->deepcopy(), this->local_tid->deepcopy(),
        this->gn_name, this->tn_name, this->sync_name);
}

BaseNode *ReductionCStmt::deepcopy()
{
    return new ReductionCStmt(this->name, this->type->deepcopy(),
        this->tn->deepcopy(), this->m->deepcopy(), this->n->deepcopy(),
        this->ldc->deepcopy(), this->cptr->deepcopy(),
        this->bufcptr->deepcopy(), this->betaref->deepcopy());
}

}