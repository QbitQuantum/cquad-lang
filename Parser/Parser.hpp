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

    size_t Size() const noexcept { return Buffer.size(); }

    void skipTrivia() const noexcept {
        while (!eof() && isTriviaToken()) ++Pos;
    }

    const Token& peek(size_t offset = 0) const noexcept {
        size_t idx = Pos + offset;
        if (idx >= Buffer.size()) return eofToken;
        return Buffer[idx];
    }

    bool eof() const noexcept { return Pos >= Buffer.size(); }

    bool match(TokenKind id) const noexcept {
        if (peek().type == id) { ++Pos; skipTrivia(); return true; }
        return false;
    }

    const Token& consume(TokenKind id) const noexcept {
        const Token& tok = peek();
        if (tok.type == id) { ++Pos; skipTrivia(); return tok; }
        return tok;
    }

    size_t savePosition() const noexcept { return Pos; }
    void restorePosition(size_t pos) const noexcept { Pos = pos; }
};

class Parser
{
private:
    TokenStream stream;
    std::vector<Node*> ast;

    size_t streamSize() const noexcept { return stream.Size(); }
    bool   atEnd()      const noexcept { return stream.eof(); }
    TokenKind token()   const noexcept { return stream.peek().type; }
    const Token& peek(size_t offset = 0) const noexcept { return stream.peek(offset); }
    bool match(TokenKind id) const noexcept { return stream.match(id); }
    const Token& consume(TokenKind id) const noexcept { return stream.consume(id); }
    void advance() const noexcept { stream.consume(stream.peek().type); }
    void skipTrivia() const noexcept { stream.skipTrivia(); }
    size_t savePosition() const noexcept { return stream.savePosition(); }
    void restorePosition(size_t pos) const noexcept { stream.restorePosition(pos); skipTrivia(); }

    bool is(TokenKind kind) const noexcept { return token() == kind; }
    bool isNot(TokenKind kind) const noexcept { return !is(kind); }

    void raise(const std::string& msg) {
        auto t = peek();
        throw ParseError::ParseError(t.line, t.column, msg);
    }

    void expect(TokenKind kind, const std::string& msg, bool soft = false) {
        if (!match(kind)) if (!soft) raise(msg);
    }

    Node* parseTopLevel();
    Node* parseStatement(int scope);
    Node* parseDeclaration();
    Node* parseDeclarationPrimary();

    Node* parseIdentifier(int exprKind = typeexpression::Unknown);
    Node* parseIdentifierExpr(int exprKind = typeexpression::Unknown);
    Node* parseIdentifierComponent(int exprKind = typeexpression::Unknown);
    Node* parsePrimary();
    Node* parseExpression(int minPrec = 0, int exprKind = typeexpression::Expression);
    Node* parseCall(Node* callee);
    Node* parseInitializerList();
    Node* parseInitializerListBody();

    Node* parseInteger();
    Node* parseFloating();
    Node* parseBoolean();
    Node* parseString();
    Node* parseCharacter();
    Node* parseNullptr();
    Node* parseDefault();
    Node* parseNew();
    Node* parseDelete();

    Node* parseType();
    Node* parseArraySize();

    Node* parseFunction();
    Node* parseFunctionParams();
    Node* parseFunctionParam();
    Node* parseFunctionBody();
    Node* parseFunctionBlock();

    Node* parseSwitch();
    Node* parseSwitchCond();
    Node* parseSwitchBody();
    Node* parseCase();
    Node* parseCaseValue();
    Node* parseCaseBody();
    Node* parseDefaultCase();
    Node* parseBreak();

    Node* parseVar();
    Node* parseVarType();
    Node* parseVarDecl(bool IsPrimary = false);
    Node* parseVarDeclList();

    Node* parseClass();
    Node* parseClassName();
    Node* parseClassBody();
    Node* parseClassBase();
    Node* parseClassBlock();

    Node* parseCondition();
    Node* parseIf();
    Node* parseIfCond();
    Node* parseIfBody();
    Node* parseElse();
    Node* parseElseBody();
    Node* parseWhile();
    Node* parseWhileCond();
    Node* parseWhileBody();
    Node* parseReturn();

    Node* parseTemplateDecl();
    Node* parseTemplate();
    Node* parseTemplateParam();
    Node* parseTemplateParamList();
    Node* parseTemplateArg();
    Node* parseTemplateArgList();
    Node* wrapTemplate(Node* params, Node* decl);

    Node* parseFor();
    Node* parseForHeader();
    Node* parseForBody();

    Node* parseForDecl();
    Node* parseForType();
    Node* parseForClassicDecl();
    Node* parseForRangeDecl();

    void rejectTopLevelComma(TokenKind terminator);

public:
    Parser(const PostLexer& lexer) : Parser(lexer.GetBufferPostLexerToken()) {}
    Parser(const std::vector<Token>& buffer) : stream(buffer) {}

    ~Parser() {
        for (auto& n : ast) delete n;
    }

    void Parse() {
        if (streamSize() == 0) return;
        while (!atEnd()) {
            if (Node* node = parseTopLevel()) ast.push_back(node);
            else advance();
        }
    }

    const std::vector<Node*>& GetAst() const { return ast; }
};

Node* Parser::parseTopLevel()
{
    Node* tmpl = parseTemplateDecl();
    Node* stmt = nullptr;
    switch (token()) {
    case TokenKind::Class: stmt = parseClass(); break;
    default:               stmt = parseStatement(typescope::Global); break;
    }
    if (tmpl) stmt = wrapTemplate(tmpl, stmt);
    return stmt;
}

Node* Parser::parseIdentifier(int exprKind) {
    if (isNot(TokenKind::IdentifierLiteral))
        raise("Expected identifier");

    Node* first = parseIdentifierComponent(exprKind);

    if (isNot(TokenKind::ScResOp))
        return first;

    std::vector<Node*> parts;
    parts.push_back(first);

    while (match(TokenKind::ScResOp)) {
        match(TokenKind::Template); // пропускаем дизамбигуатор временно
        parts.push_back(parseIdentifierComponent(exprKind));
    }

    return new NodeScope(std::move(parts));
}

Node* Parser::parseIdentifierComponent(int exprKind) {
    if (isNot(TokenKind::IdentifierLiteral))
        raise("Expected identifier");

    std::string name = consume(TokenKind::IdentifierLiteral).value;

    Node* tmplArgs = nullptr;
    if (is(TokenKind::Less) && exprKind != typeexpression::Condition)
        tmplArgs = parseTemplateArgList();

    return new NodeIdentifier(tmplArgs, std::move(name));
}

Node* Parser::parseIdentifierExpr(int exprKind) {
    Node* expr = parseIdentifier(exprKind);
    if (is(TokenKind::LeftParen))
        expr = parseCall(expr);
    if (is(TokenKind::Dot) || is(TokenKind::Arrow)) {
        bool arrow = is(TokenKind::Arrow); advance();
        expr = new NodeMemberCall(expr, parseIdentifierExpr(exprKind), arrow);
    }
    return expr;
}

Node* Parser::parseExpression(int minPrec, int exprKind) {
    Node* left = parsePrimary();
    while (true) {
        TokenKind op = token();
        bool ok = (exprKind == typeexpression::Expression)
            ? tok::IsBinaryOperator(op)
            : tok::IsConditionalOperator(op);
        if (!ok) break;
        int prec = tok::GetBinaryOperatorPriority(op);
        if (prec < minPrec) break;
        consume(op);
        Node* right = parseExpression(prec + 1, exprKind);
        left = new NodeBinaryOp(BinOparand::getBinaryOperand(op), left, right);
    }
    return left;
}

Node* Parser::parseInitializerList() {
    expect(TokenKind::LeftBrace, "Expected '{' for initializer list");
    Node* body = parseInitializerListBody();
    expect(TokenKind::RightBrace, "Expected '}' after initializer list");
    return body;
}

Node* Parser::parseInitializerListBody() {
    std::vector<Node*> elems;
    if (isNot(TokenKind::RightBrace)) {
        elems.push_back(parseExpression());
        while (match(TokenKind::Comma)) {
            elems.push_back(parseExpression());
        }
    }
    return new NodeInitializerList(elems);
}

Node* Parser::parseCondition() {
    Node* ifNode = parseIf();
    Node* elseNode = nullptr;
    if (is(TokenKind::Else)) elseNode = parseElse();
    return new NodeCondition(ifNode, elseNode);
}

Node* Parser::parseIf() {
    consume(TokenKind::If);
    Node* cond = parseIfCond();
    Node* body = parseIfBody();
    return new NodeIf(cond, body);
}

Node* Parser::parseIfCond() {
    expect(TokenKind::LeftParen, "Expected '(' after 'if'");
    Node* cond = parseExpression(0, typeexpression::Condition);
    expect(TokenKind::RightParen, "Expected ')' after if-condition");
    return cond;
}

Node* Parser::parseIfBody() {
    std::vector<Node*> elem;
    if (match(TokenKind::LeftBrace)) {
        elem.push_back(parseFunctionBlock());
        expect(TokenKind::RightBrace, "Expected '}' after if-body");
    }
    else
    {
        Node* stmt = nullptr;
        switch (token()) {
        case TokenKind::If:     stmt = parseCondition(); break;
        case TokenKind::Switch: stmt = parseSwitch();    break;
        default:                stmt = parseStatement(typescope::Function); break;
        }
        if (stmt) elem.push_back(stmt);
    }
    return new NodeBlock(elem);
}

Node* Parser::parseElse() {
    consume(TokenKind::Else);
    Node* body = parseElseBody();
    return new NodeElse(body);
}

Node* Parser::parseElseBody() {
    return parseIfBody();
}

Node* Parser::parseWhile() {
    consume(TokenKind::While);
    Node* cond = parseWhileCond();
    bool hasDo = match(TokenKind::Do);
    Node* body = parseWhileBody();
    expect(TokenKind::Semicolon, "", true);
    return new NodeWhile(cond, body, hasDo);
}

Node* Parser::parseWhileCond() {
    expect(TokenKind::LeftParen, "Expected '(' after 'while'");
    Node* cond = parseExpression(0, typeexpression::Condition);
    expect(TokenKind::RightParen, "Expected ')' after while-condition");
    return cond;
}

Node* Parser::parseWhileBody() {
    if (match(TokenKind::LeftBrace)) {
        Node* Block = parseFunctionBlock();
        expect(TokenKind::RightBrace, "Expected '}' after while-body");
        return Block;
    }
    return parseStatement(typescope::Function);
}

Node* Parser::parseReturn() {
    consume(TokenKind::Return);
    Node* expr = nullptr;
    if (isNot(TokenKind::Semicolon)) expr = parseExpression();
    expect(TokenKind::Semicolon, "Expected ';' after return statement");
    return new NodeReturn(expr);
}

Node* Parser::parseNew() {
    consume(TokenKind::New);
    Node* size = nullptr;
    if (is(TokenKind::LeftBracket)) size = parseArraySize();
    Node* expr = parseIdentifierExpr();
    return new NodeNew(expr, size);
}

Node* Parser::parseDelete() {
    consume(TokenKind::Delete_);
    Node* size = nullptr;
    if (is(TokenKind::LeftBracket)) size = parseArraySize();
    Node* expr = parseIdentifierExpr();
    expect(TokenKind::Semicolon, "Expected ';' after delete statement");
    return new NodeDelete(expr, size);
}

Node* Parser::parseNullptr() {
    consume(TokenKind::NullptrLiteral);
    return new NodeNullptr();
}

Node* Parser::parseDefault() {
    consume(TokenKind::Default);
    return new NodeDefault();
}

Node* Parser::parseInteger() { return new NodeInteger(consume(token()).value); }
Node* Parser::parseFloating() { return new NodeFloating(consume(token()).value); }
Node* Parser::parseBoolean() { return new NodeBoolean(consume(token()).value); }
Node* Parser::parseString() { return new NodeString(consume(token()).value); }
Node* Parser::parseCharacter() { return new NodeCharacter(consume(token()).value); }

Node* Parser::parseCall(Node* callee) {
    expect(TokenKind::LeftParen, "Expected '(' after Identifier");

    std::vector<Node*> args;
    if (isNot(TokenKind::RightParen)) {
        args.push_back(parseExpression());
        while (match(TokenKind::Comma))
            args.push_back(parseExpression());
    }
    expect(TokenKind::RightParen, "Expected ')'");
    return new NodeCall(callee, args);
}

Node* Parser::parseArraySize() {
    expect(TokenKind::LeftBracket, "Expected '[' for CArray");
    Node* stmt = nullptr;
    switch (token()) {
    case TokenKind::IdentifierLiteral: stmt = parseIdentifier(); break;
    case TokenKind::IntegerLiteral:
    case TokenKind::HexLiteral:
    case TokenKind::BinaryLiteral:     stmt = parseInteger();    break;
    default: raise("Not correct token for array size");
    }
    expect(TokenKind::RightBracket, "Expected ']' for CArray");
    return stmt;
}

Node* Parser::parseType() {
    Node* type = nullptr;
    Node* size = nullptr;
    bool isConst = false;
    NodeType::EType eType = NodeType::EType::None;

    bool isAuto = match(TokenKind::Auto);

    if (!isAuto) {
        if (match(TokenKind::Const)) isConst = true;

        if (isNot(TokenKind::IdentifierLiteral))
            raise("Expected identifier token");

        type = parseIdentifier();

        if (is(TokenKind::LeftBracket)) size = parseArraySize();

        switch (token()) {
        case TokenKind::Asterisk:  consume(TokenKind::Asterisk);  eType = NodeType::EType::Pointer; break;
        case TokenKind::Ampersand: consume(TokenKind::Ampersand); eType = NodeType::EType::Ref;     break;
        case TokenKind::And:       consume(TokenKind::And);       eType = NodeType::EType::RValue;  break;
        default: break;
        }
    }
    return new NodeType(type, size, isConst, eType, isAuto);
}

Node* Parser::parsePrimary() {
    UnaryOperand unary = UnaryOperand::Unknown;
    if (tok::IsUnaryOperator(token())) {
        unary = UnOparand::getUnaryOperand(token());
        consume(token());
    }

    Node* right = nullptr;
    switch (token()) {
    case TokenKind::New:            right = parseNew();        break;
    case TokenKind::Delete_:        right = parseDelete();     break;
    case TokenKind::NullptrLiteral: right = parseNullptr();    break;
    case TokenKind::Default:        right = parseDefault();    break;
    case TokenKind::IdentifierLiteral:
        right = parseIdentifierExpr(typeexpression::Condition); break;
    case TokenKind::IntegerLiteral:
    case TokenKind::HexLiteral:
    case TokenKind::BinaryLiteral:  right = parseInteger();    break;
    case TokenKind::FloatLiteral:
    case TokenKind::DoubleLiteral:
    case TokenKind::LongDoubleLiteral: right = parseFloating(); break;
    case TokenKind::TrueLiteral:
    case TokenKind::FalseLiteral:   right = parseBoolean();    break;
    case TokenKind::StringLiteral:
    case TokenKind::WStringLiteral: right = parseString();     break;
    case TokenKind::CharLiteral:
    case TokenKind::WCharLiteral:   right = parseCharacter();  break;
    case TokenKind::LeftBrace:      right = parseInitializerList(); break;
    case TokenKind::LeftParen:
        consume(TokenKind::LeftParen);
        right = parseExpression();
        expect(TokenKind::RightParen, "Expected ')'");
        break;
    default: raise("Unexpected token in primary expression");
    }
    return unary == UnaryOperand::Unknown ? right : new NodeUnaryOp(unary, right);
}

Node* Parser::parseTemplateParam() {
    // template<typename C>
    if (is(TokenKind::Template)) return parseTemplate();

    // typename A [= default]
    if (is(TokenKind::Typename)) {
        consume(token());
        Node* name = is(TokenKind::IdentifierLiteral) ? parseIdentifier() : nullptr;
        Node* def = match(TokenKind::Equal) ? parseType() : nullptr;
        return new NodeTemplateTypeParam(name, def);
    }

    // int B [= 3]
    Node* type = parseType();
    Node* name = parseIdentifier();
    Node* def = match(TokenKind::Equal) ? parseExpression() : nullptr;
    return new NodeTemplateValueParam(type, name, def);
}

Node* Parser::parseTemplateDecl() {
    if (isNot(TokenKind::Template)) return nullptr;
    return parseTemplate();
}

Node* Parser::parseTemplate() {
    expect(TokenKind::Template, "Expected 'template'");
    Node* params = parseTemplateParamList();
    return new NodeTemplate(params);
}

Node* Parser::parseTemplateParamList() {
    expect(TokenKind::Less, "Expected '<' after 'template'");
    std::vector<Node*> params;
    if (isNot(TokenKind::Greater)) {
        params.push_back(parseTemplateParam());
        while (match(TokenKind::Comma))
            params.push_back(parseTemplateParam());
    }
    expect(TokenKind::Greater, "Expected '>' after template parameter list");
    return new NodeTemplateParameterList(std::move(params));
}

Node* Parser::wrapTemplate(Node* params, Node* decl) {
    switch (decl->DeclType) {
    case Node::EDeclType::Function:             return new NodeFunctionTemplate(params, decl);
    case Node::EDeclType::Class:                return new NodeClassTemplate(params, decl);
    case Node::EDeclType::VarDeclarationList:   return new NodeVarDeclarationListTemplate(params, decl);
    }
    raise("Not template used: " + std::to_string(static_cast<int>(decl->DeclType)));
    return nullptr;
}

Node* Parser::parseTemplateArg() {
    return parsePrimary();
}

Node* Parser::parseTemplateArgList() {
    consume(TokenKind::Less);
    std::vector<Node*> args;
    if (isNot(TokenKind::Greater)) {
        args.push_back(parseTemplateArg());
        while (match(TokenKind::Comma))
            args.push_back(parseTemplateArg());
    }
    expect(TokenKind::Greater, "Expected Greater token");
    return new NodeTemplateParameterInstantiationList(args);
}

Node* Parser::parseStatement(int scope) {
    size_t saved = savePosition();

    Node* type = nullptr;
    bool typeOk = false;
    try { type = parseType(); typeOk = true; }
    catch (...) { restorePosition(saved); typeOk = false; }

    if (typeOk && type) {
        switch (token()) {
        case TokenKind::IdentifierLiteral: {
            Node* name = parseIdentifier();
            TokenKind next = token();
            delete name;

            if (next == TokenKind::LeftParen) {
                restorePosition(saved);
                delete type;
                return parseFunction();
            }
            restorePosition(saved);
            delete type;
            return parseVar();
        }
        case TokenKind::Equal:
            if (scope == typescope::Function) {
                restorePosition(saved);
                delete type;
                Node* decl = parseDeclaration();
                expect(TokenKind::Semicolon, "Expected ';' after expression");
                return decl;
            }
            break;
        case TokenKind::LeftBrace:
            restorePosition(saved);
            delete type;
            return parseVar();
        case TokenKind::LeftParen:
            if (scope == typescope::Function) {
                restorePosition(saved);
                delete type;
                Node* expr = parseIdentifierExpr();
                expect(TokenKind::Semicolon, "Expected ';' after expression");
                return expr;
            }
            break;
        case TokenKind::Caret:
        case TokenKind::Tilde:
            if (scope == typescope::Class) {
                restorePosition(saved);
                delete type;
                return parseFunction();
            }
            break;
        default: break;
        }
        delete type;
    }

    restorePosition(saved);
    if (is(TokenKind::IdentifierLiteral)) {
        Node* expr = parseIdentifierExpr();
        expect(TokenKind::Semicolon, "Expected ';' after expression");
        return expr;
    }

    raise("Unrecognized statement");
    return nullptr;
}

Node* Parser::parseFunction() {
    Node* returnType = parseType();

    int typef = typefunction::Function;
    switch (token())
    {
    case TokenKind::Caret:
    case TokenKind::Tilde:
        typef = token() == TokenKind::Caret ? typefunction::Constructor : typefunction::Destructor;
        advance();
        break;
    default: break;
    }

    Node* name = parseIdentifier();
    Node* params = parseFunctionParams();
    Node* body = parseFunctionBody();
    
    Node* Stmt = nullptr;
    switch (typef)
    {
    case typefunction::Constructor:
        Stmt = new NodeConstructor(returnType, name, params, body); break;
    case typefunction::Destructor:
        Stmt = new NodeDestructor(returnType, name, params, body); break;
    default:
        Stmt = new NodeFunction(returnType, name, params, body); break;
    }
    return Stmt;
}

Node* Parser::parseFunctionParams() {
    expect(TokenKind::LeftParen, "Expected '('");
    std::vector<Node*> args;
    if (isNot(TokenKind::RightParen)) {
        args.push_back(parseFunctionParam());
        while (match(TokenKind::Comma))
            args.push_back(parseFunctionParam());
    }
    expect(TokenKind::RightParen, "Expected ')'");
    return new NodeParameterList(args);
}

Node* Parser::parseFunctionParam() {
    Node* type = parseType();
    // Parse parameter name (optional)
    Node* defaultValue = parseVarDecl(true);
    return new NodeVarDeclarationList(type, defaultValue);
}

Node* Parser::parseFunctionBody() {
    if (match(TokenKind::LeftBrace)) {
        Node* body = parseFunctionBlock();
        expect(TokenKind::RightBrace, "Expected '}'");
        return body;
    }
    expect(TokenKind::Semicolon, "Expected ';'");
    return nullptr;
}

Node* Parser::parseFunctionBlock() {
    std::vector<Node*> elem;
    while (!atEnd() && isNot(TokenKind::RightBrace)) {
        Node* stmt = nullptr;
        switch (token()) {
        case TokenKind::If:      stmt = parseCondition(); break;
        case TokenKind::While:   stmt = parseWhile();     break;
        case TokenKind::Return:  stmt = parseReturn();    break;
        case TokenKind::Delete_: stmt = parseDelete();    break;
        case TokenKind::Break:   stmt = parseBreak();     break;
        case TokenKind::Switch:  stmt = parseSwitch();    break;
        case TokenKind::For:     stmt = parseFor();    break;
        default:                 stmt = parseStatement(typescope::Function); break;
        }
        if (stmt) elem.push_back(stmt);
    }
    return new NodeBlock(elem);
}

Node* Parser::parseSwitch() {
    consume(TokenKind::Switch);
    Node* cond = parseSwitchCond();
    Node* body = parseSwitchBody();
    return new NodeSwitch(cond, body);
}

Node* Parser::parseSwitchCond() {
    expect(TokenKind::LeftParen, "Expected '(' after 'switch'");
    Node* cond = parseExpression(0, typeexpression::Condition);
    expect(TokenKind::RightParen, "Expected ')' after switch-condition");
    return cond;
}

Node* Parser::parseSwitchBody() {
    expect(TokenKind::LeftBrace, "Expected '{' after switch-condition");
    std::vector<Node*> elem;
    while (!atEnd() && isNot(TokenKind::RightBrace)) {
        Node* stmt = nullptr;
        switch (token()) {
        case TokenKind::Case:      stmt = parseCase(); break;
        case TokenKind::Default:   stmt = parseDefaultCase();     break;
        default:                   raise("Not correct token parse in switch section");
        }
        if (stmt) elem.push_back(stmt);
    }
    expect(TokenKind::RightBrace, "Expected '}' after switch body");
    return new NodeBlock(elem);
}

Node* Parser::parseCase() {
    consume(TokenKind::Case);
    Node* value = parseCaseValue();
    expect(TokenKind::Colon, "Expected ':' after case value");
    Node* body = parseCaseBody();
    return new NodeCase(value, body);
}

Node* Parser::parseCaseValue() {
    // case-значение — это константное выражение (обычно целое/символьное/строковое)
    return parseExpression(0, typeexpression::Condition);
}

Node* Parser::parseCaseBody() {
    std::vector<Node*> elem;
    if (match(TokenKind::LeftBrace))
    {
        std::vector<Node*> elem_block;
        while (!atEnd() && isNot(TokenKind::RightBrace))
        {
            Node* stmt = nullptr;
            switch (token()) {
            case TokenKind::If:      stmt = parseCondition(); break;
            case TokenKind::While:   stmt = parseWhile();     break;
            case TokenKind::Switch:  stmt = parseSwitch();    break;
            case TokenKind::Return:  stmt = parseReturn();    break;
            case TokenKind::Delete_: stmt = parseDelete();    break;
            default:                 stmt = parseStatement(typescope::Function); break;
            }
            if (stmt) elem_block.push_back(stmt);
        }
        elem.push_back(new NodeBlock(std::move(elem_block)));
        expect(TokenKind::RightBrace, "Expected '}' after case value");
    }
    else elem.push_back(parseStatement(typescope::Function));
    
    if (token() == TokenKind::Break)
        elem.push_back(parseBreak());

    return new NodeCaseBody(std::move(elem));
}

Node* Parser::parseDefaultCase() {
    consume(TokenKind::Default);
    expect(TokenKind::Colon, "Expected ':' after 'default'");
    Node* body = parseCaseBody();
    return new NodeCaseDefault(body);
}

Node* Parser::parseBreak() {
    consume(TokenKind::Break);
    expect(TokenKind::Semicolon, "Expected ';' after 'break'");
    return new NodeBreak();
}

Node* Parser::parseDeclaration() {
    Node* name = is(TokenKind::IdentifierLiteral) ? parseIdentifier() : nullptr;
    Node* expr = nullptr;
    if (is(TokenKind::Equal)) {
        if (!name) raise("Expected identifier");
        consume(TokenKind::Equal);
        expr = parseExpression();
    }
    return new NodeDeclaration(name, expr, typeinitialization::Copy);
}

Node* Parser::parseDeclarationPrimary() {
    Node* name = is(TokenKind::IdentifierLiteral) ? parseIdentifier() : nullptr;
    Node* expr = nullptr;
    if (is(TokenKind::Equal)) {
        if (!name) raise("Expected identifier");
        consume(TokenKind::Equal);
        expr = parsePrimary();
    }
    return new NodeDeclaration(name, expr, typeinitialization::Copy);
}

Node* Parser::parseVar() {
    Node* type = parseVarType();
    Node* list = parseVarDeclList();
    expect(TokenKind::Semicolon, "Expected ';' after declaration");
    return new NodeVarDeclarationList(type, list);
}

Node* Parser::parseVarType() {
    return parseType();
}

Node* Parser::parseVarDecl(bool IsPrimary) {
    if (isNot(TokenKind::IdentifierLiteral))
        raise("Expected identifier in declaration");

    Node* name = parseIdentifier();
    int initKind = typeinitialization::Unknown;
    Node* init = nullptr;

    if (IsPrimary)
    {
        if (match(TokenKind::Equal)) {
            initKind = typeinitialization::Copy;
            init = parseExpression();
        }
        else {
            initKind = typeinitialization::Default;
        }
    }
    else
    {
        if (match(TokenKind::Equal)) {
            if (is(TokenKind::LeftBrace)) {
                initKind = typeinitialization::CopyList;
                init = parseInitializerList();
            }
            else {
                initKind = typeinitialization::Copy;
                init = parseExpression();
            }
        }
        else if (match(TokenKind::LeftBrace)) {
            initKind = is(TokenKind::RightBrace)
                ? typeinitialization::Value
                : typeinitialization::DirectList;
            init = parseInitializerListBody();
            expect(TokenKind::RightBrace, "Expected '}' after initializer list");
        }
        else {
            initKind = typeinitialization::Default;
        }
    }
    return new NodeDeclaration(name, init, initKind);
}

Node* Parser::parseVarDeclList() {
    std::vector<Node*> list;
    list.push_back(parseVarDecl());
    while (match(TokenKind::Comma))
        list.push_back(parseVarDecl());
    return new NodeDeclarationList(list);
}

// ---------- classes ----------

Node* Parser::parseClass() {
    consume(TokenKind::Class);
    Node* name = parseClassName();
    Node* base = parseClassBase();
    Node* body = parseClassBody();
    return new NodeClass(name, base, body);
}

Node* Parser::parseClassName() {
    if (isNot(TokenKind::IdentifierLiteral))
        raise("Expected class name");
    return parseIdentifier();
}

Node* Parser::parseClassBody() {
    if (is(TokenKind::LeftBrace)) {
        consume(TokenKind::LeftBrace);
        Node* body = parseClassBlock();
        expect(TokenKind::RightBrace, "Expected '}' after class declaration");
        return body;
    }
    expect(TokenKind::Semicolon, "Expected ';' after class forward declaration");
    return nullptr;
}

Node* Parser::parseClassBase() {
    if (!match(TokenKind::Colon)) return nullptr;

    using Inherit = NodeBaseClass::InheritanceType;
    Inherit type = Inherit::None;

    switch (token()) {
    case TokenKind::Public:  consume(TokenKind::Public);  type = Inherit::Public;  break;
    case TokenKind::Private: consume(TokenKind::Private); type = Inherit::Private; break;
    default: break;
    }
    Node* name = parseClassName();
    return new NodeBaseClass(name, type);
}

Node* Parser::parseClassBlock() {
    using Field = NodeBlockClass::FieldType;
    std::vector<Node*> stmts;
    std::vector<std::pair<Field, std::vector<Node*>>> fields;
    Field current = Field::None;

    auto toField = [](TokenKind k) -> Field {
        switch (k) {
        case TokenKind::Private: return Field::Private;
        case TokenKind::Public:  return Field::Public;
        case TokenKind::Static:  return Field::Static;
        default:                 return Field::None;
        }
        };

    while (!atEnd() && isNot(TokenKind::RightBrace)) {
        Node* stmt = nullptr;
        switch (token()) {
        case TokenKind::Private:
        case TokenKind::Public:
        case TokenKind::Static: {
            TokenKind scope = token();
            if (!stmts.empty() || current != Field::None) {
                fields.push_back({ current, stmts });
                stmts.clear();
            }
            current = toField(scope);
            consume(scope);
            consume(TokenKind::Colon);
            break;
        }
        case TokenKind::Class: stmt = parseClass(); break;
        default:               stmt = parseStatement(typescope::Class); break;
        }
        if (stmt) stmts.push_back(stmt);
    }

    if (!stmts.empty() || current != Field::None)
        fields.push_back({ current, stmts });

    return new NodeBlockClass(fields);
}

void Parser::rejectTopLevelComma(TokenKind terminator)
{
    int parenDepth = 0;
    int bracketDepth = 0;
    int braceDepth = 0;

    for (size_t i = 0;; ++i) {
        TokenKind k = peek(i).type;

        if (k == TokenKind::neof)
            return;

        switch (k) {
        case TokenKind::LeftParen:
            ++parenDepth;
            break;

        case TokenKind::RightParen:
            if (parenDepth == 0 &&
                bracketDepth == 0 &&
                braceDepth == 0 &&
                terminator == TokenKind::RightParen)
                return;

            if (parenDepth > 0)
                --parenDepth;
            break;

        case TokenKind::LeftBracket:
            ++bracketDepth;
            break;

        case TokenKind::RightBracket:
            if (bracketDepth > 0)
                --bracketDepth;
            break;

        case TokenKind::LeftBrace:
            ++braceDepth;
            break;

        case TokenKind::RightBrace:
            if (braceDepth > 0)
                --braceDepth;
            break;

        case TokenKind::Comma:
            if (parenDepth == 0 && bracketDepth == 0 && braceDepth == 0)
                raise("Comma is not allowed in for-header expression");
            break;

        default:
            break;
        }

        if (k == terminator &&
            parenDepth == 0 &&
            bracketDepth == 0 &&
            braceDepth == 0)
            return;
    }
}

Node* Parser::parseForType()
{
    bool isConst = match(TokenKind::Const);
    bool isAuto = match(TokenKind::Auto);

    Node* typeName = nullptr;

    if (!isAuto) {
        if (isNot(TokenKind::IdentifierLiteral))
            raise("Expected type in for declaration");

        typeName = parseIdentifier();
    }

    NodeType::EType refKind = NodeType::EType::None;
    if (match(TokenKind::Ampersand)) {
        refKind = NodeType::EType::Ref;
    }
    else if (match(TokenKind::And)) {
        refKind = NodeType::EType::RValue;
    }
    else if (match(TokenKind::Asterisk)) {
        refKind = NodeType::EType::Pointer;
    }
    return new NodeType(typeName, nullptr, isConst, refKind, isAuto);
}


Node* Parser::parseForDecl()
{
    Node* type = parseForType();

    // Structured binding:
    if (match(TokenKind::LeftBracket)) {
        std::vector<Node*> names;

        if (isNot(TokenKind::IdentifierLiteral))
            raise("Expected identifier in structured binding");

        names.push_back(parseIdentifier());
        while (match(TokenKind::Comma))
            names.push_back(parseIdentifier());

        expect(TokenKind::RightBracket, "Expected ']' after structured binding identifiers");

        // По условию structured binding в init запрещён.
        if (is(TokenKind::Equal))
            raise("Structured binding with initializer is not allowed in for");

        return new NodeStructuredBinding(type, std::move(names));
    }

    // Обычная декларация: auto i [= expression]
    if (isNot(TokenKind::IdentifierLiteral)) {
        delete type;
        raise("Expected variable name in for declaration");
    }

    Node* name = parseIdentifier();
    Node* init = nullptr;
    int initKind = typeinitialization::Default;

    if (match(TokenKind::Equal)) {
        // Лямбда в init всё равно не будет разобрана parseExpression(),
        // поскольку выражение начинается с '['.
        rejectTopLevelComma(TokenKind::Semicolon);

        initKind = typeinitialization::Copy;
        init = parseExpression();
    }

    // После одного declarator запятая запрещена.
    if (is(TokenKind::Comma)) {
        delete type;
        delete name;
        delete init;
        raise("Only one variable declaration is allowed in for");
    }

    Node* declaration = new NodeDeclaration(name, init, initKind);
    return new NodeVarDeclarationList(type, declaration);
}

Node* Parser::parseForBody()
{
    if (match(TokenKind::LeftBrace)) {
        Node* block = parseFunctionBlock();
        expect(TokenKind::RightBrace,"Expected '}' after for-body");
        return block;
    }
    return parseStatement(typescope::Function);
}

Node* Parser::parseFor()
{
    consume(TokenKind::For);

    expect(TokenKind::LeftParen, "Expected '(' after 'for'");

    // ------------------------------------------------------------
    // Бесконечный цикл:
    //
    // for (;;) { }
    // ------------------------------------------------------------
    if (match(TokenKind::Semicolon)) {
        expect(TokenKind::Semicolon, "Only 'for (;;)' is allowed with empty initialization");
        expect(TokenKind::RightParen, "Expected ')' after for-header");

        Node* body = parseForBody();

        return new NodeFor(
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            body,
            false
        );
    }

    // В этой реализации init обязан быть декларацией.
    Node* firstDecl = parseForDecl();

    // Range-based:
    // for (auto x : vec)
    // for (auto& [k, v] : map)
    if (match(TokenKind::Colon)) {
        rejectTopLevelComma(TokenKind::RightParen);

        Node* range = parseExpression();

        expect(
            TokenKind::RightParen,
            "Expected ')' after range-for expression"
        );

        Node* body = parseForBody();

        return new NodeFor(
            firstDecl,
            nullptr,
            nullptr,
            range,
            body,
            true
        );
    }

    expect(TokenKind::Semicolon, "Expected ';' or ':' after for declaration");

    // C++20 init + range:
    // for (auto offset = compute(); auto x : vec)
    size_t pos = savePosition();
    try {
        Node* rangeDecl = parseForDecl();

        if (match(TokenKind::Colon)) {
            rejectTopLevelComma(TokenKind::RightParen);

            Node* range = parseExpression();
            expect(TokenKind::RightParen, "Expected ')' after range-for expression");
            Node* body = parseForBody();
            return new NodeFor(
                firstDecl,
                rangeDecl,
                nullptr,
                range,
                body,
                true
            );
        }

        delete rangeDecl;
        restorePosition(pos);
    }
    catch (...) {
        restorePosition(pos);
    }

    // Классический цикл:
    // for (auto i = 0; i < n; ++i)
    if (is(TokenKind::Semicolon)) {
        delete firstDecl;
        raise("Empty for condition is not allowed");
    }

    rejectTopLevelComma(TokenKind::Semicolon);
    Node* condition = parseExpression(0, typeexpression::Condition);

    expect(
        TokenKind::Semicolon,
        "Expected ';' after for condition"
    );

    if (is(TokenKind::RightParen)) {
        delete firstDecl;
        delete condition;
        raise("Empty for step is not allowed");
    }

    rejectTopLevelComma(TokenKind::RightParen);
    Node* step = parseExpression();

    expect(
        TokenKind::RightParen,
        "Expected ')' after for-header"
    );

    Node* body = parseForBody();

    return new NodeFor(
        firstDecl,
        condition,
        step,
        nullptr,
        body,
        false
    );
}

#endif // PARSER_HPP