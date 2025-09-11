#pragma once
#include "base_node.h"
#include "parallel_scheme.h"
#include "scheme_info.h"

namespace loopmaster
{

enum class PassID
{
    DfsGetTargetNodeID = 0,
    InsertIncludeStmt,
    InsertOmpSetNestedStmt,

    PassCount
};

/**
 * @brief Some general pass for AST
 *
 */
class PASS
{
public:
    GemmGenerator &cur_gemm;
    ModuleAST *ast;

    PASS() = default;

    ~PASS() = default;

    /**
     * @brief Construct a new PASS object
     *
     * @param gemm_generator
     */
    PASS(GemmGenerator &gemm_generator)
        : cur_gemm(gemm_generator)
        , ast(&(gemm_generator.getGemmASTRoot()))
    {
    }

    /**
     * @brief Insert include stmt, e.g. "omp.h"
     *
     */
    void insertIncludeStmt(const std::string filename);

    /**
     * @brief Find all VectorLoop and refine them to support all size
     * for (int i = 0; i < n; i += 4)
     * {
     *     VectorCopy...
     * }
     * ==============================
     * for (int i = 0; i < n - n % 4; i += 4)
     * {
     *     VectorCopy...
     * }
     *
     * for (int i = n - n % 4; i < n; i+=1)
     * {
     *     Copy...
     * }
     */
    void refineVectorLoop();

    /**
     * @brief Insert thread num for template func
     *
     * @param ps
     */
    void insertTNTemplateArgs(const ParallelScheme &ps);

    /**
     * @brief Insert omp_set_nested(1) at func body begin
     *
     * @param enable
     */
    void insertOmpSetNestedStmt(bool enable);

    /**
     * @brief Modify the buffer size expr when alloc for parallel gemm ABC
     *
     * @param ps
     */
    void refineParallelBufSize(const ParallelScheme &ps);

    /**
     * @brief Insert omp_get_tid in parallel for
     *
     * @param ps
     */
    void insertGetTidForParallelLoop(const ParallelScheme &ps);

    /**
     * @brief Modify the buffer ptr based on tid
     *
     * @param ps
     */
    void refineParallelBufPtr(const ParallelScheme &ps);

    void refineParallelBufPtrBak(const ParallelScheme &ps);

    /**
     * @brief Insert stmt that get total thread num
     *
     * @param ps
     */
    void insertTotalNumThreads(const ParallelScheme &ps);

    BaseNode *declAllTids(const ParallelScheme &ps);

    void parallelizeForLoop(const std::string &loop_name,
        const std::string &tn_name, const std::string &tid_name, const ParallelScheme &ps);

    VarDeclWithRVStmt *declLocalBufC(const ParallelScheme &ps);

    BaseNode *buildLocalStartRV(Var *tid, Var *step, Var *iter_pt,
        Var *iter_tail, Var *init);

    void parallelizeMemset(const ParallelScheme &ps);

    void parallelizeCBetaOp(const ParallelScheme &ps);

    void insertParallelPtrOff(const std::string &loop_name,
        const std::string &tid_name, const std::string &ptr_name,
        const std::string &pre_len_name, const std::string &init_name);

    void parallelizeGemmASTFuncBody(const ParallelScheme &ps);

    /**
     * @brief Insert #pragma omp parallel for num_threads(x)
     *
     * @param loop_name
     * @param num_threads
     */
    void insertOmpParallelForStmt(const std::string &loop_name,
        const std::string &tn_name, const std::string &tid_name);

    /**
     * @brief Parallelize the gemm_ast to nested parallel for
     *
     * @param ps
     */
    void parallelizeGemmAST(const ParallelScheme &ps);

    void insertReductionTNArg(const ParallelScheme &ps);

    void removeCbetaOp(const ParallelScheme &ps);

    BaseNode *buildOneBufCElemNum(const ParallelScheme &ps);

    void insertReductionCBuf(const ParallelScheme &ps);

    void parallelizeReductionBody(const ParallelScheme &ps);

    static BaseNode *buildCeilDivExpr(Var *len, Var *step);
};
}