
#include <stdexcept>
#include <string>

class ParseError : public std::runtime_error
{
private:
    int line_;
    int column_;
public:
    ParseError(int line, int column, const std::string& msg) :
        std::runtime_error(msg), line_(line), column_(column)
    {
    }

    int line() const { return line_; }
    int column() const { return column_; }
};
