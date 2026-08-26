// c-quad.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <fstream>
#include "Lexer.hpp"
#include "PostLexer.hpp"
#include "Parser.hpp"

std::string ReadFile(std::string filepath) {

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
    std::string code = ReadFile("code.cqq");

    Lexer lexer(code);
	auto lexerbuffer = lexer.GetBufferLexerToken();

	PostLexer postLexer(lexerbuffer);
	auto postlexerbuffer = postLexer.GetBufferPostLexerToken();

	Parser parser(postlexerbuffer);
	parser.Parse();
	std::cout << "node->print()" << "\n";
	const auto& ast = parser.GetAst();
	for (auto* node : ast) {
		if (node)
			std::cout << node->print() << "\n";
	}
}