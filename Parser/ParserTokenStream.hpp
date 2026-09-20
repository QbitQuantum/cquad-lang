
#ifndef PARSER_TOKEN_STREAM_HPP
#define PARSER_TOKEN_STREAM_HPP
#pragma once

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

#endif // PARSER_TOKEN_STREAM_HPP