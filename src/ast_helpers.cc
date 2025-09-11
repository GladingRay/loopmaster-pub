#include "common.h"

namespace loopmaster
{
bool CodeBlockStmt::hasLoop() 
{
    for (BaseNode *stmt : this->stmts_in_block)
    {
        if (stmt->node_type == NodeType::LoopStmtKind)
            return true;
        else if (CodeBlockStmt *codes = dynamic_cast<CodeBlockStmt *>(stmt))
        {
            if (codes->hasLoop())
            {
                return true;
            }
        }
    }
    return false;
}
}