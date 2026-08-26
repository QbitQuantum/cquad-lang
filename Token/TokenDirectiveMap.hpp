
#ifndef TOKEN_DERECTIVE_MAP_HPP
#define TOKEN_DERECTIVE_MAP_HPP
#pragma once

#include "TokenKinds.h"

#include <string>
#include <unordered_map>

const std::unordered_map<std::string, TokenKind> TokenDirectiveMap = {
    {"#ifdef",      TokenKind::IfDefDirective},
    {"#ifndef",     TokenKind::IfNDefDirective},
    {"#endif",      TokenKind::EndIfDirective},
    {"#define",     TokenKind::DefineDirective},
    {"#undef",      TokenKind::UndefDirective},
    {"#if",         TokenKind::IfDirective},
    {"#else",       TokenKind::ElseDirective},
    {"#include",       TokenKind::IncludeDirective},
};

#endif // TOKEN_DERECTIVE_MAP_HPP