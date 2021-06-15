#pragma once
#include <vector>
#include <hlslmath.h>

namespace PK::Utilities
{
    using namespace PK::Math;

    template<typename T>
    void ValidateVectorSize(std::vector<T>& v, size_t newSize)
    {
        if (v.size() < newSize)
        {
            v.resize(PK::Math::Functions::GetNextExponentialSize(v.size(), newSize));
        }
    }

    template<typename T> 
    void PushVectorElementRef(std::vector<T>& v, size_t* count, T& newElement)
    {
        Utilities::ValidateVectorSize(v, *count + 1u);
        v[(*count)++] = newElement;
    }

    template<typename T>
    void PushVectorElement(std::vector<T>& v, size_t* count, T newElement)
    {
        Utilities::ValidateVectorSize(v, *count + 1u);
        v[(*count)++] = newElement;
    }


    template<typename T>
    void PushVectorElementRef(std::vector<T>& v, uint* count, T& newElement)
    {
        Utilities::ValidateVectorSize(v, *count + 1u);
        v[(*count)++] = newElement;
    }

    template<typename T>
    void PushVectorElement(std::vector<T>& v, uint* count, T newElement)
    {
        Utilities::ValidateVectorSize(v, *count + 1u);
        v[(*count)++] = newElement;
    }
}