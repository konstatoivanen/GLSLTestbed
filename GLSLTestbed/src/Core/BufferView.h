#pragma once

template<typename T>
struct BufferView
{
    T* data;
    size_t count;

    T& operator[](size_t);
};

template<typename T>
T& BufferView<T>::operator[](size_t index)
{
    if (index >= count)
    {
        throw std::invalid_argument("Out of bounds index");
    }

    return data[index];
}