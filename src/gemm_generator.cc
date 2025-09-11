#include "common.h"

namespace loopmaster
{
GemmGenerator::GemmGenerator(const SchemeInfo &scheme_)
    : scheme(scheme_)
{
    std::string scheme_name = this->scheme.toString();
    this->file_name = scheme_name + ".hpp";
    this->func_name = scheme_name + "_func";

    this->packa_flag = this->scheme.packa;
    this->packb_flag = this->scheme.packb;
    this->unpackc_flag = this->scheme.unpackc;

    this->packa_pos = this->scheme.packa_pos;
    this->packb_pos = this->scheme.packb_pos;
    this->unpackc_pos = this->scheme.unpackc_pos;

    root = ModuleAST(this->file_name, {});

    getABCLoopIndexOrder();
}

void GemmGenerator::getABCLoopIndexOrder()
{
    int m_idx = 0;
    int n_idx = 0;
    int k_idx = 0;
    for (size_t i = 0; i < this->scheme.loop_order.size(); i++)
    {
        LoopIndex index;
        index.is_unit = false;
        index.packa_here = (i == this->packa_pos);

        index.packb_here = (i == this->packb_pos);

        index.unpackc_here = (i == this->unpackc_pos);

        if (this->scheme.loop_order[i] == Dim::M)
        {
            index.dim = Dim::M;
            index.idx_name = "m_" + std::to_string(m_idx++);
            this->a_loop_indexs.push_back(index);
            this->c_loop_indexs.push_back(index);
            this->loop_indexs.push_back(index);
        }
        else if (this->scheme.loop_order[i] == Dim::N)
        {
            index.dim = Dim::N;
            index.idx_name = "n_" + std::to_string(n_idx++);
            this->b_loop_indexs.push_back(index);
            this->c_loop_indexs.push_back(index);
            this->loop_indexs.push_back(index);
        }
        else if (this->scheme.loop_order[i] == Dim::K)
        {
            index.dim = Dim::K;
            index.idx_name = "k_" + std::to_string(k_idx++);
            this->a_loop_indexs.push_back(index);
            this->b_loop_indexs.push_back(index);
            this->loop_indexs.push_back(index);
        }
    }
}

std::vector<BaseNode *> GemmGenerator::buildTargList()
{
    std::vector<BaseNode *> ret;

    ret.push_back(new ArgDeclStmt(DataType::Typename, "T"));

    for (size_t m_idx = 0; m_idx < this->scheme.m_loop_num - 1; m_idx++)
        ret.push_back(
            new ArgDeclStmt(DataType::I, "BM" + std::to_string(m_idx)));

    for (size_t n_idx = 0; n_idx < this->scheme.n_loop_num - 1; n_idx++)
        ret.push_back(
            new ArgDeclStmt(DataType::I, "BN" + std::to_string(n_idx)));

    for (size_t k_idx = 0; k_idx < this->scheme.k_loop_num - 1; k_idx++)
        ret.push_back(
            new ArgDeclStmt(DataType::I, "BK" + std::to_string(k_idx)));

    return ret;
}

std::vector<BaseNode *> GemmGenerator::buildFargList()
{
    std::vector<BaseNode *> ret;

    ret.push_back(new ArgDeclStmt(DataType::I, "m", true));
    ret.push_back(new ArgDeclStmt(DataType::I, "n", true));
    ret.push_back(new ArgDeclStmt(DataType::I, "k", true));
    ret.push_back(new ArgDeclStmt(DataType::T, "alpha", true));

    ret.push_back(new ArgDeclStmt(DataType::TPtr, "A", true));
    ret.push_back(new ArgDeclStmt(DataType::I, "lda", true));
    ret.push_back(new ArgDeclStmt(DataType::TPtr, "B", true));
    ret.push_back(new ArgDeclStmt(DataType::I, "ldb", true));
    ret.push_back(new ArgDeclStmt(DataType::T, "beta", true));
    ret.push_back(new ArgDeclStmt(DataType::TPtr, "C"));
    ret.push_back(new ArgDeclStmt(DataType::I, "ldc", true));

    return ret;
}

Expr *GemmGenerator::buildPackedKerAptrExpr()
{
    Expr *bufa_ptr = new VarRef(DataType::TPtr, "buf_a");

    std::vector<BaseNode *> offset_exprs;
    offset_exprs.push_back(bufa_ptr);

    size_t m_idx = this->scheme.kernel_m_loop_num;
    size_t k_idx = this->scheme.kernel_k_loop_num;

    std::string pre_m_len_name = "bm" + std::to_string(m_idx);
    std::string pre_k_len_name = "bk" + std::to_string(k_idx);

    for (size_t i = this->scheme.kernel_order.size(); i < this->packa_pos; i++)
    {
        Dim cur_dim = this->scheme.loop_order[i];

        if (cur_dim == Dim::M)
        {
            Expr *m_idx_ref
                = new VarRef(DataType::I, "m_idx" + std::to_string(m_idx));
            Expr *k_len_ref = new VarRef(DataType::I, pre_k_len_name);
            Expr *mul_expr
                = new BinaryOpExpr(OpType::Mul, m_idx_ref, k_len_ref);
            offset_exprs.push_back(mul_expr);

            m_idx++;
            pre_m_len_name = "bm" + std::to_string(m_idx);
        }
        else if (cur_dim == Dim::K)
        {
            Expr *k_idx_ref
                = new VarRef(DataType::I, "k_idx" + std::to_string(k_idx));
            Expr *m_len_ref = new VarRef(DataType::I, pre_m_len_name);
            Expr *mul_expr
                = new BinaryOpExpr(OpType::Mul, k_idx_ref, m_len_ref);
            offset_exprs.push_back(mul_expr);

            k_idx++;
            pre_k_len_name = "bk" + std::to_string(k_idx);
        }
    }

    return new MultiCalcExpr(OpType::Add, offset_exprs);
}

Expr *GemmGenerator::buildPackedKerBptrExpr()
{
    Expr *bufb_ptr = new VarRef(DataType::TPtr, "buf_b");

    std::vector<BaseNode *> offset_exprs;
    offset_exprs.push_back(bufb_ptr);

    size_t n_idx = this->scheme.kernel_n_loop_num;
    size_t k_idx = this->scheme.kernel_k_loop_num;

    std::string pre_n_len_name = "bn" + std::to_string(n_idx);
    std::string pre_k_len_name = "bk" + std::to_string(k_idx);

    for (size_t i = this->scheme.kernel_order.size(); i < this->packb_pos; i++)
    {
        Dim cur_dim = this->scheme.loop_order[i];

        if (cur_dim == Dim::N)
        {
            Expr *n_idx_ref
                = new VarRef(DataType::I, "n_idx" + std::to_string(n_idx));
            Expr *k_len_ref = new VarRef(DataType::I, pre_k_len_name);
            Expr *mul_expr
                = new BinaryOpExpr(OpType::Mul, n_idx_ref, k_len_ref);
            offset_exprs.push_back(mul_expr);

            n_idx++;
            pre_n_len_name = "bn" + std::to_string(n_idx);
        }
        else if (cur_dim == Dim::K)
        {
            Expr *k_idx_ref
                = new VarRef(DataType::I, "k_idx" + std::to_string(k_idx));
            Expr *n_len_ref = new VarRef(DataType::I, pre_n_len_name);
            Expr *mul_expr
                = new BinaryOpExpr(OpType::Mul, k_idx_ref, n_len_ref);
            offset_exprs.push_back(mul_expr);

            k_idx++;
            pre_k_len_name = "bk" + std::to_string(k_idx);
        }
    }

    return new MultiCalcExpr(OpType::Add, offset_exprs);
}

Expr *GemmGenerator::buildPackedKerCptrExpr()
{
    Expr *bufc_ptr = new VarRef(DataType::TPtr, "buf_c");

    std::vector<BaseNode *> offset_exprs;
    offset_exprs.push_back(bufc_ptr);

    size_t m_idx = this->scheme.kernel_m_loop_num;
    size_t n_idx = this->scheme.kernel_n_loop_num;

    std::string pre_m_len_name = "bm" + std::to_string(m_idx);
    std::string pre_n_len_name = "bn" + std::to_string(n_idx);

    for (size_t i = this->scheme.kernel_order.size(); i < this->unpackc_pos;
        i++)
    {
        Dim cur_dim = this->scheme.loop_order[i];

        if (cur_dim == Dim::M)
        {
            Expr *m_idx_ref
                = new VarRef(DataType::I, "m_idx" + std::to_string(m_idx));
            Expr *n_len_ref = new VarRef(DataType::I, pre_n_len_name);
            Expr *mul_expr
                = new BinaryOpExpr(OpType::Mul, m_idx_ref, n_len_ref);
            offset_exprs.push_back(mul_expr);

            m_idx++;
            pre_m_len_name = "bm" + std::to_string(m_idx);
        }
        else if (cur_dim == Dim::N)
        {
            Expr *n_idx_ref
                = new VarRef(DataType::I, "n_idx" + std::to_string(n_idx));
            Expr *m_len_ref = new VarRef(DataType::I, pre_m_len_name);
            Expr *mul_expr
                = new BinaryOpExpr(OpType::Mul, n_idx_ref, m_len_ref);
            offset_exprs.push_back(mul_expr);

            n_idx++;
            pre_n_len_name = "bn" + std::to_string(n_idx);
        }
    }

    return new MultiCalcExpr(OpType::Add, offset_exprs);
}

Expr *GemmGenerator::buildColMajorOffsetExpr(Dim dim0, int dim0_out_num,
    int dim0_ker_num, Dim dim1, int dim1_out_num, int dim1_ker_num,
    const std::string &ld_name)
{
    std::vector<BaseNode *> dim0_idxs;
    std::vector<BaseNode *> dim1_idxs;

    for (size_t i = 0; i < dim0_out_num; i++)
        dim0_idxs.push_back(new VarRef(
            DataType::I, toIdx(dim0) + std::to_string(i + dim0_ker_num)));
    for (size_t i = 0; i < dim1_out_num; i++)
        dim1_idxs.push_back(new VarRef(
            DataType::I, toIdx(dim1) + std::to_string(i + dim1_ker_num)));
    Expr *sum_dim0_idx = nullptr;
    if (!dim0_idxs.empty())
        sum_dim0_idx = new MultiCalcExpr(OpType::Add, dim0_idxs);
    else
        sum_dim0_idx = new ConstNumExpr(0);

    Expr *sum_dim1_idx = nullptr;
    if (!dim1_idxs.empty())
        sum_dim1_idx = new MultiCalcExpr(OpType::Add, dim1_idxs);
    else
        sum_dim1_idx = new ConstNumExpr(0);

    Expr *mul_ld = new BinaryOpExpr(
        OpType::Mul, sum_dim1_idx, new VarRef(DataType::I, ld_name));

    Expr *ret = new BinaryOpExpr(OpType::Add, sum_dim0_idx, mul_ld);

    return ret;
}

Expr *GemmGenerator::buildColMajorAoffsetExpr()
{
    std::vector<BaseNode *> m_idxs;
    std::vector<BaseNode *> k_idxs;

    for (size_t i = 0; i < this->scheme.out_m_loop_num; i++)
        m_idxs.push_back(new VarRef(DataType::I,
            "m_idx" + std::to_string(i + this->scheme.kernel_m_loop_num)));

    for (size_t i = 0; i < this->scheme.out_k_loop_num; i++)
        k_idxs.push_back(new VarRef(DataType::I,
            "k_idx" + std::to_string(i + this->scheme.kernel_k_loop_num)));
    Expr *sum_m_idx = nullptr;
    if (!m_idxs.empty())
        sum_m_idx = new MultiCalcExpr(OpType::Add, m_idxs);
    else
        sum_m_idx = new ConstNumExpr(0);

    Expr *sum_k_idx = nullptr;
    if (!k_idxs.empty())
        sum_k_idx = new MultiCalcExpr(OpType::Add, k_idxs);
    else
        sum_k_idx = new ConstNumExpr(1);

    Expr *k_mul_lda = new BinaryOpExpr(
        OpType::Mul, sum_k_idx, new VarRef(DataType::I, "lda"));

    Expr *ret = new BinaryOpExpr(OpType::Add, sum_m_idx, k_mul_lda);

    return ret;
}

Expr *GemmGenerator::buildColMajorKerAptrExpr()
{
    Expr *a_offset = buildColMajorOffsetExpr(Dim::M,
        this->scheme.out_m_loop_num, this->scheme.kernel_m_loop_num, Dim::K,
        this->scheme.out_k_loop_num, this->scheme.kernel_k_loop_num, "lda");

    return new BinaryOpExpr(
        OpType::Add, new VarRef(DataType::TPtr, "A"), a_offset);
}

Expr *GemmGenerator::buildColMajorBoffsetExpr()
{
    std::vector<BaseNode *> n_idxs;
    std::vector<BaseNode *> k_idxs;

    for (size_t i = 0; i < this->scheme.out_n_loop_num; i++)
        n_idxs.push_back(new VarRef(DataType::I,
            "n_idx" + std::to_string(i + this->scheme.kernel_n_loop_num)));

    for (size_t i = 0; i < this->scheme.out_k_loop_num; i++)
        k_idxs.push_back(new VarRef(DataType::I,
            "k_idx" + std::to_string(i + this->scheme.kernel_k_loop_num)));

    Expr *sum_n_idx = nullptr;
    if (!n_idxs.empty())
        sum_n_idx = new MultiCalcExpr(OpType::Add, n_idxs);
    else
        sum_n_idx = new ConstNumExpr(1);
    Expr *sum_k_idx = nullptr;
    if (!k_idxs.empty())
        sum_k_idx = new MultiCalcExpr(OpType::Add, k_idxs);
    else
        sum_k_idx = new ConstNumExpr(0);

    Expr *n_mul_ldb = new BinaryOpExpr(
        OpType::Mul, sum_n_idx, new VarRef(DataType::I, "ldb"));

    Expr *ret = new BinaryOpExpr(OpType::Add, sum_k_idx, n_mul_ldb);

    return ret;
}

Expr *GemmGenerator::buildColMajorKerBptrExpr()
{
    Expr *b_offset = buildColMajorOffsetExpr(Dim::K,
        this->scheme.out_k_loop_num, this->scheme.kernel_k_loop_num, Dim::N,
        this->scheme.out_n_loop_num, this->scheme.kernel_n_loop_num, "ldb");
    return new BinaryOpExpr(
        OpType::Add, new VarRef(DataType::TPtr, "B"), b_offset);
}

Expr *GemmGenerator::buildColMajorCoffsetExpr()
{
    std::vector<BaseNode *> m_idxs;
    std::vector<BaseNode *> n_idxs;

    for (size_t i = 0; i < this->scheme.out_n_loop_num; i++)
        n_idxs.push_back(new VarRef(DataType::I,
            "n_idx" + std::to_string(i + this->scheme.kernel_n_loop_num)));

    for (size_t i = 0; i < this->scheme.out_m_loop_num; i++)
        m_idxs.push_back(new VarRef(DataType::I,
            "m_idx" + std::to_string(i + this->scheme.kernel_m_loop_num)));

    Expr *sum_n_idx = new MultiCalcExpr(OpType::Add, n_idxs);
    Expr *sum_m_idx = new MultiCalcExpr(OpType::Add, m_idxs);

    Expr *n_mul_ldc = new BinaryOpExpr(
        OpType::Mul, sum_n_idx, new VarRef(DataType::I, "ldc"));

    Expr *ret = new BinaryOpExpr(OpType::Add, sum_m_idx, n_mul_ldc);

    return ret;
}

Expr *GemmGenerator::buildColMajorKerCptrExpr()
{
    Expr *c_offset = buildColMajorOffsetExpr(Dim::M,
        this->scheme.out_m_loop_num, this->scheme.kernel_m_loop_num, Dim::N,
        this->scheme.out_n_loop_num, this->scheme.kernel_n_loop_num, "ldc");
    return new BinaryOpExpr(
        OpType::Add, new VarRef(DataType::TPtr, "C"), c_offset);
}

Stmt *GemmGenerator::buildGemmLoop(
    size_t loop_id, size_t m_loops, size_t n_loops, size_t k_loops)
{
    if (loop_id == 0)
    {
        std::vector<BaseNode *> body;
        Expr *aptr_rv_expr = ((this->packa_flag != PackType::NoPack)
                ? buildPackedKerAptrExpr()
                : buildColMajorKerAptrExpr());
        Expr *bptr_rv_expr = ((this->packb_flag != PackType::NoPack)
                ? buildPackedKerBptrExpr()
                : buildColMajorKerBptrExpr());
        Expr *cptr_rv_expr = ((this->unpackc_flag != PackType::NoPack)
                ? buildPackedKerCptrExpr()
                : buildColMajorKerCptrExpr());

        VarDeclWithRVStmt *decl_aptr_arg = new VarDeclWithRVStmt(
            DataType::TPtr, "ker_aptr", aptr_rv_expr, true);
        VarDeclWithRVStmt *decl_bptr_arg = new VarDeclWithRVStmt(
            DataType::TPtr, "ker_bptr", bptr_rv_expr, true);
        VarDeclWithRVStmt *decl_cptr_arg
            = new VarDeclWithRVStmt(DataType::TPtr, "ker_cptr", cptr_rv_expr);

        Expr *m_arg_ref
            = new VarRef(DataType::I, "bm" + std::to_string(m_loops));
        Expr *n_arg_ref
            = new VarRef(DataType::I, "bn" + std::to_string(n_loops));
        Expr *k_arg_ref
            = new VarRef(DataType::I, "bk" + std::to_string(k_loops));

        std::string lda_arg_name
            = ((this->packa_flag != PackType::NoPack) ? "BM0" : "lda");
        std::string ldb_arg_name
            = ((this->packb_flag != PackType::NoPack) ? "BN0" : "ldb");
        std::string ldc_arg_name
            = ((this->unpackc_flag != PackType::NoPack) ? "BM0" : "ldc");

        Expr *lda_arg_ref = new VarRef(DataType::I, lda_arg_name);
        Expr *ldb_arg_ref = new VarRef(DataType::I, ldb_arg_name);
        Expr *ldc_arg_ref = new VarRef(DataType::I, ldc_arg_name);

        Expr *alpha_arg_ref = new VarRef(DataType::T, "alpha");

        std::vector<BaseNode *> func_args { m_arg_ref, n_arg_ref, k_arg_ref,
            alpha_arg_ref, new VarRef(DataType::TPtr, "ker_aptr"), lda_arg_ref,
            new VarRef(DataType::TPtr, "ker_bptr"), ldb_arg_ref,
            new VarRef(DataType::TPtr, "ker_cptr"), ldc_arg_ref };

        Stmt *kernel_call_stmt = new KernelCallStmt("GemmKernel",
            this->scheme.kernel_id, this->packa_flag != PackType::NoPack,
            this->packb_flag != PackType::NoPack,
            this->unpackc_flag != PackType::NoPack, func_args);
        body
            = { decl_aptr_arg, decl_bptr_arg, decl_cptr_arg, kernel_call_stmt };
        return new CodeBlockStmt(body);
    }

    LoopIndexDeclStmt *init = nullptr;

    size_t cur_loop = loop_id - 1 + this->scheme.kernel_order.size();

    if (this->scheme.loop_order[cur_loop] == Dim::M)
        init = new LoopIndexDeclStmt(DataType::I,
            "m_idx" + std::to_string(m_loops - 1),
            new VarRef(DataType::I, "0"));
    else if (this->scheme.loop_order[cur_loop] == Dim::N)
        init = new LoopIndexDeclStmt(DataType::I,
            "n_idx" + std::to_string(n_loops - 1),
            new VarRef(DataType::I, "0"));
    else if (this->scheme.loop_order[cur_loop] == Dim::K)
        init = new LoopIndexDeclStmt(DataType::I,
            "k_idx" + std::to_string(k_loops - 1),
            new VarRef(DataType::I, "0"));

    Expr *end_flag = nullptr;
    if (this->scheme.loop_order[cur_loop] == Dim::M)
        end_flag = new BinaryOpExpr(OpType::Less,
            new VarRef(init->getDecledVar()),
            new VarRef(
                BaseNode::getVar(DataType::I, "bm" + std::to_string(m_loops))));
    else if (this->scheme.loop_order[cur_loop] == Dim::N)
        end_flag = new BinaryOpExpr(OpType::Less,
            new VarRef(init->getDecledVar()),
            new VarRef(
                BaseNode::getVar(DataType::I, "bn" + std::to_string(n_loops))));
    else if (this->scheme.loop_order[cur_loop] == Dim::K)
        end_flag = new BinaryOpExpr(OpType::Less,
            new VarRef(init->getDecledVar()),
            new VarRef(
                BaseNode::getVar(DataType::I, "bk" + std::to_string(k_loops))));

    Expr *update = nullptr;
    if (this->scheme.loop_order[cur_loop] == Dim::M)
        update = new BinaryOpExpr(OpType::AddAssign,
            new VarRef(init->getDecledVar()),
            new VarRef(BaseNode::getVar(
                DataType::I, "BM" + std::to_string(m_loops - 2))));
    else if (this->scheme.loop_order[cur_loop] == Dim::N)
        update = new BinaryOpExpr(OpType::AddAssign,
            new VarRef(init->getDecledVar()),
            new VarRef(BaseNode::getVar(
                DataType::I, "BN" + std::to_string(n_loops - 2))));
    else if (this->scheme.loop_order[cur_loop] == Dim::K)
        update = new BinaryOpExpr(OpType::AddAssign,
            new VarRef(init->getDecledVar()),
            new VarRef(BaseNode::getVar(
                DataType::I, "BK" + std::to_string(k_loops - 2))));

    std::vector<BaseNode *> loop_body;

    Stmt *block_size_decl = nullptr;

    if (this->scheme.loop_order[cur_loop] == Dim::M)
    {
        Expr *minus_expr = new BinaryOpExpr(OpType::Minus,
            new VarRef(
                BaseNode::getVar(DataType::I, "bm" + std::to_string(m_loops))),
            new VarRef(init->getDecledVar()));

        Expr *min_expr = new MinExpr(minus_expr,
            new VarRef(BaseNode::getVar(
                DataType::I, "BM" + std::to_string(m_loops - 2))));

        block_size_decl = new VarDeclWithRVStmt(
            DataType::I, "bm" + std::to_string(m_loops - 1), min_expr);
    }
    else if (this->scheme.loop_order[cur_loop] == Dim::N)
    {
        Expr *minus_expr = new BinaryOpExpr(OpType::Minus,
            new VarRef(
                BaseNode::getVar(DataType::I, "bn" + std::to_string(n_loops))),
            new VarRef(init->getDecledVar()));

        Expr *min_expr = new MinExpr(minus_expr,
            new VarRef(BaseNode::getVar(
                DataType::I, "BN" + std::to_string(n_loops - 2))));

        block_size_decl = new VarDeclWithRVStmt(
            DataType::I, "bn" + std::to_string(n_loops - 1), min_expr);
    }
    else if (this->scheme.loop_order[cur_loop] == Dim::K)
    {
        Expr *minus_expr = new BinaryOpExpr(OpType::Minus,
            new VarRef(
                BaseNode::getVar(DataType::I, "bk" + std::to_string(k_loops))),
            new VarRef(init->getDecledVar()));

        Expr *min_expr = new MinExpr(minus_expr,
            new VarRef(BaseNode::getVar(
                DataType::I, "BK" + std::to_string(k_loops - 2))));

        block_size_decl = new VarDeclWithRVStmt(
            DataType::I, "bk" + std::to_string(k_loops - 1), min_expr);
    }

    loop_body.push_back(block_size_decl);

    if (this->packa_flag != PackType::NoPack && cur_loop == this->packa_pos)
    {
        // add packa
        loop_body.push_back(new CommentStmt("here packa."));

        // add packa code block
        if (this->packa_flag == PackType::ReadContinuous)
            loop_body.push_back(buildReadContinuousPackA());
        else if (this->packa_flag == PackType::WriteContinuous)
            loop_body.push_back(buildWriteContinuousPackA());
    }

    if (this->packb_flag != PackType::NoPack && cur_loop == this->packb_pos)
    {
        // add packb
        loop_body.push_back(new CommentStmt("here packb."));

        // add packb code block
        if (this->packb_flag == PackType::ReadContinuous)
            loop_body.push_back(buildReadContinuousPackB());
        else if (this->packb_flag == PackType::WriteContinuous)
            loop_body.push_back(buildWriteContinuousPackB());
    }

    // init c buf
    if (this->unpackc_flag != PackType::NoPack && cur_loop == this->unpackc_pos)
    {
        loop_body.push_back(new CommentStmt("here zero c_buf."));
        loop_body.push_back(buildBufCInitElemNum());
        loop_body.push_back(new ZeroBufStmt(
            new VarRef(BaseNode::getVar(DataType::TPtr, "buf_c")),
            buildBufCInitSizeExpr()));
    }

    Stmt *child_loop = nullptr;

    if (this->scheme.loop_order[cur_loop] == Dim::M)
        child_loop = buildGemmLoop(loop_id - 1, m_loops - 1, n_loops, k_loops);
    else if (this->scheme.loop_order[cur_loop] == Dim::N)
        child_loop = buildGemmLoop(loop_id - 1, m_loops, n_loops - 1, k_loops);
    else if (this->scheme.loop_order[cur_loop] == Dim::K)
        child_loop = buildGemmLoop(loop_id - 1, m_loops, n_loops, k_loops - 1);

    loop_body.push_back(child_loop);

    if (this->unpackc_flag != PackType::NoPack && cur_loop == this->unpackc_pos)
    {
        loop_body.push_back(new CommentStmt("here unpack c."));
        loop_body.push_back(buildReadContinuousUnpackC());
    }

    LoopStmt *lstmt = new LoopStmt(init, end_flag, update, loop_body);

    return lstmt;
}

MultiCalcExpr *GemmGenerator::buildBufAElemNumExpr()
{
    MultiCalcExpr *ret = nullptr;

    size_t outest_m_id
        = this->scheme.out_m_loop_num + this->scheme.kernel_m_loop_num;
    size_t outest_k_id
        = this->scheme.out_k_loop_num + this->scheme.kernel_k_loop_num;
    size_t m_id = outest_m_id;
    size_t k_id = outest_k_id;

    for (int i = this->scheme.loop_order.size() - 1;
        i >= this->packa_pos && i >= 0; i--)
    {
        Dim &cur_dim = this->scheme.loop_order[i];
        if (cur_dim == Dim::M)
            m_id--;
        if (cur_dim == Dim::K)
            k_id--;
    }

    std::string m_size_name
        = (m_id == outest_m_id) ? "m" : "BM" + std::to_string(m_id - 1);
    std::string k_size_name
        = (k_id == outest_k_id) ? "k" : "BK" + std::to_string(k_id - 1);

    Expr *outestBM_ref = new VarRef(BaseNode::getVar(DataType::I, m_size_name));
    Expr *outestBK_ref = new VarRef(BaseNode::getVar(DataType::I, k_size_name));

    Expr *outestBM_real
        = new MinExpr(outestBM_ref, new VarRef(DataType::I, "m"));
    Expr *outestBK_real
        = new MinExpr(outestBK_ref, new VarRef(DataType::I, "k"));

    std::vector<BaseNode *> ops { outestBM_real, outestBK_real };
    ret = new MultiCalcExpr(OpType::Mul, ops);

    return ret;
}

MultiCalcExpr *GemmGenerator::buildBufASizeExpr()
{
    MultiCalcExpr *ret = buildBufAElemNumExpr();
    ret->operands.push_back(new SizeofTypeExpr(DataType::T));
    return ret;
}

MultiCalcExpr *GemmGenerator::buildBufBElemNumExpr()
{
    MultiCalcExpr *ret = nullptr;

    size_t outest_n_id
        = this->scheme.out_n_loop_num + this->scheme.kernel_n_loop_num;
    size_t outest_k_id
        = this->scheme.out_k_loop_num + this->scheme.kernel_k_loop_num;
    size_t n_id = outest_n_id;
    size_t k_id = outest_k_id;

    for (int i = this->scheme.loop_order.size() - 1;
        i >= this->packb_pos && i >= 0; i--)
    {
        Dim &cur_dim = this->scheme.loop_order[i];
        if (cur_dim == Dim::N)
            n_id--;
        if (cur_dim == Dim::K)
            k_id--;
    }

    std::string n_size_name
        = (n_id == outest_n_id) ? "n" : "BN" + std::to_string(n_id - 1);
    std::string k_size_name
        = (k_id == outest_k_id) ? "k" : "BK" + std::to_string(k_id - 1);

    Expr *outestBN_ref = new VarRef(BaseNode::getVar(DataType::I, n_size_name));
    Expr *outestBK_ref = new VarRef(BaseNode::getVar(DataType::I, k_size_name));

    Expr *outestBN_real
        = new MinExpr(outestBN_ref, new VarRef(DataType::I, "n"));
    Expr *outestBK_real
        = new MinExpr(outestBK_ref, new VarRef(DataType::I, "k"));

    std::vector<BaseNode *> ops { outestBN_real, outestBK_real };
    ret = new MultiCalcExpr(OpType::Mul, ops);

    return ret;
}

MultiCalcExpr *GemmGenerator::buildBufBSizeExpr()
{
    MultiCalcExpr *ret = buildBufBElemNumExpr();
    ret->operands.push_back(new SizeofTypeExpr(DataType::T));
    return ret;
}

MultiCalcExpr *GemmGenerator::buildBufCElemNumExpr()
{
    MultiCalcExpr *ret = nullptr;

    size_t outest_k_loopid = 0;

    size_t outest_m_id
        = this->scheme.out_m_loop_num + this->scheme.kernel_m_loop_num;
    size_t outest_n_id
        = this->scheme.out_n_loop_num + this->scheme.kernel_n_loop_num;
    size_t out_m_id = outest_m_id;
    size_t out_n_id = outest_n_id;

    for (int i = this->scheme.loop_order.size() - 1;
        i >= this->unpackc_pos && i >= 0; i--)
    {
        if (this->scheme.loop_order[i] == Dim::M)
            out_m_id--;
        else if (this->scheme.loop_order[i] == Dim::N)
            out_n_id--;
    }

    std::string m_size_name
        = (out_m_id == outest_m_id ? "m" : "BM" + std::to_string(out_m_id - 1));
    std::string n_size_name
        = (out_n_id == outest_n_id ? "n" : "BN" + std::to_string(out_n_id - 1));

    Expr *m_ref = new VarRef(BaseNode::getVar(DataType::I, m_size_name));
    Expr *n_ref = new VarRef(BaseNode::getVar(DataType::I, n_size_name));

    Expr *m_real = new MinExpr(m_ref, new VarRef(DataType::I, "m"));
    Expr *n_real = new MinExpr(n_ref, new VarRef(DataType::I, "n"));

    std::vector<BaseNode *> ops { m_real, n_real };
    ret = new MultiCalcExpr(OpType::Mul, ops);

    return ret;
}

MultiCalcExpr *GemmGenerator::buildBufCSizeExpr()
{
    MultiCalcExpr *ret = buildBufCElemNumExpr();
    ret->operands.push_back(new SizeofTypeExpr(DataType::T));
    return ret;
}

VarDeclWithRVStmt *GemmGenerator::buildBufCInitElemNum()
{
    VarDeclWithRVStmt *bufc_elem = new VarDeclWithRVStmt(
        DataType::I, "buf_c_elem_num", buildBufCElemNumExpr());
    return bufc_elem;
}

Expr *GemmGenerator::buildBufCInitSizeExpr()
{
    Expr *ret = nullptr;

    Expr *buf_c_elem_num_ref = new VarRef(DataType::I, "buf_c_elem_num");
    Expr *sizeof_expr = new SizeofTypeExpr(DataType::T);

    ret = new BinaryOpExpr(OpType::Mul, buf_c_elem_num_ref, sizeof_expr);

    return ret;
}

std::vector<BaseNode *> GemmGenerator::buildFuncBody()
{
    std::vector<BaseNode *> ret;

    size_t outest_m_idx
        = this->scheme.out_m_loop_num + this->scheme.kernel_m_loop_num;
    size_t outest_n_idx
        = this->scheme.out_n_loop_num + this->scheme.kernel_n_loop_num;
    size_t outest_k_idx
        = this->scheme.out_k_loop_num + this->scheme.kernel_k_loop_num;

    /* alias m n k */
    ret.push_back(
        new VarDeclWithRVStmt(DataType::I, "bm" + std::to_string(outest_m_idx),
            new VarRef(BaseNode::getVar(DataType::I, "m"))));

    ret.push_back(
        new VarDeclWithRVStmt(DataType::I, "bn" + std::to_string(outest_n_idx),
            new VarRef(BaseNode::getVar(DataType::I, "n"))));

    ret.push_back(
        new VarDeclWithRVStmt(DataType::I, "bk" + std::to_string(outest_k_idx),
            new VarRef(BaseNode::getVar(DataType::I, "k"))));

    /* alloc buf for packa, packb, unpackc */

    if (this->packa_flag != PackType::NoPack)
    {
        Expr *bufa_size = buildBufASizeExpr();
        ret.push_back(new VarDeclWithRVStmt(DataType::TPtr, "buf_a",
            new AlignedAllocExpr(new ConstNumExpr(128), bufa_size)));
    }

    if (this->packb_flag != PackType::NoPack)
    {
        Expr *bufb_size = buildBufBSizeExpr();
        ret.push_back(new VarDeclWithRVStmt(DataType::TPtr, "buf_b",
            new AlignedAllocExpr(new ConstNumExpr(128), bufb_size)));
    }

    if (this->unpackc_flag != PackType::NoPack)
    {
        Expr *bufc_size = buildBufCSizeExpr();
        ret.push_back(new VarDeclWithRVStmt(DataType::TPtr, "buf_c",
            new AlignedAllocExpr(new ConstNumExpr(128), bufc_size)));
    }
    else
    {
        // if no pack c, we should do c beta additionally
        ret.push_back(new CbetaOpStmt(new VarRef(DataType::I, "m"),
            new VarRef(DataType::I, "n"), new VarRef(DataType::TPtr, "C"),
            new VarRef(DataType::I, "ldc"), new VarRef(DataType::T, "beta"),
            new VarRef(DataType::I, "0"), new VarRef(DataType::I, "1")));
    }

    if (this->unpackc_pos == this->scheme.loop_order.size())
    {
        // here memset c buf
        ret.push_back(new CommentStmt("here memset c"));
        ret.push_back(buildBufCInitElemNum());
        ret.push_back(new ZeroBufStmt(
            new VarRef(BaseNode::getVar(DataType::TPtr, "buf_c")),
            buildBufCInitSizeExpr()));
    }

    if (this->packa_pos == this->scheme.loop_order.size())
    {
        ret.push_back(new CommentStmt("here packa."));
        if (this->packa_flag == PackType::ReadContinuous)
        {
            ret.push_back(this->buildReadContinuousPackA());
        }
        else if (this->packa_flag == PackType::WriteContinuous)
        {
            ret.push_back(this->buildWriteContinuousPackA());
        }
    }

    if (this->packb_pos == this->scheme.loop_order.size())
    {
        ret.push_back(new CommentStmt("here packb."));
        if (this->packb_flag == PackType::ReadContinuous)
        {
            ret.push_back(this->buildReadContinuousPackB());
        }
        else if (this->packb_flag == PackType::WriteContinuous)
        {
            ret.push_back(this->buildWriteContinuousPackB());
        }
    }

    Stmt *loop_stmt = buildGemmLoop(
        this->scheme.total_loop_num - this->scheme.kernel_order.size(),
        this->scheme.out_m_loop_num + this->scheme.kernel_m_loop_num,
        this->scheme.out_n_loop_num + this->scheme.kernel_n_loop_num,
        this->scheme.out_k_loop_num + this->scheme.kernel_k_loop_num);
    ret.push_back(loop_stmt);

    if (this->unpackc_pos == this->scheme.loop_order.size())
    {
        // here unpack c
        ret.push_back(new CommentStmt("here unpackc"));
        ret.push_back(buildReadContinuousUnpackC());
    }

    if (this->packa_flag != PackType::NoPack)
        ret.push_back(new FreeBufStmt(
            new VarRef(BaseNode::getVar(DataType::TPtr, "buf_a"))));

    if (this->packb_flag != PackType::NoPack)
        ret.push_back(new FreeBufStmt(
            new VarRef(BaseNode::getVar(DataType::TPtr, "buf_b"))));

    if (this->unpackc_flag != PackType::NoPack)
        ret.push_back(new FreeBufStmt(
            new VarRef(BaseNode::getVar(DataType::TPtr, "buf_c"))));

    return ret;
}

std::vector<BaseNode *> GemmGenerator::buildNameSpaceBody()
{
    std::vector<BaseNode *> ret;

    std::vector<BaseNode *> targ_list = buildTargList();

    std::vector<BaseNode *> farg_list = buildFargList();

    std::vector<BaseNode *> body = buildFuncBody();

    ret.push_back(new TemplateStructFuncStmt(
        this->func_name, targ_list, farg_list, body));

    return ret;
}

void GemmGenerator::buildGemmAST()
{
    std::cout << "Building AST for " << this->file_name << std::endl;
    root.addChild(new PragmaStmt("once"));
    root.addChild(new VoidLineStmt());
    root.addChild(new IncludeStmt("cstdio"));
    root.addChild(new IncludeStmt("cstdlib"));
    root.addChild(new IncludeStmt("cstring"));
    root.addChild(new IncludeStmt("runtime.h"));
    root.addChild(new VoidLineStmt());

    std::vector<BaseNode *> namespace_body = buildNameSpaceBody();

    root.addChild(new NameSpaceStmt("gencode", namespace_body));

    std::cout << "Build done AST for " << this->file_name << std::endl;
}

void GemmGenerator::codeGen(const std::string &place_dir)
{
    if (place_dir == "stdout")
    {
        root.codeGen(0, std::cout);
        std::cout << "code generated\n";
    }
    else
    {
        std::string file_path = (place_dir
            + (place_dir.back() == '/' ? "" : "/") + this->file_name);
        std::ofstream of(file_path);
        root.codeGen(0, of);
        of.close();
        std::cout << file_path << " generated\n";
    }
}

GemmGenerator::~GemmGenerator() { BaseNode::destoryDeclVars(); }

} // namespace loopmaster