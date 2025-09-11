#include "common.h"

namespace loopmaster
{
bool BaseNode::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type;
}

bool ModuleAST::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type &&
        node_info.target_name == this->module_name;
}

bool PragmaStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->inst;
}

bool IncludeStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->header_name;
}

bool NameSpaceStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->namespace_name;
}

bool VarDeclStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->var_name;
}

bool TemplateStructFuncStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->func_name;
}

bool LoopStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->bind_var->name;
}

bool KernelCallStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->kernel_name;
}

bool OmpParallelForStmt::istarget(NodeInfo &node_info)
{
    NodeInfo loop_node_info{NodeType::LoopStmtKind, node_info.target_name};
    return node_info.node_type == this->node_type
        && this->loop->istarget(loop_node_info);
}

bool VarRef::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->v->name;
}

bool CommentStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->comment;
}

bool SyncVarDeclInitStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type
        && node_info.target_name == this->name;
}

bool SyncOnStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type &&
        node_info.target_name == this->sync_name;
}

bool ReductionCStmt::istarget(NodeInfo &node_info)
{
    return node_info.node_type == this->node_type &&
        node_info.target_name == this->name;
}

}