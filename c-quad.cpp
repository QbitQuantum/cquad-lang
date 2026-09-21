// c-quad.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "Lexer.hpp"
#include "PostLexer.hpp"
#include "Parser.hpp"

void print_diagnostic(const std::string& source_code, const std::string& filepath, int line, int column, const std::string& message) {
    std::cerr << "\033[1;31mParseError:\033[0m " << message << "\n";
    std::cerr << "  --> " << filepath << ":" << line << ":" << column << "\n";
    std::cerr << "   |\n";

    std::stringstream ss(source_code);
    std::string current_line;
    int current_line_num = 1;

    while (std::getline(ss, current_line)) {
        if (current_line_num == line) {
            std::cerr << " " << line << " | " << current_line << "\n";
            std::cerr << "   | ";
            for (int i = 1; i < column; ++i) {
                if (current_line[i - 1] == '\t') std::cerr << "\t";
                else std::cerr << " ";
            }
            std::cerr << "\033[1;31m^\033[0m\n";
            break;
        }
        current_line_num++;
    }
    std::cerr << "   |\n";
}

std::string ReadFile(const std::string& filepath) {

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file)
        return "";
    auto size = file.tellg();
    std::string content(size, '\0');
    file.seekg(0);
    file.read(&content[0], size);
    return content;
};

int main()
{
    const std::string filepath = "code.cqq";
    const std::string code = ReadFile(filepath);

    Lexer lexer(code);
	auto lexerbuffer = lexer.GetBufferLexerToken();

	PostLexer postLexer(lexerbuffer);
	auto postlexerbuffer = postLexer.GetBufferPostLexerToken();

	Parser parser(postlexerbuffer);
	try
	{
		parser.Parse();
	}
    catch (const ParseError& err)
    {
        print_diagnostic(code, filepath, err.line(), err.column(), err.what());
    }

	std::cout << "node->print()" << "\n";
	const auto& ast = parser.GetAst();
	for (auto* node : ast) {
		if (node)
			std::cout << node->print() << "\n";
	}
    std::cout << "========================" << "\n";
    parser.GetSymbolTable().dump();
}