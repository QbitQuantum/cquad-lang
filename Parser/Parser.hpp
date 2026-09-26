#ifndef PARSER_HPP
#define PARSER_HPP
#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include "PostLexer.hpp"
#include "ParserError.hpp"
#include "ParserTokenStream.hpp"
#include "SymbolTable.hpp"
#include "ParserConstant.h"
#include "Node.hpp"

class Parser
{
private:
    TokenStream stream;
    std::vector<Node*> ast;
    std::vector<Node*> ambiguousNodes;
    SymbolTable symbols;

    // ---- помощники для работы с токенами ----
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

    static std::string identifierName(Node* node) {
        return node->print();
    }

    Node* parseStatementList(int scope);
    Node* parseBlockOrStatement(int scope);
    Node* parseBracedBlock(int scope);
    Node* parseBody(int scope);

    Node* parseTopLevel();
    Node* parseStatement(int scope);
    Node* parseDeclaration();
    Node* parseDeclarationPrimary();

    Node* parseIdentifier(int exprKind = typeexpression::Unknown);
    Node* parseIdentifierScope(int exprKind = typeexpression::Unknown, Node* qualifier = nullptr);
    Node* parseDeclarationName(SymbolKind kind, Node* typeNode = nullptr, bool throwOnRedeclare = true);
    Node* parsePrimary();
    Node* parseExpression(int minPrec = 0, int exprKind = typeexpression::Expression);

    Node* parseCall();
    Node* parseCall(Node* callee);
    Node* parseIndex(Node* callee);

    Node* parseInitializerList();
    Node* parseInitializerListBody();

    Node* parseValue();
    Node* parseDefault();
    Node* parseNew();
    Node* parseDelete();

    Node* parseType();
    Node* parseTypeArray();

    Node* parseArraySize();

    Node* parseFunction();
    Node* parseFunctionParams();
    Node* parseFunctionParam();
    Node* parseFunctionBody();

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
    Node* parseVarDecl(bool IsPrimary = false, SymbolKind kind = SymbolKind::Variable);
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
    Node* parseForBody();
    Node* parseForDecl();

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
    const std::vector<Node*>& GetAmbiguousNodes() const { return ambiguousNodes; }

    const SymbolTable& GetSymbolTable() const { return symbols; }
    SymbolTable& GetSymbolTable() { return symbols; }
};


Node* Parser::parseStatementList(int scope)
{
    symbols.enterScope(ScopeKind::Block);

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
        case TokenKind::For:     stmt = parseFor();       break;
        default:                 stmt = parseStatement(scope); break;
        }
        if (stmt) elem.push_back(stmt);
    }

    symbols.exitScope();
    return new NodeBlock(elem);
}

Node* Parser::parseBlockOrStatement(int scope)
{
    if (is(TokenKind::LeftBrace))
        return parseBracedBlock(scope);

    symbols.enterScope(ScopeKind::Block);

    std::vector<Node*> elem;
    Node* stmt = nullptr;
    switch (token()) {
    case TokenKind::If:     stmt = parseCondition(); break;
    case TokenKind::Switch: stmt = parseSwitch();    break;
    case TokenKind::While:  stmt = parseWhile();     break;
    case TokenKind::For:    stmt = parseFor();       break;
    default:                stmt = parseStatement(scope); break;
    }
    if (stmt) elem.push_back(stmt);

    symbols.exitScope();
    return new NodeBlock(elem);
}

Node* Parser::parseBracedBlock(int scope)
{
    expect(TokenKind::LeftBrace, "Expected '{'");
    Node* body = parseStatementList(scope);
    expect(TokenKind::RightBrace, "Expected '}'");
    return body;
}

Node* Parser::parseBody(int scope)
{
    if (typescope::requiresBracedBlock(scope))
        return parseBracedBlock(scope);
    return parseBlockOrStatement(scope);
}

Node* Parser::parseTopLevel()
{
    if (is(TokenKind::Template)) return parseTemplateDecl();
    switch (token()) {
    case TokenKind::Class: return parseClass();
    default:               return parseStatement(typescope::Global);
    }
}

Node* Parser::parseIdentifier(int exprKind) {
    Node* base = parseIdentifierScope(exprKind);
    while (match(TokenKind::ScResOp)) {
        bool tplKw = match(TokenKind::Template); // not used
        base = parseIdentifierScope(exprKind, base);
    }
    return base;
}

Node* Parser::parseIdentifierScope(int exprKind, Node* qualifier) {
    std::string name = consume(TokenKind::IdentifierLiteral).value;
    Node* tmplArgs = nullptr;
    if (is(TokenKind::Less) && exprKind != typeexpression::Condition)
        tmplArgs = parseTemplateArgList();
    return new NodeIdentifier(tmplArgs, std::move(name), qualifier);
}

Node* Parser::parseDeclarationName(SymbolKind kind, Node* typeNode, bool throwOnRedeclare)
{
    if (isNot(TokenKind::IdentifierLiteral))
        raise("Expected identifier in declaration");
    Token t = peek();
    Node* name = parseIdentifier();
    symbols.declare(identifierName(name), kind, typeNode, name, t.line, t.column, throwOnRedeclare);
    return name;
}

Node* Parser::parsePrimary() {
    UnaryOperand unary = UnaryOperand::Unknown;
    if (tok::IsPrefixUnaryOperator(token())) {
        unary = UnOparand::getUnaryOperand(token());
        advance();
    }

    Node* right = nullptr;
    switch (token()) {
    case TokenKind::New:            right = parseNew();        break;
    case TokenKind::Delete_:        right = parseDelete();     break;
    case TokenKind::Default:        right = parseDefault();    break;
    case TokenKind::IdentifierLiteral:
        right = parseIdentifier(typeexpression::Condition); break;
    case TokenKind::LeftBrace:      right = parseInitializerList(); break;
    case TokenKind::LeftParen:
        consume(TokenKind::LeftParen);
        right = parseExpression();
        expect(TokenKind::RightParen, "Expected ')'");
        break;
    default:
    {
        if (tok::isLiteral(token())) right = parseValue();
        else raise("Unexpected token in primary expression");
    }
    }

    if (unary != UnaryOperand::Unknown)
        right = new NodeUnaryOp(unary, right);

    while (true) {
        if (is(TokenKind::LeftBracket)) {
            right = parseIndex(right);
            continue;
        }
        if (is(TokenKind::LeftParen)) {
            right = parseCall(right);
            continue;
        }
        if (is(TokenKind::Dot) || is(TokenKind::Arrow)) {
            bool arrow = is(TokenKind::Arrow); advance();
            right = new NodeMemberCall(right, parsePrimary(), arrow);
            continue;
        }
        if (tok::IsPostfixUnaryOperator(token())) {
            auto rUnary = UnOparand::getUnaryOperand(token());
            advance();
            right = new NodeUnaryOp(rUnary, right, true);
            continue;
        }
        break;
    }
    return right;
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

Node* Parser::parseCall() {
    Node* callee = parseIdentifier();
    return parseCall(callee);
}

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

Node* Parser::parseIndex(Node* callee) {
    expect(TokenKind::LeftBracket, "Expected '[' for index expression");
    Node* index = nullptr;
    if (isNot(TokenKind::RightBracket)) {
        index = parseExpression();
    }
    expect(TokenKind::RightBracket, "Expected ']' after index expression");
    return new NodeIndexAccess(callee, index);
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
    return parseBody(typescope::If);
}

Node* Parser::parseElse() {
    consume(TokenKind::Else);
    Node* body = parseElseBody();
    return new NodeElse(body);
}

Node* Parser::parseElseBody() {
    return parseBody(typescope::Else);
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
    return parseBody(typescope::Function);
}

Node* Parser::parseReturn() {
    consume(TokenKind::Return);
    Node* expr = nullptr;
    if (isNot(TokenKind::Semicolon)) expr = parseExpression();
    expect(TokenKind::Semicolon, "Expected ';' after return statement");
    return new NodeReturn(expr);
}

Node* Parser::parseBreak() {
    consume(TokenKind::Break);
    expect(TokenKind::Semicolon, "Expected ';' after 'break'");
    return new NodeBreak();
}

Node* Parser::parseNew() {
    consume(TokenKind::New);
    Node* size = nullptr;
    if (is(TokenKind::LeftBracket)) size = parseArraySize();
    Node* call = parseCall();
    return new NodeNew(call, size);
}

Node* Parser::parseDelete() {
    consume(TokenKind::Delete_);
    Node* size = nullptr;
    if (is(TokenKind::LeftBracket)) size = parseArraySize();
    Node* expr = parseIdentifier();
    expect(TokenKind::Semicolon, "Expected ';' after delete statement");
    return new NodeDelete(expr, size);
}

Node* Parser::parseValue() {
    Token ValueToken = peek();
    std::string raw_value = ValueToken.value;
    TokenKind raw_type = ValueToken.type;
    Node* right = nullptr;
    switch (raw_type) {
    case TokenKind::IntegerLiteral:
    case TokenKind::HexLiteral:
    case TokenKind::BinaryLiteral:  right = new NodeInteger(raw_value);    break;
    case TokenKind::FloatLiteral:
    case TokenKind::DoubleLiteral:
    case TokenKind::LongDoubleLiteral: right = new NodeFloating(raw_value);    break;
    case TokenKind::TrueLiteral:
    case TokenKind::FalseLiteral:   right = new NodeBoolean(raw_value);    break;
    case TokenKind::StringLiteral:
    case TokenKind::WStringLiteral: right = new NodeString(raw_value);    break;
    case TokenKind::CharLiteral:
    case TokenKind::WCharLiteral:   right = new NodeCharacter(raw_value);    break;
    case TokenKind::NullptrLiteral: right = new NodeNullptr();    break;
    default: raise("Unexpected token in literal");
    }
    advance();
    return right;
}

Node* Parser::parseDefault() {
    consume(TokenKind::Default);
    return new NodeDefault();
}

Node* Parser::parseArraySize() {
    expect(TokenKind::LeftBracket, "Expected '[' for CArray");
    Node* stmt = nullptr;
    switch (token()) {
    case TokenKind::IdentifierLiteral: stmt = parseExpression(); break;
    case TokenKind::IntegerLiteral:
    case TokenKind::HexLiteral:
    case TokenKind::BinaryLiteral:     stmt = parseValue();    break;
    default: raise("Not correct token for array size");
    }
    expect(TokenKind::RightBracket, "Expected ']' for CArray");
    return stmt;
}

Node* Parser::parseTypeArray() {
    if (!is(TokenKind::LeftBracket)) return nullptr;
    return parseArraySize();
}

Node* Parser::parseType() {
    Node* type = nullptr;
    Node* size = nullptr;
    NodeType::EType eType = NodeType::EType::None;

    bool isConst = match(TokenKind::Const);
    bool isAuto = match(TokenKind::Auto);

    if (!isAuto) {
        type = parseIdentifier();
        size = parseTypeArray();
    }

    switch (token()) {
    case TokenKind::Asterisk:  advance();   eType = NodeType::EType::Pointer; break;
    case TokenKind::Ampersand: advance();   eType = NodeType::EType::Ref;     break;
    case TokenKind::And:       advance();   eType = NodeType::EType::RValue;  break;
    default: break;
    }

    return new NodeType(type, size, isConst, eType, isAuto);
}

Node* Parser::parseTemplateParam() {
    // template<typename C>
    if (is(TokenKind::Template)) return parseTemplate();

    // typename A [= default]
    if (is(TokenKind::Typename)) {
        advance();
        Node* name = nullptr;
        if (is(TokenKind::IdentifierLiteral))
            name = parseDeclarationName(SymbolKind::TemplateTypeParam);
        Node* def = match(TokenKind::Equal) ? parseType() : nullptr;
        return new NodeTemplateTypeParam(name, def);
    }

    // int B [= 3]
    Node* type = parseType();
    Node* name = parseDeclarationName(SymbolKind::TemplateValueParam, type);
    Node* def = match(TokenKind::Equal) ? parseExpression() : nullptr;
    return new NodeTemplateValueParam(type, name, def);
}

Node* Parser::parseTemplateDecl() {
    if (isNot(TokenKind::Template)) return nullptr;

    expect(TokenKind::Template, "Expected 'template'");
    symbols.enterScope(ScopeKind::Template);

    Node* params = parseTemplateParamList();

    Node* decl = nullptr;
    switch (token()) {
    case TokenKind::Class: decl = parseClass(); break;
    default:               decl = parseStatement(typescope::Global); break;
    }

    symbols.exitScope();
    return wrapTemplate(params, decl);
}

Node* Parser::parseTemplate() {
    expect(TokenKind::Template, "Expected 'template'");
    symbols.enterScope(ScopeKind::Template);
    Node* params = parseTemplateParamList();
    symbols.exitScope();
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

    static auto canStartType = [](TokenKind d) {
        switch (d) {
        case TokenKind::Const:
        case TokenKind::Auto:
        case TokenKind::IdentifierLiteral:
            return true;
        default:
            return false;
        }
        };

    if (tok::IsPrefixUnaryOperator(token())) {
        Node* Stmt = parseExpression();
        expect(TokenKind::Semicolon, "Expected ';' after expression");
        return Stmt;
    }

    if (!canStartType(token())) {
        Node* expr = parseExpression();
        expect(TokenKind::Semicolon, "Expected ';' after expression");
        return expr;
    }

    size_t saved = savePosition();
    Node* type = parseType();

    if (tok::IsPostfixUnaryOperator(token())) {
        if (scope == typescope::Function) {
            restorePosition(saved);
            delete type;
            Node* Stmt = parseExpression();
            expect(TokenKind::Semicolon, "Expected ';' after expression");
            return Stmt;
        }
    }

    if (tok::IsAssignmentOperator(token()))
    {
        if (scope == typescope::Class && !is(TokenKind::Equal))
            raise("No correct token assignment in body class");
        restorePosition(saved);
        delete type;
        Node* decl = parseDeclaration();
        expect(TokenKind::Semicolon, "Expected ';' after expression");
        return decl;
    }

    switch (token()) {
    case TokenKind::IdentifierLiteral:
    case TokenKind::LeftBrace:
    {
        if (is(TokenKind::IdentifierLiteral))
        {
            Node* name = parseIdentifier();
            TokenKind next = token();
            delete name;
            if (next == TokenKind::LeftParen) {
                restorePosition(saved);
                delete type;
                return parseFunction();
            }
        }
        restorePosition(saved);
        delete type;
        return parseVar();
    }
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
    restorePosition(saved);
    delete type;
    Node* Stmt = parseExpression();
    expect(TokenKind::Semicolon, "Expected ';' after expression");
    return Stmt;
}

Node* Parser::parseFunction() {
    Node* returnType = parseType();

    int typef = typefunction::Function;
    switch (token())
    {
    case TokenKind::Caret:
    case TokenKind::Tilde:
        typef = is(TokenKind::Caret) ? typefunction::Constructor : typefunction::Destructor;
        advance();
        break;
    default: break;
    }

    Node* name = parseDeclarationName(SymbolKind::Function, returnType, false);
    symbols.enterScope(ScopeKind::Function, identifierName(name));
    Symbol* fnSym = symbols.currentScope()->parent->findLocal(identifierName(name));
    if (fnSym) fnSym->scope = symbols.currentScope();

    Node* params = parseFunctionParams();
    Node* body = parseFunctionBody();
    symbols.exitScope();

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
    Node* defaultValue = parseVarDecl(true, SymbolKind::Parameter);
    return new NodeVarDeclarationList(type, defaultValue);
}

Node* Parser::parseFunctionBody() {
    if (is(TokenKind::LeftBrace))
        return parseBody(typescope::Function);
    expect(TokenKind::Semicolon, "Expected ';'");
    return nullptr;
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

    symbols.enterScope(ScopeKind::Block);

    std::vector<Node*> elem;
    while (!atEnd() && isNot(TokenKind::RightBrace)) {
        Node* stmt = nullptr;
        switch (token()) {
        case TokenKind::Case:      stmt = parseCase();        break;
        case TokenKind::Default:   stmt = parseDefaultCase(); break;
        default:                   raise("Not correct token parse in switch section");
        }
        if (stmt) elem.push_back(stmt);
    }

    symbols.exitScope();
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
    symbols.enterScope(ScopeKind::Block);
    elem.push_back(parseBody(typescope::Case));
    symbols.exitScope();
    if (is(TokenKind::Break))
        elem.push_back(parseBreak());
    return new NodeCaseBody(std::move(elem));
}

Node* Parser::parseDefaultCase() {
    consume(TokenKind::Default);
    expect(TokenKind::Colon, "Expected ':' after 'default'");
    Node* body = parseCaseBody();
    return new NodeCaseDefault(body);
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
    Node* var = new NodeVarDeclarationList(type, list);
    ambiguousNodes.push_back(var);
    return var;
}

Node* Parser::parseVarType() {
    return parseType();
}

Node* Parser::parseVarDecl(bool IsPrimary, SymbolKind kind) {
    Node* name = parseDeclarationName(kind);
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
    Node* name = parseDeclarationName(SymbolKind::Class);
    symbols.enterScope(ScopeKind::Class, identifierName(name));
    Symbol* clsSym = symbols.currentScope()->parent->findLocal(identifierName(name));
    if (clsSym) clsSym->scope = symbols.currentScope();

    Node* base = parseClassBase();
    Node* body = parseClassBody();

    symbols.exitScope();

    return new NodeClass(name, base, body);
}

Node* Parser::parseClassName() {
    if (isNot(TokenKind::IdentifierLiteral))
        raise("Expected class name");
    return parseIdentifier();
}

Node* Parser::parseClassBody() {
    if (is(TokenKind::LeftBrace)) {
        expect(TokenKind::LeftBrace, "Expected '{' after class declaration");
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

Node* Parser::parseForDecl()
{
    Node* type = parseType();

    // Structured binding:
    if (match(TokenKind::LeftBracket)) {
        std::vector<Node*> names;

        if (isNot(TokenKind::IdentifierLiteral))
            raise("Expected identifier in structured binding");

        names.push_back(parseDeclarationName(SymbolKind::Variable, type));
        while (match(TokenKind::Comma))
            names.push_back(parseDeclarationName(SymbolKind::Variable, type));

        expect(TokenKind::RightBracket, "Expected ']' after structured binding identifiers");

        return new NodeStructuredBinding(type, std::move(names));
    }

    // Обычная декларация: auto i [= expression]
    if (isNot(TokenKind::IdentifierLiteral)) {
        delete type;
        raise("Expected variable name in for declaration");
    }

    Node* name = parseDeclarationName(SymbolKind::Variable, type);
    Node* init = nullptr;
    int initKind = typeinitialization::Default;

    if (match(TokenKind::Equal)) {
        initKind = typeinitialization::Copy;
        init = parseExpression();
    }

    Node* declaration = new NodeDeclaration(name, init, initKind);
    return new NodeVarDeclarationList(type, declaration);
}

Node* Parser::parseForBody()
{
    return parseBody(typescope::For);
}

Node* Parser::parseFor()
{
    consume(TokenKind::For);

    expect(TokenKind::LeftParen, "Expected '(' after 'for'");

    symbols.enterScope(ScopeKind::For);

    // for (;;) { }
    if (match(TokenKind::Semicolon)) {
        expect(TokenKind::Semicolon, "Only 'for (;;)' is allowed with empty initialization");
        expect(TokenKind::RightParen, "Expected ')' after for-header");
        Node* body = parseForBody();
        symbols.exitScope();
        return new NodeFor(nullptr, nullptr, nullptr, body);
    }

    Node* firstDecl = parseForDecl();

    // Range-based: for (auto x : vec)
    if (match(TokenKind::Colon)) {
        Node* range = parseExpression();
        expect(TokenKind::RightParen, "Expected ')' after range-for expression");
        Node* body = parseForBody();
        symbols.exitScope();
        return new NodeForRange(nullptr, firstDecl, range, body);
    }
    expect(TokenKind::Semicolon, "Expected ';' or ':' after for declaration");

    // for (auto offset = compute(); auto x : vec)
    size_t pos = savePosition();
    try {
        Node* rangeDecl = parseForDecl();

        if (match(TokenKind::Colon)) {
            Node* range = parseExpression();
            expect(TokenKind::RightParen, "Expected ')' after range-for expression");
            Node* body = parseForBody();
            symbols.exitScope();
            return new NodeForRange(firstDecl, rangeDecl, range, body);
        }
        delete rangeDecl;
    }
    catch (...) {}
    restorePosition(pos);

    Node* condition = parseExpression(0, typeexpression::Condition);
    expect(TokenKind::Semicolon, "Expected ';' after for condition");

    if (is(TokenKind::RightParen)) {
        delete firstDecl;
        delete condition;
        symbols.exitScope();
        raise("Empty for step is not allowed");
    }

    Node* step = parseExpression();
    expect(TokenKind::RightParen, "Expected ')' after for-header");
    Node* body = parseForBody();

    symbols.exitScope();
    return new NodeFor(firstDecl, condition, step, body);
}

#endif // PARSER_HPP