#include "runtime.h"

template <typename T>
std::string dataTypeToStr()
{
    return "Data Type";
}

template <>
std::string dataTypeToStr<float>()
{
    return "float";
}

template <>
std::string dataTypeToStr<double>()
{
    return "double";
}

template <>
std::string dataTypeToStr<std::complex<float>>()
{
    return "complex<float>";
}

template <>
std::string dataTypeToStr<std::complex<double>>()
{
    return "complex<double>";
}