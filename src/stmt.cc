#include "stmt.h"

namespace loopmaster
{

    Var BaseNode::static_one = Var(DataType::I, "1", true);
    Var BaseNode::static_zero = Var(DataType::I, "0", true);
    Var BaseNode::static_vec_len = Var(DataType::I, "VecLen<T>::value", true);

    std::unordered_map<std::string, Var *> BaseNode::decl_vars = {
        {"int 1", &static_one},
        {"int 0", &static_zero},
        {"int VecLen<T>::value", &static_vec_len}
    };

    std::unordered_map<BaseNode *, BaseNode *> BaseNode::son2father;

    void Stmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "// " << "I' m a statement;\n";
    }

    void VoidLineStmt::codeGen(int indent, std::ostream &os)
    {
        os << std::endl;
    }

    void CommentStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "// " << this->comment << std::endl;
    }

    void PragmaStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "#pragma " << this->inst << "\n";
    }

    void IncludeStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "#include \"" << this->header_name << "\"\n";
    }

    void NameSpaceStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "namespace " << this->namespace_name << "\n";
        printIndent(indent, os);
        os << "{\n";

        for (BaseNode *stmt : body)
            stmt->codeGen(indent + 1, os);
        printIndent(indent, os);
        os << "}\n";
    }

    void VarDeclStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << this->toVarName() << ";\n";
    }

    void ArgDeclStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << this->toVarName();
    }

    void VarDeclWithRVStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << this->toVarName() << " = ";
        rv->codeGen(0, os);
        os << ";\n";
    }

    void LoopIndexDeclStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << this->toVarName() << " = ";
        rv->codeGen(0, os);
    }

    void LoopStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "for (";

        if (init)
            init->codeGen(0, os);
        os << "; ";

        if (end_flag)
            end_flag->codeGen(0, os);
        os << "; ";

        if (update)
            update->codeGen(0, os);
        os << ")\n";
        printIndent(indent, os);
        os << "{\n";

        for (BaseNode *stmt : loop_body)
            stmt->codeGen(indent + 1, os);
        printIndent(indent, os);
        os << "}";
        LoopIndexDeclStmt *decl_init = dynamic_cast<LoopIndexDeclStmt *>(init);
        if (decl_init)
        {
            os << " // " << decl_init->decled_var->name;
        }
        else if (bind_var)
            os << " // " << bind_var->name;
        os << "\n";
    }

    void TemplateStructFuncStmt::codeGen(
        int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "template <";

        for (size_t i = 0; i < targ_list.size(); ++i)
        {
            targ_list[i]->codeGen(0, os);

            os << ((i == targ_list.size() - 1) ? ">\n" : ", ");
        }
        printIndent(indent, os);
        os << "struct " << this->func_name << std::endl;
        printIndent(indent, os);
        os << "{\n";
        printIndent(indent + 1, os);
        os << "void operator()(";

        for (size_t i = 0; i < farg_list.size(); ++i)
        {
            farg_list[i]->codeGen(0, os);

            os << ((i == farg_list.size() - 1) ? ")\n" : ", ");
        }
        printIndent(indent + 1, os);
        os << "{\n";

        for (BaseNode *stmt : body)
            stmt->codeGen(indent + 2, os);
        printIndent(indent + 1, os);
        os << "}\n";
        printIndent(indent, os);
        os << "};\n";
    }

    void ZeroBufStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "memset(";
        buf_ptr->codeGen(0, os);
        os << ", 0x0, ";
        buf_size->codeGen(0, os);
        os << ");\n";
    }

    void FreeBufStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "free(";
        buf_ptr->codeGen(0, os);
        os << ");\n";
    }

    void KernelCallStmt::codeGen(int indent, std::ostream  &os)
    {
        printIndent(indent, os);
        os << kernel_name << "<" << "T, " << kernel_id << ", "
            << (packa_flag ? "true" : "false") << ", "
            << (packb_flag ? "true" : "false") << ", "
            << (unpackc_flag ? "true" : "false") << ">(\n";

        printIndent(indent, os);

        for (size_t i = 0; i < func_args.size()-1; i ++)
        {
            func_args[i]->codeGen(0, os);
            os << ", ";
        }

        func_args.back()->codeGen(0, os); os << ");\n";
    }

    void CodeBlockStmt::codeGen(int indent, std::ostream &os)
    {
        for (BaseNode *stmt : this->stmts_in_block)
        {
            if (stmt)
                stmt->codeGen(indent, os);
        }
    }

    void VarAssignStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        this->left->codeGen(0, os); os << " = ";
        this->right->codeGen(0, os); os << ";\n";
    }

    void VectorCopyStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "VectorCopy<T>(";
        this->dst_ptr->codeGen(0, os);
        os << ", ";
        this->src_ptr->codeGen(0, os);
        os << ");\n";
    }

    void VectorCopyAddStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "VectorCopyAdd<T>(";
        this->dst_ptr->codeGen(0, os);
        os << ", ";
        this->src_ptr->codeGen(0, os);
        os << ", ";
        this->beta_ref->codeGen(0, os);
        os << ");\n";
    }

    void CbetaOpStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "CbetaOp<T>(";
        this->m_len->codeGen(0, os);
        os << ", ";
        this->n_len->codeGen(0, os);
        os << ", ";
        this->c_ptr->codeGen(0, os);
        os << ", ";
        this->ldc->codeGen(0, os);
        os << ", ";
        this->beta_ref->codeGen(0, os);
        os << ", ";
        this->tid->codeGen(0, os);
        os << ", ";
        this->tn->codeGen(0, os);
        os << ");\n";
    }

    void ExprToStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        this->expr->codeGen(0, os); os << ";\n";
    }

    void OmpParallelStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "#pragma omp parallel";
        if (this->num_threads)
        {
            os << " num_threads(";
            this->num_threads->codeGen(0, os);
            os << ")";
        }
        os << "\n";
        printIndent(indent, os);
        os << "{\n";

        this->parallel_body->codeGen(indent + 1, os);
        printIndent(indent, os);
        os << "}\n";
    }

    void OmpBarrierStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "#pragma omp barrier\n";
    }

    void OmpParallelForStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "#pragma omp parallel for";
        if (this->num_threads)
        {
            os << " num_threads(";
            this->num_threads->codeGen(0, os);
            os << ")";
        }
        os << "\n";
        this->loop->codeGen(indent, os);
    }

    void OmpSetNestedStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "omp_set_nested(";
        if (this->enable_nested)
            os << "1";
        else
            os << "0";
        os << ");\n";
    }

    void OmpAtomicStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "#pragma omp atomic\n";
        printIndent(indent, os);
        os << "{\n";
        this->body->codeGen(indent, os);
        os << "}\n";
    }

    void SyncVarDeclInitStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "const int " << this->gn_name << " = NUM_THREADS / "; this->num->codeGen(0, os); os << ";\n";
        printIndent(indent, os);
        os << "volatile int " << this->name << "[" << this->gn_name;
        os << "]["; this->num->codeGen(0, os); os << "]["; this->num->codeGen(0, os); os << "];\n";
        printIndent(indent, os);
        os << "InitSync<" << this->gn_name << ", "; this->num->codeGen(0, os); os << ">(" << this->name << ");\n";
    }

    void SyncOnStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "SyncOn<" << this->gn_name << ", " << this->tn_name << ">("; this->tid->codeGen(0, os);
        os << ", "; this->local_tid->codeGen(0, os); os << ", " << this->sync_name << ");\n";
    }

    void ReductionCStmt::codeGen(int indent, std::ostream &os)
    {
        printIndent(indent, os);
        os << "ReductionC<"; this->type->codeGen(0, os); os << ", ";
        this->tn->codeGen(0, os); os << ">(";
        this->m->codeGen(0, os); os << ", "; this->n->codeGen(0, os);
        os << ", "; this->ldc->codeGen(0, os); os << ", ";
        this->cptr->codeGen(0, os); os << ", "; this->bufcptr->codeGen(0, os);
        os << ", "; this->betaref->codeGen(0, os); os << ");\n";
    }

} // namespace loopmaster