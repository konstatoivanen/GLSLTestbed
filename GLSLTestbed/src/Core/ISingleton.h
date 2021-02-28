#pragma once

template<typename T>
class ISingleton
{
    public: 
        ISingleton() { s_Instance = static_cast<T*>(this); }
        virtual ~ISingleton() = 0 {};
        static T* Get() { return s_Instance; }

    private: inline static T* s_Instance = nullptr;
};