
#ifndef LEXER_HPP
#define LEXER_HPP
#pragma once

#include "TokenKinds.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
private:
    using LexEnginePtr = void (Lexer::*)();
private:
    
    int CurrentColumn = 1;
    int CurrentLine = 1;
    int PosBuffer = 0;

    void LexNumericConstant();
    bool isPreprocessingNumberBody(char C) const;
    void AddToken(char C);
    char GetChar() const;

private:

    const std::unordered_map<char, LexEnginePtr> map{ {
    {'\n',&Lexer::LineFeed},
    {'\r',&Lexer::CarriageReturn},
    {'/', &Lexer::Slash},
    {'\t',&Lexer::Tab},

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

    void CarriageReturn();
    void LineFeed();
    void Tab();
    void Slash();

    void UpdatePosition()
    {
        CurrentColumn++;
        PosBuffer++;
    }

    bool neof() {
        return PosBuffer < Source.size();
    }

    void LexerRun();

    std::string Source = "";
    std::vector<Token> BufferToken;
public:
    Lexer(const std::string& source) : Source(source) 
    {
        LexerRun();
    };

    std::vector<Token> GetBufferLexerToken () {
        return BufferToken;
    }
};

void Lexer::LexerRun() {
    while (neof()) {
        char currentChar = GetChar();

        // Обработка идентификаторов
        if (tok::is_unicode_identifier_start(GetChar()))
        {
            size_t start = PosBuffer;
            while (neof() && (tok::is_unicode_identifier_start(GetChar()) || isdigit(GetChar())))
                PosBuffer++;

            // sizeof(identifier)
            size_t CurrentSize = PosBuffer - start;
            std::string identifier = Source.substr(start, CurrentSize);

            Token Token
            {
                TokenKind::Literal, identifier,
                CurrentLine, CurrentColumn,
            };

            CurrentColumn += CurrentSize;
            BufferToken.push_back(Token);
            continue;
        }

        if (auto it = map.find(currentChar); it != map.end()) {
            (this->*it->second)();
            continue;
        }

        AddToken(currentChar);
        UpdatePosition();
    }
}

bool Lexer::isPreprocessingNumberBody(char C) const {
    return isalnum(C) || C == '.' || C == '_' || C == '$';
}

char Lexer::GetChar() const {
    return Source[PosBuffer];
}

void Lexer::AddToken(char C) {
    Token Token{
    tok::constexprToTTokenID(C),
    std::to_string(C),
    CurrentLine, CurrentColumn };
    BufferToken.push_back(Token);
}

void Lexer::LexNumericConstant() {
    
    char C = GetChar();

    if (C == '.' && PosBuffer + 1 < Source.size() && !isdigit(Source[PosBuffer + 1]))
    {
        AddToken('.');
        UpdatePosition();
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
        C = GetChar();
    }

    // Обработка экспоненты: 1e+12, 1e-12
    if ((C == '-' || C == '+') && (PrevCh == 'E' || PrevCh == 'e')) {
        numericValue += C;
        UpdatePosition();
        C = GetChar();

        // Продолжаем сбор
        while (isPreprocessingNumberBody(C)) {
            numericValue += C;
            UpdatePosition();
            C = GetChar();
        }
    }

    // Обработка шестнадцатеричной плавающей точки: 0x1.2p+3
    if ((C == '-' || C == '+') && (PrevCh == 'P' || PrevCh == 'p')) {
        numericValue += C;
        UpdatePosition();
        C = GetChar();

        while (isPreprocessingNumberBody(C)) {
            numericValue += C;
            UpdatePosition();
            C = GetChar();
        }
    }

    // Обработка разделителей разрядов: 1'000'000
    if (C == '\'' && PosBuffer + 1 < Source.size()) {
        char Next = Source[PosBuffer + 1];
        if (isalnum(Next) || Next == '_') {
            numericValue += C;
            UpdatePosition();
            C = GetChar();

            while (isPreprocessingNumberBody(C)) {
                numericValue += C;
                UpdatePosition();
                C = GetChar();
            }
        }
    }

    token.value = numericValue;
    BufferToken.push_back(token);
}

void Lexer::LineFeed() {
    AddToken('\n');
    CurrentColumn = 1;
    CurrentLine++;
    PosBuffer++;
}

void Lexer::Tab() {
    // FIX
    UpdatePosition();
}

void Lexer::CarriageReturn() {
    // Если после `\r` идёт `\n` (Windows: `\r\n`), пропускаем `\n`
    if (PosBuffer + 1 < Source.size() && Source[PosBuffer + 1] == '\n') {
        PosBuffer++;
        LineFeed();  // Пропускаем `\n`, чтобы не дублировать LineFeed
    }
    else
    {
        AddToken('\r');
        UpdatePosition();
    }
}

void Lexer::Slash() {
    if (PosBuffer + 1 < Source.size() && (Source[PosBuffer + 1] == '/' || Source[PosBuffer + 1] == '*'))
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
        AddToken('/');
        UpdatePosition();
    }
}
#endif // LEXER_HPP