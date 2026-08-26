
#ifndef TOKEN_KEYWORD_MAP_HPP
#define TOKEN_KEYWORD_MAP_HPP
#pragma once

#include "TokenKinds.h"

#include <unordered_map>
#include <string>

constexpr char const_tolower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

constexpr uint32_t ConstexprCppHash(const char* str, uint32_t seed = 0) {
    uint32_t hash = seed;
    for (size_t i = 0; str[i] != '\0'; ++i) {
        uint32_t k = static_cast<uint8_t>(str[i]);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        hash ^= k;
        hash = (hash << 13) | (hash >> 19);
        hash = hash * 5 + 0xe6546b64;
    }
    hash ^= std::char_traits<char>::length(str);
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash;
}

uint32_t CppHash(const char* str, uint32_t seed = 0) {
    uint32_t hash = seed;
    for (size_t i = 0; str[i] != '\0'; ++i) {
        uint32_t k = static_cast<uint8_t>(str[i]);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        hash ^= k;
        hash = (hash << 13) | (hash >> 19);
        hash = hash * 5 + 0xe6546b64;
    }
    hash ^= std::char_traits<char>::length(str);
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash;
}

uint32_t CppHash(const std::string& s, uint32_t seed = 0) {
    uint32_t hash = seed;
    for (char c : s) {
        uint32_t k = static_cast<uint8_t>(c);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        hash ^= k;
        hash = (hash << 13) | (hash >> 19);
        hash = hash * 5 + 0xe6546b64;
    }
    hash ^= s.size();
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash;
}

static const std::unordered_map<uint32_t, TokenKind> TokenKeywordMap = {

    // ===== Встроенные значение типов =====
    {ConstexprCppHash("true"),         TokenKind::TrueLiteral},
    {ConstexprCppHash("false"),        TokenKind::FalseLiteral},
    {ConstexprCppHash("nullptr"),      TokenKind::NullptrLiteral},

    /*
    // ===== Встроенные типы =====
    {ConstexprCppHash("char"),         TokenKind::Char},
    {ConstexprCppHash("wchar_t"),      TokenKind::WChar_t},
    {ConstexprCppHash("short"),        TokenKind::Short},
    {ConstexprCppHash("int"),          TokenKind::Int},
    {ConstexprCppHash("long"),         TokenKind::Long},
    {ConstexprCppHash("double"),       TokenKind::Double},
    {ConstexprCppHash("float"),        TokenKind::Float},
    {ConstexprCppHash("bool"),         TokenKind::Bool},
    {ConstexprCppHash("void"),         TokenKind::Void},
    {ConstexprCppHash("signed"),       TokenKind::Signed},
    {ConstexprCppHash("unsigned"),     TokenKind::Unsigned},
    */
    

    // ===== Управляющие конструкции =====
    {ConstexprCppHash("if"),           TokenKind::If},
    {ConstexprCppHash("else"),         TokenKind::Else},
    {ConstexprCppHash("case"),         TokenKind::Case},
    {ConstexprCppHash("for"),          TokenKind::For},
    {ConstexprCppHash("while"),        TokenKind::While},
    {ConstexprCppHash("do"),           TokenKind::Do},
    {ConstexprCppHash("try"),          TokenKind::Try},
    {ConstexprCppHash("catch"),        TokenKind::Catch},
    {ConstexprCppHash("return"),       TokenKind::Return},

    {ConstexprCppHash("break"),        TokenKind::Break},
    {ConstexprCppHash("continue"),     TokenKind::Continue},
    {ConstexprCppHash("switch"),       TokenKind::Switch},
    {ConstexprCppHash("default"),      TokenKind::Default},
    {ConstexprCppHash("new"),          TokenKind::New},
    {ConstexprCppHash("delete"),       TokenKind::Delete_},

    // ===== Объявление типов =====
    {ConstexprCppHash("class"),        TokenKind::Class},
    {ConstexprCppHash("struct"),       TokenKind::Struct},
    {ConstexprCppHash("namespace"),    TokenKind::Namespace},
    {ConstexprCppHash("enum"),         TokenKind::Enum},
    {ConstexprCppHash("override"),     TokenKind::Override},
    {ConstexprCppHash("virtual"),      TokenKind::Virtual},
    {ConstexprCppHash("final"),        TokenKind::Final},
    {ConstexprCppHash("auto"),         TokenKind::Auto},

    // ===== квалификаторы =====
    {ConstexprCppHash("const"),        TokenKind::Const},

    // ===== Модификаторы =====
    {ConstexprCppHash("private"),      TokenKind::Private},
    {ConstexprCppHash("protected"),    TokenKind::Protected},
    {ConstexprCppHash("public"),       TokenKind::Public},
    {ConstexprCppHash("static"),       TokenKind::Static },
    {ConstexprCppHash("operator"),     TokenKind::Operator },

    // ===== Прочее =====
    {ConstexprCppHash("property"),     TokenKind::Property },
    {ConstexprCppHash("read"),         TokenKind::Read },
    {ConstexprCppHash("write"),        TokenKind::Write },
    {ConstexprCppHash("delegate"),     TokenKind::Delegate },
    {ConstexprCppHash("var"),          TokenKind::Var },
    {ConstexprCppHash("function"),     TokenKind::Function },
    {ConstexprCppHash("lambda"),       TokenKind::Lambda },
    {ConstexprCppHash("default"),      TokenKind::Default },
    {ConstexprCppHash("using"),         TokenKind::Using },
    {ConstexprCppHash("pointer"),      TokenKind::Pointer },

    {ConstexprCppHash("constructor"),  TokenKind::Constructor },
    {ConstexprCppHash("destructor"),   TokenKind::Destructor },

    {ConstexprCppHash("template"),     TokenKind::Template },
    {ConstexprCppHash("typename"),     TokenKind::Typename },
};

static_assert(ConstexprCppHash("div") != ConstexprCppHash("mod"));

#endif //TOKEN_KEYWORD_MAP_HPP