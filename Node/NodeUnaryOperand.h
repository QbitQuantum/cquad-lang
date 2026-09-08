
#ifndef NODEUNARYOPERAND_H
#define NODEUNARYOPERAND_H

#include <TokenKinds.h>

enum class UnaryOperand {
    Unknown = -1,

    // Арифметические
    Plus,
    Minus,

    // Логические
    LogicalNot,

    // Побитовые
    BitwiseNot,

    // Инкремент/Декремент
    PreIncrement,
    PreDecrement,
    PostIncrement,
    PostDecrement,

    // Указатели и ссылки
    AddressOf,      // & (взятие адреса)
    Dereference,    // * (разыменование)
};

namespace UnOparand
{
    UnaryOperand getUnaryOperand(TokenKind op) {
        switch (op) {
            // Арифметические
        case TokenKind::Plus: return UnaryOperand::Plus;
        case TokenKind::Minus: return UnaryOperand::Minus;

            // Логические
        case TokenKind::Exclamation: return UnaryOperand::LogicalNot;

            // Побитовые
        case TokenKind::Tilde: return UnaryOperand::BitwiseNot;

            // Инкремент/Декремент (префиксные)
        case TokenKind::Inc: return UnaryOperand::PreIncrement;
        case TokenKind::Dec: return UnaryOperand::PreDecrement;

            // Указатели и ссылки
        case TokenKind::Ampersand: return UnaryOperand::AddressOf;
        case TokenKind::Asterisk: return UnaryOperand::Dereference;

        default: return UnaryOperand::Unknown;
        }
    }

    static std::string opToString(UnaryOperand op) {
        switch (op) {
        case UnaryOperand::Plus: return "+";
        case UnaryOperand::Minus: return "-";
        case UnaryOperand::LogicalNot: return "!";
        case UnaryOperand::BitwiseNot: return "~";
        case UnaryOperand::PreIncrement: return "++";
        case UnaryOperand::PreDecrement: return "--";
        case UnaryOperand::PostIncrement: return "++";
        case UnaryOperand::PostDecrement: return "--";
        case UnaryOperand::AddressOf: return "&";
        case UnaryOperand::Dereference: return "*";
        default: return "unknown";
        }
    }
}


#endif // NODEUNARYOPERAND_H
