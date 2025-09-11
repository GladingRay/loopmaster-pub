#pragma once

#include <string>
#include <vector>

#include "stmt.h"
#include "tile.h"

#include "scheme_info.h"

namespace loopmaster
{
class GemmGenerator
{
private:
    std::string file_name;

    std::string func_name;

    SchemeInfo scheme;

    PackType packa_flag;
    PackType packb_flag;
    PackType unpackc_flag;

    int packa_pos;
    int packb_pos;
    int unpackc_pos;

    std::vector<LoopIndex> a_loop_indexs;
    std::vector<LoopIndex> b_loop_indexs;
    std::vector<LoopIndex> c_loop_indexs;

    std::vector<LoopIndex> loop_indexs;

    ModuleAST root;

public:
    GemmGenerator(const SchemeInfo &scheme_);

    void getABCLoopIndexOrder();

    std::vector<BaseNode *> buildTargList();

    std::vector<BaseNode *> buildFargList();

    Stmt *buildGemmLoop(size_t loop_id, size_t m_loop_num, size_t n_loop_num,
        size_t k_loop_num);

    std::vector<BaseNode *> buildNameSpaceBody();

    std::vector<BaseNode *> buildFuncBody();

    MultiCalcExpr *buildBufAElemNumExpr();

    MultiCalcExpr *buildBufASizeExpr();

    MultiCalcExpr *buildBufBElemNumExpr();

    MultiCalcExpr *buildBufBSizeExpr();

    MultiCalcExpr *buildBufCElemNumExpr();

    MultiCalcExpr *buildBufCSizeExpr();

    VarDeclWithRVStmt *buildBufCInitElemNum();

    Expr *buildBufCInitSizeExpr();

    Expr *buildPackedIdxLenExpr(const std::string &idx_name,
        const std::string &max_len_name, const std::string &step_name);

    Stmt *buildReadContinuousPackA();

    Stmt *buildReadContinuousPackB();

    Stmt *buildReadContinuousNestedUnpackCLoop(
        const std::vector<Dim> &unpackc_order, size_t depth, size_t m_loops,
        size_t n_loops, std::vector<LoopIndex> &loop_idxs);
    Stmt *buildReadContinuousUnpackC();

    Stmt *buildWriteContinuousNestedPackALoop(
        const std::vector<Dim> &packa_order, size_t depth, size_t m_loops,
        size_t k_loops, std::vector<LoopIndex> &loop_idxs);
    Stmt *buildWriteContinuousPackA();

    Stmt *buildWriteContinuousNestedPackBLoop(
        const std::vector<Dim> &packb_order, size_t depth, size_t n_loops,
        size_t k_loops, std::vector<LoopIndex> &loop_idxs);
    Stmt *buildWriteContinuousPackB();

    Expr *buildPackedKerAptrExpr();

    Expr *buildPackedKerBptrExpr();

    Expr *buildPackedKerCptrExpr();

    Expr *buildColMajorAoffsetExpr();

    Expr *buildColMajorBoffsetExpr();

    Expr *buildColMajorCoffsetExpr();

    Expr *buildColMajorOffsetExpr(
        Dim dim0, int dim0_out_num, int dim0_ker_num,
        Dim dim1, int dim1_out_num, int dim1_ker_num,
        const std::string &ld_name);

    Expr *buildColMajorKerAptrExpr();

    Expr *buildColMajorKerBptrExpr();

    Expr *buildColMajorKerCptrExpr();

    void buildGemmAST();

    ModuleAST &getGemmASTRoot() { return this->root; }

    std::string &getFuncName() { return this->func_name; }
    std::string &getFileName() { return this->file_name; }

    SchemeInfo &getSchemeInfo() { return this->scheme; }

    void codeGen(const std::string &place_dir);

    /* For Test and Debug */

    void printPackPos()
    {
        printf("%d, %d, %d\n", this->packa_pos, this->packb_pos,
            this->unpackc_pos);
    }

    void printABCLoopIndex()
    {
        printPackPos();
        printf("ALoopIndex:\n");

        for (int i = 0; i < this->a_loop_indexs.size(); i++)
            printf("%d: %s, ", i, this->a_loop_indexs[i].toStr().c_str());
        printf("\n");

        printf("BLoopIndex:\n");

        for (int i = 0; i < this->b_loop_indexs.size(); i++)
            printf("%d: %s, ", i, this->b_loop_indexs[i].toStr().c_str());
        printf("\n");

        printf("CLoopIndex:\n");

        for (int i = 0; i < this->c_loop_indexs.size(); i++)
            printf("%d: %s, ", i, this->c_loop_indexs[i].toStr().c_str());
        printf("\n");
    }

    ~GemmGenerator();
};
}
