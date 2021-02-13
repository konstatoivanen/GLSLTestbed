#include "PrecompiledHeader.h"
#include "Utilities/StringHashID.h"

uint32_t StringHashID::id_counter;
std::unordered_map<std::string, uint32_t> StringHashID::string_id_map;
std::unordered_map<uint32_t, std::string> StringHashID::id_string_map;

uint32_t StringHashID::StringToID(const std::string& str)
{
    if (string_id_map.count(str) > 0)
    {
        return string_id_map.at(str);
    }

    string_id_map[str] = ++id_counter;
    id_string_map[id_counter] = str;
    return id_counter;
}

uint32_t StringHashID::StringToID(const char* str)
{
    return StringToID(std::string(str));
}

const std::string& StringHashID::IDToString(uint32_t id)
{
    if (id > id_counter)
    {
        throw std::invalid_argument("Trying to get a string using an invalid id: " + std::to_string(id));
    }

    return id_string_map[id];
}