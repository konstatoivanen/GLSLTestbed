#pragma once
#include "Utilities/Ref.h"
#include "Utilities/Log.h"

class IService
{
    protected: virtual ~IService() = 0 {};
};

class ServiceRegister
{
    public:
        template <typename T>
        Ref<T> GetService() { return std::static_pointer_cast<T>(m_services.at(std::type_index(typeid(T)))); }

        template<typename T, typename ... Args>
        Ref<T> CreateService(Args&& ... args)
        {
            auto idx = std::type_index(typeid(T));
            PK_CORE_ASSERT(m_services.count(idx) == 0, "Service of type (%s) is already registered", typeid(T).name());
            auto service = CreateRef<T, Args...>(args...);
            m_services[idx] = service;
            return service;
        }

        void Clear() { m_services.clear(); }

    private:
        std::unordered_map<std::type_index, Ref<IService>> m_services;
};