#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include "base_node.h"

namespace loopmaster
{
class Expr : public BaseNode
{
private:
public:
    Expr(NodeType et = NodeType::AbstractExprKind)
        : BaseNode(et)
    {
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~Expr() { }
};

class ConstNumExpr : public Expr
{
private:
public:
    int val;

    ConstNumExpr(int val_)
        : val(val_)
        , Expr(NodeType::ConstNumExprKind)
    {
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~ConstNumExpr() { }
};

class VarRef : public Expr
{
private:
public:
    Var *v;
    VarRef(Var *v_)
        : v(v_)
        , Expr(NodeType::VarRefKind)
    {
    }

    VarRef(DataType dt, const std::string &name)
        : Expr(NodeType::VarRefKind)
    {
        v = BaseNode::getVar(dt, name);
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool istarget(NodeInfo &node_info) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~VarRef() { }
};

class MinExpr : public Expr
{
private:
public:
    BaseNode *first;
    BaseNode *second;

    MinExpr(BaseNode *first_, BaseNode *second_)
        : first(first_)
        , second(second_)
        , Expr(NodeType::MinExprKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (first != nullptr)
            BaseNode::son2father[first] = this;
        if (second != nullptr)
            BaseNode::son2father[second] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~MinExpr()
    {
        if (first)
        {
            delete first;
            first = nullptr;
        }

        if (second)
        {
            delete second;
            second = nullptr;
        }
    }
};

class UnaryOpExpr : public Expr
{
private:
public:
    OpType op;
    BaseNode *e;

    UnaryOpExpr(const OpType &op_, BaseNode *e_)
        : op(op_)
        , e(e_)
        , Expr(NodeType::UaryOpExprKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (e != nullptr)
            BaseNode::son2father[e] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~UnaryOpExpr()
    {
        if (e)
        {
            delete e;
            e = nullptr;
        }
    }
};

class BinaryOpExpr : public Expr
{
private:
public:
    OpType op;
    BaseNode *left;
    BaseNode *right;

    bool is_loop_increment;

    BinaryOpExpr(const OpType &op_, BaseNode *left_, BaseNode *right_)
        : op(op_)
        , left(left_)
        , right(right_)
        , is_loop_increment(false)
        , Expr(NodeType::BinaryOpExprKind)
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

    virtual ~BinaryOpExpr()
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

class AlignedAllocExpr : public Expr
{
private:
public:
    BaseNode *align_size;
    BaseNode *bytes;

    AlignedAllocExpr(BaseNode *align_size_, BaseNode *bytes_)
        : align_size(align_size_)
        , bytes(bytes_)
        , Expr(NodeType::AlignedAllocExprKind)
    {
        this->updateSon();
    }

    virtual void updateSon()
    {
        if (align_size != nullptr)
            BaseNode::son2father[align_size] = this;
        if (bytes != nullptr)
            BaseNode::son2father[bytes] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~AlignedAllocExpr()
    {
        if (align_size)
        {
            delete align_size;
            align_size = nullptr;
        }

        if (bytes)
        {
            delete bytes;
            bytes = nullptr;
        }
    }
};

class SizeofTypeExpr : public Expr
{
private:
public:
    DataType dt;

    SizeofTypeExpr(DataType dt_)
        : dt(dt_)
        , Expr(NodeType::SizeofTypeExprKind)
    {
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~SizeofTypeExpr() { }
};

class MultiCalcExpr : public Expr
{
private:
public:
    OpType op;
    std::vector<BaseNode *> operands;

    MultiCalcExpr(OpType op_, const std::vector<BaseNode *> &operands_)
        : op(op_)
        , operands(operands_)
        , Expr(NodeType::MultiCalcExprKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        for (BaseNode *e : operands)
        {
            if (e != nullptr)
                BaseNode::son2father[e] = this;
        }
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~MultiCalcExpr()
    {
        for (size_t i = 0; i < operands.size(); i++)
        {
            if (operands[i] != nullptr)
            {
                delete operands[i];
                operands[i] = nullptr;
            }
        }
    }
};

class TernaryExpr : public Expr
{
private:
public:
    BaseNode *bool_expr;
    BaseNode *first_expr;
    BaseNode *second_expr;

    TernaryExpr(
        BaseNode *bool_expr_, BaseNode *first_expr_, BaseNode *second_expr_)
        : bool_expr(bool_expr_)
        , first_expr(first_expr_)
        , second_expr(second_expr_)
        , Expr(NodeType::TernaryExprKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (bool_expr != nullptr)
            BaseNode::son2father[bool_expr] = this;
        if (first_expr != nullptr)
            BaseNode::son2father[first_expr] = this;
        if (second_expr != nullptr)
            BaseNode::son2father[second_expr] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~TernaryExpr()
    {
        if (bool_expr)
        {
            delete bool_expr;
            bool_expr = nullptr;
        }

        if (first_expr)
        {
            delete first_expr;
            first_expr = nullptr;
        }

        if (second_expr)
        {
            delete second_expr;
            second_expr = nullptr;
        }
    }
};

class SubscriptExpr : public Expr
{
private:
public:
    BaseNode *ptr_expr;
    BaseNode *offset_expr;

    SubscriptExpr(BaseNode *ptr_expr_, BaseNode *offset_expr_)
        : ptr_expr(ptr_expr_)
        , offset_expr(offset_expr_)
        , Expr(NodeType::SubscriptExprKind)
    {
        this->updateSon();
    }

    virtual void updateSon() override
    {
        if (ptr_expr)
            BaseNode::son2father[ptr_expr] = this;
        if (offset_expr)
            BaseNode::son2father[offset_expr] = this;
    }

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;

    virtual ~SubscriptExpr()
    {
        if (ptr_expr)
        {
            delete ptr_expr;
            ptr_expr = nullptr;
        }

        if (offset_expr)
        {
            delete offset_expr;
            offset_expr = nullptr;
        }
    }
};

/**
 * @brief omp_get_thread_num()
 *
 */
class OmpGetTidExpr : public Expr
{
private:
public:
    OmpGetTidExpr()
        : Expr(NodeType::OmpGetTidExprKind)
    {
    }

    virtual ~OmpGetTidExpr() = default;

    virtual void codeGen(int indent, std::ostream &os) override;

    virtual BaseNode *walkFindNode(NodeInfo &node_info) override;

    virtual void walkFindAllNodes(
        NodeInfo &node_info, std::vector<BaseNode *> &results) override;

    virtual bool isend() override;

    virtual BaseNode *deepcopy() override;
};
}