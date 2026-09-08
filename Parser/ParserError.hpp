
#include <stdexcept>
#include <string>

class ParseError : public std::runtime_error
{
public:
    ParseError(int line, int column, const std::string& msg)
        : std::runtime_error(msg) { }
};