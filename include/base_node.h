#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "lm_assert.h"
#include "var.h"

namespace loopmaster
{

enum class NodeType
{
    BaseNodeKind,
    AbstractStmtKind,
    AbstractExprKind,

    ModuleASTKind,

    ConstNumExprKind,
    VarRefKind,
    MinExprKind,
    UaryOpExprKind,
    BinaryOpExprKind,
    AlignedAllocExprKind,
    SizeofTypeExprKind,
    MultiCalcExprKind,
    TernaryExprKind,
    SubscriptExprKind,
    OmpGetTidExprKind,

    VoidLineStmtKind,
    CommentStmtKind,
    PragmaStmtKind,
    IncludeStmtKind,
    NameSpaceStmtKind,
    VarDeclStmtKind,
    VarDeclWithRVStmtKind,
    LoopIndexDeclStmtKind,
    ArgDeclStmtKind,
    VarAssignStmtKind,
    TemplateStructFuncStmtKind,
    LoopStmtKind,
    KernelCallStmtKind,
    ZeroBufStmtKind,
    FreeBufStmtKind,
    CodeBlockStmtKind,
    VectorCopyStmtKind,
    VectorCopyAddStmtKind,
    CbetaOpStmtKind,
    ExprToStmtKind,
    OmpParallelStmtKind,
    OmpBarrierStmtKind,
    OmpParallelForStmtKind,
    OmpSetNestedStmtKind,

    OmpAtomicStmtKind,
    SyncVarDeclInitStmtKind,
    SyncOnStmtKind,
    ReductionCStmtKind,

    NodeTypeCount
};

class NodeInfo
{
public:
    NodeType node_type;
    std::string target_name;
};

class BaseNode
{
public:
    NodeType node_type;

    static Var static_one;
    static Var static_zero;
    static Var static_vec_len;
    static std::unordered_map<std::string, Var *> decl_vars;
    static std::unordered_map<BaseNode *, BaseNode *> son2father;

    /**
     * @brief Construct a new Base Node object
     *
     */
    BaseNode(NodeType nt = NodeType::BaseNodeKind)
        : node_type(nt)
    {
    }

    /**
     * @brief Update son2father, now every nodes use the same interface, it's a
     * simple but redundant impl, mayby here should impl specific update for
     * every node.
     * 
     */
    virtual void updateSon() {}

    static void printIndent(int indent_level, std::ostream &os)
    {
        for (int i = 0; i < indent_level * 4; i++)
            os << " ";
    }

    /**
     * @brief Code generation to cxx
     *
     * @param indent
     * @param os
     */
    virtual void codeGen(int indent, std::ostream &os)
    {
        LM_ASSERT(false, "BaseNode type cann't codegen directly.");
    }

    /**
     * @brief Walk start from this node to find the first target node info
     *
     * @param node_info
     * @return BaseNode *
     */
    virtual BaseNode *walkFindNode(NodeInfo &node_info);

    /**
     * @brief Walk from this node and find all nodes matched target node info
     * 
     * @param node_info
     * @param results
     */
    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results);

    /**
     * @brief Check this node is eq taregt_node through node_info
     *
     * @param node_info
     * @return true
     * @return false
     */
    virtual bool istarget(NodeInfo &node_info);

    /**
     * @brief Deep copy the AST rooted at this node
     *
     * @return BaseNode*
     */
    virtual BaseNode *deepcopy();

    /**
     * @brief Check this node is end (has no child)
     *
     * @return true
     * @return false
     */
    virtual bool isend();

    static void addVarDecl(Var *var)
    {
        std::string var_name = var->toHashStr();
        decl_vars[var_name] = var;
    }

    static Var *getVar(DataType dt, const std::string &name)
    {
        std::string var_name = Var(dt, name).toHashStr();

        LM_ASSERT(decl_vars.count(var_name), ("Can't find " + var_name));

        return decl_vars[var_name];
    }

    template <typename FT, typename ST>
    static FT *getFatherPtr(ST *son)
    {
        if (son2father.count(son) == 0) return nullptr;
        return dynamic_cast<FT *>(static_cast<BaseNode *>(son2father[son]));
    }

    static BaseNode *getFatherPtr(BaseNode *son)
    {
        if (son2father.count(son) == 0) return nullptr;
        return son2father[son];
    }

    static void destoryDeclVars()
    {
        decl_vars.clear();
        decl_vars = {
            { "int 1", &static_one }, { "int 0", &static_zero },
            { "int VecLen<T>::value", &static_vec_len } // I'm sorry
        };
    }

    virtual ~BaseNode()
    {
        if (son2father.count(this))
            son2father.erase(this);
    }
};

#define CHECK_AND_REG_SON2FA(child_)                                           \
    if (child_)                                                                \
        BaseNode::son2father[child_] = this;

#define CHECK_AND_DELETE(child_)                                               \
    if (child_)                                                                \
    {                                                                          \
        delete child_;                                                         \
        child_ = nullptr;                                                      \
    } 

class ModuleAST : public BaseNode
{
public:
    std::vector<BaseNode *> children;

    std::string module_name;

    ModuleAST() = default;

    ModuleAST(const std::string &module_name,
        const std::vector<BaseNode *> children)
        : module_name(module_name)
        , children(children)
        , BaseNode(NodeType::ModuleASTKind)
    {
        for (BaseNode *child : this->children)
        {
            CHECK_AND_REG_SON2FA(child)
        }
    }

    void addChild(BaseNode *child)
    {
        this->children.push_back(child);
        CHECK_AND_REG_SON2FA(child)
    }

    std::vector<BaseNode *> &getChildren() { return this->children; }

    virtual void updateSon()
    {
        for (BaseNode *child : this->children)
        {
            CHECK_AND_REG_SON2FA(child);
        }
    }

    virtual void codeGen(int indent, std::ostream &os)
    {
        for (BaseNode *child : this->children)
        {
            if (child != nullptr)
                child->codeGen(indent, os);
        }
    }

    virtual bool isend();

    virtual bool istarget(NodeInfo &node_info);

    virtual BaseNode *deepcopy();

    virtual BaseNode *walkFindNode(NodeInfo &node_info);

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results);

    virtual ~ModuleAST()
    {
        for (BaseNode *&child : this->children)
        {
            CHECK_AND_DELETE(child);
        }
    }

};


}