
#ifndef LEXER_ERROR_HH
#define LEXER_ERROR_HH
#pragma once

#include <string>

enum class LexErrorType {
    UnclosedString,    // Не закрыта кавычка '
    UnclosedDirective, // Не закрыта директива
    UnclosedComment,   // Не закрыт комментарий { или (*
    InvalidNumber,     // Например, 1.2.3 или 0xGG
    InvalidChar,       // Недопустимый символ
};

// Функция, принимающая enum и строковый литерал + callback
std::string GetErrorString(LexErrorType type) {
    switch (type) {
    case LexErrorType::UnclosedString:    return "UnclosedString";
    case LexErrorType::UnclosedDirective:       return "UnclosedDirective";
    case LexErrorType::UnclosedComment: return "UnclosedComment";
    case LexErrorType::InvalidNumber:       return "InvalidNumber";
    case LexErrorType::InvalidChar:       return "InvalidChar";
    }
}

#endif // LEXER_ERROR_HH