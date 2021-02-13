#pragma once
#include "PrecompiledHeader.h"

class StringHashID
{
    private:
        static std::unordered_map<std::string, uint32_t> string_id_map;
        static std::unordered_map<uint32_t, std::string> id_string_map;
        static uint32_t id_counter;

    public:
        static uint32_t StringToID(const std::string& str);
        static uint32_t StringToID(const char* str);
        static const std::string& IDToString(uint32_t id);
};