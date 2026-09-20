
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