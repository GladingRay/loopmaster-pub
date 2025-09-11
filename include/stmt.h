#pragma once
#include <fstream>
#include <iostream>
#include <ostream>

#include "base_node.h"
#include "enum_type.h"
#include "expr.h"
#include "var.h"

namespace loopmaster
{
class Stmt : public BaseNode
{
private:
public:
    Stmt(NodeType st = NodeType::AbstractStmtKind)
        : BaseNode(st)
    {
    }

    virtual ~Stmt() = default;
    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;
};

class VoidLineStmt : public Stmt
{
private:
public:
    VoidLineStmt()
        : Stmt(NodeType::VoidLineStmtKind)
    {
    }

    virtual ~VoidLineStmt() = default;
    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;
};

class CommentStmt : public Stmt
{
private:
public:
    std::string comment;
    CommentStmt(const std::string &comment_)
        : comment(comment_)
        , Stmt(NodeType::CommentStmtKind)
    {
    }
    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual BaseNode *deepcopy() override;

    virtual ~CommentStmt() { }
};

class PragmaStmt : public Stmt
{
private:
public:
    std::string inst;
    PragmaStmt(const std::string &inst_)
        : inst(inst_)
        , Stmt(NodeType::PragmaStmtKind)
    {
    }
    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~PragmaStmt() { }
};

/**
 * @brief #include "xxx.h"
 *
 */
class IncludeStmt : public Stmt
{
private:
public:
    std::string header_name;

    IncludeStmt(const std::string &header_name_)
        : header_name(header_name_)
        , Stmt(NodeType::IncludeStmtKind)
    {
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~IncludeStmt() { }
};

/**
 * @brief namespace {
 *
 *        }
 *
 */
class NameSpaceStmt : public Stmt
{
private:
public:
    std::string namespace_name;

    std::vector<BaseNode *> body;

    NameSpaceStmt();
    NameSpaceStmt(
        const std::string &namesapce_name_, const std::vector<BaseNode *> body_)
        : namespace_name(namesapce_name_)
        , body(body_)
        , Stmt(NodeType::NameSpaceStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        for (BaseNode *stmt : body)
        {
            if (stmt != nullptr)
                BaseNode::son2father[stmt] = this;
        }
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~NameSpaceStmt()
    {
        for (BaseNode *&stmt : body)
        {
            if (stmt)
            {
                delete stmt;
                stmt = nullptr;
            }
        }
    }
};

class VarDeclStmt : public Stmt
{
private:
public:
    DataType type;
    std::string var_name;

    bool is_constant;

    Var *decled_var;

    VarDeclStmt();
    VarDeclStmt(const DataType &type_, const std::string &var_name_,
        bool is_const = false)
        : type(type_)
        , var_name(var_name_)
        , is_constant(is_const)
        , Stmt(NodeType::VarDeclStmtKind)
    {
        decled_var = new Var(type_, var_name_, is_const);
        BaseNode::addVarDecl(decled_var);
    }

    Var *getDecledVar() { return decled_var; }

    std::string toVarName() { return decled_var->toStr(); }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~VarDeclStmt() { delete decled_var; }
};

class VarDeclWithRVStmt : public VarDeclStmt
{
private:
public:
    BaseNode *rv;

    VarDeclWithRVStmt(const DataType &type_, const std::string &var_name_,
        BaseNode *rv_, bool is_const = false)
        : VarDeclStmt(type_, var_name_, is_const)
        , rv(rv_)
    {
        this->node_type = NodeType::VarDeclWithRVStmtKind;

        this->updateSon();
    }

    void updateRV(BaseNode *new_rv)
    {
        if (this->rv != nullptr)
        {
            BaseNode::son2father.erase(this->rv);
            delete this->rv;
        }
        this->rv = new_rv;
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (this->rv != nullptr)
            BaseNode::son2father[this->rv] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~VarDeclWithRVStmt()
    {
        if (rv)
        {
            delete rv;
            rv = nullptr;
        }
    }
};

class LoopIndexDeclStmt : public VarDeclStmt
{
private:
public:
    BaseNode *rv;

    LoopIndexDeclStmt(const DataType &type_, const std::string &var_name_,
        BaseNode *rv_, bool is_const = false)
        : VarDeclStmt(type_, var_name_, is_const)
        , rv(rv_)
    {
        this->node_type = NodeType::LoopIndexDeclStmtKind;
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (rv != nullptr)
            BaseNode::son2father[rv] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~LoopIndexDeclStmt()
    {
        if (rv)
        {
            delete rv;
            rv = nullptr;
        }
    }
};

class ArgDeclStmt : public VarDeclStmt
{
private:
public:
    ArgDeclStmt();

    ArgDeclStmt(const DataType &type_, const std::string &var_name_,
        bool is_const = false)
        : VarDeclStmt(type_, var_name_, is_const)
    {
        this->node_type = NodeType::ArgDeclStmtKind;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~ArgDeclStmt() { }
};

/**
 * @brief VarRef = i * k + l * m;
 *
 */
class VarAssignStmt : public Stmt
{
private:
public:
    BaseNode *left;
    BaseNode *right;

    VarAssignStmt(BaseNode *left_, BaseNode *right_)
        : left(left_)
        , right(right_)
        , Stmt(NodeType::VarAssignStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (left != nullptr)
            BaseNode::son2father[left] = this;
        if (right != nullptr)
            BaseNode::son2father[right] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~VarAssignStmt()
    {
        if (left)
        {
            delete left;
            left = nullptr;
        }
        if (right)
        {
            delete right;
            right = nullptr;
        }
    }
};

/**
 * @brief template <typename T, ....>
 *        struct LoopTypex {
 *            void operator()(T arg1, T arg2) {}
 *        }
 *
 */
class TemplateStructFuncStmt : public Stmt
{
private:
public:
    const std::string &func_name;

    std::vector<BaseNode *> targ_list;

    std::vector<BaseNode *> farg_list;

    std::vector<BaseNode *> body;

    TemplateStructFuncStmt(const std::string &func_name_,
        const std::vector<BaseNode *> &targ_list_,
        const std::vector<BaseNode *> &farg_list_,
        const std::vector<BaseNode *> &body_)
        : func_name(func_name_)
        , targ_list(targ_list_)
        , farg_list(farg_list_)
        , body(body_)
        , Stmt(NodeType::TemplateStructFuncStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        for (BaseNode *stmt : targ_list)
        {
            if (stmt != nullptr)
                BaseNode::son2father[stmt] = this;
        }

        for (BaseNode *stmt : farg_list)
        {
            if (stmt != nullptr)
                BaseNode::son2father[stmt] = this;
        }

        for (BaseNode *stmt : body)
        {
            if (stmt != nullptr)
                BaseNode::son2father[stmt] = this;
        }
    }

    void insertAtBegin(BaseNode *stmt)
    {
        if (stmt != nullptr)
        {
            this->body.insert(this->body.begin(), stmt);
        }
    }

    void insertAtEnd(BaseNode *stmt)
    {
        if (stmt != nullptr)
        {
            this->body.insert(this->body.end(), stmt);
        }
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~TemplateStructFuncStmt()
    {
        for (BaseNode *&vds : targ_list)
        {
            if (vds)
            {
                delete vds;
                vds = nullptr;
            }
        }

        for (BaseNode *&vds : farg_list)
        {
            if (vds)
            {
                delete vds;
                vds = nullptr;
            }
        }

        for (BaseNode *&stmt : body)
        {
            if (stmt)
            {
                delete stmt;
                stmt = nullptr;
            }
        }
    }
};

class CodeBlockStmt : public Stmt
{
private:
public:
    std::vector<BaseNode *> stmts_in_block;

    CodeBlockStmt(const std::vector<BaseNode *> &stmts)
        : stmts_in_block(stmts)
        , Stmt(NodeType::CodeBlockStmtKind)
    {
        this->updateSon();
    }

    bool updateBodyStmt(BaseNode *old_stmt, BaseNode *new_stmt)
    {
        for (BaseNode *&stmt : this->stmts_in_block)
        {
            if (stmt == old_stmt)
            {
                BaseNode::son2father.erase(old_stmt);
                delete old_stmt;
                stmt = new_stmt;
                BaseNode::son2father[new_stmt] = this;
                return true;
            }

            if (CodeBlockStmt *cbs = dynamic_cast<CodeBlockStmt *>(stmt))
            {
                if (cbs->updateBodyStmt(old_stmt, new_stmt))
                    return true;
            }
        }
        return false;
    }

    bool hasLoop();

    virtual void updateSon() override
    {
        for (BaseNode *stmt : stmts_in_block)
        {
            if (stmt != nullptr)
                BaseNode::son2father[stmt] = this;
        }
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~CodeBlockStmt()
    {
        for (BaseNode *&stmt : stmts_in_block)
        {
            if (stmt)
            {
                delete stmt;
                stmt = nullptr;
            }
        }
    }
};
/**
 * @brief For loop
 *
 */
class LoopStmt : public Stmt
{
private:
public:
    BaseNode *init;
    BaseNode *end_flag;
    BaseNode *update;

    std::vector<BaseNode *> loop_body;

    Dim dim;
    bool is_unit;

    Var *bind_var;

    LoopStmt(BaseNode *init_, BaseNode *end_flag_, BaseNode *update_,
        const std::vector<BaseNode *> &body_)
        : init(init_)
        , end_flag(end_flag_)
        , update(update_)
        , loop_body(body_)
        , Stmt(NodeType::LoopStmtKind)
    {
        if (init)
        {
            if (LoopIndexDeclStmt *decl
                = dynamic_cast<LoopIndexDeclStmt *>(this->init))
                this->bind_var = decl->decled_var;
        }

        this->updateSon();

        if (BinaryOpExpr *be = dynamic_cast<BinaryOpExpr *>(update))
        {
            be->is_loop_increment = true;
        }
    }

    Var *getStepVar()
    {
        if (BinaryOpExpr *u = dynamic_cast<BinaryOpExpr *>(this->update))
        {
            if (VarRef *vr = dynamic_cast<VarRef *>(u->right))
            {
                return vr->v;
            }
        }
        return nullptr;
    }

    void updateInitRV(BaseNode *rv)
    {
        if (LoopIndexDeclStmt *init_stmt
            = dynamic_cast<LoopIndexDeclStmt *>(this->init))
        {
            if (init_stmt->rv)
            {
                BaseNode::son2father.erase(init_stmt);
                delete init_stmt->rv;
            }
            init_stmt->rv = rv;
            BaseNode::son2father[rv] = init_stmt;
        }
    }

    void updateEnd(BaseNode *rv)
    {
        if (BinaryOpExpr *end_expr = dynamic_cast<BinaryOpExpr *>(this->end_flag))
        {
            if (end_expr->right)
            {
                BaseNode::son2father.erase(end_expr->right);
                delete end_expr->right;
            }
            end_expr->right = rv;
            BaseNode::son2father[rv] = end_expr;
        }
    }

    void updateStep(BaseNode *rv)
    {
        if (BinaryOpExpr *update_expr = dynamic_cast<BinaryOpExpr *>(this->update))
        {
            if (update_expr->right)
            {
                BaseNode::son2father.erase(update_expr->right);
                delete update_expr->right;
            }

            update_expr->right = rv;
            BaseNode::son2father[rv] = update_expr;
        }
    }

    void updateBodyStmt(BaseNode *old_stmt, BaseNode *new_stmt)
    {
        for (BaseNode *&stmt : this->loop_body)
        {
            if (stmt == old_stmt)
            {
                BaseNode::son2father.erase(old_stmt);
                delete old_stmt;
                stmt = new_stmt;
                BaseNode::son2father[new_stmt] = this;
                return;
            }

            if (CodeBlockStmt *cbs = dynamic_cast<CodeBlockStmt *>(stmt))
            {
                cbs->updateBodyStmt(old_stmt, new_stmt);
            }
        }
    }

    virtual void updateSon() override
    {
        if (init != nullptr)
            BaseNode::son2father[init] = this;
        if (end_flag != nullptr)
            BaseNode::son2father[end_flag] = this;
        if (update != nullptr)
            BaseNode::son2father[update] = this;

        for (BaseNode *stmt : loop_body)
        {
            if (stmt != nullptr)
                BaseNode::son2father[stmt] = this;
        }
    }

    void bindVarWhenInitNull(Var *var) { this->bind_var = var; }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~LoopStmt()
    {
        if (init)
        {
            delete init;
            init = nullptr;
        }
        if (end_flag)
        {
            delete end_flag;
            end_flag = nullptr;
        }
        if (update)
        {
            delete update;
            update = nullptr;
        }

        for (BaseNode *&stmt : loop_body)
        {
            if (stmt)
            {
                delete stmt;
                stmt = nullptr;
            }
        }
    }
};

/**
 * @brief funcname<targ1, targ2>(arg1, arg2);
 *
 */
class KernelCallStmt : public Stmt
{
private:
public:
    std::string kernel_name;

    /* template args */
    size_t kernel_id;
    bool packa_flag;
    bool packb_flag;
    bool unpackc_flag;

    /* func args */

    std::vector<BaseNode *> func_args;

    KernelCallStmt(const std::string &kernel_name_, size_t kernel_id_,
        bool packa_flag_, bool packb_flag_, bool unpackc_flag_,
        std::vector<BaseNode *> &func_args_)
        : kernel_name(kernel_name_)
        , kernel_id(kernel_id_)
        , packa_flag(packa_flag_)
        , packb_flag(packb_flag_)
        , unpackc_flag(unpackc_flag_)
        , func_args(func_args_)
        , Stmt(NodeType::KernelCallStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        for (BaseNode *e : func_args)
        {
            if (e != nullptr)
                BaseNode::son2father[e] = this;
        }
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~KernelCallStmt()
    {
        for (size_t i = 0; i < func_args.size(); i++)
        {
            if (func_args[i])
            {
                delete func_args[i];
            }
            func_args[i] = nullptr;
        }
    }
};

class ZeroBufStmt : public Stmt
{
private:
public:
    BaseNode *buf_ptr;
    BaseNode *buf_size;

    ZeroBufStmt(BaseNode *buf_ptr_, BaseNode *buf_size_)
        : buf_ptr(buf_ptr_)
        , buf_size(buf_size_)
        , Stmt(NodeType::ZeroBufStmtKind)
    {
        this->updateSon();
    }

    void updateBufPtr(BaseNode *new_buf_ptr)
    {
        if (this->buf_ptr != nullptr)
        {
            BaseNode::son2father.erase(this->buf_ptr);
            delete buf_ptr;
        }
        this->buf_ptr = new_buf_ptr;
        BaseNode::son2father[this->buf_ptr] = this;
    }

    virtual void updateSon() override
    {
        if (buf_ptr != nullptr)
            BaseNode::son2father[buf_ptr] = this;
        if (buf_size != nullptr)
            BaseNode::son2father[buf_size] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~ZeroBufStmt()
    {
        if (buf_ptr)
        {
            delete buf_ptr;
            buf_ptr = nullptr;
        }
        if (buf_size)
        {
            delete buf_size;
            buf_size = nullptr;
        }
    }
};

class FreeBufStmt : public Stmt
{
private:
public:
    BaseNode *buf_ptr;

    FreeBufStmt(BaseNode *buf_ptr_)
        : buf_ptr(buf_ptr_)
        , Stmt(NodeType::FreeBufStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (buf_ptr != nullptr)
            BaseNode::son2father[buf_ptr] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~FreeBufStmt()
    {
        if (buf_ptr)
        {
            delete buf_ptr;
            buf_ptr = nullptr;
        }
    }
};

class VectorCopyStmt : public Stmt
{
private:
public:
    BaseNode *src_ptr;
    BaseNode *dst_ptr;

    VectorCopyStmt(BaseNode *dst_ptr_, BaseNode *src_ptr_)
        : dst_ptr(dst_ptr_)
        , src_ptr(src_ptr_)
        , Stmt(NodeType::VectorCopyStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (src_ptr != nullptr)
            BaseNode::son2father[src_ptr] = this;
        if (dst_ptr != nullptr)
            BaseNode::son2father[dst_ptr] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~VectorCopyStmt()
    {
        if (src_ptr)
        {
            delete src_ptr;
            src_ptr = nullptr;
        }

        if (dst_ptr)
        {
            delete dst_ptr;
            dst_ptr = nullptr;
        }
    }
};

class VectorCopyAddStmt : public Stmt
{
private:
public:
    BaseNode *src_ptr;
    BaseNode *dst_ptr;
    BaseNode *beta_ref;

    VectorCopyAddStmt(
        BaseNode *dst_ptr_, BaseNode *src_ptr_, BaseNode *beta_ref_)
        : dst_ptr(dst_ptr_)
        , src_ptr(src_ptr_)
        , beta_ref(beta_ref_)
        , Stmt(NodeType::VectorCopyAddStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (src_ptr != nullptr)
            BaseNode::son2father[src_ptr] = this;
        if (dst_ptr != nullptr)
            BaseNode::son2father[dst_ptr] = this;
        if (beta_ref != nullptr)
            BaseNode::son2father[beta_ref] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~VectorCopyAddStmt()
    {
        if (src_ptr)
        {
            delete src_ptr;
            src_ptr = nullptr;
        }

        if (dst_ptr)
        {
            delete dst_ptr;
            dst_ptr = nullptr;
        }

        if (beta_ref)
        {
            delete beta_ref;
            beta_ref = nullptr;
        }
    }
};

class CbetaOpStmt : public Stmt
{
private:
public:
    BaseNode *m_len;
    BaseNode *n_len;
    BaseNode *c_ptr;
    BaseNode *ldc;
    BaseNode *beta_ref;
    BaseNode *tid;
    BaseNode *tn;

    CbetaOpStmt(BaseNode *m_len_, BaseNode *n_len_, BaseNode *c_ptr_,
        BaseNode *ldc_, BaseNode *beta_ref_, BaseNode *tid_, BaseNode *tn_)
        : m_len(m_len_)
        , n_len(n_len_)
        , c_ptr(c_ptr_)
        , ldc(ldc_)
        , beta_ref(beta_ref_)
        , tid(tid_)
        , tn(tn_)
        , Stmt(NodeType::CbetaOpStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (m_len != nullptr)
            BaseNode::son2father[m_len] = this;
        if (n_len != nullptr)
            BaseNode::son2father[n_len] = this;
        if (c_ptr != nullptr)
            BaseNode::son2father[c_ptr] = this;
        if (ldc != nullptr)
            BaseNode::son2father[ldc] = this;
        if (beta_ref != nullptr)
            BaseNode::son2father[beta_ref] = this;
        CHECK_AND_REG_SON2FA(tid)
        CHECK_AND_REG_SON2FA(tn)
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~CbetaOpStmt()
    {
        if (m_len)
        {
            delete m_len;
            m_len = nullptr;
        }

        if (n_len)
        {
            delete n_len;
            n_len = nullptr;
        }

        if (c_ptr)
        {
            delete c_ptr;
            c_ptr = nullptr;
        }

        if (ldc)
        {
            delete ldc;
            ldc = nullptr;
        }

        if (beta_ref)
        {
            delete beta_ref;
            beta_ref = nullptr;
        }

        CHECK_AND_DELETE(tid)
        CHECK_AND_DELETE(tn)
    }
};

class ExprToStmt : public Stmt
{
private:
public:
    BaseNode *expr;

    ExprToStmt(BaseNode *expr_)
        : expr(expr_)
        , Stmt(NodeType::ExprToStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (expr != nullptr)
            BaseNode::son2father[expr] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~ExprToStmt()
    {
        if (expr)
        {
            delete expr;
            expr = nullptr;
        }
    }
};

/**
 * @brief openmp parallel region
 * #pragma omp parallel num_threads(thread_num)
 * {
 * }
 */
class OmpParallelStmt : public Stmt
{
private:
public:
    BaseNode *parallel_body;
    BaseNode *num_threads;
    OmpParallelStmt(BaseNode *body, BaseNode *thread_num = nullptr)
        : parallel_body(body)
        , num_threads(thread_num)
        , Stmt(NodeType::OmpParallelStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (parallel_body != nullptr)
            BaseNode::son2father[parallel_body] = this;
        if (num_threads != nullptr)
            BaseNode::son2father[num_threads] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~OmpParallelStmt()
    {
        if (parallel_body)
        {
            delete parallel_body;
            parallel_body = nullptr;
        }

        if (num_threads)
        {
            delete num_threads;
            num_threads = nullptr;
        }
    }
};

/**
 * @brief #pragma omp barrier
 *
 */
class OmpBarrierStmt : public Stmt
{
private:
public:
    OmpBarrierStmt()
        : Stmt(NodeType::OmpBarrierStmtKind)
    {
    }

    virtual ~OmpBarrierStmt() = default;

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;
};

/**
 * @brief #pragma omp parallel for num_threads(tn)
 *
 */
class OmpParallelForStmt : public Stmt
{
private:
public:
    BaseNode *loop;
    BaseNode *num_threads;
    OmpParallelForStmt(BaseNode *for_loop, BaseNode *thread_num = nullptr)
        : loop(for_loop)
        , num_threads(thread_num)
        , Stmt(NodeType::OmpParallelForStmtKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (loop != nullptr)
            BaseNode::son2father[loop] = this;
        if (num_threads != nullptr)
            BaseNode::son2father[num_threads] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~OmpParallelForStmt()
    {
        if (loop)
        {
            delete loop;
            loop = nullptr;
        }

        if (num_threads)
        {
            delete num_threads;
            num_threads = nullptr;
        }
    }
};

class OmpSetNestedStmt : public Stmt
{
private:
public:
    bool enable_nested;
    OmpSetNestedStmt(bool enable)
        : enable_nested(enable)
        , Stmt(NodeType::OmpSetNestedStmtKind)
    {
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~OmpSetNestedStmt() = default;
};

class OmpAtomicStmt : public Stmt
{
private:
public:
    BaseNode *body;

    OmpAtomicStmt(BaseNode *body_)
        : body(body_)
        , Stmt(NodeType::OmpAtomicStmtKind)
    {
        if (this->body)
            BaseNode::son2father[this->body] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~OmpAtomicStmt()
    {
        if (this->body)
        {
            delete this->body;
        }
    }
};

class SyncVarDeclInitStmt : public Stmt
{
private:
public:
    std::string name;
    std::string gn_name;
    BaseNode *num;
    SyncVarDeclInitStmt(std::string name_, std::string gn_name_, BaseNode *num_)
        : name(name_)
        , gn_name(gn_name_)
        , num(num_)
        , Stmt(NodeType::SyncVarDeclInitStmtKind)
    {
        if (this->num)
            BaseNode::son2father[this->num] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual BaseNode *deepcopy() override;

    virtual ~SyncVarDeclInitStmt()
    {
        if (this->num)
        {
            delete this->num;
            this->num = nullptr;
        }
    }
};

class SyncOnStmt : public Stmt
{
private:
public:
    BaseNode *tid;
    BaseNode *local_tid;
    std::string gn_name;
    std::string tn_name;
    std::string sync_name;

    SyncOnStmt(BaseNode *tid_, BaseNode *local_tid_, std::string gn_name_, std::string tn_name_, std::string sync_name_)
        : tid(tid_)
        , local_tid(local_tid_)
        , gn_name(gn_name_)
        , tn_name(tn_name_)
        , sync_name(sync_name_)
        , Stmt(NodeType::SyncOnStmtKind)
    {
        if (this->tid)
            BaseNode::son2father[this->tid] = this;
        if (this->tid)
            BaseNode::son2father[this->local_tid] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual BaseNode *deepcopy() override;

    virtual ~SyncOnStmt()
    {
        if (this->tid)
        {
            delete this->tid;
        }

        if (this->local_tid)
        {
            delete this->local_tid;
            this->local_tid = nullptr;
        }
    }
};

class ReductionCStmt : public Stmt
{
private:
public:
//===----------------------------------------------------------------------===//
// template args
//===----------------------------------------------------------------------===//
    BaseNode *type;
    BaseNode *tn;
//===----------------------------------------------------------------------===//
// func args
//===----------------------------------------------------------------------===//
    BaseNode *m;
    BaseNode *n;
    BaseNode *ldc;
    BaseNode *cptr;
    BaseNode *bufcptr;
    BaseNode *betaref;
    std::string name;

    ReductionCStmt(const std::string &name, BaseNode *type, BaseNode *tn,
        BaseNode *m, BaseNode *n, BaseNode *ldc, BaseNode *cptr,
        BaseNode *bufcptr, BaseNode *betaref)
        : name(name)
        , type(type)
        , tn(tn)
        , m(m)
        , n(n)
        , ldc(ldc)
        , cptr(cptr)
        , bufcptr(bufcptr)
        , betaref(betaref)
        , Stmt(NodeType::ReductionCStmtKind)
    {
        CHECK_AND_REG_SON2FA(this->type)
        CHECK_AND_REG_SON2FA(this->tn)
        CHECK_AND_REG_SON2FA(this->m)
        CHECK_AND_REG_SON2FA(this->n)
        CHECK_AND_REG_SON2FA(this->ldc)
        CHECK_AND_REG_SON2FA(this->cptr)
        CHECK_AND_REG_SON2FA(this->bufcptr)
        CHECK_AND_REG_SON2FA(this->betaref)
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual bool isend() override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual BaseNode *deepcopy() override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual ~ReductionCStmt()
    {
        CHECK_AND_DELETE(this->type)
        CHECK_AND_DELETE(this->tn)
        CHECK_AND_DELETE(this->m)
        CHECK_AND_DELETE(this->n)
        CHECK_AND_DELETE(this->ldc)
        CHECK_AND_DELETE(this->cptr)
        CHECK_AND_DELETE(this->bufcptr)
        CHECK_AND_DELETE(this->betaref)
    }
};

}