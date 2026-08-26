// c-quad.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>

#include "Lexer.hpp"
#include "PostLexer.hpp"
#include "Parser.hpp"

const std::string code =
R"(
class M
{
    // Одиночные объявления
    int a = 10;
    Data* data = nullptr;
    std::vector<int> a;

    // Групповые объявления
    int x, y, z;
    const int[5] arr1, arr2, arr3;
    Data* ptr1, ptr2 = nullptr, ptr3;

    // Смешанные (с инициализацией)
    int a = 10, b = 20, c;

    // Объявление функции
    int* func2();
    // Объявление функции с телом
    int* func() { int a; };
}

// Объявление инициализации функции вне класса
int* M::func2()
{
};

)";

int main()
{
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