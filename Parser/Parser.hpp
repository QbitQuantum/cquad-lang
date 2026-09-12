#ifndef PARSER_HPP
#define PARSER_HPP
#pragma once

#include <vector>
#include <string>
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

namespace typeinitialization
{
    const int i_unknown = -1;
    const int i_default = 0; // int x;
    const int i_value = 1; // int x{};
    const int i_copy = 2; // int x = 5;
    const int i_direct_list = 3; // int x{5};
    const int i_copy_list = 4; // int x = {5};
}
class TokenStream
{
    std::vector<Token> Buffer;
    mutable size_t Pos = 0;
    const Token eofToken = { TokenKind::neof, "", 0, 0 };
public:
    TokenStream() = default;

    explicit TokenStream(const std::vector<Token>& buf) : Buffer(buf), Pos(0) {
        skipTrivia();
    }

    bool isTriviaToken() const noexcept {
        const TokenKind& k = peek().type;
        return tok::isWhitespaceToken(k) || tok::isCommentToken(k);
    }

    void skipTrivia() const noexcept {
        while (!eof() && isTriviaToken()) ++Pos;
    }

    const Token& peek(size_t offset = 0) const noexcept {
        size_t idx = Pos + offset;
        if (idx >= Buffer.size()) return eofToken;
        return Buffer[idx];
    }

    bool eof() const noexcept {
        return Pos >= Buffer.size();
    }

    bool match(TokenKind id) const noexcept {
        if (peek().type == id) {
            ++Pos;
            skipTrivia();
            return true;
        }
        return false;
    }

    const Token& consume(TokenKind id) const noexcept {
        const Token& tok = peek();
        if (tok.type == id) {
            ++Pos;
            skipTrivia();
            return tok;
        }
        return tok;
    }

    size_t savePosition() const noexcept {
        return Pos;
    }

    void restorePosition(size_t pos) const noexcept {
        Pos = pos;
    }
};

class Parser
{
private:
    TokenStream stream;
    std::vector<Node*> ast;
    std::vector<Token> ParserEngineBuffer;

    bool isAtEnd() const noexcept { return stream.eof(); }
    TokenKind currentTokenKind() const noexcept { return stream.peek().type; }
    const Token& peek(size_t offset = 0) const noexcept { return stream.peek(offset); }
    bool match(TokenKind id) const noexcept { return stream.match(id); }
    const Token& consume(TokenKind id) const noexcept { return stream.consume(id); }
    void advance() const noexcept { stream.consume(stream.peek().type); }
    void skipTrivia() const noexcept { stream.skipTrivia(); }
    size_t savePosition() const noexcept { return stream.savePosition(); }
    void restorePosition(size_t pos) const noexcept { stream.restorePosition(pos); }

    void raise(const std::string& msg) {
        auto token = peek();
        throw ParseError::ParseError(token.line, token.column, msg);
    }

    void parseToken(TokenKind kind, const std::string& msg, bool soft = false) {
        if (!match(kind)) if (!soft) raise(msg);
    }

    Node* CreateTemplateDecl(Node* TemplateParams, Node* Decl);

    Node* parseTopLevel();

    Node* parseIdentifier(int _typeexpression = typeexpression::sc_unknown);
    Node* parseIdentifierExpression(int _typeexpression = typeexpression::sc_unknown);

    Node* parseTemplateDeclaration();
    Node* parseTemplate();
    Node* parseTemplateParameter();
    Node* parseTemplateParameterList();

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
    Node* parseInitializerList();
    Node* parseInitializerListBody();

    Node* ParseConditionConstruct();
    Node* parseIf();
    Node* parseIfCondition();
    Node* parseIfBody();
    Node* parseElse();
    Node* parseElseBody();

    Node* parseWhile();
    Node* parseWhileCondition();
    Node* parseWhileBody();

    Node* parseReturn();

    Node* parseNew();
    Node* parseDelete();
    Node* parseNullptr();
    Node* parseDefault();
    Node* parseNodeInteger();
    Node* parseNodeFloating();
    Node* parseNodeBoolean();
    Node* parseNodeString();
    Node* parseNodeCharacter();
    Node* parseNodeCall(Node* Identifier);

    Node* parseSizeArgCArray();

    Node* parseType();

public:

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
        while (!isAtEnd()) {
            if (Node* node = parseTopLevel()) {
                ast.push_back(node);
            }
            else {
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
    Node* Template = parseTemplateDeclaration();
    Node* Stmt = nullptr;
    switch (peek().type) {
    case TokenKind::Class:    Stmt = parseClass(); break;
    default: Stmt = parseStatement(typescope::sc_global); break;
    }
    if (Template) Stmt = CreateTemplateDecl(Template, Stmt);
    return Stmt;
}

Node* Parser::parseIdentifier(int _typeexpression) {
    if (peek().type != TokenKind::IdentifierLiteral)
        raise("Expected identifier");

    std::string Identifier = consume(TokenKind::IdentifierLiteral).value;
    std::vector<std::string> Scope;

    while (match(TokenKind::ScResOp)) {
        if (peek().type != TokenKind::IdentifierLiteral)
            raise("Expected identifier after '::'");
        Scope.push_back(std::move(Identifier));
        Identifier = consume(TokenKind::IdentifierLiteral).value;
    }

    Node* TemplateArgs = nullptr;
    if (peek().type == TokenKind::Less &&
        _typeexpression != typeexpression::sc_condition) {
        TemplateArgs = parseTemplateParameterInstantiationList();
    }

    return new NodeIdentifier(TemplateArgs, Identifier, new NodeScope(std::move(Scope)));
}

Node* Parser::parseIdentifierExpression(int _typeexpression) {
    Node* IdentExpression = parseIdentifier(_typeexpression);
    if (peek().type == TokenKind::LeftParen)
        IdentExpression = parseNodeCall(IdentExpression);
    if (peek().type == TokenKind::Dot || peek().type == TokenKind::Arrow)
    {
        bool IsArrow = peek().type == TokenKind::Arrow; advance();
        IdentExpression = new NodeMemberCall(IdentExpression, parseIdentifierExpression(_typeexpression), IsArrow);
    }
    return IdentExpression;
}

Node* Parser::parseExpression(int MinPrec, int _typeexpression) {
    Node* Left = parsePrimary();
    while (true) {
        TokenKind op = peek().type;
        bool typeexpression = _typeexpression == typeexpression::sc_expression ? tok::IsBinaryOperator(op) : tok::IsConditionalOperator(op);
        if (!typeexpression)
            break;
        int currentPriority = tok::GetBinaryOperatorPriority(op);
        if (currentPriority < MinPrec)
            break;
        consume(op);
        Node* Right = parseExpression(currentPriority + 1, _typeexpression);
        Left = new NodeBinaryOp(BinOparand::getBinaryOperand(op), Left, Right);
    }
    return Left;
}

Node* Parser::parseInitializerList() {
    parseToken(TokenKind::LeftBrace, "Expected '{' for initializer list");
    Node* body = parseInitializerListBody();
    parseToken(TokenKind::RightBrace, "Expected '}' after initializer list");
    return body;
}

Node* Parser::parseInitializerListBody() {
    std::vector<Node*> InitElements;
    if (peek().type != TokenKind::RightBrace)
    {
        InitElements.push_back(parseExpression());
        while (match(TokenKind::Comma)) {
            if (peek().type == TokenKind::RightBrace) break;
            InitElements.push_back(parseExpression());
        }
    }
    return new NodeInitializerList(InitElements);
}

Node* Parser::ParseConditionConstruct() {
    Node* ifCondition = parseIf();
    Node* elseCondition = nullptr;
    if (peek().type == TokenKind::Else)
        elseCondition = parseElse();
    return new NodeCondition(ifCondition, elseCondition);
}

Node* Parser::parseIf() {
    consume(TokenKind::If);
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
    if (match(TokenKind::LeftBrace)) {
        Node* block = parseFunctionBlock();
        parseToken(TokenKind::RightBrace, "Expected '}' after if-body");
        return block;
    }

    NodeBlock* block = new NodeBlock();
    Node* stmt = (peek().type == TokenKind::If)
        ? ParseConditionConstruct()
        : parseStatement(typescope::sc_function);
    if (stmt) block->add(stmt);
    return block;
}

Node* Parser::parseElse() {
    consume(TokenKind::Else);
    Node* Body = parseElseBody();
    return new NodeElse(Body);
}

Node* Parser::parseElseBody() {
    return parseIfBody();
}

Node* Parser::parseWhile() {
    consume(TokenKind::While);
    Node* Condition = parseWhileCondition();
    bool hasDo = match(TokenKind::Do);
    Node* Body = parseWhileBody();
    parseToken(TokenKind::Semicolon, "", true);
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
    if (match(TokenKind::LeftBrace)) {
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

Node* Parser::parseReturn() {
    consume(TokenKind::Return);

    Node* expr = nullptr;
    if (peek().type != TokenKind::Semicolon) {
        expr = parseExpression();
    }
    parseToken(TokenKind::Semicolon, "Expected ';' after return statement");
    return new NodeReturn(expr);
}

Node* Parser::parseNew() {
    consume(TokenKind::New);
    Node* SizeArgCArray = nullptr;
    if (peek().type == TokenKind::LeftBracket)
        SizeArgCArray = parseSizeArgCArray();
    Node* Expression = parseIdentifierExpression();
    return new NodeNew(Expression, SizeArgCArray);
}

Node* Parser::parseDelete() {
    consume(TokenKind::Delete_);
    Node* SizeArgCArray = nullptr;
    if (peek().type == TokenKind::LeftBracket)
        SizeArgCArray = parseSizeArgCArray();
    Node* Expression = parseIdentifierExpression();
    parseToken(TokenKind::Semicolon, "Expected ';' after delete statement");
    return new NodeDelete(Expression, SizeArgCArray);
}

Node* Parser::parseNullptr() {
    consume(TokenKind::NullptrLiteral);
    return new NodeNullptr();
}

Node* Parser::parseDefault() {
    consume(TokenKind::Default);
    return new NodeDefault();
}

Node* Parser::parseNodeInteger() {
    return new NodeInteger(consume(peek().type).value);
}

Node* Parser::parseNodeFloating() {
    return new NodeFloating(consume(peek().type).value);
}

Node* Parser::parseNodeBoolean() {
    return new NodeBoolean(consume(peek().type).value);
}

Node* Parser::parseNodeString() {
    return new NodeString(consume(peek().type).value);
}

Node* Parser::parseNodeCharacter() {
    return new NodeCharacter(consume(peek().type).value);
}

Node* Parser::parseNodeCall(Node* CallName) {

    parseToken(TokenKind::LeftParen, "Expected '(' after Identifier");

    std::vector<Node*> ArgumentConcreticList;

    if (peek().type != TokenKind::RightParen)
    {
        ArgumentConcreticList.push_back(parseExpression());
        while (match(TokenKind::Comma)) {
            ArgumentConcreticList.push_back(parseExpression());
        }
    }

    parseToken(TokenKind::RightParen, "Expected ')'");
    return new NodeCall(CallName, ArgumentConcreticList);
}

Node* Parser::parseSizeArgCArray() {
    parseToken(TokenKind::LeftBracket, "Expected '[' for CArray");
    Node* stmt = nullptr;
    switch (peek().type) {
    case TokenKind::IdentifierLiteral:
        stmt = parseIdentifier(); break;
    case TokenKind::IntegerLiteral:
    case TokenKind::HexLiteral:
    case TokenKind::BinaryLiteral:
        stmt = parseNodeInteger(); break;
    default:
        raise("Not correct token for array size");
    }
    parseToken(TokenKind::RightBracket, "Expected ']' for CArray");
    return stmt;
}

Node* Parser::parseType() {
    Node* Type = nullptr;
    Node* SizeArgCArray = nullptr;
    bool IsConst = false;
    NodeType::EType eType = NodeType::EType::NONE;

    bool is_auto = match(TokenKind::Auto);

    if (!is_auto)
    {
        if (match(TokenKind::Const))
            IsConst = true;

        if (peek().type != TokenKind::IdentifierLiteral)
            raise("Expected identifier token");

        Type = parseIdentifier();

        if (peek().type == TokenKind::LeftBracket)
            SizeArgCArray = parseSizeArgCArray();

        switch (peek().type)
        {
        case TokenKind::Asterisk:
            consume(TokenKind::Asterisk);
            eType = NodeType::EType::POINTER;
            break;
        case TokenKind::Ampersand:
            consume(TokenKind::Ampersand);
            eType = NodeType::EType::REF;
            break;
        case TokenKind::And:
            consume(TokenKind::And);
            eType = NodeType::EType::RVALUE;
            break;
        default:
            break;
        }
    }
    return new NodeType(Type, SizeArgCArray, IsConst, eType, is_auto);
}

Node* Parser::parsePrimary() {
    UnaryOperand UnaryOp = UnaryOperand::Unknown;
    if (tok::IsUnaryOperator(peek().type))
    {
        UnaryOp = UnOparand::getUnaryOperand(peek().type);
        consume(peek().type);
    }

    Node* Right = nullptr;

    switch (peek().type) {
    case TokenKind::New:
        Right = parseNew(); break;
    case TokenKind::Delete_:
        Right = parseDelete(); break;
    case TokenKind::NullptrLiteral:
        Right = parseNullptr(); break;
    case TokenKind::Default:
        Right = parseDefault(); break;
    case TokenKind::IdentifierLiteral:
        Right = parseIdentifierExpression(typeexpression::sc_condition);
        break;
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
    case TokenKind::LeftBrace:
        Right = parseInitializerList(); break;
    case TokenKind::LeftParen:
    {
        consume(TokenKind::LeftParen);
        Right = parseExpression();
        parseToken(TokenKind::RightParen, "Expected ')'");
        break;
    }
    default: raise("Unexpected token in primary expression");
    }
    return UnaryOp == UnaryOperand::Unknown ? Right : new NodeUnaryOp(UnaryOp, Right);
}

Node* Parser::parseTemplateParameter() {
    // template<typename C>  Ч template template parameter без имени
    if (peek().type == TokenKind::Template) {
        return parseTemplate();
    }

    // typename A  [= default]
    if (peek().type == TokenKind::Typename)
    {
        consume(peek().type);

        Node* name = nullptr;
        if (peek().type == TokenKind::IdentifierLiteral)
            name = parseIdentifier();

        Node* defaultArg = nullptr;
        if (match(TokenKind::Equal))
            defaultArg = parseType();

        return new NodeTemplateTypeParam(name, defaultArg);
    }

    // int B [= 3]  Ч non-type parameter
    Node* type = parseType();
    Node* name = parseIdentifier();

    Node* defaultArg = nullptr;
    if (match(TokenKind::Equal))
        defaultArg = parseExpression();

    return new NodeTemplateValueParam(type, name, defaultArg);
}


Node* Parser::parseTemplateDeclaration() {
    if (peek().type != TokenKind::Template)
        return nullptr;
    return parseTemplate();
}

Node* Parser::parseTemplate() {
    parseToken(TokenKind::Template, "Expected 'template'");
    Node* TemplateParameterList = parseTemplateParameterList();
    return new NodeTemplate(TemplateParameterList);
}


Node* Parser::parseTemplateParameterList() {
    parseToken(TokenKind::Less, "Expected '<' after 'template'");
    std::vector<Node*> Params;
    if (peek().type != TokenKind::Greater) {
        Params.push_back(parseTemplateParameter());
        while (match(TokenKind::Comma))
            Params.push_back(parseTemplateParameter());
    }
    parseToken(TokenKind::Greater, "Expected '>' after template parameter list");
    return new NodeTemplateParameterList(std::move(Params));
}

Node* Parser::CreateTemplateDecl(Node* TemplateParams, Node* Decl) {
    switch (Decl->DeclType) {
    case Node::EDeclType::FUNCTION:
        return new NodeFunctionTemplate(TemplateParams, Decl);
    case Node::EDeclType::CLASS:
        return new NodeClassTemplate(TemplateParams, Decl);
    case Node::EDeclType::VAR_DECLARATION_LIST:
        return new NodeVarDeclarationListTemplate(TemplateParams, Decl);
    }
    parseToken(TokenKind::Greater, "Not template used: " + std::to_string(static_cast<int>(Decl->DeclType)));
    return nullptr;
}

Node* Parser::parseTemplateParameterInstantiation() {
    return parsePrimary();
}

Node* Parser::parseTemplateParameterInstantiationList() {
    consume(TokenKind::Less);

    std::vector<Node*> TemplateParameterInstantiationList;
    if (peek().type != TokenKind::Greater)
    {
        TemplateParameterInstantiationList.push_back(parseTemplateParameterInstantiation());
        while (match(TokenKind::Comma)) {
            TemplateParameterInstantiationList.push_back(parseTemplateParameterInstantiation());
        }
    }
    parseToken(TokenKind::Greater, "Expected Greater token");
    return new NodeTemplateParameterInstantiationList(TemplateParameterInstantiationList);
}

Node* Parser::parseStatement(int type_scope) {
    size_t savedPos = savePosition();

    Node* type = nullptr;
    bool typeParsed = false;
    try {
        type = parseType();
        typeParsed = true;
    }
    catch (...) {
        restorePosition(savedPos);
        typeParsed = false;
    }

    if (typeParsed && type) {
        switch (peek().type)
        {
        case TokenKind::IdentifierLiteral:
        {
            size_t afterType = savePosition();
            Node* name = parseIdentifier();
            TokenKind next = peek().type;
            delete name;

            if (next == TokenKind::LeftParen) {
                restorePosition(savedPos);
                delete type;
                return parseFunction();
            }

            restorePosition(savedPos);
            delete type;
            return parseVar();
        }
        case TokenKind::Equal:
        {
            if (type_scope == typescope::sc_function)
            {
                restorePosition(savedPos);
                delete type;
                Node* Declaration = parseDeclaration();
                parseToken(TokenKind::Semicolon, "Expected ';' after expression");
                return Declaration;
            }
            break;
        }
        case TokenKind::LeftBrace:
        {
            restorePosition(savedPos);
            delete type;
            return parseVar();
        }
        case TokenKind::LeftParen:
        {
            if (type_scope == typescope::sc_function)
            {
                restorePosition(savedPos);
                delete type;
                Node* Expression = parseIdentifierExpression();
                parseToken(TokenKind::Semicolon, "Expected ';' after expression");
                return Expression;
            }
            break;
        }
        default:
            break;
        }

        delete type;
    }

    restorePosition(savedPos);
    if (peek().type == TokenKind::IdentifierLiteral) {

        Node* Expression = parseIdentifierExpression();
        parseToken(TokenKind::Semicolon, "Expected ';' after expression");
        return Expression;
    }

    raise("Unrecognized statement");
    return nullptr;
}

Node* Parser::parseFunction() {
    Node* returnType = parseType();

    if (peek().type != TokenKind::IdentifierLiteral) {
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

    if (peek().type != TokenKind::RightParen)
    {
        ArgumentList.push_back(parseFunctionParameter());
        while (match(TokenKind::Comma)) {
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
    if (peek().type == TokenKind::IdentifierLiteral) {
        name = parseIdentifier();
    }

    Node* defaultValue = nullptr;
    int initKind = typeinitialization::i_unknown;

    if (match(TokenKind::Equal)) {
        if (peek().type == TokenKind::LeftBrace) {
            initKind = typeinitialization::i_copy_list;
            defaultValue = parseInitializerList();
        }
        else {
            initKind = typeinitialization::i_copy;
            defaultValue = parseExpression();
        }
    }

    return new NodeDeclaration(name, defaultValue, initKind);
}

Node* Parser::parseFunctionBody() {
    Node* Body = nullptr;

    if (match(TokenKind::LeftBrace))
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

    while (!isAtEnd() && peek().type != TokenKind::RightBrace) {
        Node* stmt = nullptr;
        switch (peek().type) {
        case TokenKind::If:     stmt = ParseConditionConstruct(); break;
        case TokenKind::While:  stmt = parseWhile(); break;
        case TokenKind::Return: stmt = parseReturn(); break;
        case TokenKind::Delete_: stmt = parseDelete(); break;
        default: stmt = parseStatement(typescope::sc_function); break;
        }
        if (stmt) block->add(stmt);
    }

    return block;
}

Node* Parser::parseDeclaration() {
    Node* Identifier = nullptr;
    Node* Expression = nullptr;

    if (peek().type == TokenKind::IdentifierLiteral)
        Identifier = parseIdentifier();

    if (peek().type == TokenKind::Equal)
    {
        if (!Identifier)
            raise("Expected identifier");
        consume(TokenKind::Equal);
        Expression = parseExpression();
    }

    return new NodeDeclaration(Identifier, Expression, typeinitialization::i_copy);
}

Node* Parser::parseDeclarationPrimary() {
    Node* Identifier = nullptr;
    Node* Expression = nullptr;

    if (peek().type == TokenKind::IdentifierLiteral)
        Identifier = parseIdentifier();

    if (peek().type == TokenKind::Equal)
    {
        if (!Identifier)
            raise("Expected identifier");
        consume(TokenKind::Equal);
        Expression = parsePrimary();
    }

    return new NodeDeclaration(Identifier, Expression, typeinitialization::i_copy);
}

Node* Parser::parseVar() {
    Node* VarTemplateParameterDeclarationList = nullptr;
    Node* VarType = parseVarType();
    Node* VarDeclarationList = parseVarDeclarationList();
    parseToken(TokenKind::Semicolon, "Expected ';' after declaration");
    return new NodeVarDeclarationList(VarTemplateParameterDeclarationList, VarType, VarDeclarationList);
}

Node* Parser::parseVarType() {
    return parseType();
}

Node* Parser::parseVarDeclaration() {
    if (peek().type != TokenKind::IdentifierLiteral)
        raise("Expected identifier in declaration");

    Node* name = parseIdentifier();

    int initKind = typeinitialization::i_unknown;
    Node* init = nullptr;

    if (match(TokenKind::Equal)) {
        if (peek().type == TokenKind::LeftBrace) {
            initKind = typeinitialization::i_copy_list;
            init = parseInitializerList();
        }
        else {
            initKind = typeinitialization::i_copy;
            init = parseExpression();
        }
    }
    else if (match(TokenKind::LeftBrace)) {
        if (peek().type == TokenKind::RightBrace) {
            initKind = typeinitialization::i_value;
        }
        else {
            initKind = typeinitialization::i_direct_list;
        }
        init = parseInitializerListBody();
        parseToken(TokenKind::RightBrace, "Expected '}' after initializer list");
    }
    else {
        initKind = typeinitialization::i_default;
    }

    return new NodeDeclaration(name, init, initKind);
}

Node* Parser::parseVarDeclarationList() {
    std::vector<Node*> ContainerDeclarationList;
    ContainerDeclarationList.push_back(parseVarDeclaration());
    while (match(TokenKind::Comma)) {
        ContainerDeclarationList.push_back(parseVarDeclaration());
    }
    return new NodeDeclarationList(ContainerDeclarationList);
}

Node* Parser::parseClass() {
    consume(TokenKind::Class);

    Node* ClassTemplateParameterDeclarationList = nullptr;
    Node* ClassName = parseClassName();
    Node* ClassBaseClass = parseClassBaseClass();
    Node* ClassBody = parseClassBody();

    return new NodeClass(ClassName, ClassTemplateParameterDeclarationList, ClassBaseClass, ClassBody);
}

Node* Parser::parseClassName() {
    if (peek().type != TokenKind::IdentifierLiteral)
        raise("Expected class name");
    return parseIdentifier();
}

Node* Parser::parseClassBody() {
    Node* Body = nullptr;

    if (peek().type == TokenKind::LeftBrace)
    {
        consume(TokenKind::LeftBrace);
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

    if (match(TokenKind::Colon))
    {
        using ClassInheritanceType = NodeBaseClass::InheritanceType;
        ClassInheritanceType Type = ClassInheritanceType::NONE;

        switch (peek().type)
        {
        case TokenKind::Public:
            consume(TokenKind::Public);
            Type = ClassInheritanceType::PUBLIC;
            break;
        case TokenKind::Private:
            consume(TokenKind::Private);
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

    while (!isAtEnd() && peek().type != TokenKind::RightBrace) {
        Node* stmt = nullptr;
        switch (peek().type) {
        case TokenKind::Private:
        case TokenKind::Public:
        case TokenKind::Static:
        {
            TokenKind Scope = peek().type;
            if (!Statements.empty() || Type != ClassFieldType::NONE) {
                FieldStatements.push_back({ Type, Statements });
                Statements.clear();
            }
            Type = getClassFieldType(Scope);
            consume(Scope);
            consume(TokenKind::Colon);
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