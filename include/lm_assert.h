#pragma once

#include <iostream>

#define LM_ASSERT(cond, msg)                                                   \
    if (!(cond))                                                               \
    {                                                                          \
        std::cerr << "Assert failed: " << #cond << "\n";                       \
        std::cerr << "Message: " << msg << "\n";                               \
        std::abort();                                                          \
    }
