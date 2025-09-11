#include "common.h"

namespace loopmaster
{
void PASS::insertIncludeStmt(const std::string filename)
{
    std::vector<BaseNode *> &children = this->ast->getChildren();
    for (auto itr = children.begin(); itr != children.end(); itr++)
    {
        if ((*itr)->node_type == NodeType::IncludeStmtKind)
        {
            itr = children.insert(itr, new IncludeStmt(filename));
            return;
        }
    }

    for (auto itr = children.begin(); itr != children.end(); itr++)
    {
        if ((*itr)->node_type == NodeType::VoidLineStmtKind)
        {
            itr = children.insert(itr, new IncludeStmt(filename));
            return;
        }
    }
}

void PASS::refineVectorLoop()
{
    NodeInfo target { NodeType::VectorCopyStmtKind, "" };

    std::vector<BaseNode *> vec_copys;
    this->ast->walkFindAllNodes(target, vec_copys);
    target.node_type = NodeType::VectorCopyAddStmtKind;
    this->ast->walkFindAllNodes(target, vec_copys);

    for (BaseNode *stmt : vec_copys)
    {
        Stmt *vec_copy = dynamic_cast<VectorCopyStmt *>(stmt);
        if (vec_copy == nullptr)
            vec_copy = dynamic_cast<VectorCopyAddStmt *>(stmt);
        if (vec_copy == nullptr)
            return;
        BaseNode *son = vec_copy;
        while (CodeBlockStmt *bs = BaseNode::getFatherPtr<CodeBlockStmt>(son))
        {
            son = bs;
        }
        LoopStmt *loop = BaseNode::getFatherPtr<LoopStmt>(son);
        if (loop == nullptr)
            return;
        BinaryOpExpr *end_flag = dynamic_cast<BinaryOpExpr *>(loop->end_flag);
        if (end_flag == nullptr)
            return;
        BinaryOpExpr *update = dynamic_cast<BinaryOpExpr *>(loop->update);
        if (update == nullptr)
            return;
        VarRef *end_ref = dynamic_cast<VarRef *>(end_flag->right);
        if (end_ref == nullptr)
            return;
        VarRef *step_ref = dynamic_cast<VarRef *>(update->right);
        if (step_ref == nullptr)
            return;

        Var *end_var = end_ref->v;
        Var *step_var = step_ref->v;

        Expr *vec_end = new BinaryOpExpr(OpType::Minus, new VarRef(end_var),
            new BinaryOpExpr(
                OpType::Mod, new VarRef(end_var), new VarRef(step_var)));
        if (LoopStmt *tail_loop = dynamic_cast<LoopStmt *>(loop->deepcopy()))
        {
            NodeInfo vec_len_varref { NodeType::VarRefKind,
                "VecLen<T>::value" };

            std::vector<BaseNode *> vec_len_refs;
            tail_loop->walkFindAllNodes(vec_len_varref, vec_len_refs);

            for (BaseNode *node : vec_len_refs)
            {
                if (VarRef *var_ref = dynamic_cast<VarRef *>(node))
                {
                    var_ref->v = BaseNode::getVar(DataType::I, "1");
                }
            }

            loop->updateEnd(vec_end);
            tail_loop->updateInitRV(vec_end->deepcopy());

            NodeInfo vec_copy_target { NodeType::VectorCopyStmtKind, "" };

            if (VectorCopyStmt *vcopy = dynamic_cast<VectorCopyStmt *>(
                    tail_loop->walkFindNode(vec_copy_target)))
            {
                BinaryOpExpr *scalar_assign = new BinaryOpExpr(OpType::Assign,
                    new UnaryOpExpr(OpType::DeAddr, vcopy->dst_ptr->deepcopy()),
                    new UnaryOpExpr(
                        OpType::DeAddr, vcopy->src_ptr->deepcopy()));
                tail_loop->updateBodyStmt(vcopy, new ExprToStmt(scalar_assign));
            }

            vec_copy_target.node_type = NodeType::VectorCopyAddStmtKind;

            if (VectorCopyAddStmt *vcopy_add
                = dynamic_cast<VectorCopyAddStmt *>(
                    tail_loop->walkFindNode(vec_copy_target)))
            {
                BinaryOpExpr *scalar_stmt = new BinaryOpExpr(OpType::Assign,
                    new UnaryOpExpr(
                        OpType::DeAddr, vcopy_add->dst_ptr->deepcopy()),
                    new BinaryOpExpr(OpType::Add,
                        new BinaryOpExpr(OpType::Mul,
                            new UnaryOpExpr(
                                OpType::DeAddr, vcopy_add->dst_ptr->deepcopy()),
                            vcopy_add->beta_ref->deepcopy()),
                        new UnaryOpExpr(
                            OpType::DeAddr, vcopy_add->src_ptr->deepcopy())));
                tail_loop->updateBodyStmt(
                    vcopy_add, new ExprToStmt(scalar_stmt));
            }

            if (LoopStmt *loop_father = BaseNode::getFatherPtr<LoopStmt>(loop))
            {
                std::vector<BaseNode *> &body = loop_father->loop_body;
                for (auto itr = body.begin(); itr != body.end(); ++itr)
                {
                    if (*itr == loop)
                    {
                        body.insert(itr + 1, tail_loop);
                        break;
                    }
                }
                loop_father->updateSon();
            }
        }
    }
}

void PASS::insertTNTemplateArgs(const ParallelScheme &ps)
{
    NodeInfo target { NodeType::TemplateStructFuncStmtKind,
        cur_gemm.getFuncName() };
    if (TemplateStructFuncStmt *func = dynamic_cast<TemplateStructFuncStmt *>(
            this->ast->walkFindNode(target)))
    {
        for (const ParallelUnit &pu : ps.parallel_policy)
        {
            func->targ_list.push_back(new ArgDeclStmt(DataType::I, pu.tn_name));
        }
        func->updateSon();
    }
}

void PASS::insertReductionTNArg(const ParallelScheme &ps)
{
    NodeInfo target { NodeType::TemplateStructFuncStmtKind,
        cur_gemm.getFuncName() };
    if (TemplateStructFuncStmt *func = dynamic_cast<TemplateStructFuncStmt *>(
        this->ast->walkFindNode(target)))
    {
        for (const ParallelUnit &pu : ps.parallel_policy)
        {
            func->targ_list.push_back(
                new ArgDeclStmt(DataType::I, pu.tn_name));
            func->updateSon();
        }
    }
}

void PASS::removeCbetaOp(const ParallelScheme &ps)
{
    NodeInfo target { NodeType::CbetaOpStmtKind, "" };

    if (CbetaOpStmt *cbeta =
        dynamic_cast<CbetaOpStmt *>(this->ast->walkFindNode(target)))
    {
        if (CodeBlockStmt *fa = BaseNode::getFatherPtr<CodeBlockStmt>(cbeta))
        {
            std::vector<BaseNode *> &body = fa->stmts_in_block;
            for (auto itr = body.begin(); itr != body.end(); ++itr)
            {
                if (*itr == cbeta)
                {
                    delete cbeta;
                    body.erase(itr);
                    break;
                }
            }
            fa->updateSon();
        }
        else if (TemplateStructFuncStmt *fa =
                BaseNode::getFatherPtr<TemplateStructFuncStmt>(cbeta))
        {
            std::vector<BaseNode *> &body = fa->body;
            for (auto itr = body.begin(); itr != body.end(); ++itr)
            {
                if (*itr == cbeta)
                {
                    delete cbeta;
                    body.erase(itr);
                    break;
                }
            }
            fa->updateSon();
        }
    }
}

BaseNode *PASS::buildOneBufCElemNum(const ParallelScheme &ps)
{
    return new MultiCalcExpr(OpType::Mul,
        {new VarRef(DataType::I, "m"), new VarRef(DataType::I, "n")});
}

void PASS::insertReductionCBuf(const ParallelScheme &ps)
{
    NodeInfo target { NodeType::TemplateStructFuncStmtKind,
        cur_gemm.getFuncName() };
    TemplateStructFuncStmt *func =
        dynamic_cast<TemplateStructFuncStmt *>(this->ast->walkFindNode(target));
    if (func == nullptr) return;

    BaseNode *buf_size = this->buildOneBufCElemNum(ps);
    if (MultiCalcExpr *size_expr =
            dynamic_cast<MultiCalcExpr *>(buf_size))
    {
        size_expr->operands.push_back(new SizeofTypeExpr(DataType::T));
        size_expr->operands.push_back(
            new VarRef(DataType::I, ps.parallel_policy[0].tn_name));
    }
    AlignedAllocExpr *alloc = new AlignedAllocExpr(new ConstNumExpr(128), buf_size);
    VarDeclWithRVStmt *alloc_decl = new VarDeclWithRVStmt(DataType::TPtr, "buf_c", alloc);

    func->insertAtBegin(alloc_decl);
    func->insertAtEnd(new FreeBufStmt(new VarRef(alloc_decl->decled_var)));

    func->updateSon();
}

void PASS::insertOmpSetNestedStmt(bool enable)
{
    NodeInfo target { NodeType::TemplateStructFuncStmtKind,
        cur_gemm.getFuncName() };
    if (TemplateStructFuncStmt *func = dynamic_cast<TemplateStructFuncStmt *>(
            this->ast->walkFindNode(target)))
    {
        func->body.insert(func->body.begin(), new OmpSetNestedStmt(enable));
        func->updateSon();
    }
    else
    {
        LM_ASSERT(
            false, "There is a error occur in insertOmpSetNestedStmt pass")
    }
}

void PASS::insertOmpParallelForStmt(const std::string &loop_name,
    const std::string &tn_name, const std::string &tid_name)
{
    NodeInfo target { NodeType::LoopStmtKind, loop_name };

    if (LoopStmt *loop
        = dynamic_cast<LoopStmt *>(this->ast->walkFindNode(target)))
    {
        loop->loop_body.insert(loop->loop_body.begin(),
            new VarDeclWithRVStmt(DataType::I, tid_name, new OmpGetTidExpr()));
        loop->updateSon();
        if (TemplateStructFuncStmt *loop_father
            = BaseNode::getFatherPtr<TemplateStructFuncStmt>(loop))
        {
            std::vector<BaseNode *> &body = loop_father->body;
            OmpParallelForStmt *parallel_for = new OmpParallelForStmt(
                loop, new VarRef(DataType::I, tn_name));

            for (BaseNode *&stmt : body)
            {
                if (stmt == loop)
                    stmt = parallel_for;
            }
            loop_father->updateSon();
        }
        else if (LoopStmt *loop_father = BaseNode::getFatherPtr<LoopStmt>(loop))
        {
            std::vector<BaseNode *> &body = loop_father->loop_body;
            OmpParallelForStmt *parallel_for = new OmpParallelForStmt(
                loop, new VarRef(DataType::I, tn_name));

            for (BaseNode *&stmt : body)
            {
                if (stmt == loop)
                    stmt = parallel_for;
            }
            loop_father->updateSon();
        }
        else if (CodeBlockStmt *loop_father
            = BaseNode::getFatherPtr<CodeBlockStmt>(loop))
        {
            std::vector<BaseNode *> &body = loop_father->stmts_in_block;
            OmpParallelForStmt *parallel_for = new OmpParallelForStmt(
                loop, new VarRef(DataType::I, tn_name));

            for (BaseNode *&stmt : body)
            {
                if (stmt == loop)
                    stmt = parallel_for;
            }
            loop_father->updateSon();
        }
        else if (OmpParallelForStmt *loop_father
            = BaseNode::getFatherPtr<OmpParallelForStmt>(loop))
        {
            std::cout << loop_name << " loop had been parallelized.\n";
        }
    }
}

void PASS::refineParallelBufSize(const ParallelScheme &ps)
{
    NodeInfo target { NodeType::VarDeclWithRVStmtKind, "buf_a" };

    if (VarDeclWithRVStmt *decl
        = dynamic_cast<VarDeclWithRVStmt *>(this->ast->walkFindNode(target)))
    {
        if (AlignedAllocExpr *alloc
            = dynamic_cast<AlignedAllocExpr *>(decl->rv))
        {
            if (MultiCalcExpr *size
                = dynamic_cast<MultiCalcExpr *>(alloc->bytes))
            {
                for (int i = ps.parallel_policy.size() - 1;
                    i + ps.scheme.kernel_order.size() >= ps.scheme.packa_pos;
                    i--)
                {
                    size->operands.push_back(
                        new VarRef(DataType::I, ps.parallel_policy[i].tn_name));
                }
                size->updateSon();
            }
        }
    }

    target.target_name = "buf_b";

    if (VarDeclWithRVStmt *decl
        = dynamic_cast<VarDeclWithRVStmt *>(this->ast->walkFindNode(target)))
    {
        if (AlignedAllocExpr *alloc
            = dynamic_cast<AlignedAllocExpr *>(decl->rv))
        {
            if (MultiCalcExpr *size
                = dynamic_cast<MultiCalcExpr *>(alloc->bytes))
            {
                for (int i = ps.parallel_policy.size() - 1;
                    i + ps.scheme.kernel_order.size() >= ps.scheme.packb_pos;
                    i--)
                {
                    size->operands.push_back(
                        new VarRef(DataType::I, ps.parallel_policy[i].tn_name));
                }
                size->updateSon();
            }
        }
    }

    target.target_name = "buf_c";

    if (VarDeclWithRVStmt *decl
        = dynamic_cast<VarDeclWithRVStmt *>(this->ast->walkFindNode(target)))
    {
        if (AlignedAllocExpr *alloc
            = dynamic_cast<AlignedAllocExpr *>(decl->rv))
        {
            if (MultiCalcExpr *size
                = dynamic_cast<MultiCalcExpr *>(alloc->bytes))
            {
                for (int i = ps.parallel_policy.size() - 1;
                    i + ps.scheme.kernel_order.size() >= ps.scheme.unpackc_pos;
                    i--)
                {
                    size->operands.push_back(
                        new VarRef(DataType::I, ps.parallel_policy[i].tn_name));
                }
                size->updateSon();
            }
        }
    }
}

void PASS::parallelizeReductionBody(const ParallelScheme &ps)
{
    const ParallelUnit &pu = ps.parallel_policy[0];
    NodeInfo target { NodeType::LoopStmtKind, pu.idx_name };

    LoopStmt *loop = dynamic_cast<LoopStmt *>(this->ast->walkFindNode(target));
    if (loop == nullptr) return;
    
    std::vector<BaseNode *> parallel_body;
    // Declare tid var
    parallel_body.push_back(
        new VarDeclWithRVStmt(DataType::I, pu.tid_name, new OmpGetTidExpr()));
    // Declare local buf_c
    parallel_body.push_back(new VarDeclWithRVStmt(DataType::TPtr, "local_buf_c",
            new BinaryOpExpr(OpType::Add, new VarRef(DataType::TPtr, "buf_c"),
            new BinaryOpExpr(OpType::Mul, new VarRef(DataType::I, pu.tid_name),
            this->buildOneBufCElemNum(ps)))));
    // Zero local buf_c
    MultiCalcExpr *size =
        dynamic_cast<MultiCalcExpr *>(this->buildOneBufCElemNum(ps));
    size->operands.push_back(new SizeofTypeExpr(DataType::T));
    parallel_body.push_back(new ZeroBufStmt(
        new VarRef(DataType::TPtr, "local_buf_c"), size));

    NodeInfo c_ref_info { NodeType::VarRefKind, "C" };
    std::vector<BaseNode *> c_refs;
    loop->walkFindAllNodes(c_ref_info, c_refs);
    for (BaseNode *bn : c_refs)
    {
        if (VarRef *ref = dynamic_cast<VarRef *>(bn))
        {
            ref->v = BaseNode::getVar(DataType::TPtr, "local_buf_c");
        }
    }

    parallel_body.push_back(loop);

    parallel_body.push_back(new OmpBarrierStmt());

    parallel_body.push_back(new ReductionCStmt("reduction",
        new VarRef(DataType::Typename, "T"),
        new VarRef(DataType::I, pu.tn_name),
        new VarRef(DataType::I, "m"), new VarRef(DataType::I, "n"),
        new VarRef(DataType::I, "ldc"), new VarRef(DataType::TPtr, "C"),
        new VarRef(DataType::TPtr, "buf_c"),
        new VarRef(DataType::T, "beta")));

    if (TemplateStructFuncStmt *fa =
            BaseNode::getFatherPtr<TemplateStructFuncStmt>(loop))
    {
        OmpParallelStmt *ompp =
            new OmpParallelStmt(new CodeBlockStmt(parallel_body),
                                new VarRef(DataType::I, pu.tn_name));
        std::vector<BaseNode *> &body = fa->body;
        BaseNode *debug_fa = BaseNode::getFatherPtr<BaseNode>(loop);
        for (auto itr = body.begin(); itr != body.end(); ++itr)
        {
            if (*itr == loop)
            {
                *itr = ompp;
                break;
            }
        }
        fa->updateSon();
    }
    else if (CodeBlockStmt *fa = BaseNode::getFatherPtr<CodeBlockStmt>(loop))
    {
        OmpParallelStmt *ompp =
            new OmpParallelStmt(new CodeBlockStmt(parallel_body),
                                new VarRef(DataType::I, pu.tn_name));
        std::vector<BaseNode *> &body = fa->stmts_in_block;
        for (auto itr = body.begin(); itr != body.end(); ++itr)
        {
            if (*itr == loop)
            {
                *itr = ompp;
                break;
            }
        }
        fa->updateSon();
    }

    this->parallelizeForLoop(pu.idx_name, pu.tn_name, pu.tid_name, ps);
}

void PASS::insertGetTidForParallelLoop(const ParallelScheme &ps) { }
/*
    0 1 2 | 3 4 5 6 7 |
    packapos = 5
    kernelsize = 3
*/

void PASS::refineParallelBufPtr(const ParallelScheme &ps)
{
    NodeInfo target { NodeType::VarDeclWithRVStmtKind, "packa_dst" };
    const SchemeInfo &scheme = ps.scheme;
    const std::vector<ParallelUnit> &policy = ps.parallel_policy;

    BaseNode *except_bufa = nullptr;

    if (VarDeclWithRVStmt *packa_dst_decl
        = dynamic_cast<VarDeclWithRVStmt *>(this->ast->walkFindNode(target)))
    {
        if (CodeBlockStmt *packa_block
            = BaseNode::getFatherPtr<CodeBlockStmt>(packa_dst_decl))
        {
            std::vector<BaseNode *> &body = packa_block->stmts_in_block;
            for (auto itr = body.begin(); itr != body.end(); itr++)
            {
                if (*itr == packa_dst_decl)
                {
                    std::vector<BaseNode *> add_ops;
                    int idx_in_policy
                        = scheme.packa_pos - scheme.kernel_order.size();
                    for (int i = idx_in_policy; i < policy.size(); i++)
                    {
                        std::vector<BaseNode *> mul_ops;
                        mul_ops.push_back(
                            new VarRef(DataType::I, policy[i].tid_name));
                        for (int j = idx_in_policy; j < i; j++)
                        {
                            mul_ops.push_back(
                                new VarRef(DataType::I, policy[j].tn_name));
                        }
                        add_ops.push_back(
                            new MultiCalcExpr(OpType::Mul, mul_ops));
                    }
                    BaseNode *offset = new BinaryOpExpr(OpType::Mul,
                        new MultiCalcExpr(OpType::Add, add_ops),
                        this->cur_gemm.buildBufAElemNumExpr());
                    except_bufa = new VarRef(DataType::TPtr, "buf_a");
                    BaseNode *local_bufa_expr
                        = new BinaryOpExpr(OpType::Add, except_bufa, offset);

                    VarDeclWithRVStmt *local_bufa_decl = new VarDeclWithRVStmt(
                        DataType::TPtr, "local_buf_a", local_bufa_expr);
                    body.insert(itr, local_bufa_decl);
                    break;
                }
            }
        }
    }

    target.target_name = "packb_dst";

    BaseNode *except_bufb = nullptr;

    if (VarDeclWithRVStmt *packb_dst_decl
        = dynamic_cast<VarDeclWithRVStmt *>(this->ast->walkFindNode(target)))
    {
        if (CodeBlockStmt *packb_block
            = BaseNode::getFatherPtr<CodeBlockStmt>(packb_dst_decl))
        {
            std::vector<BaseNode *> &body = packb_block->stmts_in_block;
            for (auto itr = body.begin(); itr != body.end(); itr++)
            {
                if (*itr == packb_dst_decl)
                {
                    std::vector<BaseNode *> add_ops;
                    int idx_in_policy
                        = scheme.packb_pos - scheme.kernel_order.size();
                    for (int i = idx_in_policy; i < policy.size(); i++)
                    {
                        std::vector<BaseNode *> mul_ops;
                        mul_ops.push_back(
                            new VarRef(DataType::I, policy[i].tid_name));
                        for (int j = idx_in_policy; j < i; j++)
                        {
                            mul_ops.push_back(
                                new VarRef(DataType::I, policy[j].tn_name));
                        }
                        add_ops.push_back(
                            new MultiCalcExpr(OpType::Mul, mul_ops));
                    }
                    BaseNode *offset = new BinaryOpExpr(OpType::Mul,
                        new MultiCalcExpr(OpType::Add, add_ops),
                        this->cur_gemm.buildBufBElemNumExpr());
                    except_bufb = new VarRef(DataType::TPtr, "buf_b");
                    BaseNode *local_bufb_expr
                        = new BinaryOpExpr(OpType::Add, except_bufb, offset);

                    VarDeclWithRVStmt *local_bufb_decl = new VarDeclWithRVStmt(
                        DataType::TPtr, "local_buf_b", local_bufb_expr);
                    body.insert(itr, local_bufb_decl);
                    break;
                }
            }
        }
    }

    std::vector<std::string> depend_bufc_names { "ker_cptr", "unpackc_src" };
    std::vector<std::string> depend_bufa_names { "ker_aptr", "packa_dst" };
    std::vector<std::string> depend_bufb_names { "ker_bptr", "packb_dst" };

    std::vector<std::pair<std::vector<std::string>, std::string>> replace_tasks;

    replace_tasks.push_back({ depend_bufa_names, "buf_a" });
    replace_tasks.push_back({ depend_bufb_names, "buf_b" });
    replace_tasks.push_back({ depend_bufc_names, "buf_c" });

    for (auto &p : replace_tasks)
    {
        for (std::string &name : p.first)
        {
            NodeInfo target { NodeType::VarDeclWithRVStmtKind, name };
            if (VarDeclWithRVStmt *decl = dynamic_cast<VarDeclWithRVStmt *>(
                    this->ast->walkFindNode(target)))
            {
                NodeInfo bufref_info { NodeType::VarRefKind, p.second };
                if (VarRef *ref
                    = dynamic_cast<VarRef *>(decl->walkFindNode(bufref_info)))
                {
                    ref->v
                        = BaseNode::getVar(DataType::TPtr, "local_" + p.second);
                }
            }
        }
    }
}

void PASS::refineParallelBufPtrBak(const ParallelScheme &ps)
{
    NodeInfo target { NodeType::VarRefKind, "buf_a" };

    const SchemeInfo &scheme = ps.scheme;
    const std::vector<ParallelUnit> &policy = ps.parallel_policy;

    if (VarRef *bufa_ref
        = dynamic_cast<VarRef *>(this->ast->walkFindNode(target)))
    {
        if (VarDeclWithRVStmt *bufa_ref_fa
            = BaseNode::getFatherPtr<VarDeclWithRVStmt>(bufa_ref))
        {
            std::vector<BaseNode *> add_ops;
            int idx_in_policy = scheme.packa_pos - scheme.kernel_order.size();
            for (int i = idx_in_policy; i < policy.size(); i++)
            {
                std::vector<BaseNode *> mul_ops;
                mul_ops.push_back(new VarRef(DataType::I, policy[i].tid_name));
                for (int j = idx_in_policy; j < i; j++)
                {
                    mul_ops.push_back(
                        new VarRef(DataType::I, policy[j].tn_name));
                }
                add_ops.push_back(new MultiCalcExpr(OpType::Mul, mul_ops));
            }
            BaseNode *offset = new BinaryOpExpr(OpType::Mul,
                new MultiCalcExpr(OpType::Add, add_ops),
                this->cur_gemm.buildBufAElemNumExpr());

            bufa_ref_fa->updateRV(new BinaryOpExpr(
                OpType::Add, new VarRef(DataType::TPtr, "buf_a"), offset));
        }
    }

    target.target_name = "buf_b";

    if (VarRef *bufb_ref
        = dynamic_cast<VarRef *>(this->ast->walkFindNode(target)))
    {
        if (VarDeclWithRVStmt *bufb_ref_fa
            = BaseNode::getFatherPtr<VarDeclWithRVStmt>(bufb_ref))
        {
            std::vector<BaseNode *> add_ops;
            int idx_in_policy = scheme.packb_pos - scheme.kernel_order.size();
            for (int i = idx_in_policy; i < policy.size(); i++)
            {
                std::vector<BaseNode *> mul_ops;
                mul_ops.push_back(new VarRef(DataType::I, policy[i].tid_name));
                for (int j = idx_in_policy; j < i; j++)
                {
                    mul_ops.push_back(
                        new VarRef(DataType::I, policy[j].tn_name));
                }
                add_ops.push_back(new MultiCalcExpr(OpType::Mul, mul_ops));
            }
            BaseNode *offset = new BinaryOpExpr(OpType::Mul,
                new MultiCalcExpr(OpType::Add, add_ops),
                this->cur_gemm.buildBufBElemNumExpr());

            bufb_ref_fa->updateRV(new BinaryOpExpr(
                OpType::Add, new VarRef(DataType::TPtr, "buf_b"), offset));
        }
    }

    NodeInfo memsetc_info { NodeType::ZeroBufStmtKind, "" };

    if (ZeroBufStmt *memsetc
        = dynamic_cast<ZeroBufStmt *>(this->ast->walkFindNode(memsetc_info)))
    {
        int idx_in_policy = scheme.unpackc_pos - scheme.kernel_order.size();
        std::vector<BaseNode *> add_ops;
        for (int i = idx_in_policy; i < policy.size(); i++)
        {
            std::vector<BaseNode *> mul_ops;
            mul_ops.push_back(new VarRef(DataType::I, policy[i].tid_name));
            for (int j = idx_in_policy; j < i; j++)
            {
                mul_ops.push_back(new VarRef(DataType::I, policy[j].tn_name));
            }
            add_ops.push_back(new MultiCalcExpr(OpType::Mul, mul_ops));
        }
        BaseNode *offset = new BinaryOpExpr(OpType::Mul,
            new MultiCalcExpr(OpType::Add, add_ops),
            this->cur_gemm.buildBufCElemNumExpr());

        memsetc->updateBufPtr(new BinaryOpExpr(
            OpType::Add, new VarRef(DataType::TPtr, "buf_c"), offset));
    }

    target.target_name = "buf_c";

    if (VarRef *bufc_ref
        = dynamic_cast<VarRef *>(this->ast->walkFindNode(target)))
    {
        if (VarDeclWithRVStmt *bufc_ref_fa
            = BaseNode::getFatherPtr<VarDeclWithRVStmt>(bufc_ref))
        {
            std::vector<BaseNode *> add_ops;
            int idx_in_policy = scheme.unpackc_pos - scheme.kernel_order.size();
            for (int i = idx_in_policy; i < policy.size(); i++)
            {
                std::vector<BaseNode *> mul_ops;
                mul_ops.push_back(new VarRef(DataType::I, policy[i].tid_name));
                for (int j = idx_in_policy; j < i; j++)
                {
                    mul_ops.push_back(
                        new VarRef(DataType::I, policy[j].tn_name));
                }
                add_ops.push_back(new MultiCalcExpr(OpType::Mul, mul_ops));
            }
            BaseNode *offset = new BinaryOpExpr(OpType::Mul,
                new MultiCalcExpr(OpType::Add, add_ops),
                this->cur_gemm.buildBufCElemNumExpr());

            bufc_ref_fa->updateRV(new BinaryOpExpr(
                OpType::Add, new VarRef(DataType::TPtr, "buf_c"), offset));
        }
    }
}

void PASS::insertTotalNumThreads(const ParallelScheme &ps)
{
    NodeInfo func_info { NodeType::TemplateStructFuncStmtKind,
        this->cur_gemm.getFuncName() };

    if (TemplateStructFuncStmt *func = dynamic_cast<TemplateStructFuncStmt *>(
            this->ast->walkFindNode(func_info)))
    {
        std::vector<BaseNode *> &body = func->body;
        std::vector<BaseNode *> stmts;

        std::vector<BaseNode *> tn_refs;
        tn_refs.push_back(new ConstNumExpr(1));
        for (const ParallelUnit &pu : ps.parallel_policy)
        {
            tn_refs.push_back(new VarRef(DataType::I, pu.tn_name));
        }
        BaseNode *total_num_rv = new MultiCalcExpr(OpType::Mul, tn_refs);
        stmts.push_back(new VarDeclWithRVStmt(
            DataType::I, "NUM_THREADS", total_num_rv, true));

        if (ps.scheme.packa == PackType::WriteContinuous)
        {
            std::vector<BaseNode *> tn_refs { new ConstNumExpr(1) };
            for (const std::string &name : ps.packa_tn_names)
            {
                tn_refs.push_back(new VarRef(DataType::I, name));
            }
            BaseNode *packa_tn_rv = new MultiCalcExpr(OpType::Mul, tn_refs);
            stmts.push_back(new VarDeclWithRVStmt(
                DataType::I, "PACKA_TN", packa_tn_rv, true));
            stmts.push_back(new SyncVarDeclInitStmt(
                "PACKA_SYNC", "PACKA_GN", new VarRef(DataType::I, "PACKA_TN")));
        }

        if (ps.scheme.packb == PackType::WriteContinuous)
        {
            std::vector<BaseNode *> tn_refs { new ConstNumExpr(1) };
            for (const std::string &name : ps.packb_tn_names)
            {
                tn_refs.push_back(new VarRef(DataType::I, name));
            }
            BaseNode *packb_tn_rv = new MultiCalcExpr(OpType::Mul, tn_refs);
            stmts.push_back(new VarDeclWithRVStmt(
                DataType::I, "PACKB_TN", packb_tn_rv, true));
            stmts.push_back(new SyncVarDeclInitStmt(
                "PACKB_SYNC", "PACKB_GN", new VarRef(DataType::I, "PACKB_TN")));
        }

        if (ps.scheme.unpackc == PackType::ReadContinuous)
        {
            std::vector<BaseNode *> tn_refs { new ConstNumExpr(1) };
            for (const std::string &name : ps.unpackc_tn_names)
            {
                tn_refs.push_back(new VarRef(DataType::I, name));
            }
            BaseNode *unpackc_tn_rv = new MultiCalcExpr(OpType::Mul, tn_refs);
            stmts.push_back(new VarDeclWithRVStmt(
                DataType::I, "UNPACKC_TN", unpackc_tn_rv, true));
            stmts.push_back(new SyncVarDeclInitStmt("UNPACKC_SYNC",
                "UNPACKC_GN", new VarRef(DataType::I, "UNPACKC_TN")));
        }

        if (!stmts.empty())
            body.insert(body.begin(), stmts.begin(), stmts.end());
    }
}

BaseNode *PASS::declAllTids(const ParallelScheme &ps)
{
    const std::vector<ParallelUnit> &pus = ps.parallel_policy;
    std::vector<BaseNode *> ret;

    for (int i = 0; i < pus.size(); i++)
    {
        std::vector<BaseNode *> pre_tns;
        for (int j = 0; j < i; j++)
        {
            pre_tns.push_back(new VarRef(DataType::I, pus[j].tn_name));
        }
        BaseNode *tid_div_stride = nullptr;
        if (pre_tns.empty())
        {
            tid_div_stride = new VarRef(DataType::I, "tid");
        }
        else
        {
            BaseNode *stride = new MultiCalcExpr(OpType::Mul, pre_tns);
            tid_div_stride = new BinaryOpExpr(
                OpType::Div, new VarRef(DataType::I, "tid"), stride);
        }

        BaseNode *decl_rv = new BinaryOpExpr(OpType::Mod, tid_div_stride,
            new VarRef(DataType::I, pus[i].tn_name));

        BaseNode *decl_stmt
            = new VarDeclWithRVStmt(DataType::I, pus[i].tid_name, decl_rv);

        ret.push_back(decl_stmt);
    }
    if (ps.scheme.packa == PackType::WriteContinuous)
    {
        BaseNode *packa_tid_decl
            = new VarDeclWithRVStmt(DataType::I, ps.packa_policy.tid_name,
                new BinaryOpExpr(OpType::Mod, new VarRef(DataType::I, "tid"),
                    new VarRef(DataType::I, ps.packa_policy.tn_name)));
        ret.push_back(packa_tid_decl);
    }
    if (ps.scheme.packb == PackType::WriteContinuous)
    {
        BaseNode *packb_tid_decl
            = new VarDeclWithRVStmt(DataType::I, ps.packb_policy.tid_name,
                new BinaryOpExpr(OpType::Mod, new VarRef(DataType::I, "tid"),
                    new VarRef(DataType::I, ps.packb_policy.tn_name)));
        ret.push_back(packb_tid_decl);
    }
    if (ps.scheme.unpackc == PackType::ReadContinuous)
    {
        BaseNode *unpackc_tid_decl
            = new VarDeclWithRVStmt(DataType::I, ps.unpackc_policy.tid_name,
                new BinaryOpExpr(OpType::Mod, new VarRef(DataType::I, "tid"),
                    new VarRef(DataType::I, ps.unpackc_policy.tn_name)));
        ret.push_back(unpackc_tid_decl);
    }

    return new CodeBlockStmt(ret);
}

BaseNode *PASS::buildCeilDivExpr(Var *len, Var *step)
{
    BaseNode *div_expr
        = new BinaryOpExpr(OpType::Div, new VarRef(len), new VarRef(step));
    BaseNode *flag_expr = new BinaryOpExpr(OpType::G,
        new BinaryOpExpr(OpType::Mod, new VarRef(len), new VarRef(step)),
        new ConstNumExpr(0));
    BaseNode *tail_expr
        = new TernaryExpr(flag_expr, new ConstNumExpr(1), new ConstNumExpr(0));
    BaseNode *ret = new BinaryOpExpr(OpType::Add, div_expr, tail_expr);
    return ret;
}

BaseNode *PASS::buildLocalStartRV(
    Var *tid, Var *step, Var *iter_pt, Var *iter_tail, Var *init)
{
    BaseNode *pt1 = new BinaryOpExpr(
        OpType::Add, new VarRef(iter_pt), new ConstNumExpr(1));
    BaseNode *flag
        = new BinaryOpExpr(OpType::LE, new VarRef(tid), new VarRef(iter_tail));

    BaseNode *tidxpt1 = new BinaryOpExpr(OpType::Mul, new VarRef(tid), pt1);
    BaseNode *tailxpt1
        = new BinaryOpExpr(OpType::Mul, new VarRef(iter_tail), pt1->deepcopy());
    BaseNode *tid_sub_tail = new BinaryOpExpr(
        OpType::Minus, new VarRef(tid), new VarRef(iter_tail));

    BaseNode *tid_sub_tailxpt
        = new BinaryOpExpr(OpType::Mul, tid_sub_tail, new VarRef(iter_pt));
    BaseNode *iter_num = new TernaryExpr(flag, tidxpt1,
        new BinaryOpExpr(OpType::Add, tailxpt1, tid_sub_tailxpt));

    BaseNode *ret = new BinaryOpExpr(OpType::Add,
        (new BinaryOpExpr(OpType::Mul, iter_num, new VarRef(step))),
        new VarRef(init));

    return ret;
}

void PASS::parallelizeForLoop(const std::string &loop_name,
    const std::string &tn_name, const std::string &tid_name,
    const ParallelScheme &ps)
{
    NodeInfo loop_info { NodeType::LoopStmtKind, loop_name };

    bool ispacka = loop_name.substr(0, 5) == "packa";
    bool ispackb = loop_name.substr(0, 5) == "packb";
    bool isunpackc = loop_name.substr(0, 7) == "unpackc";

    if (LoopStmt *loop
        = dynamic_cast<LoopStmt *>(this->ast->walkFindNode(loop_info)))
    {
        LoopIndexDeclStmt *init_decl
            = dynamic_cast<LoopIndexDeclStmt *>(loop->init);
        if (init_decl == nullptr)
            return;
        BinaryOpExpr *end_flag_expr
            = dynamic_cast<BinaryOpExpr *>(loop->end_flag);
        if (end_flag_expr == nullptr)
            return;
        BinaryOpExpr *update_expr = dynamic_cast<BinaryOpExpr *>(loop->update);
        if (update_expr == nullptr)
            return;

        VarRef *init_ref = dynamic_cast<VarRef *>(init_decl->rv);
        if (init_ref == nullptr)
            return;
        Var *init_var = init_ref->v;

        VarRef *end_ref = dynamic_cast<VarRef *>(end_flag_expr->right);
        if (end_ref == nullptr)
            return;
        Var *end_var = end_ref->v;

        VarRef *step_ref = dynamic_cast<VarRef *>(update_expr->right);
        if (step_ref == nullptr)
            return;
        Var *step_var = step_ref->v;

        Var *tid = BaseNode::getVar(DataType::I, tid_name);

        VarDeclWithRVStmt *loop_len_decl
            = new VarDeclWithRVStmt(DataType::I, loop_name + "_len",
                new BinaryOpExpr(
                    OpType::Minus, new VarRef(end_var), new VarRef(init_var)));
        Var *len_var = loop_len_decl->decled_var;

        VarDeclWithRVStmt *iter_num_decl = new VarDeclWithRVStmt(DataType::I,
            loop_name + "_iter_num", PASS::buildCeilDivExpr(len_var, step_var));
        VarDeclWithRVStmt *iter_num_pt_decl = new VarDeclWithRVStmt(DataType::I,
            loop_name + "_iter_num_pt",
            new BinaryOpExpr(OpType::Div, new VarRef(iter_num_decl->decled_var),
                new VarRef(DataType::I, tn_name)));
        VarDeclWithRVStmt *iter_num_tail_decl = new VarDeclWithRVStmt(
            DataType::I, loop_name + "_iter_num_tail",
            new BinaryOpExpr(OpType::Mod, new VarRef(iter_num_decl->decled_var),
                new VarRef(DataType::I, tn_name)));
        // TODO:
        VarDeclWithRVStmt *next_tid
            = new VarDeclWithRVStmt(DataType::I, "next_" + tid_name,
                new BinaryOpExpr(
                    OpType::Add, new VarRef(tid), new ConstNumExpr(1)));

        VarDeclWithRVStmt *local_start_decl = new VarDeclWithRVStmt(DataType::I,
            loop_name + "_local_start",
            this->buildLocalStartRV(tid, step_var, iter_num_pt_decl->decled_var,
                iter_num_tail_decl->decled_var, init_var));

        VarDeclWithRVStmt *local_end_decl
            = new VarDeclWithRVStmt(DataType::I, loop_name + "_local_end",
                new MinExpr(new VarRef(end_var),
                    this->buildLocalStartRV(next_tid->decled_var, step_var,
                        iter_num_pt_decl->decled_var,
                        iter_num_tail_decl->decled_var, init_var)));
        std::vector<BaseNode *> stmts { loop_len_decl, iter_num_decl,
            iter_num_pt_decl, iter_num_tail_decl, next_tid, local_start_decl,
            local_end_decl };
        init_ref->v = local_start_decl->decled_var;
        end_ref->v = local_end_decl->decled_var;
        if (LoopStmt *loop_father = BaseNode::getFatherPtr<LoopStmt>(loop))
        {
            std::vector<BaseNode *> &body = loop_father->loop_body;
            for (auto itr = body.begin(); itr != body.end(); itr++)
            {
                if ((*itr) == loop)
                {
                    if (ispacka)
                    {
                        stmts.push_back(
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "PACKA_GN",
                                tn_name, "PACKA_SYNC"));
                    }
                    else if (ispackb)
                    {
                        stmts.push_back(
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "PACKB_GN",
                                tn_name, "PACKB_SYNC"));
                    }
                    else if (isunpackc)
                    {
                        stmts.push_back(
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "UNPACKC_GN",
                                tn_name, "UNPACKC_SYNC"));
                    }
                    itr = body.insert(itr, stmts.begin(), stmts.end());

                    if (ispacka)
                    {
                        itr = body.insert(itr + stmts.size() + 1,
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "PACKA_GN",
                                tn_name, "PACKA_SYNC"));
                    }
                    else if (ispackb)
                    {
                        itr = body.insert(itr + stmts.size() + 1,
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "PACKB_GN",
                                tn_name, "PACKB_SYNC"));
                    }
                    else if (isunpackc)
                    {
                        itr = body.insert(itr + stmts.size() + 1,
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "UNPACKC_GN",
                                tn_name, "UNPACKC_SYNC"));
                    }
                    break;
                }
            }
            loop_father->updateSon();
        }
        else if (CodeBlockStmt *loop_father
            = BaseNode::getFatherPtr<CodeBlockStmt>(loop))
        {
            std::vector<BaseNode *> &body = loop_father->stmts_in_block;
            for (auto itr = body.begin(); itr != body.end(); itr++)
            {
                if ((*itr) == loop)
                {
                    if (ispacka)
                    {
                        stmts.push_back(
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "PACKA_GN",
                                tn_name, "PACKA_SYNC"));
                    }
                    else if (ispackb)
                    {
                        stmts.push_back(
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "PACKB_GN",
                                tn_name, "PACKB_SYNC"));
                    }
                    else if (isunpackc)
                    {
                        stmts.push_back(
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "UNPACKC_GN",
                                tn_name, "UNPACKC_SYNC"));
                    }
                    itr = body.insert(itr, stmts.begin(), stmts.end());

                    if (ispacka)
                    {
                        itr = body.insert(itr + stmts.size() + 1,
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "PACKA_GN",
                                tn_name, "PACKA_SYNC"));
                    }
                    else if (ispackb)
                    {
                        itr = body.insert(itr + stmts.size() + 1,
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "PACKB_GN",
                                tn_name, "PACKB_SYNC"));
                    }
                    else if (isunpackc)
                    {
                        itr = body.insert(itr + stmts.size() + 1,
                            new SyncOnStmt(new VarRef(DataType::I, "tid"),
                                new VarRef(DataType::I, tid_name), "UNPACKC_GN",
                                tn_name, "UNPACKC_SYNC"));
                    }
                    break;
                }
            }
            loop_father->updateSon();
        }
        if (ispacka)
            this->insertParallelPtrOff(loop_name, tid_name, "packa_dst",
                ps.packa_pre_len, init_var->name);
        if (ispackb)
            this->insertParallelPtrOff(loop_name, tid_name, "packb_dst",
                ps.packb_pre_len, init_var->name);
    }
}

VarDeclWithRVStmt *PASS::declLocalBufC(const ParallelScheme &ps)
{
    const std::vector<ParallelUnit> &policy = ps.parallel_policy;
    const SchemeInfo &scheme = ps.scheme;
    std::vector<BaseNode *> add_ops;
    int idx_in_policy = scheme.unpackc_pos - scheme.kernel_order.size();
    for (int i = idx_in_policy; i < policy.size(); i++)
    {
        std::vector<BaseNode *> mul_ops;
        mul_ops.push_back(new VarRef(DataType::I, policy[i].tid_name));
        for (int j = idx_in_policy; j < i; j++)
        {
            mul_ops.push_back(new VarRef(DataType::I, policy[j].tn_name));
        }
        add_ops.push_back(new MultiCalcExpr(OpType::Mul, mul_ops));
    }
    BaseNode *thread_off = add_ops.empty()
        ? static_cast<BaseNode *>(new ConstNumExpr(0))
        : new MultiCalcExpr(OpType::Add, add_ops);
    BaseNode *offset = new BinaryOpExpr(
        OpType::Mul, thread_off, this->cur_gemm.buildBufCElemNumExpr());

    BaseNode *except_bufc = new VarRef(DataType::TPtr, "buf_c");
    BaseNode *local_bufc_expr
        = new BinaryOpExpr(OpType::Add, except_bufc, offset);
    VarDeclWithRVStmt *local_bufc_decl
        = new VarDeclWithRVStmt(DataType::TPtr, "local_buf_c", local_bufc_expr);
    return local_bufc_decl;
}

void PASS::parallelizeCBetaOp(const ParallelScheme &ps)
{
    NodeInfo cbeta_info { NodeType::CbetaOpStmtKind, "" };

    CbetaOpStmt *cbeta
        = dynamic_cast<CbetaOpStmt *>(this->ast->walkFindNode(cbeta_info));
    if (cbeta == nullptr)
        return;
    if (VarRef *tid = dynamic_cast<VarRef *>(cbeta->tid))
    {
        tid->v = BaseNode::getVar(DataType::I, "tid");
    }

    if (VarRef *tn = dynamic_cast<VarRef *>(cbeta->tn))
    {
        tn->v = BaseNode::getVar(DataType::I, "NUM_THREADS");
    }

    if (CodeBlockStmt *fa = BaseNode::getFatherPtr<CodeBlockStmt>(cbeta))
    {
        std::vector<BaseNode *> &body = fa->stmts_in_block;

        for (auto itr = body.begin(); itr != body.end(); itr++)
        {
            if ((*itr) == cbeta)
            {
                itr = body.insert(itr + 1, new OmpBarrierStmt());
                break;
            }
        }
        fa->updateSon();
    }
    else if (TemplateStructFuncStmt *fa =
        BaseNode::getFatherPtr<TemplateStructFuncStmt>(cbeta))
    {
        std::vector<BaseNode *> &body = fa->body;

        for (auto itr = body.begin(); itr != body.end(); itr++)
        {
            if ((*itr) == cbeta)
            {
                itr = body.insert(itr + 1, new OmpBarrierStmt());
                break;
            }
        }
        fa->updateSon();
    }
}

void PASS::parallelizeMemset(const ParallelScheme &ps)
{
    const std::string &tn_name = ps.unpackc_policy.tn_name;
    const std::string &tid_name = ps.unpackc_policy.tid_name;
    NodeInfo memset_info { NodeType::ZeroBufStmtKind, "" };

    ZeroBufStmt *memset_c_stmt
        = dynamic_cast<ZeroBufStmt *>(this->ast->walkFindNode(memset_info));
    if (memset_c_stmt == nullptr)
        return;

    VarDeclWithRVStmt *elem_num_left_decl = new VarDeclWithRVStmt(DataType::I,
        "buf_c_elem_num_left",
        new BinaryOpExpr(OpType::Mod, new VarRef(DataType::I, "buf_c_elem_num"),
            new VarRef(DataType::I, tn_name)));
    VarDeclWithRVStmt *elem_num_pt_decl = new VarDeclWithRVStmt(DataType::I,
        "buf_c_elem_num_pt",
        new BinaryOpExpr(OpType::Div, new VarRef(DataType::I, "buf_c_elem_num"),
            new VarRef(DataType::I, tn_name)));
    BaseNode *tail_flag
        = new BinaryOpExpr(OpType::EQ, new VarRef(DataType::I, tid_name),
            new BinaryOpExpr(OpType::Minus, new VarRef(DataType::I, tn_name),
                new ConstNumExpr(1)));
    BaseNode *choose_expr = new TernaryExpr(tail_flag,
        new BinaryOpExpr(OpType::Add, new VarRef(elem_num_pt_decl->decled_var),
            new VarRef(elem_num_left_decl->decled_var)),
        new VarRef(elem_num_pt_decl->decled_var));
    VarDeclWithRVStmt *local_buf_c_elem_num = new VarDeclWithRVStmt(
        DataType::I, "local_buf_c_elem_num", choose_expr);

    VarDeclWithRVStmt *local_buf_c_decl = this->declLocalBufC(ps);

    VarDeclWithRVStmt *local_buf_c_memset_decl = new VarDeclWithRVStmt(
        DataType::TPtr, "local_buf_c_memset",
        new BinaryOpExpr(OpType::Add, new VarRef(local_buf_c_decl->decled_var),
            new BinaryOpExpr(OpType::Mul, new VarRef(DataType::I, tid_name),
                new VarRef(elem_num_pt_decl->decled_var))));
    std::vector<BaseNode *> parallel_stmts { elem_num_left_decl,
        elem_num_pt_decl, local_buf_c_elem_num, local_buf_c_decl,
        local_buf_c_memset_decl };
    if (VarRef *buf_ptr = dynamic_cast<VarRef *>(memset_c_stmt->buf_ptr))
    {
        buf_ptr->v = local_buf_c_memset_decl->decled_var;
    }

    if (BinaryOpExpr *size
        = dynamic_cast<BinaryOpExpr *>(memset_c_stmt->buf_size))
    {
        if (VarRef *elem_num = dynamic_cast<VarRef *>(size->left))
        {
            elem_num->v = local_buf_c_elem_num->decled_var;
        }
    }

    if (LoopStmt *loop = BaseNode::getFatherPtr<LoopStmt>(memset_c_stmt))
    {
        std::vector<BaseNode *> &body = loop->loop_body;

        for (auto itr = body.begin(); itr != body.end(); itr++)
        {
            if (*itr == memset_c_stmt)
            {
                itr = body.insert(
                    itr, parallel_stmts.begin(), parallel_stmts.end());

                body.insert(itr + parallel_stmts.size() + 1,
                    new SyncOnStmt(new VarRef(DataType::I, "tid"),
                        new VarRef(DataType::I, "unpackc_tid"), "UNPACKC_GN",
                        "UNPACKC_TN", "UNPACKC_SYNC"));
                break;
            }
        }
        loop->updateSon();
    }
    else if (TemplateStructFuncStmt *func
        = BaseNode::getFatherPtr<TemplateStructFuncStmt>(memset_c_stmt))
    {
        std::vector<BaseNode *> &body = func->body;

        for (auto itr = body.begin(); itr != body.end(); itr++)
        {
            if (*itr == memset_c_stmt)
            {
                itr = body.insert(
                    itr, parallel_stmts.begin(), parallel_stmts.end());

                body.insert(itr + parallel_stmts.size() + 1,
                    new SyncOnStmt(new VarRef(DataType::I, "tid"),
                        new VarRef(DataType::I, "unpackc_tid"), "UNPACKC_GN",
                        "UNPACKC_TN", "UNPACKC_SYNC"));
                break;
            }
        }
        func->updateSon();
    }
    else if (CodeBlockStmt *fa
        = BaseNode::getFatherPtr<CodeBlockStmt>(memset_c_stmt))
    {
        std::vector<BaseNode *> &body = fa->stmts_in_block;

        for (auto itr = body.begin(); itr != body.end(); itr++)
        {
            if (*itr == memset_c_stmt)
            {
                itr = body.insert(
                    itr, parallel_stmts.begin(), parallel_stmts.end());

                body.insert(itr + parallel_stmts.size() + 1,
                    new SyncOnStmt(new VarRef(DataType::I, "tid"),
                        new VarRef(DataType::I, "unpackc_tid"), "UNPACKC_GN",
                        "UNPACKC_TN", "UNPACKC_SYNC"));
                break;
            }
        }
        fa->updateSon();
    }
}

void PASS::parallelizeGemmASTFuncBody(const ParallelScheme &ps)
{
    NodeInfo func_info { NodeType::TemplateStructFuncStmtKind,
        this->cur_gemm.getFuncName() };

    TemplateStructFuncStmt *func = dynamic_cast<TemplateStructFuncStmt *>(
        this->ast->walkFindNode(func_info));
    if (func == nullptr)
        return;
    std::vector<BaseNode *> &body = func->body;
    std::vector<BaseNode *> parallel_sections;
    for (auto itr = body.begin(); itr != body.end();)
    {
        if ((*itr)->node_type == NodeType::ZeroBufStmtKind
            || (*itr)->node_type == NodeType::LoopStmtKind
            || (*itr)->node_type == NodeType::CbetaOpStmtKind)
        {
            parallel_sections.push_back(*itr);
            BaseNode::son2father.erase(*itr);
            itr = body.erase(itr);
        }
        else if (VarDeclWithRVStmt *decl
            = dynamic_cast<VarDeclWithRVStmt *>(*itr))
        {
            if (decl->var_name == "buf_c_elem_num"
                || decl->var_name.substr(0, 4) == "pack"
                || decl->var_name.substr(0, 4) == "unpa")
            {
                parallel_sections.push_back(*itr);
                BaseNode::son2father.erase(*itr);
                itr = body.erase(itr);
            }
            else
                itr++;
        }
        else if (CodeBlockStmt *codes = dynamic_cast<CodeBlockStmt *>(*itr))
        {
            if (codes->hasLoop())
            {
                parallel_sections.push_back(*itr);
                BaseNode::son2father.erase(*itr);
                itr = body.erase(itr);
            }
        }
        else
            itr++;
    }

    parallel_sections.insert(parallel_sections.begin(),
        new VarDeclWithRVStmt(DataType::I, "tid", new OmpGetTidExpr()));
    parallel_sections.insert(
        parallel_sections.begin() + 1, this->declAllTids(ps));
    auto itr = body.begin();
    for (; itr != body.end(); itr++)
    {
        if ((*itr)->node_type == NodeType::FreeBufStmtKind)
            break;
    }
    body.insert(itr,
        new OmpParallelStmt(new CodeBlockStmt(parallel_sections),
            new VarRef(DataType::I, "NUM_THREADS")));

    func->updateSon();

    const std::vector<ParallelUnit> &pus = ps.parallel_policy;

    for (const ParallelUnit &pu : pus)
    {
        if (pu.dim != Dim::K)
            this->parallelizeForLoop(pu.idx_name, pu.tn_name, pu.tid_name, ps);
    }

    this->parallelizeCBetaOp(ps);

    this->parallelizeMemset(ps);

    this->refineParallelBufPtr(ps);

    this->parallelizeForLoop(ps.packa_policy.idx_name, ps.packa_policy.tn_name,
        ps.packa_policy.tid_name, ps);
    this->parallelizeForLoop(ps.packb_policy.idx_name, ps.packb_policy.tn_name,
        ps.packb_policy.tid_name, ps);
    this->parallelizeForLoop(ps.unpackc_policy.idx_name,
        ps.unpackc_policy.tn_name, ps.unpackc_policy.tid_name, ps);
}

void PASS::insertParallelPtrOff(const std::string &loop_name,
    const std::string &tid_name, const std::string &ptr_name,
    const std::string &pre_len_name, const std::string &init_name)
{
    NodeInfo loop_info { NodeType::LoopStmtKind, loop_name };
    LoopStmt *loop
        = dynamic_cast<LoopStmt *>(this->ast->walkFindNode(loop_info));
    if (loop == nullptr)
        return;
    Var *step_var = loop->getStepVar();
    if (step_var == nullptr)
        return;

    BaseNode *off = new BinaryOpExpr(OpType::Mul,
        new BinaryOpExpr(OpType::Minus,
            new VarRef(DataType::I, loop_name + "_local_start"),
            new VarRef(DataType::I, init_name)),
        new VarRef(DataType::I, pre_len_name));
    BaseNode *right = new BinaryOpExpr(
        OpType::Add, new VarRef(DataType::TPtr, ptr_name), off);
    VarDeclWithRVStmt *local_decl
        = new VarDeclWithRVStmt(DataType::TPtr, "local_" + ptr_name, right);

    if (LoopStmt *fa = BaseNode::getFatherPtr<LoopStmt>(loop))
    {
        for (auto itr = fa->loop_body.begin(); itr != fa->loop_body.end();
            itr++)
        {
            if (*itr == loop)
            {
                fa->loop_body.insert(itr, local_decl);
                break;
            }
        }
        fa->updateSon();
    }
    else if (CodeBlockStmt *fa = BaseNode::getFatherPtr<CodeBlockStmt>(loop))
    {
        for (auto itr = fa->stmts_in_block.begin();
            itr != fa->stmts_in_block.end(); itr++)
        {
            if (*itr == loop)
            {
                fa->stmts_in_block.insert(itr, local_decl);
                break;
            }
        }
        fa->updateSon();
    }
    else if (TemplateStructFuncStmt *fa
        = BaseNode::getFatherPtr<TemplateStructFuncStmt>(loop))
    {
        for (auto itr = fa->body.begin(); itr != fa->body.end(); itr++)
        {
            if (*itr == loop)
            {
                fa->body.insert(itr, local_decl);
                break;
            }
        }
        fa->updateSon();
    }

    NodeInfo ptr_ref_info { NodeType::VarRefKind, ptr_name };
    std::vector<BaseNode *> results;

    loop->walkFindAllNodes(ptr_ref_info, results);

    for (BaseNode *node : results)
    {
        if (VarRef *ref = dynamic_cast<VarRef *>(node))
        {
            ref->v = local_decl->decled_var;
        }
    }
}

void PASS::parallelizeGemmAST(const ParallelScheme &ps)
{
    this->insertIncludeStmt("omp.h");

    if (ps.parallel_on_k)
    {
        LM_ASSERT(cur_gemm.getSchemeInfo().loop_order.back() == Dim::K,
            "The outest loop must be K.")
        this->insertReductionTNArg(ps);
        this->removeCbetaOp(ps);
        this->insertReductionCBuf(ps);
        this->parallelizeReductionBody(ps);
    }
    else
    {
        this->insertTNTemplateArgs(ps);
        this->insertTotalNumThreads(ps);
        this->refineParallelBufSize(ps);
        this->parallelizeGemmASTFuncBody(ps);
    }
    std::string &filename = this->cur_gemm.getFileName();
    std::string &funcname = this->cur_gemm.getFuncName();

    filename = (ps.parallel_on_k ? "Reduction" : "Parallel") + filename;
    funcname = (ps.parallel_on_k ? "Reduction" : "Parallel") + funcname;

    std::cout << "Parallelize gemm AST done.\n";
}

}