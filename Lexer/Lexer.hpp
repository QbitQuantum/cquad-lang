
#ifndef LEXER_HPP
#define LEXER_HPP
#pragma once

#include "TokenKinds.h"
#include <string>
#include <vector>
#include <unordered_map>

using Callback = void(*)(std::string);
Callback LexError;

#define DEF_GENERATION_BASE(X) \
Token Token{ TokenKind::##X, std::string(1, tok::constexprToChar(TokenKind::##X)), CurrentLine, CurrentColumn}; \
BufferToken.push_back(Token); \
UpdatePosition(); \

class Lexer {
private:
    using LexEnginePtr = void (Lexer::*)();
private:
    
    int CurrentColumn = 1;
    int CurrentLine = 1;
    int PosBuffer = 0;

    void LexNumericConstant();
    bool isPreprocessingNumberBody(char C) const;
    const char* consumeChar(const char* ptr, unsigned size);

private:

    std::unordered_map<char, LexEnginePtr> map{ {
    {' ', &Lexer::Space},
    {'"', &Lexer::Quotation},
    {'#', &Lexer::Hash},
    {'$', &Lexer::Dollar},
    {'%', &Lexer::Percent},
    {'&', &Lexer::Ampersand},
    {'(', &Lexer::LeftParen},
    {')', &Lexer::RightParen},
    {'{', &Lexer::LeftBrace},
    {'}', &Lexer::RightBrace},
    {'[', &Lexer::LeftBracket},
    {']', &Lexer::RightBracket},
    {'+', &Lexer::Plus},
    {',', &Lexer::Comma},
    {'-', &Lexer::Minus},
    {':', &Lexer::Colon},
    {';', &Lexer::Semicolon},
    {'<', &Lexer::Less},
    {'=', &Lexer::Equal},
    {'>', &Lexer::Greater},
    {'@', &Lexer::At},
    {'\\', &Lexer::Backslash},
    {'^', &Lexer::Caret},
    {'`', &Lexer::Backtick},
    {'|', &Lexer::Pipe},

    {'\n', &Lexer::LineFeed},
    {'\r', &Lexer::CarriageReturn},
    {'*', &Lexer::Asterisk},
    {'\'', &Lexer::Apostrophe},
    {'/', &Lexer::Slash},

    {'.', &Lexer::LexNumericConstant},
    {'0', &Lexer::LexNumericConstant},
    {'1', &Lexer::LexNumericConstant},
    {'2', &Lexer::LexNumericConstant},
    {'3', &Lexer::LexNumericConstant},
    {'4', &Lexer::LexNumericConstant},
    {'5', &Lexer::LexNumericConstant},
    {'6', &Lexer::LexNumericConstant},
    {'7', &Lexer::LexNumericConstant},
    {'8', &Lexer::LexNumericConstant},
    {'9', &Lexer::LexNumericConstant},

    }};


    void Space() { DEF_GENERATION_BASE(Space);};

    void Hash() { DEF_GENERATION_BASE(Hash); };
    void Dollar() { DEF_GENERATION_BASE(Dollar); };
    void Percent() { DEF_GENERATION_BASE(Percent); };
    void Ampersand() { DEF_GENERATION_BASE(Ampersand); };

    void RightParen() { DEF_GENERATION_BASE(RightParen); };
    void LeftParen() { DEF_GENERATION_BASE(LeftParen); };

    void RightBrace() { DEF_GENERATION_BASE(RightBrace); };
    void LeftBrace() { DEF_GENERATION_BASE(LeftBrace); };

    void LeftBracket() { DEF_GENERATION_BASE(LeftBracket); };
    void RightBracket() { DEF_GENERATION_BASE(RightBracket); };

    void Plus() { DEF_GENERATION_BASE(Plus); };
    void Comma() { DEF_GENERATION_BASE(Comma); };
    void Minus() { DEF_GENERATION_BASE(Minus); };
    void Colon() { DEF_GENERATION_BASE(Colon); };
    void Semicolon() { DEF_GENERATION_BASE(Semicolon); };
    void Less() { DEF_GENERATION_BASE(Less); };
    void Equal() { DEF_GENERATION_BASE(Equal); };
    void Greater() { DEF_GENERATION_BASE(Greater); };
    void At() { DEF_GENERATION_BASE(At); };
    void Backslash() { DEF_GENERATION_BASE(Backslash); };
    void Caret() { DEF_GENERATION_BASE(Caret); };
    void Backtick() { DEF_GENERATION_BASE(Backtick); };
    void Pipe() { DEF_GENERATION_BASE(Pipe); };
    void Tilde() { DEF_GENERATION_BASE(Tilde); };
    void Asterisk() { DEF_GENERATION_BASE(Asterisk); };
    void Apostrophe() { DEF_GENERATION_BASE(Apostrophe); };
    void Quotation() { DEF_GENERATION_BASE(Quotation);};

    void CarriageReturn();
    void LineFeed();
    void Slash();

    void UpdatePosition()
    {
        CurrentColumn++;
        PosBuffer++;
    }

    bool neof() {
        return PosBuffer < SourceCode.size();
    }

    char GetChar() const;

    void LexerRun();

    std::string SourceCode = "";
    std::vector<Token> BufferToken;
public:
     Lexer(const std::string source) {
        SourceCode = source;
        LexerRun();
    };

    std::vector<Token> GetBufferLexerToken () {
        return BufferToken;
    }
};

void Lexer::LexerRun() {
    while (neof()) {
        char currentChar = GetChar();

        if (auto it = map.find(currentChar); it != map.end()) {
            (this->*it->second)();
        }
        else
        {
            // Обработка идентификаторов
            size_t start = PosBuffer;
            while (neof() && (tok::is_unicode_identifier_start(GetChar()) || isdigit(GetChar())))
                PosBuffer++;

            // sizeof(identifier)
            size_t CurrentSize = PosBuffer - start;
            std::string identifier = SourceCode.substr(start, CurrentSize);

            Token Token
            {
                TokenKind::Literal, identifier,
                CurrentLine, CurrentColumn,
            };

            CurrentColumn += CurrentSize;
            BufferToken.push_back(Token);
        }
    }
}

bool Lexer::isPreprocessingNumberBody(char C) const {
    return isalnum(C) || C == '.' || C == '_' || C == '$';
}

const char* Lexer::consumeChar(const char* ptr, unsigned size) {
    PosBuffer += size;
    return ptr + size;
}

void Lexer::LexNumericConstant() {
    
    char C = SourceCode[PosBuffer];

    if (C == '.' && PosBuffer + 1 < SourceCode.size() && !isdigit(SourceCode[PosBuffer + 1]))
    {
        DEF_GENERATION_BASE(Dot);
        return;
    }

    Token token 
    {
        TokenKind::Literal, "",
        CurrentLine, CurrentColumn
    };

    std::string numericValue = "";
    char PrevCh = 0;

    // Собираем тело числа
    while (isPreprocessingNumberBody(C)) {
        PrevCh = C;
        numericValue += C;
        UpdatePosition();
        C = SourceCode[PosBuffer];
    }

    // Обработка экспоненты: 1e+12, 1e-12
    if ((C == '-' || C == '+') && (PrevCh == 'E' || PrevCh == 'e')) {
        numericValue += C;
        UpdatePosition();
        C = SourceCode[PosBuffer];

        // Продолжаем сбор
        while (isPreprocessingNumberBody(C)) {
            numericValue += C;
            UpdatePosition();
            C = SourceCode[PosBuffer];
        }
    }

    // Обработка шестнадцатеричной плавающей точки: 0x1.2p+3
    if ((C == '-' || C == '+') && (PrevCh == 'P' || PrevCh == 'p')) {
        numericValue += C;
        UpdatePosition();
        C = SourceCode[PosBuffer];

        while (isPreprocessingNumberBody(C)) {
            numericValue += C;
            UpdatePosition();
            C = SourceCode[PosBuffer];
        }
    }

    // Обработка разделителей разрядов: 1'000'000
    if (C == '\'' && PosBuffer + 1 < SourceCode.size()) {
        char Next = SourceCode[PosBuffer + 1];
        if (isalnum(Next) || Next == '_') {
            numericValue += C;
            UpdatePosition();
            C = SourceCode[PosBuffer];

            while (isPreprocessingNumberBody(C)) {
                numericValue += C;
                UpdatePosition();
                C = SourceCode[PosBuffer];
            }
        }
    }

    token.value = numericValue;
    BufferToken.push_back(token);
}

char Lexer::GetChar() const {
    return SourceCode[PosBuffer];
}

void Lexer::LineFeed() {
    Token Token{ TokenKind::LineFeed, std::string(1, tok::constexprToChar(TokenKind::LineFeed)), CurrentLine, CurrentColumn };
    BufferToken.push_back(Token);
    CurrentColumn = 1;
    CurrentLine++;
    PosBuffer++;
}

void Lexer::CarriageReturn() {
    // Если после `\r` идёт `\n` (Windows: `\r\n`), пропускаем `\n`
    if (PosBuffer + 1 < SourceCode.size() && SourceCode[PosBuffer + 1] == '\n') {
        PosBuffer++;
        LineFeed();  // Пропускаем `\n`, чтобы не дублировать LineFeed
    }
    else
    {
        DEF_GENERATION_BASE(CarriageReturn);
    }
}

void Lexer::Slash() {
    if (PosBuffer + 1 < SourceCode.size() && (SourceCode[PosBuffer + 1] == '/' || SourceCode[PosBuffer + 1] == '*'))
    {
        UpdatePosition();
        char _getchar = GetChar();
        if (_getchar == '/')
        {
            while (neof() && GetChar() != '\n') {
                UpdatePosition();
            }
        }
        else if (_getchar == '*')
        {
            UpdatePosition();
            while (neof()) {
                char current = GetChar();
                UpdatePosition();
                // Ищем последовательность */
                if (current == '*' && neof() && GetChar() == '/') {
                    UpdatePosition();
                    break; // Конец комментария
                }
            }
        }
    }
    else
    {
        DEF_GENERATION_BASE(Slash);
    }
}
#endif // LEXER_HPP