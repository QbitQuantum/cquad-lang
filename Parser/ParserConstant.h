
#ifndef PARSER_CONSTANT_H
#define PARSER_CONSTANT_H
#pragma once

namespace typefunction
{
    const int Function = 1;
    const int Constructor = 2;
    const int Destructor = 3;
}

namespace typescope
{
    const int Unknown = -1;
    const int Global = 0;
    const int Class = 1;
    const int Function = 2;
    const int While = 3;
    const int For = 4;
    const int Case = 5;
    const int If = 6;
    const int Else = 7;
    const int Switch = 8;

    static bool requiresBracedBlock(int scope) {
        switch (scope) {
        case typescope::Function:
        case typescope::Class:
        case typescope::Switch:
            return true;
        default:
            return false;
        }
    }
}

namespace typeexpression
{
    const int Unknown = -1;
    const int Condition = 0;
    const int Expression = 1;
}

namespace typeinitialization
{
    const int Unknown = -1;
    const int Default = 0; // int x;
    const int Value = 1; // int x{};
    const int Copy = 2; // int x = 5;
    const int DirectList = 3; // int x{5};
    const int CopyList = 4; // int x = {5};
}

#endif // PARSER_CONSTANT_H