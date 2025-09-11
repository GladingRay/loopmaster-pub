/**
 * Only support avx512
 */

#include "runtime.h"
template <>
struct VecLen<double>
{
    static constexpr int value = 8;
};

template <>
struct VecLen<float>
{
    static constexpr int value = 16;
};