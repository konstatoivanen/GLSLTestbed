#pragma once
#include "PrecompiledHeader.h"

namespace PK_Debug
{
    #define CONS_BLACK 0
    #define CONS_DARK_BLUE 1
    #define CONS_DARK_GREEN 2
    #define CONS_DARK_CYAN 3
    #define CONS_DARK_RED 4
    #define CONS_DARK_MAGENTA 5
    #define CONS_DARK_YELLOW 6
    #define CONS_DARK_WHITE 7
    #define CONS_GRAY 8
    #define CONS_BLUE 9
    #define CONS_GREEN 10
    #define CONS_CYAN 11
    #define CONS_RED 12
    #define CONS_MAGENTA 13
    #define CONS_YELLOW 14
    #define CONS_WHITE 15
    
    #define CONS_COLOR(fore, back) ((unsigned)back<<4)|(unsigned)fore

    #define CONS_COLOR_PARA CONS_COLOR(CONS_WHITE, CONS_BLACK)
    #define CONS_COLOR_HEAD CONS_COLOR(CONS_BLACK, CONS_WHITE)
    #define CONS_COLOR_ERROR CONS_COLOR(CONS_BLACK, CONS_RED)

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
        return std::exception(message);
    }
}

#define PK_CORE_LOG_HEADER(...) PK_Debug::PKLog(CONS_COLOR_HEAD, __VA_ARGS__)
#define PK_CORE_LOG(...) PK_Debug::PKLog(CONS_COLOR_PARA, __VA_ARGS__)
#define PK_CORE_LOG_OVERWRITE(...) PK_Debug::PKLogOverwrite(CONS_COLOR_PARA, __VA_ARGS__)
#define PK_CORE_EXCEPTION(...) PK_Debug::PKException(CONS_COLOR_ERROR, __VA_ARGS__)
#define PK_CORE_ERROR(...) throw PK_CORE_EXCEPTION(__VA_ARGS__)
#define PK_CORE_ASSERT(value, ...) { if(!(value)) { PK_CORE_ERROR(__VA_ARGS__); } }