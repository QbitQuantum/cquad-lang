
#ifndef NODEBINARYOPERAND_H
#define NODEBINARYOPERAND_H

#include <TokenKinds.h>

enum class BinaryOperand {
    Unknown = -1,

    // Арифметические
    Plus,
    Minus,
    Asterisk,
    Slash,
    Percent,

    // Сравнения
    EqualEqual,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    // Логические
    LogicalAnd,
    LogicalOr,

    // Побитовые
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    LeftShift,
    RightShift,

    // Присваивания
    Assign,
    PlusEqual,
    MinusEqual,
    AsteriskEqual,
    SlashEqual,
    PercentEqual,
    AmpersandEqual,
    PipeEqual,
    CaretEqual,
    LeftShiftEqual,
    RightShiftEqual,

    // Доступ к членам
    Dot,
    Arrow,

    // Другие
    Comma,
    Ternary
};

namespace BinOparand
{
    BinaryOperand getBinaryOperand(TokenKind op) {
        switch (op) {
            // Арифметические
        case TokenKind::Plus: return BinaryOperand::Plus;
        case TokenKind::Minus: return BinaryOperand::Minus;
        case TokenKind::Asterisk: return BinaryOperand::Asterisk;
        case TokenKind::Slash: return BinaryOperand::Slash;
        case TokenKind::Percent: return BinaryOperand::Percent;

            // Сравнения
        case TokenKind::Equals: return BinaryOperand::EqualEqual;
        case TokenKind::NotEqual: return BinaryOperand::NotEqual;
        case TokenKind::Less: return BinaryOperand::Less;
        case TokenKind::LessEqual: return BinaryOperand::LessEqual;
        case TokenKind::Greater: return BinaryOperand::Greater;
        case TokenKind::GreaterEqual: return BinaryOperand::GreaterEqual;

            // Логические
        case TokenKind::And: return BinaryOperand::LogicalAnd;
        case TokenKind::Or: return BinaryOperand::LogicalOr;

            // Побитовые
        case TokenKind::Ampersand: return BinaryOperand::BitwiseAnd;
        case TokenKind::Pipe: return BinaryOperand::BitwiseOr;
        case TokenKind::Caret: return BinaryOperand::BitwiseXor;
        case TokenKind::Shl: return BinaryOperand::LeftShift;
        case TokenKind::Shr: return BinaryOperand::RightShift;

            // Присваивания
        case TokenKind::Equal: return BinaryOperand::Assign;
        case TokenKind::PlusAssign: return BinaryOperand::PlusEqual;
        case TokenKind::MinusAssign: return BinaryOperand::MinusEqual;
        case TokenKind::MultAssign: return BinaryOperand::AsteriskEqual;
        case TokenKind::DivAssign: return BinaryOperand::SlashEqual;
        case TokenKind::ModAssign: return BinaryOperand::PercentEqual;
        case TokenKind::AndAssign: return BinaryOperand::AmpersandEqual;
        case TokenKind::OrAssign: return BinaryOperand::PipeEqual;
        case TokenKind::XorAssign: return BinaryOperand::CaretEqual;
        case TokenKind::ShlAssign: return BinaryOperand::LeftShiftEqual;
        case TokenKind::ShrAssign: return BinaryOperand::RightShiftEqual;

            // Доступ к членам
        case TokenKind::Dot: return BinaryOperand::Dot;
        case TokenKind::Arrow: return BinaryOperand::Arrow;

            // Другие
        case TokenKind::Comma: return BinaryOperand::Comma;
        case TokenKind::Question: return BinaryOperand::Ternary;

        default: return BinaryOperand::Unknown;
        }
    }

    static std::string opToString(BinaryOperand op) {
        switch (op) {
        case BinaryOperand::Plus: return "+";
        case BinaryOperand::Minus: return "-";
        case BinaryOperand::Asterisk: return "*";
        case BinaryOperand::Slash: return "/";
        case BinaryOperand::Percent: return "%";
        case BinaryOperand::EqualEqual: return "==";
        case BinaryOperand::NotEqual: return "!=";
        case BinaryOperand::Less: return "<";
        case BinaryOperand::LessEqual: return "<=";
        case BinaryOperand::Greater: return ">";
        case BinaryOperand::GreaterEqual: return ">=";
        case BinaryOperand::LogicalAnd: return "&&";
        case BinaryOperand::LogicalOr: return "||";
        case BinaryOperand::BitwiseAnd: return "&";
        case BinaryOperand::BitwiseOr: return "|";
        case BinaryOperand::BitwiseXor: return "^";
        case BinaryOperand::LeftShift: return "<<";
        case BinaryOperand::RightShift: return ">>";
        case BinaryOperand::Assign: return "=";
        case BinaryOperand::PlusEqual: return "+=";
        case BinaryOperand::MinusEqual: return "-=";
        case BinaryOperand::AsteriskEqual: return "*=";
        case BinaryOperand::SlashEqual: return "/=";
        case BinaryOperand::PercentEqual: return "%=";
        case BinaryOperand::AmpersandEqual: return "&=";
        case BinaryOperand::PipeEqual: return "|=";
        case BinaryOperand::CaretEqual: return "^=";
        case BinaryOperand::LeftShiftEqual: return "<<=";
        case BinaryOperand::RightShiftEqual: return ">>=";
        case BinaryOperand::Dot: return ".";
        case BinaryOperand::Arrow: return "->";
        case BinaryOperand::Comma: return ",";
        case BinaryOperand::Ternary: return "?";
        default: return "unknown";
        }
    }
}


#endif // NODEBINARYOPERAND_H
