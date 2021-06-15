#include "PrecompiledHeader.h"
#include "Log.h"

static int LineClearLength = 0;

void PK::Utilities::Debug::ClearLineRemainder(int length)
{
    auto l = LineClearLength - length;

    if (LineClearLength < length)
    {
        LineClearLength = length;
    }

    if (l <= 0)
    {
        return;
    }

    printf(std::string(l, ' ').c_str());
}

void PK::Utilities::Debug::InsertNewLine()
{
    printf("\n");
    LineClearLength = 0;
}
