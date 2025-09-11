#pragma once
#include <string>
namespace loopmaster
{
enum class Dim
{
    M       = 'M',
    N       = 'N',
    K       = 'K',
    Void    = 'V'
};

inline char toString(Dim d)
{
    return static_cast<char>(d);
}

inline char toLower(Dim d)
{
    return toString(d) + 32;
}

inline std::string toIdx(Dim d)
{
    return std::string(1, toLower(d)) + "_idx";
}

inline std::string toLen(Dim d)
{
    return "b" + toLower(d);
}

inline std::string toBlock(Dim d)
{
    return "B" + toString(d);
}

enum class DataType
{
    I = 0,
    Void,
    T,

    IPtr,
    VoidPtr,
    TPtr,
    TransOp,
    Typename,
};

inline const char *toString(const DataType dt)
{
    static const char *dt2str[] = { "int", "void", "T", "int *", "void *",
        "T *", "ds_blas::ds_op_t", "typename" };

    return dt2str[static_cast<int>(dt)];
}

enum class OpType
{
    Anti = 0,
    DeAddr,
    GetAddr,

    Assign,
    AddAssign,
    Add,
    Minus,
    Mul,
    Div,
    Mod,

    EQ,
    Less,
    LE,
    GE,
    G,
    And,
    Or,

    Inc,
    Dec
};

inline const char *toString(const OpType ot)
{
    static const char *ot2str[] = { "!", "*", "&", "=", "+=", "+", "-", "*",
        "/", "%", "==", "<", "<=", ">=", ">", "&&", "||", "++", "--" };

    return ot2str[static_cast<int>(ot)];
}

enum class PackType
{
    NoPack          = 'N',
    ReadContinuous  = 'R',
    WriteContinuous = 'W'
};

enum class TransType
{
    N = 'N', // notrans, col-major
    T = 'T', // trans, row-major
};

}