#pragma once
#include <vector>
#include <hlslmath.h>

namespace PK::Utilities
{
    template<typename T>
    void ValidateVectorSize(std::vector<T>& v, size_t newSize)
    {
        if (v.size() < newSize)
        {
            v.resize(PK::Math::Functions::GetNextExponentialSize(v.size(), newSize));
        }
    }
}