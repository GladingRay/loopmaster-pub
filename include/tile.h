#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "enum_type.h"
#include "var.h"

namespace loopmaster
{
struct LoopIndex
{
    bool is_unit;

    bool packa_here;
    bool packb_here;
    bool unpackc_here;

    std::string idx_name;

    Dim dim;
    Var *step; // nullptr means 1
    Var *len; // nullptr means 1
    Var *stride; // nullptr means 1
    Var *idx;

    /* For Test and Debug */

    std::string toStr()
    {
        std::string ret;

        ret += toString(dim);
        ret += packa_here ? "pa" : "";
        ret += packb_here ? "pb" : "";
        ret += unpackc_here ? "upc" : "";

        return ret;
    }
};
}