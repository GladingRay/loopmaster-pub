#include "expr.h"


namespace loopmaster
{
    void Expr::codeGen(int indent, std::ostream &os)
    {
        os << "I'm Expr\n";
    }

    void ConstNumExpr::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << std::to_string(this->val);
    }

    void VarRef::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << this->v->name;
    }

    void MinExpr::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "std::min(";
        this->first->codeGen(indent, os);
        os << ", ";
        this->second->codeGen(indent, os);
        os <<")";
    }

    void UnaryOpExpr::codeGen(int indent, std::ostream &os)
    {
        if (this->op == OpType::Inc || this->op == OpType::Dec)
        {
            printIndent(indent, os);
            this->e->codeGen(0, os);
            os << toString(this->op);
        }
        else
        {
            printIndent(indent, os);
            os << toString(this->op);
            this->e->codeGen(0, os);
        }
    }

    void BinaryOpExpr::codeGen(int indent, std::ostream &os)
    {
        // FIXME: Here disable '()' when the binaryexpr is loopstmt's increment
        // expression, because #pragma omp parallel for loopstmt's increment
        // expression cannot be surrounded by '()'.
        // But this solution is not professional.
        printIndent(indent, os);
        if (!this->is_loop_increment)
            os << "(";
        this->left->codeGen(0, os);
        os << " " << toString(op) << " ";
        this->right->codeGen(0, os);
        if (!this->is_loop_increment)
            os << ")";
    }

    void AlignedAllocExpr::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "static_cast<T *>(aligned_alloc(";
        this->align_size->codeGen(0, os);
        os << ", ";
        this->bytes->codeGen(0, os);
        os << "))";
    }

    void SizeofTypeExpr::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "sizeof(" << toString(this->dt) << ")";
    }

    void MultiCalcExpr::codeGen(int indent, std::ostream &os)
    {
        if (!this->operands.empty())
        {
            printIndent(indent, os);
            os << "(";
            // NOTE: here int and size_t type product a problem
            for (int i = 0; i < this->operands.size() - 1; i ++)
            {
                this->operands[i]->codeGen(0, os);
                os << " " << toString(this->op) << " ";
            }
            this->operands.back()->codeGen(0, os);
            os << ")";
        }
    }

    void TernaryExpr::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "(";
        this->bool_expr->codeGen(0, os); os << " ? ";
        this->first_expr->codeGen(0, os); os << " : ";
        this->second_expr->codeGen(0, os);
        os << ")";
    }

    void SubscriptExpr::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        this->ptr_expr->codeGen(0, os);
        os << "["; this->offset_expr->codeGen(0, os); os << "]";
    }

    void OmpGetTidExpr::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "omp_get_thread_num()";
    }

} // namespace loopmaster