#pragma once
#include <iostream>
#include <string>

#include "enum_type.h"

namespace loopmaster
{
class Var
{

private:
public:
    DataType dt;
    std::string name;

    bool is_constant;

    Var(DataType dt_, const std::string &name_, bool is_const = false)
        : dt(dt_)
        , name(name_)
        , is_constant(is_const)
    {
    }

    std::string toStr()
    {
        return (is_constant ? "const " : "") + std::string(toString(this->dt))
            + " " + this->name;
    }

    std::string toHashStr()
    {
        return std::string(toString(this->dt)) + " " + this->name;
    }

    ~Var() { }
};
}