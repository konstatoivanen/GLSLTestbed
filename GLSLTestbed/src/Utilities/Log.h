#pragma once
#include "PrecompiledHeader.h"
#include <conio.h>

namespace PK::Utilities::Debug
{
    constexpr unsigned short ComposeConsoleColor(unsigned short fore, unsigned short back)
    {
        return ((unsigned)back << 4) | (unsigned)fore;
    }

    enum class ConsoleColor
    {
        BLACK = 0,
        DARK_BLUE = 1,
        DARK_GREEN = 2,
        DARK_CYAN = 3,
        DARK_RED = 4,
        DARK_MAGENTA = 5,
        DARK_YELLOW = 6,
        DARK_WHITE = 7,
        GRAY = 8,
        BLUE = 9,
        GREEN = 10,
        CYAN = 11,
        RED = 12,
        MAGENTA = 13,
        YELLOW = 14,
        WHITE = 15,
        LOG_PARAMETER = ComposeConsoleColor(15, 0),
        LOG_HEADER = ComposeConsoleColor(0, 15),
        LOG_ERROR = ComposeConsoleColor(0, 4)
    };

    template<typename T, typename... Args>
    void PKLog(int color, const T* message, const Args&...args)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
        printf(message, args...);
        printf("\n");
    }

    template<typename T, typename... Args>
    void PKLogOverwrite(int color, const T* message, const Args&...args)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
        printf(message, args...);
        printf("\r");
    }

    template<typename T, typename ... Args>
    std::exception PKException(int color, const T* message, const Args&...args)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
        printf(message, args...);
        printf("\n");
        _getch();
        return std::exception(message);
    }
}

#define PK_CORE_LOG_HEADER(...) PK::Utilities::Debug::PKLog((unsigned short)PK::Utilities::Debug::ConsoleColor::LOG_HEADER, __VA_ARGS__)
#define PK_CORE_LOG(...) PK::Utilities::Debug::PKLog((unsigned short)PK::Utilities::Debug::ConsoleColor::LOG_PARAMETER, __VA_ARGS__)
#define PK_CORE_LOG_OVERWRITE(...) PK::Utilities::Debug::PKLogOverwrite((unsigned short)PK::Utilities::Debug::ConsoleColor::LOG_PARAMETER, __VA_ARGS__)
#define PK_CORE_EXCEPTION(...) PK::Utilities::Debug::PKException((unsigned short)PK::Utilities::Debug::ConsoleColor::LOG_ERROR, __VA_ARGS__)
#define PK_CORE_ERROR(...) throw PK_CORE_EXCEPTION(__VA_ARGS__)
#define PK_CORE_ASSERT(value, ...) { if(!(value)) { PK_CORE_ERROR(__VA_ARGS__); } }
