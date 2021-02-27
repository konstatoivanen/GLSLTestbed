#pragma once
#include "PrecompiledHeader.h"

class StringHashID
{
    public:
        struct Cache
        {
            std::unordered_map<std::string, uint32_t> string_id_map;
            std::unordered_map<uint32_t, std::string> id_string_map;
            uint32_t id_counter = 0;
        };

        static void SetCache(Cache* cache) { m_staticCache = cache; }
        static uint32_t StringToID(const std::string& str);
        static uint32_t StringToID(const char* str);
        static const std::string& IDToString(uint32_t id);

    private:
        static Cache* m_staticCache;
};