
#ifndef POST_LEXER_HPP
#define POST_LEXER_HPP
#pragma once

#include "TokenKinds.h"
#include "TokenDirectiveMap.hpp"
#include "TokenKeywordMap.hpp"
#include <vector>
#include <unordered_map>
#include <ctype.h>
#include <iostream>
class PostLexer {
private:
	using LexEnginePtr = Token(PostLexer::*)();
	int PosBuffer = 0;
	int SizeBufferBasic = 0;

	bool IsInclude = false;
private:
	std::vector<Token> LexerTokenBufferAdvance;
	std::vector<Token> LexerTokenBufferBasic;
	void Init();

	std::unordered_map<TokenKind, LexEnginePtr> map{ {
	{TokenKind::Quotation, &PostLexer::Quotation},   // "
	{TokenKind::Hash, &PostLexer::Hash},             // #
	{TokenKind::Ampersand, &PostLexer::Ampersand},   // &
	{TokenKind::Plus, &PostLexer::Plus},             // +
	{TokenKind::Minus, &PostLexer::Minus},           // -
	{TokenKind::Colon, &PostLexer::Colon},           // :
	{TokenKind::Less, &PostLexer::Less},             // <
	{TokenKind::Equal, &PostLexer::Equal},           // =
	{TokenKind::Pipe, &PostLexer::Pipe},             // |
	{TokenKind::Apostrophe, &PostLexer::Apostrophe}, // '
	{TokenKind::Exclamation,&PostLexer::Exclamation},// !
	{TokenKind::Literal, &PostLexer::Literal},       // Literal
	} };

	Token Quotation();
	Token Hash();
	Token Ampersand();
	Token Plus();
	Token Minus();
	Token Colon();
	Token Less();
	Token Equal();
	Token Greater();
	Token Pipe();
	Token Asterisk();
	Token Apostrophe();
	Token Exclamation();
	Token Literal();

	bool neof(const int& pos = 0) const {
		return (PosBuffer + pos) < SizeBufferBasic;
	}

	Token GetUnsafeToken(const int& pos = 0) const {
		return LexerTokenBufferBasic[PosBuffer + pos];
	}

	Token GetCurrentToken(const int& pos = 0) {
		if (!neof(pos))
			return Token{ TokenKind::neof, "", 0, 0 };
		return GetUnsafeToken(pos);
	}

	bool MatchToken(TokenKind Type, const int& pos = 0) const {
		if (neof(pos) && GetUnsafeToken(pos).type == Type)
			return true;
		return false;
	}

public:
	PostLexer(const std::vector<Token>& lexbuffer) {
		SizeBufferBasic = lexbuffer.size();
		LexerTokenBufferAdvance.reserve(SizeBufferBasic);
		LexerTokenBufferBasic.reserve(SizeBufferBasic);
		LexerTokenBufferBasic = lexbuffer;
		Init();
	}

	const std::vector<Token>& GetBufferPostLexerToken() const {
		return LexerTokenBufferAdvance;
	}
};

void PostLexer::Init() {

	while (neof()) {
		auto TokCurrent = GetUnsafeToken();
		auto it = map.find(TokCurrent.type);
		LexerTokenBufferAdvance.push_back(
			it != map.end() ? (this->*it->second)() : TokCurrent
		);
		PosBuffer++;
	}
}

// Обработка строковых литералов 
Token PostLexer::Quotation() /* " */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::StringLiteral;

	IsInclude = false;
	std::string value = "";

	PosBuffer++;
	while (!MatchToken(TokenKind::Quotation)) {
		value += GetUnsafeToken().value;
		PosBuffer++;
	};

	TLexToken.value = value;

	if (LexerTokenBufferAdvance.back().value == "l")
	{
		LexerTokenBufferAdvance.pop_back();
		TLexToken.type = TokenKind::WStringLiteral;
		return TLexToken;
	}
	return TLexToken;
}

// Обработка директив препроцессора
Token PostLexer::Hash() /* # */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Unknown;

	PosBuffer++;

	std::string directive = "#" + GetUnsafeToken().value;

	auto it = TokenDirectiveMap.find(directive);
	if (it != TokenDirectiveMap.end())
	{
		TLexToken.type = it->second;
		TLexToken.value = directive;
		IsInclude = TLexToken.type == TokenKind::IncludeDirective;
	}

	return TLexToken;
}

// Обработка амперсанда
Token PostLexer::Ampersand() /* & */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Ampersand;

	if (MatchToken(TokenKind::Ampersand, 1))
	{
		TLexToken.type = TokenKind::And;
		TLexToken.value = "&&";
		PosBuffer++;
	}

	return TLexToken;
}

// Обработка плюса
Token PostLexer::Plus() /* + */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Plus;

	if (MatchToken(TokenKind::Plus, 1))
	{
		TLexToken.type = TokenKind::Inc;
		TLexToken.value = "++";
		PosBuffer++;
	}

	return TLexToken;
}

// Обработка минуса
Token PostLexer::Minus() /* - */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Minus;

	if (MatchToken(TokenKind::Minus, 1))
	{
		TLexToken.type = TokenKind::Dec;
		TLexToken.value = "--";
		PosBuffer++;
	}
	else if (MatchToken(TokenKind::Greater, 1))
	{
		TLexToken.type = TokenKind::Arrow;
		TLexToken.value = "->";
		PosBuffer++;
	}
	return TLexToken;
}

// Обработка двоеточие
Token PostLexer::Colon() /* : */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Colon;

	if (MatchToken(TokenKind::Colon, 1))
	{
		TLexToken.type = TokenKind::ScResOp;
		TLexToken.value = "::";
		PosBuffer++;
	}

	return TLexToken;
}

// Обработка символа уменьшения
Token PostLexer::Less() /* < */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Less;

	if (IsInclude)
	{
		TLexToken.type = TokenKind::StringLiteral;
		std::string content = "";

		while (!MatchToken(TokenKind::Greater)) {
			content += GetUnsafeToken().value;
			PosBuffer++;
		}

		TLexToken.value = content;
		IsInclude = false;
		return TLexToken;
	}

	if (MatchToken(TokenKind::Equal, 1))
	{
		TLexToken.type = TokenKind::LessEqual;
		TLexToken.value = "<=";
		PosBuffer++;
	}

	return TLexToken;
}

// Обработка символа ровно
Token PostLexer::Equal() /* = */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Equal;

	if (MatchToken(TokenKind::Equal, 1))
	{
		TLexToken.type = TokenKind::Equals;
		TLexToken.value = "==";
		PosBuffer++;
	}

	return TLexToken;
}

// Обработка символа прямого слэша
Token PostLexer::Pipe() /* | */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Pipe;

	if (MatchToken(TokenKind::Pipe, 1))
	{
		TLexToken.type = TokenKind::Or;
		TLexToken.value = "||";
		PosBuffer++;
	}

	return TLexToken;
}

// Обработка строковых литералов 
Token PostLexer::Apostrophe() /* ' */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::CharLiteral;

	PosBuffer++;

	std::string value = "";

	while (!MatchToken(TokenKind::Apostrophe)) {
		value += GetUnsafeToken().value;
		PosBuffer++;
	};

	TLexToken.value = value;

	if (LexerTokenBufferAdvance.back().value == "l")
	{
		LexerTokenBufferAdvance.pop_back();
		TLexToken.type = TokenKind::WCharLiteral;
		return TLexToken;
	}
	return TLexToken;
}

Token PostLexer::Exclamation() /* ! */ {

	Token TLexToken = GetCurrentToken();
	TLexToken.type = TokenKind::Exclamation;

	if (MatchToken(TokenKind::Equal, 1))
	{
		TLexToken.type = TokenKind::NotEqual;
		TLexToken.value = "!=";
		PosBuffer++;
	}

	return TLexToken;
}

// Обработка типа литералов 
Token PostLexer::Literal() {
	Token TLexToken = GetCurrentToken();

	// Проверяем ключевые слова
	auto itKeywordMap = TokenKeywordMap.find(CppHash(TLexToken.value));
	if (itKeywordMap != TokenKeywordMap.end()) {
		TLexToken.type = itKeywordMap->second;
		return TLexToken;
	}

	// Определяем тип числового литерала
	const std::string& value = TLexToken.value;
	size_t len = value.length();

	if (len == 0) return TLexToken;

	// Идентификаторы не могут начинаться с .
	// Только числа
	if (value[0] != '.' && !isdigit(value[0]))
	{
		TLexToken.type = TokenKind::IdentifierLiteral;
		return TLexToken;
	}

	// Шестнадцатеричные: 0x...
	if (len > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) {
		TLexToken.type = TokenKind::HexLiteral;
		return TLexToken;
	}

	// Двоичные: 0b...
	if (len > 2 && value[0] == '0' && (value[1] == 'b' || value[1] == 'B')) {
		TLexToken.type = TokenKind::BinaryLiteral;
		return TLexToken;
	}

	// Восьмеричные: 0...
	if (len > 1 && value[0] == '0') {
		TLexToken.type = TokenKind::IntegerLiteral;
		return TLexToken;
	}

	// Проверяем, есть ли в числе суффиксы типов
	char lastChar = value.back();
	if (lastChar == 'f' || lastChar == 'F') {
		TLexToken.type = TokenKind::FloatLiteral;
		TLexToken.value = value.substr(0, len - 1);
		return TLexToken;
	}

	if (lastChar == 'l' || lastChar == 'L') {
		TLexToken.type = TokenKind::LongDoubleLiteral;
		TLexToken.value = value.substr(0, len - 1);
		return TLexToken;
	}

	// Проверяем, есть ли точка (числа с плавающей точкой)
	if (value.find('.') != std::string::npos ||
		value.find('e') != std::string::npos ||
		value.find('E') != std::string::npos ||
		value.find('p') != std::string::npos ||
		value.find('P') != std::string::npos) {
		TLexToken.type = TokenKind::DoubleLiteral;
		return TLexToken;
	}

	// Обычное целое число
	TLexToken.type = TokenKind::IntegerLiteral;
	return TLexToken;
}

#endif // POST_LEXER_HPP