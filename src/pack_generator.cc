#include "common.h"

namespace loopmaster
{
Expr *GemmGenerator::buildPackedIdxLenExpr(const std::string &idx_name,
    const std::string &max_len_name, const std::string &step_name)
{
    // len % step
    Expr *expr0
        = new BinaryOpExpr(OpType::Mod, new VarRef(DataType::I, max_len_name),
            new VarRef(DataType::I, step_name));

    // len - len % step
    Expr *expr1 = new BinaryOpExpr(
        OpType::Minus, new VarRef(DataType::I, max_len_name), expr0);

    // src >= (len - len % step)
    Expr *bool_expr = new BinaryOpExpr(
        OpType::GE, new VarRef(DataType::I, idx_name), expr1);

    // len % step
    Expr *first_expr
        = new BinaryOpExpr(OpType::Mod, new VarRef(DataType::I, max_len_name),
            new VarRef(DataType::I, step_name));

    Expr *second_expr = new VarRef(DataType::I, step_name);

    return new TernaryExpr(bool_expr, first_expr, second_expr);
}

Stmt *GemmGenerator::buildReadContinuousPackA()
{
    std::vector<BaseNode *> packA_body;

    // src m_idxs k_idxs;
    std::vector<BaseNode *> src_m_idxs;
    std::vector<BaseNode *> src_k_idxs;

    size_t m_loop_id = this->scheme.out_m_loop_num
        + this->scheme.kernel_m_loop_num; // for src
    size_t k_loop_id = this->scheme.out_k_loop_num
        + this->scheme.kernel_k_loop_num; // for src

    VarRef *pack_m_len = nullptr;
    VarRef *pack_k_len = nullptr;

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

        // determine the size of pack a
        if (i == this->packa_pos)
        {
            pack_m_len
                = new VarRef(DataType::I, "bm" + std::to_string(m_loop_id));
            pack_k_len
                = new VarRef(DataType::I, "bk" + std::to_string(k_loop_id));
        }
    }

    if (this->packa_pos == this->scheme.loop_order.size())
    {
        pack_m_len = new VarRef(DataType::I, "bm" + std::to_string(m_loop_id));
        pack_k_len = new VarRef(DataType::I, "bk" + std::to_string(k_loop_id));
    }

    // build loop index structs
    std::vector<LoopIndex> dst_indexs;
    size_t m_id = 0;
    size_t k_id = 0;
    size_t index_id = 0;

    size_t pre_m_id = 0;
    size_t pre_k_id = 0;

    size_t m_count = 0;
    size_t k_count = 0;

    for (size_t i = 0; i < this->packa_pos; i++)
    {
        if (this->scheme.loop_order[i] == Dim::M)
            m_count++;
        else if (this->scheme.loop_order[i] == Dim::K)
            k_count++;
    }

    for (size_t i = 0; i < this->packa_pos; i++)
    {
        Dim cur_dim = this->scheme.loop_order[i];

        LoopIndex index = { (index_id == 0), false, false, false, "", cur_dim,
            nullptr, nullptr, nullptr };

        // determine stride
        if (index_id == 0)
            index.stride = BaseNode::getVar(DataType::I, "1");
        else
            index.stride = dst_indexs[index_id - 1].len; // Deprecated

        if (cur_dim == Dim::M)
        {
            // determine m step
            index.step = m_id == 0 ? BaseNode::getVar(DataType::I, "1")
                                   : dst_indexs[pre_m_id].len;

            // determine m_len
            index.len = (m_id == m_count - 1)
                ? pack_m_len->v
                : BaseNode::getVar(DataType::I, "BM" + std::to_string(m_id));

            dst_indexs.push_back(index);

            pre_m_id = index_id;
            m_id++;
            index_id++;
        }
        else if (cur_dim == Dim::K)
        {
            // determine k step
            index.step = k_id == 0 ? BaseNode::getVar(DataType::I, "1")
                                   : dst_indexs[pre_k_id].len;

            // determine k len
            index.len = k_id == k_count - 1
                ? pack_k_len->v
                : BaseNode::getVar(DataType::I, "BK" + std::to_string(k_id));

            dst_indexs.push_back(index);

            pre_k_id = index_id;
            k_id++;
            index_id++;
        }
    }

    bool can_vectorized = (this->a_loop_indexs[0].dim
        == Dim::M); // NOTE: only consider notrans temporarily

    // build packa_src head addr expr: A + (i0 + i1 + i2) + (j0 + j1 + j2) * lda
    std::vector<BaseNode *> packa_src_head_vec { new VarRef(
        DataType::TPtr, "A") };
    if (!src_m_idxs.empty())
    {
        packa_src_head_vec.push_back(
            new MultiCalcExpr(OpType::Add, src_m_idxs));
    }
    if (!src_k_idxs.empty())
    {
        packa_src_head_vec.push_back(new BinaryOpExpr(OpType::Mul,
            new MultiCalcExpr(OpType::Add, src_k_idxs),
            new VarRef(DataType::I, "lda")));
    }
    Expr *packa_src_head = new MultiCalcExpr(OpType::Add, packa_src_head_vec);

    // decl src Aptr for pack
    packA_body.push_back(new VarDeclWithRVStmt(
        DataType::TPtr, "packa_src", packa_src_head, true));

    // decl dst Aptr for pack
    packA_body.push_back(new VarDeclWithRVStmt(
        DataType::TPtr, "packa_dst", new VarRef(DataType::TPtr, "buf_a")));

    // col-major read continuous loops

    /* k loop args */
    LoopIndexDeclStmt *srck_idx0 = new LoopIndexDeclStmt(
        DataType::I, "packasrc_k_idx0", new VarRef(DataType::I, "0"));
    Expr *k_end_expr = new BinaryOpExpr(
        OpType::Less, new VarRef(srck_idx0->getDecledVar()), pack_k_len);
    Expr *k_update_expr = new BinaryOpExpr(OpType::AddAssign,
        new VarRef(srck_idx0->getDecledVar()), new ConstNumExpr(1));
    std::vector<BaseNode *> k_loop_body;

    /* m loop args */
    LoopIndexDeclStmt *srcm_idx0 = new LoopIndexDeclStmt(
        DataType::I, "packasrc_m_idx0", new VarRef(DataType::I, "0"));
    Expr *m_end_expr = new BinaryOpExpr(
        OpType::Less, new VarRef(srcm_idx0->getDecledVar()), pack_m_len);
    Expr *m_update_expr = new BinaryOpExpr(OpType::AddAssign,
        new VarRef(srcm_idx0->getDecledVar()),
        (can_vectorized ? new VarRef(DataType::I, "VecLen<T>::value")
                        : new VarRef(DataType::I, "1")));
    std::vector<BaseNode *> m_loop_body;

    Stmt *srcptr_declstmt
        = new VarDeclWithRVStmt(DataType::TPtr, "packasrc_ptr",
            new MultiCalcExpr(OpType::Add,
                { new VarRef(DataType::TPtr, "packa_src"),
                    new VarRef(DataType::I, "packasrc_m_idx0"),
                    new BinaryOpExpr(OpType::Mul,
                        new VarRef(DataType::I, "packasrc_k_idx0"),
                        new VarRef(DataType::I, "lda")) }),
            true);
    m_loop_body.push_back(srcptr_declstmt);

    // build dst_offsets vec
    std::vector<BaseNode *> dst_offsets;
    dst_offsets.push_back(new VarRef(DataType::TPtr, "packa_dst"));
    size_t tiled_mid = 0;
    size_t tiled_kid = 0;

    // build tiled_idx
    Var *pre_m_len = BaseNode::getVar(DataType::I, "1");
    Var *pre_k_len = BaseNode::getVar(DataType::I, "1");
    for (size_t i = 0; i < dst_indexs.size(); i++)
    {
        if (dst_indexs[i].dim == Dim::M)
        {
            /* build tiled m_idxs decl and assign stmt */
            std::string cur_m_idx_name
                = "packatiled_m_idx" + std::to_string(tiled_mid);
            std::string cur_m_len_name
                = "packatiled_m_len" + std::to_string(tiled_mid);
            std::string &max_len_name = pack_m_len->v->name;
            std::string step_name = dst_indexs[i].len->name;
            Stmt *tiled_midx = new VarDeclWithRVStmt(DataType::I,
                cur_m_idx_name,
                new BinaryOpExpr(OpType::Minus,
                    new BinaryOpExpr(OpType::Mod,
                        new VarRef(srcm_idx0->getDecledVar()),
                        new VarRef(dst_indexs[i].len)),
                    (tiled_mid == 0 ? (Expr *)(new VarRef(DataType::I, "0"))
                                    : new BinaryOpExpr(OpType::Mod,
                                          new VarRef(srcm_idx0->getDecledVar()),
                                          new VarRef(dst_indexs[i].step)))));
            dst_offsets.push_back(new BinaryOpExpr(OpType::Mul,
                new VarRef(DataType::I, cur_m_idx_name),
                new VarRef(pre_k_len)));
            Expr *real_mlen = buildPackedIdxLenExpr(
                srcm_idx0->getDecledVar()->name, max_len_name, step_name);
            Stmt *m_len_decl
                = new VarDeclWithRVStmt(DataType::I, cur_m_len_name, real_mlen);
            pre_m_len
                = dynamic_cast<VarDeclWithRVStmt *>(m_len_decl)->decled_var;
            m_loop_body.push_back(tiled_midx);
            m_loop_body.push_back(m_len_decl);

            tiled_mid++;
        }
        else if (dst_indexs[i].dim == Dim::K)
        {
            /* build tiled k_idxs decl and assign stmt */
            std::string cur_k_idx_name
                = "packatiled_k_idx" + std::to_string(tiled_kid);
            std::string cur_k_len_name
                = "packatiled_k_len" + std::to_string(tiled_kid);
            std::string &max_len_name = pack_k_len->v->name;
            std::string step_name = dst_indexs[i].len->name;
            Stmt *tiled_kidx = new VarDeclWithRVStmt(DataType::I,
                cur_k_idx_name,
                new BinaryOpExpr(OpType::Minus,
                    new BinaryOpExpr(OpType::Mod,
                        new VarRef(srck_idx0->getDecledVar()),
                        new VarRef(dst_indexs[i].len)),
                    (tiled_kid == 0 ? (Expr *)(new VarRef(DataType::I, "0"))
                                    : new BinaryOpExpr(OpType::Mod,
                                          new VarRef(srck_idx0->getDecledVar()),
                                          new VarRef(dst_indexs[i].step)))));
            dst_offsets.push_back(new BinaryOpExpr(OpType::Mul,
                new VarRef(DataType::I, cur_k_idx_name),
                new VarRef(pre_m_len)));
            Expr *real_klen = buildPackedIdxLenExpr(
                srck_idx0->getDecledVar()->name, max_len_name, step_name);
            Stmt *k_len_decl
                = new VarDeclWithRVStmt(DataType::I, cur_k_len_name, real_klen);
            pre_k_len
                = dynamic_cast<VarDeclWithRVStmt *>(k_len_decl)->decled_var;
            k_loop_body.push_back(tiled_kidx);
            k_loop_body.push_back(k_len_decl);
            tiled_kid++;
        }
    }

    Stmt *dstptr_declstmt = new VarDeclWithRVStmt(DataType::TPtr,
        "packadst_ptr", new MultiCalcExpr(OpType::Add, dst_offsets));
    m_loop_body.push_back(dstptr_declstmt);

    // *dst_ptr = *src_ptr
    Stmt *copy_stmt = can_vectorized
        ? dynamic_cast<Stmt *>(
              new VectorCopyStmt(new VarRef(DataType::TPtr, "packadst_ptr"),
                  new VarRef(DataType::TPtr, "packasrc_ptr")))
        : dynamic_cast<Stmt *>(
              new VarAssignStmt(new UnaryOpExpr(OpType::DeAddr,
                                    new VarRef(DataType::TPtr, "packadst_ptr")),
                  new UnaryOpExpr(OpType::DeAddr,
                      new VarRef(DataType::TPtr, "packasrc_ptr"))));
    m_loop_body.push_back(copy_stmt);

    Stmt *m_loop_stmt
        = new LoopStmt(srcm_idx0, m_end_expr, m_update_expr, m_loop_body);

    k_loop_body.push_back(m_loop_stmt);

    Stmt *k_loop_stmt
        = new LoopStmt(srck_idx0, k_end_expr, k_update_expr, k_loop_body);

    packA_body.push_back(k_loop_stmt);

    return new CodeBlockStmt(packA_body);
}

Stmt *GemmGenerator::buildReadContinuousPackB()
{
    std::vector<BaseNode *> packB_body;

    // src n_idxs k_idxs
    std::vector<BaseNode *> src_n_idxs;
    std::vector<BaseNode *> src_k_idxs;

    size_t n_loop_id
        = this->scheme.out_n_loop_num + this->scheme.kernel_n_loop_num;
    size_t k_loop_id
        = this->scheme.out_k_loop_num + this->scheme.kernel_k_loop_num;

    VarRef *pack_n_len = nullptr;
    VarRef *pack_k_len = nullptr;

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

        // determine the size of pack b
        if (i == this->packb_pos)
        {
            pack_n_len
                = new VarRef(DataType::I, "bn" + std::to_string(n_loop_id));
            pack_k_len
                = new VarRef(DataType::I, "bk" + std::to_string(k_loop_id));
        }
    }

    if (this->packb_pos == this->scheme.loop_order.size())
    {
        pack_n_len = new VarRef(DataType::I, "bn" + std::to_string(n_loop_id));
        pack_k_len = new VarRef(DataType::I, "bk" + std::to_string(k_loop_id));
    }

    // build loop index structs
    std::vector<LoopIndex> dst_indexs;
    size_t n_id = 0;
    size_t k_id = 0;
    size_t index_id = 0;

    size_t pre_n_id = 0;
    size_t pre_k_id = 0;
    size_t n_count = 0;
    size_t k_count = 0;

    for (size_t i = 0; i < this->packb_pos; i++)
    {
        if (this->scheme.loop_order[i] == Dim::N)
            n_count++;
        else if (this->scheme.loop_order[i] == Dim::K)
            k_count++;
    }

    for (size_t i = 0; i < this->packb_pos; i++)
    {
        Dim cur_dim = this->scheme.loop_order[i];

        LoopIndex index = { (index_id == 0), false, false, false, "", cur_dim,
            nullptr, nullptr, nullptr };

        // determine stride (deprecated)
        if (index_id == 0)
            index.stride = BaseNode::getVar(DataType::I, "1");
        else
            index.stride = dst_indexs[index_id - 1].len;

        if (cur_dim == Dim::N)
        {
            // determine n step
            index.step = n_id == 0 ? BaseNode::getVar(DataType::I, "1")
                                   : dst_indexs[pre_n_id].len;

            // determine n len
            index.len = (n_id == n_count - 1)
                ? pack_n_len->v
                : BaseNode::getVar(DataType::I, "BN" + std::to_string(n_id));

            dst_indexs.push_back(index);

            pre_n_id = index_id;
            n_id++;
            index_id++;
        }
        else if (cur_dim == Dim::K)
        {
            // determine k step
            index.step = k_id == 0 ? BaseNode::getVar(DataType::I, "1")
                                   : dst_indexs[pre_k_id].len;

            // determine k len
            index.len = (k_id == k_count - 1)
                ? pack_k_len->v
                : BaseNode::getVar(DataType::I, "BK" + std::to_string(k_id));
            dst_indexs.push_back(index);

            pre_k_id = index_id;
            k_id++;
            index_id++;
        }
    }

    bool can_vectorized = (this->b_loop_indexs[0].dim
        == Dim::K); // NOTE: only consider notrans temporarily

    std::vector<BaseNode *> packb_src_head_vec { new VarRef(
        DataType::TPtr, "B") };
    if (!src_k_idxs.empty())
    {
        packb_src_head_vec.push_back(
            new MultiCalcExpr(OpType::Add, src_k_idxs));
    }
    if (!src_n_idxs.empty())
    {
        packb_src_head_vec.push_back(new BinaryOpExpr(OpType::Mul,
            new MultiCalcExpr(OpType::Add, src_n_idxs),
            new VarRef(DataType::I, "ldb")));
    }
    Expr *packb_src_head = new MultiCalcExpr(OpType::Add, packb_src_head_vec);

    packB_body.push_back(new VarDeclWithRVStmt(
        DataType::TPtr, "packb_src", packb_src_head, true));

    packB_body.push_back(new VarDeclWithRVStmt(
        DataType::TPtr, "packb_dst", new VarRef(DataType::TPtr, "buf_b")));

    // col-major read continuous loops

    /* k loop args */
    LoopIndexDeclStmt *srck_idx0 = new LoopIndexDeclStmt(
        DataType::I, "packbsrc_k_idx0", new VarRef(DataType::I, "0"));
    Expr *k_end_expr = new BinaryOpExpr(
        OpType::Less, new VarRef(srck_idx0->getDecledVar()), pack_k_len);
    Expr *k_update_expr = new BinaryOpExpr(OpType::AddAssign,
        new VarRef(srck_idx0->getDecledVar()),
        new VarRef(can_vectorized
                ? BaseNode::getVar(DataType::I, "VecLen<T>::value")
                : BaseNode::getVar(DataType::I, "1")));
    std::vector<BaseNode *> k_loop_body;

    /* n loop args */
    LoopIndexDeclStmt *srcn_idx0 = new LoopIndexDeclStmt(
        DataType::I, "packbsrc_n_idx0", new VarRef(DataType::I, "0"));
    Expr *n_end_expr = new BinaryOpExpr(
        OpType::Less, new VarRef(srcn_idx0->getDecledVar()), pack_n_len);
    Expr *n_update_expr = new BinaryOpExpr(OpType::AddAssign,
        new VarRef(srcn_idx0->getDecledVar()), new ConstNumExpr(1));
    std::vector<BaseNode *> n_loop_body;

    /* build src ptr decl stmt */

    Stmt *srcptr_declstmt = nullptr;
    {
        // srcn_idx0 * ldb
        Expr *expr0 = new BinaryOpExpr(OpType::Mul,
            new VarRef(DataType::I, "packbsrc_n_idx0"),
            new VarRef(DataType::I, "ldb"));
        srcptr_declstmt = new VarDeclWithRVStmt(DataType::TPtr, "packbsrc_ptr",
            new MultiCalcExpr(OpType::Add,
                { new VarRef(DataType::TPtr, "packb_src"),
                    new VarRef(DataType::I, "packbsrc_k_idx0"), expr0 }),
            true);
    }
    k_loop_body.push_back(srcptr_declstmt);

    // build dst_offsets vec
    std::vector<BaseNode *> dst_offsets;
    dst_offsets.push_back(new VarRef(DataType::TPtr, "packb_dst"));
    size_t tiled_nid = 0;
    size_t tiled_kid = 0;

    // build tiled_idx
    Var *pre_k_len = BaseNode::getVar(DataType::I, "1");
    Var *pre_n_len = BaseNode::getVar(DataType::I, "1");
    for (size_t i = 0; i < dst_indexs.size(); i++)
    {
        if (dst_indexs[i].dim == Dim::N)
        {
            /* build tiled n_idxs decl and assign stmt */
            std::string cur_n_idx_name
                = "packbtiled_n_idx" + std::to_string(tiled_nid);
            std::string cur_n_len_name
                = "packbtiled_n_len" + std::to_string(tiled_nid);
            std::string &max_len_name = pack_n_len->v->name;
            std::string step_name = dst_indexs[i].len->name;
            Stmt *tiled_nidx = nullptr;
            { // tiledn_idx = srcn_idx % len - srcn_idx % pre_nlen
                Expr *mod_expr = new BinaryOpExpr(OpType::Mod,
                    new VarRef(srcn_idx0->getDecledVar()),
                    new VarRef(dst_indexs[i].len));

                Expr *expr0 = tiled_nid == 0
                    ? static_cast<Expr *>(new VarRef(DataType::I, "0"))
                    : new BinaryOpExpr(OpType::Mod,
                          new VarRef(srcn_idx0->getDecledVar()),
                          new VarRef(dst_indexs[i].step));
                Expr *minus_expr
                    = new BinaryOpExpr(OpType::Minus, mod_expr, expr0);
                tiled_nidx = new VarDeclWithRVStmt(
                    DataType::I, cur_n_idx_name, minus_expr);
            }
            { // prepare dst_offset, tiledn_idx * pre_k_len
                Expr *mul_expr = new BinaryOpExpr(OpType::Mul,
                    new VarRef(DataType::I, cur_n_idx_name),
                    new VarRef(pre_k_len));
                dst_offsets.push_back(mul_expr);
            }
            Expr *real_nlen = buildPackedIdxLenExpr(
                srcn_idx0->getDecledVar()->name, max_len_name, step_name);
            Stmt *n_len_decl
                = new VarDeclWithRVStmt(DataType::I, cur_n_len_name, real_nlen);
            pre_n_len
                = dynamic_cast<VarDeclWithRVStmt *>(n_len_decl)->decled_var;
            n_loop_body.push_back(tiled_nidx);
            n_loop_body.push_back(n_len_decl);

            tiled_nid++;
        }
        else if (dst_indexs[i].dim == Dim::K)
        {
            /* copy from packa and simply modify the pre_n_len */
            std::string cur_k_idx_name
                = "packbtiled_k_idx" + std::to_string(tiled_kid);
            std::string cur_k_len_name
                = "packbtiled_k_len" + std::to_string(tiled_kid);
            std::string &max_len_name = pack_k_len->v->name;
            std::string step_name = dst_indexs[i].len->name;
            Stmt *tiled_kidx = new VarDeclWithRVStmt(DataType::I,
                cur_k_idx_name,
                new BinaryOpExpr(OpType::Minus,
                    new BinaryOpExpr(OpType::Mod,
                        new VarRef(srck_idx0->getDecledVar()),
                        new VarRef(dst_indexs[i].len)),
                    (tiled_kid == 0 ? (Expr *)(new VarRef(DataType::I, "0"))
                                    : new BinaryOpExpr(OpType::Mod,
                                          new VarRef(srck_idx0->getDecledVar()),
                                          new VarRef(dst_indexs[i].step)))));
            dst_offsets.push_back(new BinaryOpExpr(OpType::Mul,
                new VarRef(DataType::I, cur_k_idx_name),
                new VarRef(pre_n_len)));
            Expr *real_klen = buildPackedIdxLenExpr(
                srck_idx0->getDecledVar()->name, max_len_name, step_name);
            Stmt *k_len_decl
                = new VarDeclWithRVStmt(DataType::I, cur_k_len_name, real_klen);
            pre_k_len
                = dynamic_cast<VarDeclWithRVStmt *>(k_len_decl)->decled_var;
            k_loop_body.push_back(tiled_kidx);
            k_loop_body.push_back(k_len_decl);
            tiled_kid++;
        }
    }

    Stmt *dstptr_declstmt = new VarDeclWithRVStmt(DataType::TPtr,
        "packbdst_ptr", new MultiCalcExpr(OpType::Add, dst_offsets));
    k_loop_body.push_back(dstptr_declstmt);

    // *dst_ptr = *src_ptr
    Stmt *copy_stmt = can_vectorized
        ? dynamic_cast<Stmt *>(
              new VectorCopyStmt(new VarRef(DataType::TPtr, "packbdst_ptr"),
                  new VarRef(DataType::TPtr, "packbsrc_ptr")))
        : dynamic_cast<Stmt *>(
              new VarAssignStmt(new UnaryOpExpr(OpType::DeAddr,
                                    new VarRef(DataType::TPtr, "packbdst_ptr")),
                  new UnaryOpExpr(OpType::DeAddr,
                      new VarRef(DataType::TPtr, "packbsrc_ptr"))));
    k_loop_body.push_back(copy_stmt);
    Stmt *k_loop_stmt
        = new LoopStmt(srck_idx0, k_end_expr, k_update_expr, k_loop_body);

    n_loop_body.push_back(k_loop_stmt);
    Stmt *n_loop_stmt
        = new LoopStmt(srcn_idx0, n_end_expr, n_update_expr, n_loop_body);

    packB_body.push_back(n_loop_stmt);

    return new CodeBlockStmt(packB_body);
}

Stmt *GemmGenerator::buildReadContinuousNestedUnpackCLoop(
    const std::vector<Dim> &unpackc_order, size_t depth, size_t m_loops,
    size_t n_loops, std::vector<LoopIndex> &loop_idxs)
{
    if (depth == 0)
    {
        std::vector<BaseNode *> loop_body;

        // build dst ptr unpackc_dst + (midx...) + (nidx...) * lda

        // build src ptr buf_c + .....
        bool vectorized
            = unpackc_order[0] == Dim::M && this->scheme.vector_unpackc_enable;
        Var *pre_m_len = BaseNode::getVar(DataType::I, "1");
        Var *pre_n_len = BaseNode::getVar(DataType::I, "1");

        std::vector<BaseNode *> src_exprs;
        src_exprs.push_back(new VarRef(DataType::TPtr, "unpackc_src"));

        std::vector<BaseNode *> dst_m_idxs;
        std::vector<BaseNode *> dst_n_idxs;

        for (int i = loop_idxs.size() - 1; i >= 0; i--)
        {
            if (loop_idxs[i].dim == Dim::M)
            {
                dst_m_idxs.push_back(new VarRef(loop_idxs[i].idx));
                Expr *mul_expr = new BinaryOpExpr(OpType::Mul,
                    new VarRef(loop_idxs[i].idx), new VarRef(pre_n_len));
                pre_m_len = loop_idxs[i].len;
                src_exprs.push_back(mul_expr);
            }
            else if (loop_idxs[i].dim == Dim::N)
            {
                dst_n_idxs.push_back(new VarRef(loop_idxs[i].idx));
                Expr *mul_expr = new BinaryOpExpr(OpType::Mul,
                    new VarRef(loop_idxs[i].idx), new VarRef(pre_m_len));
                pre_n_len = loop_idxs[i].len;
                src_exprs.push_back(mul_expr);
            }
        }

        Stmt *decl_src_ptr = new VarDeclWithRVStmt(DataType::TPtr,
            "unpackcsrc_ptr", new MultiCalcExpr(OpType::Add, src_exprs), true);
        loop_body.push_back(decl_src_ptr);

        std::vector<BaseNode *> dst_ptr_exprs;
        dst_ptr_exprs.push_back(new VarRef(DataType::TPtr, "unpackc_dst"));

        Expr *sum_m_idxs = new MultiCalcExpr(OpType::Add, dst_m_idxs);
        Expr *sum_n_idxs = new MultiCalcExpr(OpType::Add, dst_n_idxs);
        Expr *mul_ldc_expr = new BinaryOpExpr(
            OpType::Mul, sum_n_idxs, new VarRef(DataType::I, "ldc"));
        dst_ptr_exprs.push_back(sum_m_idxs);
        dst_ptr_exprs.push_back(mul_ldc_expr);

        Stmt *decl_dst_ptr = new VarDeclWithRVStmt(DataType::TPtr,
            "unpackcdst_ptr", new MultiCalcExpr(OpType::Add, dst_ptr_exprs));

        loop_body.push_back(decl_dst_ptr);

        Stmt *copy_stmt = nullptr;
        if (vectorized)
        {
            copy_stmt = new VectorCopyAddStmt(
                new VarRef(BaseNode::getVar(DataType::TPtr, "unpackcdst_ptr")),
                new VarRef(BaseNode::getVar(DataType::TPtr, "unpackcsrc_ptr")),
                new VarRef(BaseNode::getVar(DataType::T, "beta")));
        }
        else
        {
            Expr *left = new UnaryOpExpr(OpType::DeAddr,
                new VarRef(BaseNode::getVar(DataType::TPtr, "unpackcdst_ptr")));
            Expr *dst_val = new UnaryOpExpr(
                OpType::DeAddr, new VarRef(DataType::TPtr, "unpackcdst_ptr"));
            Expr *src_val = new UnaryOpExpr(
                OpType::DeAddr, new VarRef(DataType::TPtr, "unpackcsrc_ptr"));
            Expr *right = new BinaryOpExpr(OpType::Add,
                new BinaryOpExpr(
                    OpType::Mul, dst_val, new VarRef(DataType::T, "beta")),
                src_val);
            copy_stmt = new VarAssignStmt(left, right);
        }

        loop_body.push_back(copy_stmt);
        return new CodeBlockStmt(loop_body);
    }
    // build cur_idx cur_len
    size_t cur_loop_id = depth - 1;
    Dim cur_dim = unpackc_order[cur_loop_id];
    std::string unpackc_idx_name = "unpackc";
    std::string unpackc_len_name = "unpackc";
    std::string unpackc_next_len_name = "unpackc";
    Expr *step_expr = nullptr;
    Expr *min_right_expr = nullptr;
    bool can_vectorize = (cur_loop_id == 0 && cur_dim == Dim::M)
        && this->scheme.vector_unpackc_enable; // NOTE: Temp disable vectorize
                                               // for unpackc
    if (cur_dim == Dim::M)
    {
        unpackc_idx_name += ("m_idx" + std::to_string(m_loops - 1));
        unpackc_len_name += ("m_len" + std::to_string(m_loops));
        unpackc_next_len_name += ("m_len" + std::to_string(m_loops - 1));

        if (m_loops - 1 == 0)
        {
            step_expr = can_vectorize
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
            min_right_expr = can_vectorize
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
    else if (cur_dim == Dim::N)
    {
        unpackc_idx_name += ("n_idx" + std::to_string(n_loops - 1));
        unpackc_len_name += ("n_len" + std::to_string(n_loops));
        unpackc_next_len_name += ("n_len" + std::to_string(n_loops - 1));

        if (n_loops - 1 == 0)
        {
            step_expr = can_vectorize
                ? new VarRef(DataType::I, "VecLen<T>::value")
                : new VarRef(DataType::I, "1");
            min_right_expr = can_vectorize
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

    LoopIndexDeclStmt *init_stmt = new LoopIndexDeclStmt(
        DataType::I, unpackc_idx_name, new VarRef(DataType::I, "0"));
    Expr *end_expr = new BinaryOpExpr(OpType::Less,
        new VarRef(DataType::I, unpackc_idx_name),
        new VarRef(DataType::I, unpackc_len_name));
    Expr *unpdate_expr = new BinaryOpExpr(OpType::AddAssign,
        new VarRef(DataType::I, unpackc_idx_name), step_expr);

    std::vector<BaseNode *> loop_body;

    Expr *sub_expr = new BinaryOpExpr(OpType::Minus,
        new VarRef(DataType::I, unpackc_len_name),
        new VarRef(DataType::I, unpackc_idx_name));
    Expr *min_expr = new MinExpr(sub_expr, min_right_expr);
    loop_body.push_back(
        new VarDeclWithRVStmt(DataType::I, unpackc_next_len_name, min_expr));

    LoopIndex loop_index;

    loop_index.dim = cur_dim;
    loop_index.len = BaseNode::getVar(DataType::I, unpackc_len_name);
    loop_index.idx = BaseNode::getVar(DataType::I, unpackc_idx_name);
    loop_idxs.push_back(loop_index);

    Stmt *child_stmt = nullptr;

    if (cur_dim == Dim::M)
    {
        child_stmt = buildReadContinuousNestedUnpackCLoop(
            unpackc_order, depth - 1, m_loops - 1, n_loops, loop_idxs);
    }
    else if (cur_dim == Dim::N)
    {
        child_stmt = buildReadContinuousNestedUnpackCLoop(
            unpackc_order, depth - 1, m_loops, n_loops - 1, loop_idxs);
    }

    loop_body.push_back(child_stmt);

    return new LoopStmt(init_stmt, end_expr, unpdate_expr, loop_body);
}

Stmt *GemmGenerator::buildReadContinuousUnpackC()
{
    std::vector<BaseNode *> unpackC_body;
    // build dst c ptr

    std::vector<BaseNode *> dst_m_idxs;
    std::vector<BaseNode *> dst_n_idxs;

    size_t m_loop_id
        = this->scheme.out_m_loop_num + this->scheme.kernel_m_loop_num;
    size_t n_loop_id
        = this->scheme.out_n_loop_num + this->scheme.kernel_n_loop_num;

    VarRef *unpackc_m_len = nullptr;
    VarRef *unpackc_n_len = nullptr;

    for (int i = this->scheme.loop_order.size() - 1; i >= this->unpackc_pos;
        i--)
    {
        if (this->scheme.loop_order[i] == Dim::M)
        {
            m_loop_id--;
            dst_m_idxs.push_back(
                new VarRef(DataType::I, "m_idx" + std::to_string(m_loop_id)));
        }
        else if (this->scheme.loop_order[i] == Dim::N)
        {
            n_loop_id--;
            dst_n_idxs.push_back(
                new VarRef(DataType::I, "n_idx" + std::to_string(n_loop_id)));
        }
    }

    unpackc_m_len = new VarRef(DataType::I, "bm" + std::to_string(m_loop_id));
    unpackc_n_len = new VarRef(DataType::I, "bn" + std::to_string(n_loop_id));

    std::vector<BaseNode *> dst_ptr_exprs;
    dst_ptr_exprs.push_back(new VarRef(DataType::TPtr, "C"));

    if (dst_m_idxs.size() != 0)
    {
        Expr *sum_m_idxs = new MultiCalcExpr(OpType::Add, dst_m_idxs);
        dst_ptr_exprs.push_back(sum_m_idxs);
    }

    if (dst_n_idxs.size() != 0)
    {
        Expr *sum_n_idxs = new MultiCalcExpr(OpType::Add, dst_n_idxs);
        Expr *mul_ldc = new BinaryOpExpr(
            OpType::Mul, sum_n_idxs, new VarRef(DataType::I, "ldc"));
        dst_ptr_exprs.push_back(mul_ldc);
    }

    Expr *sum_ptr_expr = new MultiCalcExpr(OpType::Add, dst_ptr_exprs);
    Stmt *decl_dst_ptr
        = new VarDeclWithRVStmt(DataType::TPtr, "unpackc_dst", sum_ptr_expr);
    unpackC_body.push_back(decl_dst_ptr);
    unpackC_body.push_back(new VarDeclWithRVStmt(
        DataType::TPtr, "unpackc_src", new VarRef(DataType::TPtr, "buf_c")));

    // build read continuous nested loop

    std::vector<Dim> unpackc_order;
    size_t m_loops = 0;
    size_t n_loops = 0;

    for (size_t i = 0; i < this->unpackc_pos; i++)
    {
        if (this->scheme.loop_order[i] == Dim::M)
        {
            m_loops++;
            unpackc_order.push_back(Dim::M);
        }
        else if (this->scheme.loop_order[i] == Dim::N)
        {
            n_loops++;
            unpackc_order.push_back(Dim::N);
        }
    }

    Stmt *decl_m_len = new VarDeclWithRVStmt(
        DataType::I, "unpackcm_len" + std::to_string(m_loops), unpackc_m_len);
    Stmt *decl_n_len = new VarDeclWithRVStmt(
        DataType::I, "unpackcn_len" + std::to_string(n_loops), unpackc_n_len);
    unpackC_body.push_back(decl_m_len);
    unpackC_body.push_back(decl_n_len);

    std::vector<std::string> midx_names, nidx_names;
    std::vector<LoopIndex> loop_idxs;
    unpackC_body.push_back(buildReadContinuousNestedUnpackCLoop(
        unpackc_order, unpackc_order.size(), m_loops, n_loops, loop_idxs));

    return new CodeBlockStmt(unpackC_body);
}
} // namespace loopmaster