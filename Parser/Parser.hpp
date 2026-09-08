#ifndef PARSER_HPP
#define PARSER_HPP
#pragma once

#include <vector>
#include <iostream>
#include <memory>

#include "PostLexer.hpp"
#include "ParserError.hpp"
#include "Node.hpp"

namespace typescope
{
    const int sc_unknown = -1;
    const int sc_global = 0;
    const int sc_class = 1;
    const int sc_function = 2;
}

namespace typeexpression
{
    const int sc_unknown = -1;
    const int sc_condition = 0;
    const int sc_expression = 1;
}

class TokenStream
{
    std::vector<Token> Buffer;

public:
    size_t Pos = 0;

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

    // Сохранение и восстановление позиции
    size_t savePosition() const {
        return Pos;
    }

    void restorePosition(size_t pos) {
        Pos = pos;
    }
};

class Parser
{
private:
    TokenStream stream;
    std::vector<Node*> ast;

    // Вспомогательные методы
    bool isAtEnd() const { return stream.eof(); }

    TokenKind currentTokenKind() const { return stream.peek().type; }

    void advance() { stream.consume(stream.peek().type); }

    void skipTrivia() { stream.skipTrivia(); }

    // Основные методы парсинга
    Node* parseTopLevel();

    Node* parseIdentifierScope();
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

    Node* parseVar();
    Node* parseVarType();
    Node* parseVarDeclaration();
    Node* parseVarDeclarationList();

    Node* parseClass();
    Node* parseClassName();
    Node* parseClassBody();
    Node* parseClassBaseClass();
    Node* parseClassBlock();

    Node* parsePrimary();
    Node* parseExpression(int priority = 0, int typeexpression = typeexpression::sc_expression);

    Node* ParseCondition();
    Node* parseIf();
    Node* parseIfCondition();
    Node* parseIfBody();
    Node* parseElse();
    Node* parseElseBody();

    Node* parseWhile();
    Node* parseWhileCondition();
    Node* parseWhileBody();

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

    void raise(const std::string& msg) {
        auto token = stream.peek();
        throw ParseError::ParseError(token.line, token.column, msg);
    }

    void parseToken(TokenKind kind, const std::string& msg) {
        if (!stream.match(kind)) raise(msg);
    }

    void Parse() {
        while (!isAtEnd()) {
            if (Node* node = parseTopLevel()) {
                ast.push_back(node);
            }
            else {
                // basic recovery: advance one token
                advance();
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

Node* Parser::parseExpression(int MinPrec, int _typeexpression) {
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
        bool typeexpression = _typeexpression == typeexpression::sc_expression ? tok::IsBinaryOperator(op) : tok::IsConditionalOperator(op);
        if (!typeexpression)
            break;
        int currentPriority = tok::GetBinaryOperatorPriority(op);
        if (currentPriority < MinPrec)
            break;
        stream.consume(op);
        Node* Right = parseExpression(currentPriority + 1, _typeexpression);
        Left = new NodeBinaryOp(getBinaryOperand(op), Left, Right);
    }

    return Left;
}

Node* Parser::ParseCondition() {
    Node* ifCondition = parseIf();
    Node* elseCondition = nullptr;
    if (stream.peek().type == TokenKind::Else)
        elseCondition = parseElse();
    return new NodeCondition(ifCondition, elseCondition);
}

Node* Parser::parseIf() {
    stream.consume(TokenKind::If);
    Node* Condition = parseIfCondition();
    Node* Body = parseIfBody();
    return new NodeIf(Condition, Body);
}

Node* Parser::parseIfCondition() {
    parseToken(TokenKind::LeftParen, "Expected '(' after 'if'");
    Node* Condition = parseExpression(0, typeexpression::sc_condition);
    parseToken(TokenKind::RightParen, "Expected ')' after if-condition");
    return Condition;
}

Node* Parser::parseIfBody() {
    if (stream.match(TokenKind::LeftBrace)) {
        Node* block = parseFunctionBlock();
        parseToken(TokenKind::RightBrace, "Expected '}' after if-body");
        return block;
    }

    NodeBlock* block = new NodeBlock();
    Node* stmt = (stream.peek().type == TokenKind::If)
        ? ParseCondition()
        : parseStatement(typescope::sc_function);
    if (stmt) block->add(stmt);
    return block;
}

Node* Parser::parseElse() {
    stream.consume(TokenKind::Else);
    Node* Body = parseElseBody();
    return new NodeElse(Body);
}

Node* Parser::parseElseBody() {
    return parseIfBody();
}

Node* Parser::parseWhile() {
    stream.consume(TokenKind::While);
    Node* Condition = parseWhileCondition();
    bool hasDo = stream.match(TokenKind::Do);
    Node* Body = parseWhileBody();
    return new NodeWhile(Condition, Body, hasDo);
}

Node* Parser::parseWhileCondition() {
    parseToken(TokenKind::LeftParen, "Expected '(' after 'while'");
    Node* Condition = parseExpression(0, typeexpression::sc_condition);
    parseToken(TokenKind::RightParen, "Expected ')' after while-condition");
    return Condition;
}

Node* Parser::parseWhileBody() {
    Node* Body = nullptr;
    if (stream.match(TokenKind::LeftBrace)) {
        Body = parseFunctionBlock();
        parseToken(TokenKind::RightBrace, "Expected '}' after while-body");
    }
    else
    {
        NodeBlock* block = new NodeBlock();
        Node* stmt = parseStatement(typescope::sc_function);
        if (stmt) block->add(stmt);
        Body = block;
    }
    return Body;
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
    parseToken(TokenKind::LeftParen, "Expected '(' after Identifier");

    std::vector<Node*> ArgumentConcreticList;

    if (stream.peek().type != TokenKind::RightParen)
    {
        ArgumentConcreticList.push_back(parseExpression());
        while (stream.peek().type == TokenKind::Comma) {
            stream.consume(TokenKind::Comma);
            ArgumentConcreticList.push_back(parseExpression());
        }
    }

    parseToken(TokenKind::RightParen, "Expected ')'");
    return new NodeCall(CallName, ArgumentConcreticList);
}

Node* Parser::parseSizeArgCArray() {
    switch (stream.peek().type) {
    case TokenKind::IdentifierLiteral:
        return parseIdentifier();
    case TokenKind::IntegerLiteral:
    case TokenKind::HexLiteral:
    case TokenKind::BinaryLiteral:
        return parseNodeInteger();
    default:
        raise("Not correct token for array size");
    }
    return nullptr;
}

Node* Parser::parseType() {
    Node* Type = nullptr;
    Node* SizeArgCArray = nullptr;
    bool IsConst = false;
    NodeType::EType eType = NodeType::EType::NONE;

    bool is_auto = stream.match(TokenKind::Auto);

    if (!is_auto)
    {
        if (stream.match(TokenKind::Const))
            IsConst = true;

        if (stream.peek().type != TokenKind::IdentifierLiteral)
            raise("Expected identifier token");

        Type = parseIdentifierScope();

        if (stream.match(TokenKind::LeftBracket))
        {
            SizeArgCArray = parseSizeArgCArray();
            if (!stream.match(TokenKind::RightBracket))
                raise("Expected RightBracket token");
        }

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
    }
    return new NodeType(Type, SizeArgCArray, IsConst, eType, is_auto);
}

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
        parseToken(TokenKind::RightParen, "Expected ')'");
        break;
    }
    default: raise("Unexpected token in primary expression");
    }
    return UnaryOp == UnaryOperand::Unknown ? Right : new NodeUnaryOp(UnaryOp, Right);
}

Node* Parser::parseTemplateParameterInstantiation() {
    return parsePrimary();
}

Node* Parser::parseTemplateParameterInstantiationList() {
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
        raise("Expected Greater token");
    stream.consume(TokenKind::Greater);

    return new NodeTemplateParameterInstantiationList(TemplateParameterInstantiationList);
}

Node* Parser::parseIdentifierScope() {
    std::string Identifier = "";
    std::vector<std::string> Scope;
    Node* IdentifierTemplateParameterInstantiationList = nullptr;

    while (true) {
        switch (stream.peek().type) {
        case TokenKind::IdentifierLiteral:
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
                raise("Expected identifier after '::'");
            Scope.push_back(Identifier);
            Identifier = "";
            break;
        case TokenKind::Less:
            IdentifierTemplateParameterInstantiationList = parseTemplateParameterInstantiationList();
            return new NodeIdentifier(IdentifierTemplateParameterInstantiationList, Identifier, new NodeScope(Scope));
        default:
            return new NodeIdentifier(IdentifierTemplateParameterInstantiationList, Identifier, new NodeScope(Scope));
        }
    }
}

Node* Parser::parseIdentifier() {
    return parseIdentifierScope();
}

Node* Parser::parseStatement(int type_scope) {
    Node* Statement = nullptr;
    size_t savedPos = stream.savePosition();

    Node* type = parseType();

    switch (stream.peek().type)
    {
    case TokenKind::IdentifierLiteral:
    {
        Node* name = parseIdentifier();
        if (stream.peek().type == TokenKind::LeftParen) {
            stream.restorePosition(savedPos);
            Statement = parseFunction();
        }
        else
        {
            stream.restorePosition(savedPos);
            Statement = parseVar();
        }
        delete name;
        break;
    }
    case TokenKind::Equal:
    {
        if (type_scope == typescope::sc_function)
        {
            stream.restorePosition(savedPos);
            Statement = parseDeclaration();
        }
        break;
    }
    case TokenKind::LeftParen:
    {
        if (type_scope == typescope::sc_function)
        {
            stream.restorePosition(savedPos);
            Statement = parseNodeCall();
        }
        break;
    }
    default:
        raise("Expected '=' or ';' after variable name");
        break;
    }

    stream.consume(TokenKind::Semicolon);

    return Statement;
}

Node* Parser::parseFunction() {
    Node* returnType = parseType();

    if (stream.peek().type != TokenKind::IdentifierLiteral) {
        raise("Expected function name");
    }
    Node* name = parseIdentifier();

    Node* params = parseFunctionParameterList();

    Node* body = parseFunctionBody();

    return new NodeFunction(returnType, nullptr, name, params, body);
}

Node* Parser::parseFunctionParameterList() {
    parseToken(TokenKind::LeftParen, "Expected '('");

    std::vector<Node*> ArgumentList;

    if (stream.peek().type != TokenKind::RightParen)
    {
        ArgumentList.push_back(parseFunctionParameter());
        while (stream.peek().type == TokenKind::Comma) {
            stream.consume(TokenKind::Comma);
            ArgumentList.push_back(parseFunctionParameter());
        }
    }

    parseToken(TokenKind::RightParen, "Expected ')'");
    return new NodeParameterList(ArgumentList);
}

Node* Parser::parseFunctionParameter()
{
    Node* type = parseType();
    Node* name = nullptr;
    if (stream.peek().type == TokenKind::IdentifierLiteral) {
        name = parseIdentifier();
    }

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
        parseToken(TokenKind::RightBrace, "Expected '}'");
    }
    else
    {
        parseToken(TokenKind::Semicolon, "Expected ';'");
    }
    return Body;
}

Node* Parser::parseFunctionBlock()
{
    NodeBlock* block = new NodeBlock();

    while (!isAtEnd() && stream.peek().type != TokenKind::RightBrace) {
        Node* stmt = nullptr;
        switch (stream.peek().type) {
        case TokenKind::If:    stmt = parseIf(); break;
        case TokenKind::While: stmt = parseWhile(); break;
        default: stmt = parseStatement(typescope::sc_function); break;
        }
        if (stmt) block->add(stmt);
    }

    return block;
}

Node* Parser::parseDeclaration() {
    Node* Identifier = nullptr;
    Node* Expression = nullptr;  // Исправлено: Exptression -> Expression

    if (stream.peek().type == TokenKind::IdentifierLiteral)
        Identifier = parseIdentifierScope();

    if (stream.peek().type == TokenKind::Equal)
    {
        if (!Identifier)
            raise("Expected identifier");
        stream.consume(TokenKind::Equal);
        Expression = parseExpression();
    }

    return new NodeDeclaration(Identifier, Expression);
}

Node* Parser::parseDeclarationPrimary() {
    Node* Identifier = nullptr;
    Node* Expression = nullptr;  // Исправлено: Exptression -> Expression

    if (stream.peek().type == TokenKind::IdentifierLiteral)
        Identifier = parseIdentifierScope();

    if (stream.peek().type == TokenKind::Equal)
    {
        if (!Identifier)
            raise("Expected identifier");
        stream.consume(TokenKind::Equal);
        Expression = parsePrimary();
    }

    return new NodeDeclaration(Identifier, Expression);
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
    while (stream.peek().type == TokenKind::Comma) {
        stream.consume(TokenKind::Comma);
        ContainerDeclarationList.push_back(parseDeclaration());
    }
    return new NodeDeclarationList(ContainerDeclarationList);
}

Node* Parser::parseClass() {
    stream.consume(TokenKind::Class);

    Node* ClassTemplateParameterDeclarationList = nullptr;
    Node* ClassName = parseClassName();
    Node* ClassBaseClass = parseClassBaseClass();
    Node* ClassBody = parseClassBody();

    return new NodeClass(ClassName, ClassTemplateParameterDeclarationList, ClassBaseClass, ClassBody);
}

Node* Parser::parseClassName() {
    if (stream.peek().type != TokenKind::IdentifierLiteral)
        raise("Expected class name");
    return parseIdentifierScope();
}

Node* Parser::parseClassBody() {
    Node* Body = nullptr;

    if (stream.peek().type == TokenKind::LeftBrace)
    {
        stream.consume(TokenKind::LeftBrace);
        Body = parseClassBlock();
        parseToken(TokenKind::RightBrace, "Expected '}' after class declaration");
    }
    else
    {
        parseToken(TokenKind::Semicolon, "Expected ';' after class forward declaration");
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

    while (!isAtEnd() && stream.peek().type != TokenKind::RightBrace) {
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
            stream.consume(TokenKind::Colon);
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