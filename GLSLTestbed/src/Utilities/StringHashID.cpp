#include "PrecompiledHeader.h"
#include "Utilities/StringHashID.h"

StringHashID::Cache* StringHashID::m_staticCache;

#define STR_TO_ID m_staticCache->string_id_map
#define ID_TO_STR m_staticCache->id_string_map
#define ID_COUNTER m_staticCache->id_counter

uint32_t StringHashID::StringToID(const std::string& str)
{
    if (STR_TO_ID.count(str) > 0)
    {
        return STR_TO_ID.at(str);
    }

    STR_TO_ID[str] = ++ID_COUNTER;
    ID_TO_STR[ID_COUNTER] = str;
    return ID_COUNTER;
}

uint32_t StringHashID::StringToID(const char* str)
{
    return StringToID(std::string(str));
}

const std::string& StringHashID::IDToString(uint32_t id)
{
    if (id > ID_COUNTER)
    {
        throw std::invalid_argument("Trying to get a string using an invalid id: " + std::to_string(id));
    }

    return ID_TO_STR[id];
}