#include "common.h"

namespace loopmaster
{
Stmt *GemmGenerator::buildWriteContinuousNestedPackALoop(
    const std::vector<Dim> &packa_order, size_t depth, size_t m_loops,
    size_t k_loops, std::vector<LoopIndex> &loop_idxs)
{
    if (depth == 0)
    {
        std::vector<BaseNode *> loop_body;

        std::vector<BaseNode *> m_idxs;
        std::vector<BaseNode *> k_idxs;

        bool vectorizable = packa_order[0] == Dim::M; // Only for src col-major

        for (int i = 0; i < loop_idxs.size(); i++)
        {
            if (loop_idxs[i].dim == Dim::M)
            {
                m_idxs.push_back(new VarRef(loop_idxs[i].idx));
            }
            else if (loop_idxs[i].dim == Dim::K)
            {
                k_idxs.push_back(new VarRef(loop_idxs[i].idx));
            }
        }

        Expr *m_offset_expr = new MultiCalcExpr(OpType::Add, m_idxs);
        Expr *k_offset_expr = new BinaryOpExpr(OpType::Mul,
            new MultiCalcExpr(OpType::Add, k_idxs),
            new VarRef(DataType::I, "lda"));
        Expr *offset_expr
            = new BinaryOpExpr(OpType::Add, m_offset_expr, k_offset_expr);

        if (vectorizable)
        {
            Expr *dst_ptr = new VarRef(DataType::TPtr, "packa_dst");
            Expr *src_ptr = new BinaryOpExpr(OpType::Add,
                new VarRef(DataType::TPtr, "packa_src"), offset_expr);
            loop_body.push_back(new VectorCopyStmt(dst_ptr, src_ptr));
            loop_body.push_back(new ExprToStmt(new BinaryOpExpr(
                OpType::AddAssign, new VarRef(DataType::TPtr, "packa_dst"),
                new VarRef(DataType::I, "VecLen<T>::value"))));
        }
        else
        {
            Expr *dst_expr = new UnaryOpExpr(OpType::DeAddr,
                new UnaryOpExpr(
                    OpType::Inc, new VarRef(DataType::TPtr, "packa_dst")));
            Expr *src_expr = new SubscriptExpr(
                new VarRef(DataType::TPtr, "packa_src"), offset_expr);
            loop_body.push_back(new VarAssignStmt(dst_expr, src_expr));
        }

        return new CodeBlockStmt(loop_body);
    }

    size_t cur_loop_id = depth - 1;
    Dim cur_dim = packa_order[cur_loop_id];
    std::string packa_idx_name = "packa";
    std::string packa_len_name = "packa";
    std::string packa_next_len_name = "packa";

    bool vectorizable = cur_loop_id == 0 && cur_dim == Dim::M;

    Expr *step_expr = nullptr;
    Expr *min_right_expr = nullptr;
    if (cur_dim == Dim::M)
    {
        packa_idx_name += ("m_idx" + std::to_string(m_loops - 1));
        packa_len_name += ("m_len" + std::to_string(m_loops));
        packa_next_len_name += ("m_len" + std::to_string(m_loops - 1));

        if (m_loops - 1 == 0)
        {
            step_expr = vectorizable
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
            min_right_expr = vectorizable
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
        }
        else
        {
            step_expr
                = new VarRef(DataType::I, "BM" + std::to_string(m_loops - 2));
            min_right_expr
                = new VarRef(DataType::I, "BM" + std::to_string(m_loops - 2));
        }
    }
    else if (cur_dim == Dim::K)
    {
        packa_idx_name += ("k_idx" + std::to_string(k_loops - 1));
        packa_len_name += ("k_len" + std::to_string(k_loops));
        packa_next_len_name += ("k_len" + std::to_string(k_loops - 1));

        if (k_loops - 1 == 0)
        {
            step_expr = vectorizable
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
            min_right_expr = vectorizable
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
        }
        else
        {
            step_expr
                = new VarRef(DataType::I, "BK" + std::to_string(k_loops - 2));
            min_right_expr
                = new VarRef(DataType::I, "BK" + std::to_string(k_loops - 2));
        }
    }

    LoopIndexDeclStmt *init_stmt = new LoopIndexDeclStmt(
        DataType::I, packa_idx_name, new VarRef(DataType::I, "0"));
    Expr *end_expr = new BinaryOpExpr(OpType::Less,
        new VarRef(DataType::I, packa_idx_name),
        new VarRef(DataType::I, packa_len_name));
    Expr *update_expr = new BinaryOpExpr(
        OpType::AddAssign, new VarRef(DataType::I, packa_idx_name), step_expr);

    std::vector<BaseNode *> loop_body;
    Expr *sub_expr = new BinaryOpExpr(OpType::Minus,
        new VarRef(DataType::I, packa_len_name),
        new VarRef(DataType::I, packa_idx_name));
    Expr *min_expr = new MinExpr(sub_expr, min_right_expr);
    loop_body.push_back(
        new VarDeclWithRVStmt(DataType::I, packa_next_len_name, min_expr));

    LoopIndex loop_index;
    loop_index.dim = cur_dim;
    loop_index.len = BaseNode::getVar(DataType::I, packa_len_name);
    loop_index.idx = BaseNode::getVar(DataType::I, packa_idx_name);
    loop_idxs.push_back(loop_index);

    Stmt *child_stmt = nullptr;
    if (cur_dim == Dim::M)
    {
        child_stmt = buildWriteContinuousNestedPackALoop(
            packa_order, depth - 1, m_loops - 1, k_loops, loop_idxs);
    }
    else if (cur_dim == Dim::K)
    {
        child_stmt = buildWriteContinuousNestedPackALoop(
            packa_order, depth - 1, m_loops, k_loops - 1, loop_idxs);
    }

    loop_body.push_back(child_stmt);

    return new LoopStmt(init_stmt, end_expr, update_expr, loop_body);
}

Stmt *GemmGenerator::buildWriteContinuousPackA()
{
    std::vector<BaseNode *> packa_body;

    size_t m_loop_id
        = this->scheme.out_m_loop_num + this->scheme.kernel_m_loop_num;
    size_t k_loop_id
        = this->scheme.out_k_loop_num + this->scheme.kernel_k_loop_num;

    std::vector<BaseNode *> src_m_idxs;
    std::vector<BaseNode *> src_k_idxs;

    VarRef *packa_m_len = nullptr;
    VarRef *packa_k_len = nullptr;

    for (int i = this->scheme.loop_order.size() - 1; i >= this->packa_pos; i--)
    {
        if (this->scheme.loop_order[i] == Dim::M)
        {
            m_loop_id--;
            src_m_idxs.push_back(
                new VarRef(DataType::I, "m_idx" + std::to_string(m_loop_id)));
        }
        else if (this->scheme.loop_order[i] == Dim::K)
        {
            k_loop_id--;
            src_k_idxs.push_back(
                new VarRef(DataType::I, "k_idx" + std::to_string(k_loop_id)));
        }

        if (i == this->packa_pos)
        {
            packa_m_len
                = new VarRef(DataType::I, "bm" + std::to_string(m_loop_id));
            packa_k_len
                = new VarRef(DataType::I, "bk" + std::to_string(k_loop_id));
        }
    }

    if (this->packa_pos == this->scheme.loop_order.size())
    {
        packa_m_len = new VarRef(DataType::I, "bm" + std::to_string(m_loop_id));
        packa_k_len = new VarRef(DataType::I, "bk" + std::to_string(k_loop_id));
    }

    std::vector<BaseNode *> src_ptr_exprs;
    src_ptr_exprs.push_back(new VarRef(DataType::TPtr, "A"));
    if (!src_m_idxs.empty())
    {
        src_ptr_exprs.push_back(new MultiCalcExpr(OpType::Add, src_m_idxs));
    }
    
    if (!src_k_idxs.empty())
    {
        src_ptr_exprs.push_back(new BinaryOpExpr(OpType::Mul,
            new MultiCalcExpr(OpType::Add, src_k_idxs),
            new VarRef(DataType::I, "lda")));
    }

    Expr *packa_src_expr = new MultiCalcExpr(OpType::Add, src_ptr_exprs);
    Stmt *decl_src_ptr = new VarDeclWithRVStmt(
        DataType::TPtr, "packa_src", packa_src_expr, true);
    packa_body.push_back(decl_src_ptr);

    Stmt *decl_dst_ptr = new VarDeclWithRVStmt(
        DataType::TPtr, "packa_dst", new VarRef(DataType::TPtr, "buf_a"));
    packa_body.push_back(decl_dst_ptr);

    std::vector<Dim> packa_loop_order;
    size_t m_loops = 0;
    size_t k_loops = 0;
    for (int i = 0; i < this->packa_pos; i++)
    {
        if (this->scheme.loop_order[i] == Dim::M)
        {
            m_loops++;
            packa_loop_order.push_back(Dim::M);
        }
        else if (this->scheme.loop_order[i] == Dim::K)
        {
            k_loops++;
            packa_loop_order.push_back(Dim::K);
        }
    }

    Stmt *packa_m_len_stmt = new VarDeclWithRVStmt(
        DataType::I, "packam_len" + std::to_string(m_loops), packa_m_len);
    Stmt *packa_k_len_stmt = new VarDeclWithRVStmt(
        DataType::I, "packak_len" + std::to_string(k_loops), packa_k_len);
    packa_body.push_back(packa_m_len_stmt);
    packa_body.push_back(packa_k_len_stmt);

    std::vector<LoopIndex> loop_idxs;
    packa_body.push_back(buildWriteContinuousNestedPackALoop(packa_loop_order,
        packa_loop_order.size(), m_loops, k_loops, loop_idxs));

    return new CodeBlockStmt(packa_body);
}

Stmt *GemmGenerator::buildWriteContinuousNestedPackBLoop(
    const std::vector<Dim> &packb_order, size_t depth, size_t n_loops,
    size_t k_loops, std::vector<LoopIndex> &loop_idxs)
{
    if (depth == 0)
    {
        std::vector<BaseNode *> loop_body;

        std::vector<BaseNode *> n_idxs;
        std::vector<BaseNode *> k_idxs;

        bool vectorizable = packb_order[0] == Dim::K; // Only for src col-major

        for (int i = 0; i < loop_idxs.size(); i++)
        {
            if (loop_idxs[i].dim == Dim::N)
            {
                n_idxs.push_back(new VarRef(loop_idxs[i].idx));
            }
            else if (loop_idxs[i].dim == Dim::K)
            {
                k_idxs.push_back(new VarRef(loop_idxs[i].idx));
            }
        }

        Expr *n_offset_expr = new BinaryOpExpr(OpType::Mul,
            new MultiCalcExpr(OpType::Add, n_idxs),
            new VarRef(DataType::I, "ldb"));
        Expr *k_offset_expr = new MultiCalcExpr(OpType::Add, k_idxs);
        Expr *offset_expr
            = new BinaryOpExpr(OpType::Add, n_offset_expr, k_offset_expr);

        if (vectorizable)
        {
            Expr *dst_ptr = new VarRef(DataType::TPtr, "packb_dst");
            Expr *src_ptr = new BinaryOpExpr(OpType::Add,
                new VarRef(DataType::TPtr, "packb_src"), offset_expr);
            loop_body.push_back(new VectorCopyStmt(dst_ptr, src_ptr));
            loop_body.push_back(new ExprToStmt(new BinaryOpExpr(
                OpType::AddAssign, new VarRef(DataType::TPtr, "packb_dst"),
                new VarRef(DataType::I, "VecLen<T>::value"))));
        }
        else
        {
            Expr *dst_expr = new UnaryOpExpr(OpType::DeAddr,
                new UnaryOpExpr(
                    OpType::Inc, new VarRef(DataType::TPtr, "packb_dst")));
            Expr *src_expr = new SubscriptExpr(
                new VarRef(DataType::TPtr, "packb_src"), offset_expr);
            loop_body.push_back(new VarAssignStmt(dst_expr, src_expr));
        }

        return new CodeBlockStmt(loop_body);
    }

    size_t cur_loop_id = depth - 1;
    Dim cur_dim = packb_order[cur_loop_id];
    std::string packb_idx_name = "packb";
    std::string packb_len_name = "packb";
    std::string packb_next_len_name = "packb";

    bool vectorizable = cur_loop_id == 0
        && packb_order[0] == Dim::K; // Only for src col-majoy

    Expr *step_expr = nullptr;
    Expr *min_right_expr = nullptr;
    if (cur_dim == Dim::N)
    {
        packb_idx_name += ("n_idx" + std::to_string(n_loops - 1));
        packb_len_name += ("n_len" + std::to_string(n_loops));
        packb_next_len_name += ("n_len" + std::to_string(n_loops - 1));

        if (n_loops - 1 == 0)
        {
            step_expr = vectorizable
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
            min_right_expr = vectorizable
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
        }
        else
        {
            step_expr
                = new VarRef(DataType::I, "BN" + std::to_string(n_loops - 2));
            min_right_expr
                = new VarRef(DataType::I, "BN" + std::to_string(n_loops - 2));
        }
    }
    else if (cur_dim == Dim::K)
    {
        packb_idx_name += ("k_idx" + std::to_string(k_loops - 1));
        packb_len_name += ("k_len" + std::to_string(k_loops));
        packb_next_len_name += ("k_len" + std::to_string(k_loops - 1));

        if (k_loops - 1 == 0)
        {
            step_expr = vectorizable
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
            min_right_expr = vectorizable
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
        }
        else
        {
            step_expr
                = new VarRef(DataType::I, "BK" + std::to_string(k_loops - 2));
            min_right_expr
                = new VarRef(DataType::I, "BK" + std::to_string(k_loops - 2));
        }
    }

    LoopIndexDeclStmt *init_stmt = new LoopIndexDeclStmt(
        DataType::I, packb_idx_name, new VarRef(DataType::I, "0"));
    Expr *end_expr = new BinaryOpExpr(OpType::Less,
        new VarRef(DataType::I, packb_idx_name),
        new VarRef(DataType::I, packb_len_name));
    Expr *update_expr = new BinaryOpExpr(
        OpType::AddAssign, new VarRef(DataType::I, packb_idx_name), step_expr);

    std::vector<BaseNode *> loop_body;
    Expr *sub_expr = new BinaryOpExpr(OpType::Minus,
        new VarRef(DataType::I, packb_len_name),
        new VarRef(DataType::I, packb_idx_name));
    Expr *min_expr = new MinExpr(sub_expr, min_right_expr);
    loop_body.push_back(
        new VarDeclWithRVStmt(DataType::I, packb_next_len_name, min_expr));

    LoopIndex loop_index;
    loop_index.dim = cur_dim;
    loop_index.len = BaseNode::getVar(DataType::I, packb_len_name);
    loop_index.idx = BaseNode::getVar(DataType::I, packb_idx_name);
    loop_idxs.push_back(loop_index);

    Stmt *child_stmt = nullptr;
    if (cur_dim == Dim::N)
    {
        child_stmt = buildWriteContinuousNestedPackBLoop(
            packb_order, depth - 1, n_loops - 1, k_loops, loop_idxs);
    }
    else if (cur_dim == Dim::K)
    {
        child_stmt = buildWriteContinuousNestedPackBLoop(
            packb_order, depth - 1, n_loops, k_loops - 1, loop_idxs);
    }

    loop_body.push_back(child_stmt);

    return new LoopStmt(init_stmt, end_expr, update_expr, loop_body);
}

Stmt *GemmGenerator::buildWriteContinuousPackB()
{
    std::vector<BaseNode *> packb_body;

    size_t n_loop_id
        = this->scheme.out_n_loop_num + this->scheme.kernel_n_loop_num;
    size_t k_loop_id
        = this->scheme.out_k_loop_num + this->scheme.kernel_k_loop_num;

    std::vector<BaseNode *> src_n_idxs;
    std::vector<BaseNode *> src_k_idxs;

    VarRef *packb_n_len = nullptr;
    VarRef *packb_k_len = nullptr;

    for (int i = this->scheme.loop_order.size() - 1; i >= this->packb_pos; i--)
    {
        if (this->scheme.loop_order[i] == Dim::N)
        {
            n_loop_id--;
            src_n_idxs.push_back(
                new VarRef(DataType::I, "n_idx" + std::to_string(n_loop_id)));
        }
        else if (this->scheme.loop_order[i] == Dim::K)
        {
            k_loop_id--;
            src_k_idxs.push_back(
                new VarRef(DataType::I, "k_idx" + std::to_string(k_loop_id)));
        }

        if (i == this->packb_pos)
        {
            packb_n_len
                = new VarRef(DataType::I, "bn" + std::to_string(n_loop_id));
            packb_k_len
                = new VarRef(DataType::I, "bk" + std::to_string(k_loop_id));
        }
    }

    if (this->packb_pos == this->scheme.loop_order.size())
    {
        packb_n_len = new VarRef(DataType::I, "bn" + std::to_string(n_loop_id));
        packb_k_len = new VarRef(DataType::I, "bk" + std::to_string(k_loop_id));
    }

    std::vector<BaseNode *> src_ptr_exprs;
    src_ptr_exprs.push_back(new VarRef(DataType::TPtr, "B"));
    if (!src_n_idxs.empty())
    {
        src_ptr_exprs.push_back(new BinaryOpExpr(OpType::Mul,
            new MultiCalcExpr(OpType::Add, src_n_idxs),
            new VarRef(DataType::I, "ldb")));
    }
    if (!src_k_idxs.empty())
    {
        src_ptr_exprs.push_back(new MultiCalcExpr(OpType::Add, src_k_idxs));
    }
    Expr *packb_src_expr = new MultiCalcExpr(OpType::Add, src_ptr_exprs);
    Stmt *decl_src_ptr = new VarDeclWithRVStmt(
        DataType::TPtr, "packb_src", packb_src_expr, true);
    packb_body.push_back(decl_src_ptr);

    Stmt *decl_dst_ptr = new VarDeclWithRVStmt(
        DataType::TPtr, "packb_dst", new VarRef(DataType::TPtr, "buf_b"));
    packb_body.push_back(decl_dst_ptr);

    std::vector<Dim> packb_loop_order;
    size_t n_loops = 0;
    size_t k_loops = 0;
    for (int i = 0; i < this->packb_pos; i++)
    {
        if (this->scheme.loop_order[i] == Dim::N)
        {
            n_loops++;
            packb_loop_order.push_back(Dim::N);
        }
        else if (this->scheme.loop_order[i] == Dim::K)
        {
            k_loops++;
            packb_loop_order.push_back(Dim::K);
        }
    }

    Stmt *packb_n_len_stmt = new VarDeclWithRVStmt(
        DataType::I, "packbn_len" + std::to_string(n_loops), packb_n_len);
    Stmt *packb_k_len_stmt = new VarDeclWithRVStmt(
        DataType::I, "packbk_len" + std::to_string(k_loops), packb_k_len);

    packb_body.push_back(packb_n_len_stmt);
    packb_body.push_back(packb_k_len_stmt);

    std::vector<LoopIndex> loop_idxs;
    packb_body.push_back(buildWriteContinuousNestedPackBLoop(packb_loop_order,
        packb_loop_order.size(), n_loops, k_loops, loop_idxs));

    return new CodeBlockStmt(packb_body);
}
} // namespace loopmaster
