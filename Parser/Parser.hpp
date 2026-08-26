
#ifndef PARSER_HPP
#define PARSER_HPP
#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>

#include "PostLexer.hpp"
#include "Node.hpp"

namespace typescope
{
	const int sc_unknown = -1;
	const int sc_global = 0;
	const int sc_class = 1;
	const int sc_function = 2;
}

class TokenStream
{

	std::vector<Token> Buffer;

public:
	size_t Pos = 0;

public:

	TokenStream() = default;

	explicit TokenStream(const std::vector<Token>& buf) : Buffer(buf), Pos(0) {
		skipTrivia();
	}

	void skipTrivia() {
		while (!eof() && (peek().type == TokenKind::Space || peek().type == TokenKind::LineFeed)) {
			++Pos;
		}
	}

	const Token& peek(size_t offset = 0) const {
		static Token eofToken{ TokenKind::neof, "", 0, 0 };
		size_t idx = Pos + offset;
		if (idx >= Buffer.size()) return eofToken;
		return Buffer[idx];
	}

	bool eof() const {
		return Pos >= Buffer.size();
	}

	bool match(TokenKind id) {
		if (peek().type == id) {
			++Pos;
			skipTrivia();
			return true;
		}
		return false;
	}

	const Token& consume(TokenKind id) {
		const Token& tok = peek();
		if (tok.type == id) {
			++Pos;
			skipTrivia();
			return tok;
		}
		// simple error recovery: return current token without advancing
		return tok;
	}
};

class Parser
{
private:
	TokenStream stream;
	std::vector<Node*> ast;

	Node* parseTopLevel();

	Node* parseIdeitfierScope();
	Node* parseIdentifier();

	Node* parseTemplateParameterInstantiation();
	Node* parseTemplateParameterInstantiationList();

	Node* parseFunction();
	Node* parseFunctionParameter();
	Node* parseFunctionParameterList();
	Node* parseFunctionBody();
	Node* parseFunctionBlock();
	
	Node* parseStatement(int type_scope);
	Node* parseDeclaration();
	Node* parseDeclarationPrimary();

	// Парсинг объяление переменных
	Node* parseVar();
	Node* parseVarType();
	Node* parseVarDeclaration();
	Node* parseVarDeclarationList();

	// Парсинг класса
	Node* parseClass();
	Node* parseClassName();
	Node* parseClassBody();
	Node* parseClassBaseClass();
	Node* parseClassBlock();

	Node* parsePrimary();
	Node* parseExpression(int priory = 0);

	Node* parseNew();
	Node* parseDelete();
	Node* parseNullptr();
	Node* parseDefault();
	Node* parseNodeInteger();
	Node* parseNodeFloating();
	Node* parseNodeBoolean();
	Node* parseNodeString();
	Node* parseNodeCharacter();
	Node* parseNodeCall();

	Node* parseSizeArgCArray();

	Node* parseType();

public:
	std::vector<Token> ParserEngineBuffer;

	Parser(const PostLexer& advance) :
		ParserEngineBuffer(advance.GetBufferPostLexerToken()),
		stream(ParserEngineBuffer) {
	}

	Parser(const std::vector<Token>& Buffer) :
		ParserEngineBuffer(Buffer),
		stream(Buffer) {
	}

	~Parser()
	{
		for (auto& i : ast)
			if (i) delete i;
	};

	void Parse() {

		while (!stream.eof()) {
			if (Node* node = parseTopLevel()) {
				ast.push_back(node);
			}
			else {
				// basic recovery: advance one token
				stream.consume(stream.peek().type);
			}
		}
	}

	const std::vector<Node*>& GetAst() const {
		return ast;
	}
};

Node* Parser::parseTopLevel()
{
	switch (stream.peek().type) {
	case TokenKind::Class:    return parseClass();
	default: return parseStatement(typescope::sc_global);
	}
}

Node* Parser::parseExpression(int MinPrec) {

	using BinaryOperand = NodeBinaryOp::BinaryOp;
	BinaryOperand UnaryOp = BinaryOperand::Unknown;

	auto getBinaryOperand = [](TokenKind op) -> BinaryOperand
		{
			switch (op) {
			case TokenKind::Minus: return BinaryOperand::Minus;
			case TokenKind::Plus: return BinaryOperand::Plus;
			case TokenKind::Asterisk: return BinaryOperand::Asterisk;
			case TokenKind::Slash: return BinaryOperand::Slash;
			default: return BinaryOperand::Unknown;
			}
		};

	Node* Left = parsePrimary();

	while (true) {
		TokenKind op = stream.peek().type;
		if (!tok::IsBinaryOperator(op))
			break;
		int currentPriority = tok::GetBinaryOperatorPriority(op);
		if (currentPriority < MinPrec)
			break;
		stream.consume(op);
		Node* Right = parseExpression(currentPriority + 1);
		Left = new NodeBinaryOp(getBinaryOperand(op), Left, Right);
	}

	return Left;
}

Node* Parser::parseNew() {

	stream.consume(TokenKind::New);
	return new NodeNew(parseIdentifier());
}

Node* Parser::parseDelete() {
	stream.consume(TokenKind::Delete_);
	return new NodeDelete();
}

Node* Parser::parseNullptr() {
	stream.consume(TokenKind::NullptrLiteral);
	return new NodeNullptr();
}

Node* Parser::parseDefault() {
	stream.consume(TokenKind::Default);
	return new NodeDefault();
}

Node* Parser::parseNodeInteger() {
	return new NodeInteger(stream.consume(stream.peek().type).value);
}

Node* Parser::parseNodeFloating() {
	return new NodeFloating(stream.consume(stream.peek().type).value);
}

Node* Parser::parseNodeBoolean() {
	return new NodeBoolean(stream.consume(stream.peek().type).value);
}

Node* Parser::parseNodeString() {
	return new NodeString(stream.consume(stream.peek().type).value);
}

Node* Parser::parseNodeCharacter() {
	return new NodeCharacter(stream.consume(stream.peek().type).value);
}

Node* Parser::parseNodeCall() {

	Node* CallName = parseIdentifier();

	if (stream.peek().type != TokenKind::LeftParen)
		throw std::runtime_error("Expected LeftParen token");
	stream.consume(TokenKind::LeftParen);

	std::vector<Node*> ArgumentConcreticList;

	if (stream.peek().type != TokenKind::RightParen)
	{
		ArgumentConcreticList.push_back(parseExpression());
		while (stream.peek().type == TokenKind::Comma) {
			stream.consume(TokenKind::Comma);
			ArgumentConcreticList.push_back(parseExpression());
		}
	}

	if (stream.peek().type != TokenKind::RightParen)
		throw std::runtime_error("Expected RightParen token");
	stream.consume(TokenKind::RightParen);

	return new NodeCall(CallName, ArgumentConcreticList);

}

Node* Parser::parseSizeArgCArray() {
	switch (stream.peek().type) {
	case TokenKind::IdentifierLiteral:
		return parseIdentifier();
	case TokenKind::IntegerLiteral:
	case TokenKind::HexLiteral:
	case TokenKind::BinaryLiteral:
		return parseNodeInteger(); break;
	default: break;
	}
	throw std::runtime_error("Not correct token");
}

Node* Parser::parseType() {

	/*
	Допустимые вариации типов
	T&          // изменяемая ссылка
	const T&    // неизменяемая ссылка
	T*          // указатель
	const T*    // указатель на константу
	T&&         // rvalue-ссылка (move)
	const T&&   // - бессмысленно, но для простоты парсинга
	*/

	Node* Type = nullptr;
	Node* SizeArgCArray = nullptr;
	bool IsConst = false;
	NodeType::EType eType = NodeType::EType::NONE;

	// Проверям на константность
	if (stream.match(TokenKind::Const))
		IsConst = true;

	// Проверяем наличие типа
	if (stream.peek().type != TokenKind::IdentifierLiteral)
		throw std::runtime_error("Expected identifier token");

	// Парсим имя типа
	Type = parseIdeitfierScope();

	if (stream.match(TokenKind::LeftBracket))
	{
		SizeArgCArray = parseSizeArgCArray();
		if (!stream.match(TokenKind::RightBracket))
			throw std::runtime_error("Expected RightBracket token");
	}

	// Проверяем семантику 
	switch (stream.peek().type)
	{
	case TokenKind::Asterisk:
		stream.consume(TokenKind::Asterisk);
		eType = NodeType::EType::POINTER;
		break;
	case TokenKind::Ampersand:
		stream.consume(TokenKind::Ampersand);
		eType = NodeType::EType::REF;
		break;
	case TokenKind::And:
		stream.consume(TokenKind::And);
		eType = NodeType::EType::RVALUE;
		break;
	default:
		break;
	}

	return new NodeType(Type, SizeArgCArray, IsConst, eType);
};

Node* Parser::parsePrimary() {

	using UnaryOperand = NodeUnaryOp::UnaryOp;
	UnaryOperand UnaryOp = UnaryOperand::Unknown;

	auto getUnaryOperand = [](TokenKind op) -> UnaryOperand
		{
			switch (op) {
			case TokenKind::Minus: return UnaryOperand::Minus;
			default: return UnaryOperand::Unknown;
			}
		};

	if (tok::IsUnaryOperator(stream.peek().type))
	{
		UnaryOp = getUnaryOperand(stream.peek().type);
		stream.consume(stream.peek().type);
	}

	Node* Right = nullptr;

	switch (stream.peek().type) {
	case TokenKind::New:
		Right = parseNew(); break;
	case TokenKind::Delete_:
		Right = parseDelete(); break;
	case TokenKind::NullptrLiteral:
		Right = parseNullptr(); break;
	case TokenKind::Default:
		Right = parseDefault(); break;
	case TokenKind::IdentifierLiteral:
		Right = parseIdentifier(); break;
	case TokenKind::IntegerLiteral:
	case TokenKind::HexLiteral:
	case TokenKind::BinaryLiteral:
		Right = parseNodeInteger(); break;
	case TokenKind::FloatLiteral:
	case TokenKind::DoubleLiteral:
	case TokenKind::LongDoubleLiteral:
		Right = parseNodeFloating(); break;
	case TokenKind::TrueLiteral:
	case TokenKind::FalseLiteral:
		Right = parseNodeBoolean(); break;
	case TokenKind::StringLiteral:
	case TokenKind::WStringLiteral:
		Right = parseNodeString(); break;
	case TokenKind::CharLiteral:
	case TokenKind::WCharLiteral:
		Right = parseNodeCharacter(); break;
	case TokenKind::LeftParen:
	{
		stream.consume(TokenKind::LeftParen);
		Right = parseExpression();
		if (stream.peek().type != TokenKind::RightParen) {
			throw std::runtime_error("Expected ')'");
		}
		stream.consume(TokenKind::RightParen);
		break;
	}
	default: throw std::runtime_error("Unexpected token in primary expression");
	}
	return UnaryOp == UnaryOperand::Unknown ? Right : new NodeUnaryOp(UnaryOp, Right);
}

Node* Parser::parseTemplateParameterInstantiation() {
	// TODO.1
	return parsePrimary();
}

Node* Parser::parseTemplateParameterInstantiationList() {

	// Instantiation-параметры: PublicMethodBase<int, std::vector<int>>
	stream.consume(TokenKind::Less);

	std::vector<Node*> TemplateParameterInstantiationList;
	if (stream.peek().type != TokenKind::Greater)
	{
		TemplateParameterInstantiationList.push_back(parseTemplateParameterInstantiation());
		while (stream.peek().type == TokenKind::Comma) {
			stream.consume(TokenKind::Comma);
			TemplateParameterInstantiationList.push_back(parseTemplateParameterInstantiation());
		}
	}
	if (stream.peek().type != TokenKind::Greater)
		throw std::runtime_error("Expected Greater token");
	stream.consume(TokenKind::Greater);

	return new NodeTemplateParameterInstantiationList(TemplateParameterInstantiationList);
}


Node* Parser::parseIdeitfierScope() {
	std::string Identifier = "";
	std::vector<std::string> Scope;
	Node* IdentifierTemplateParameterInstantiationList = nullptr;
	while (true) {
		switch (stream.peek().type) {
		case TokenKind::IdentifierLiteral:
			// Костыль
			if (Identifier.empty())
			{
				Identifier = stream.consume(TokenKind::IdentifierLiteral).value;
			}
			else
			{
				return new NodeIdentifier(IdentifierTemplateParameterInstantiationList, Identifier, new NodeScope(Scope));
			}
			break;
		case TokenKind::ScResOp:
			stream.consume(TokenKind::ScResOp);
			if (stream.peek().type != TokenKind::IdentifierLiteral)
				throw std::runtime_error("Expected identifier after '::'");
			Scope.push_back(Identifier);
			Identifier = "";
			break;
		case TokenKind::Less:
			IdentifierTemplateParameterInstantiationList = parseTemplateParameterInstantiationList();
			return new NodeIdentifier(IdentifierTemplateParameterInstantiationList, Identifier, new NodeScope(Scope));
			break;
		default:
			return new NodeIdentifier(IdentifierTemplateParameterInstantiationList, Identifier, new NodeScope(Scope));
		}
	}
}

Node* Parser::parseIdentifier() {
	return parseIdeitfierScope();
}

Node* Parser::parseStatement(int type_scope) {
	
	Node* Statement = nullptr;

	// сохраняем состояние потока
	int tempStream = stream.Pos;

	// Парсим тип (int, Data*, const int[5] и т.д.)
	Node* type = parseType();

	switch (stream.peek().type)
	{
	case TokenKind::IdentifierLiteral:
	{
		Node* name = parseIdentifier();
		// если после имени идет '(' - это ФУНКЦИЯ
		if (stream.peek().type == TokenKind::LeftParen) {
			stream.Pos = tempStream;
			Statement = parseFunction();
		}
		else
		{
			stream.Pos = tempStream;
			Statement = parseVar();
		}
		delete name;
		break;
	}
	case TokenKind::Equal:
	{
		if (type_scope == typescope::sc_function)
		{
			stream.Pos = tempStream;
			Statement = parseDeclaration();
		}
		break;
	}
	case TokenKind::LeftParen:
	{
		if (type_scope == typescope::sc_function)
		{
			stream.Pos = tempStream;
			Statement = parseNodeCall();
		}
		break;
	}
	default:
		throw std::runtime_error("Expected '=' or ';' after variable name");
		break;
	}

	stream.consume(TokenKind::Semicolon);

	return Statement;
}

Node* Parser::parseFunction() {
	// Парсим возвращаемый тип
	Node* returnType = parseType();

	// Парсим имя функции
	if (stream.peek().type != TokenKind::IdentifierLiteral) {
		throw std::runtime_error("Expected function name");
	}
	Node* name = parseIdentifier();

	// Парсим параметры
	Node* params = parseFunctionParameterList();

	// Парсим тело или ';'
	Node* body = parseFunctionBody();

	return new NodeFunction(returnType, nullptr, name, params, body);
}

Node* Parser::parseFunctionParameterList() {

	if (stream.peek().type != TokenKind::LeftParen)
		throw std::runtime_error("Expected LeftParen token");
	stream.consume(TokenKind::LeftParen);

	std::vector<Node*> ArgumentList;

	if (stream.peek().type != TokenKind::RightParen)
	{
		ArgumentList.push_back(parseFunctionParameter());
		while (stream.peek().type == TokenKind::Comma) {
			stream.consume(TokenKind::Comma);
			ArgumentList.push_back(parseFunctionParameter());
		}
	}

	if (stream.peek().type != TokenKind::RightParen)
		throw std::runtime_error("Expected RightParen token");
	stream.consume(TokenKind::RightParen);

	return new NodeParameterList(ArgumentList);
}

Node* Parser::parseFunctionParameter()
{
	// Parse parameter type
	Node* type = parseType();

	// Parse parameter name (optional)
	Node* name = nullptr;
	if (stream.peek().type == TokenKind::IdentifierLiteral) {
		name = parseIdentifier();
	}

	// Check for default value
	Node* defaultValue = nullptr;
	if (stream.match(TokenKind::Equal)) {
		defaultValue = parseExpression();
	}

	return new NodeVarDeclarationList(type, name, defaultValue);
}

Node* Parser::parseFunctionBody() {

	Node* Body = nullptr;

	if (stream.match(TokenKind::LeftBrace))
	{
		Body = parseFunctionBlock();
		if (stream.peek().type != TokenKind::RightBrace)
			throw std::runtime_error("Expected RightBrace token");
		stream.consume(TokenKind::RightBrace);
	}
	else
	{
		if (stream.peek().type != TokenKind::Semicolon)
			throw std::runtime_error("not expected Semicolon token");
		stream.consume(TokenKind::Semicolon);
	}
	return Body;
}

Node* Parser::parseFunctionBlock()
{
	NodeBlock* block = new NodeBlock();

	while (!stream.eof() && stream.peek().type != TokenKind::RightBrace) {
		Node* stmt = parseStatement(typescope::sc_function);
		if (stmt) block->add(stmt);
	}

	return block;
}

Node* Parser::parseDeclaration() {

	Node* Identifier = nullptr;
	Node* Exptression = nullptr;

	// Имя может быть пустое. По хорошему исключить такую фигню
	if (stream.peek().type == TokenKind::IdentifierLiteral)
		// То что может быть Namespace::Name в имене идентикатора - работа семантера
		Identifier = parseIdeitfierScope();

	if (stream.peek().type == TokenKind::Equal)
	{
		if (!Identifier)
			throw std::runtime_error("Expected identifier");
		stream.consume(TokenKind::Equal);
		Exptression = parseExpression();
	}

	return new NodeDeclaration(Identifier, Exptression);
}

Node* Parser::parseDeclarationPrimary() {

	Node* Identifier = nullptr;
	Node* Exptression = nullptr;

	// Имя может быть пустое. По хорошему исключить такую фигню
	if (stream.peek().type == TokenKind::IdentifierLiteral)
		// То что может быть Namespace::Name в имене идентикатора - работа семантера
		Identifier = parseIdeitfierScope();

	if (stream.peek().type == TokenKind::Equal)
	{
		if (!Identifier)
			throw std::runtime_error("Expected identifier");
		stream.consume(TokenKind::Equal);
		Exptression = parsePrimary();
	}

	return new NodeDeclaration(Identifier, Exptression);
}

Node* Parser::parseVar() {

	Node* VarTemplateParameterDeclarationList = nullptr;

	Node* VarType = parseVarType();

	Node* VarDeclarationList = parseVarDeclarationList();

	stream.consume(TokenKind::Semicolon);

	return new NodeVarDeclarationList(VarTemplateParameterDeclarationList, VarType, VarDeclarationList);
}

Node* Parser::parseVarType() {
	return parseType();
}

Node* Parser::parseVarDeclaration() {
	return parseDeclaration();
}

Node* Parser::parseVarDeclarationList() {
	std::vector<Node*> ContainerDeclarationList;
	ContainerDeclarationList.push_back(parseDeclaration());
	// Парсим аргументы: name = default
	while (stream.peek().type == TokenKind::Comma) {
		stream.consume(TokenKind::Comma);
		ContainerDeclarationList.push_back(parseDeclaration());
	}
	return new NodeDeclarationList(ContainerDeclarationList);
}

Node* Parser::parseClass() {
	// assume current token is Class
	stream.consume(TokenKind::Class);

	Node* ClassTemplateParameterDeclarationList = nullptr;

	Node* ClassName = parseClassName();

	Node* ClassBaseClass = parseClassBaseClass();

	Node* ClassBody = parseClassBody();

	return new NodeClass(ClassName, ClassTemplateParameterDeclarationList, ClassBaseClass, ClassBody);
}


Node* Parser::parseClassName() {
	if (stream.peek().type != TokenKind::IdentifierLiteral)
		throw std::runtime_error("Expected class name");
	return parseIdeitfierScope();
}

Node* Parser::parseClassBody() {

	Node* Body = nullptr;

	if (stream.peek().type == TokenKind::LeftBrace)
	{
		stream.consume(TokenKind::LeftBrace);
		Body = parseClassBlock();
		if (stream.peek().type != TokenKind::RightBrace)
			throw std::runtime_error("Expected '}' after class declaration");
		stream.consume(TokenKind::RightBrace);
	}
	else
	{
		if (stream.peek().type != TokenKind::Semicolon)
			throw std::runtime_error("Expected ';' after class forward declaration");
	}

	return Body;
}

Node* Parser::parseClassBaseClass() {
	Node* BaseClass = nullptr;

	if (stream.match(TokenKind::Colon))
	{
		using ClassInheritanceType = NodeBaseClass::InheritanceType;
		ClassInheritanceType Type = ClassInheritanceType::NONE;

		switch (stream.peek().type)
		{
		case TokenKind::Public:
			stream.consume(TokenKind::Public);
			Type = ClassInheritanceType::PUBLIC;
			break;
		case TokenKind::Private:
			stream.consume(TokenKind::Private);
			Type = ClassInheritanceType::PRIVATE;
			break;
		default:
			Type = ClassInheritanceType::NONE; break;
		}

		Node* ClassName = parseClassName();

		BaseClass = new NodeBaseClass(ClassName, Type);
	}
	return BaseClass;
}

Node* Parser::parseClassBlock() {

	// Ужас. Надо будет переделать
	using ClassFieldType = NodeBlockClass::FieldType;
	std::vector<Node*> Statements;
	std::vector<std::pair<ClassFieldType, std::vector<Node*>>> FieldStatements;
	ClassFieldType Type = ClassFieldType::NONE;

	auto getClassFieldType = [](TokenKind op) -> ClassFieldType
		{
			switch (op) {
			case TokenKind::Private: return ClassFieldType::PRIVATE;
			case TokenKind::Public: return ClassFieldType::PUBLIC;
			case TokenKind::Static: return ClassFieldType::STATIC;
			default: return ClassFieldType::NONE;
			}
		};

	while (!stream.eof() && stream.peek().type != TokenKind::RightBrace) {
		Node* stmt = nullptr;
		switch (stream.peek().type) {
		case TokenKind::Private:
		case TokenKind::Public:
		case TokenKind::Static:
		{
			TokenKind Scope = stream.peek().type;
			if (!Statements.empty() || Type != ClassFieldType::NONE) {
				FieldStatements.push_back({ Type, Statements });
				Statements.clear();
			}
			Type = getClassFieldType(Scope);
			stream.consume(Scope);
			break;
		}
		case TokenKind::Class:    stmt = parseClass(); break;
		default: stmt = parseStatement(typescope::sc_class); break;
		}
		if (stmt) Statements.push_back(stmt);
	}

	if (!Statements.empty() || Type != ClassFieldType::NONE) {
		FieldStatements.push_back({ Type, Statements });
	}

	return new NodeBlockClass(FieldStatements);
}

#endif // PARSER_HPP