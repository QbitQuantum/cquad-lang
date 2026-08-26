
#ifndef TOKEN_ID_HPP
#define TOKEN_ID_HPP
#pragma once

#include <string>

enum class TokenKind : unsigned short {
    
    /*************************** Токены лексера ***************************/

    // Управляющие символы (0–31, 127)
    Null = '\0',  // 0
    StartOfHeading = 1,     // SOH
    StartOfText = 2,     // STX
    EndOfText = 3,     // ETX
    EndOfTransmission = 4,  // EOT
    Enquiry = 5,     // ENQ
    Acknowledge = 6,     // ACK
    Bell = 7,     // '\a' (BEL)
    Backspace = 8,     // '\b' (BS)
    Tab = 9,     // '\t' (HT)
    LineFeed = 10,    // '\n' (LF)
    VerticalTab = 11,    // '\v' (VT)
    FormFeed = 12,    // '\f' (FF)
    CarriageReturn = 13,    // '\r' (CR)
    ShiftOut = 14,    // SO
    ShiftIn = 15,    // SI
    DataLinkEscape = 16,    // DLE
    DeviceControl1 = 17,    // DC1 (XON)
    DeviceControl2 = 18,    // DC2
    DeviceControl3 = 19,    // DC3 (XOFF)
    DeviceControl4 = 20,    // DC4
    NegativeAcknowledge = 21, // NAK
    SynchronousIdle = 22,   // SYN
    EndOfTransmissionBlock = 23, // ETB
    Cancel = 24,    // CAN
    EndOfMedium = 25,     // EM
    Substitute = 26,     // SUB
    Escape = 27,     // '\e' (ESC)
    FileSeparator = 28,     // FS
    GroupSeparator = 29,    // GS
    RecordSeparator = 30,   // RS
    UnitSeparator = 31,     // US

    // Знаки препинания и символы
    Space = ' ',
    Exclamation = '!',   // !
    Quotation = '"',   // "
    Hash = '#',   // #
    Dollar = '$',   // $
    Percent = '%',   // %
    Ampersand = '&',   // &
    Apostrophe = '\'',  // '
    LeftParen = '(',   // (
    RightParen = ')',   // )
    LeftBrace = '{',   // {
    RightBrace = '}',   // }
    LeftBracket = '[',   // [
    RightBracket = ']',   // ]
    Asterisk = '*',   // *
    Plus = '+',   // +
    Comma = ',',   // ,
    Minus = '-',   // -
    Dot = '.',   // .
    Slash = '/',   // /
    Colon = ':',   // :
    Semicolon = ';',   // ;
    Less = '<',   // <
    Equal = '=',   // =
    Greater = '>',   // >
    Question = '?',   // ?
    At = '@',   // @
    Backslash = '\\',  // /
    Caret = '^',   // ^
    Underscore = '_',   // _
    Backtick = '`',   // `

    Pipe = '|',   // |

    Tilde = '~',   // ~

    Delete = 127,    // DEL
    
    // ===== Специальные токены =====
    FirstSpecialToken = 256,
    neof,          // Конец файла
    Unknown,      // Неизвестный символ

    /*************************** Токены пре-лексера ***************************/
    // ===== Литералы =====
    Literal,
    IntegerLiteral,      // 123, -456, 0, 1000 (целочисленные значения в десятичной системе)
    FloatLiteral,        // литералы типа float: 3.14f, 2.5e10f, -0.5f, 1.0e-5f (обязательный суффикс f/F)
    DoubleLiteral,       // литералы типа double: 3.14, 2.5e10, -0.5, 1.0e-5 (по умолчанию или с суффиксом d/D)
    LongDoubleLiteral,   // литералы типа long double: 3.14l, 2.5e10L, -0.5l (обязательный суффикс l/L)
    CharLiteral,         // 'A', '\n', '#', '9' (одиночные символы в апострофах, включая escape-последовательности)
    WCharLiteral,        // L'A', L'Я', L'字' (wide-символы с префиксом L)
    StringLiteral,       // "text", "Hello World!", "123" (строки в двойных кавычках)
    WStringLiteral,      // L"text", L"Привет", L"中文文本" (wide-строки с префиксом L)
    IdentifierLiteral,   // name, age, counter, myVariable (имена переменных, функций, классов)
    HexLiteral,          // 0xFF, 0x1A3, 0xDEADBEEF (шестнадцатеричные числа с префиксом 0x)
    BinaryLiteral,       // 0b1010, 0b11001100, 0b1 (двоичные числа с префиксом 0b, C++14 и выше)
    TrueLiteral,         // true
    FalseLiteral,        // false
    NullptrLiteral,      // nullptr (нулевой указатель, C++11 и выше)
    
    // Встроенные типы
    Char,
    WChar_t,
    Short,
    Int,
    Long,
    Double,
    Float,
    Bool,
    Void,
    Signed,
    Unsigned,

    // Присваивания
    PlusAssign,  // +=
    MinusAssign, // -=
    MultAssign,  // *=
    DivAssign,   // /=
    ModAssign,   // %=
    AndAssign,   // &=
    OrAssign,    // |=
    XorAssign,   // ^=
    ShlAssign,   // Shl= аналог: <<=
    ShrAssign,   // Shr= аналог: >>=

    // Сравнения
    Equals,      // ==
    NotEqual,    // !=
    LessEqual,   // <=
    GreaterEqual,// >=

    // ===== Управляющие конструкции =====
    If, // if
    Else, // else 
    While, // while
    Do, // do
    For, // for
    Try, // try
    Catch, // catch
    Case, // case
    Return, // return
    Break,       // break
    Continue,    // continue
    Switch,      // switch
    New,         // new
    Delete_,      // delete

    // ===== Объявление типов =====
    Class, // class
    Struct, // struct
    Namespace, // namespace
    Enum, // enum
    Override, // ovveride
    Virtual, // virtual
    Constructor, // constructor
    Destructor, // destructor

    Template, // template
    Typename, // typename

    // ===== квалификаторы =====
    Const, // const

    // ===== Модификаторы =====
    Private, // private
    Protected, // protected
    Public, // public
    Static, // static
    Final, // final
    Auto, // auto
    Operator, // operator

    // ===== Прочее =====
    Property, // __property
    Read,     // read
    Write,     // write
    Delegate,     // delegate
    Var, // var
    Function, // function
    Lambda,     // lambda
    Default,  // default
    Using, // using
    Pointer, // pointer
    // ===== Комментарии =====
    LineComment,    // //
    BlockComment,   // /* */

    // ===== Директивы компилятора =====
    DefineDirective,
    IfDefDirective,
    IfNDefDirective,
    EndIfDirective,
    UndefDirective,
    IfDirective,
    ElseDirective,
    IncludeDirective,

    // ===== Операторы (расширенный) =====
    // Арифметические
    Inc,         // ++ (инкремент)
    Dec,         // -- (декремент)

    // Логические/битовые
    And,         // &&
    Or,          // ||
    Shl,         // Shl аналог: <<
    Shr,         // Shl аналог: >>

    ScResOp,     // ::

    Arrow,     // ->
};

namespace tok
{
    bool static is_unicode_identifier_start(const char32_t& c) {
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '_')
            return true;
        // Unicode символы (выше ASCII)
        return (c >= 0xC0);
    }
    
    // Вспомогательная функция для преобразования enum в char
    constexpr char constexprToChar(TokenKind s) {
        return static_cast<char>(s);
    }

    // Вспомогательная функция для преобразования char в enum
    constexpr TokenKind constexprToTTokenID(char s) {
        return static_cast<TokenKind>(s);
    }

    bool static IsBinaryOperator(TokenKind Kind) {
        switch (Kind)
        {
            // Арифметические
        case TokenKind::Minus:
        case TokenKind::Plus:
        case TokenKind::Asterisk:
        case TokenKind::Slash:
        case TokenKind::Percent:
            // Операции сравнения
        case TokenKind::Equals:
        case TokenKind::NotEqual:
        case TokenKind::Less:
        case TokenKind::Greater:
        case TokenKind::LessEqual:
        case TokenKind::GreaterEqual:
            // <=>	Трёхстороннее сравнение
            // Побитовые (для целых чисел)
        case TokenKind::Ampersand:
        case TokenKind::Pipe:
        case TokenKind::Caret:
        case TokenKind::Shl:
        case TokenKind::Shr:
            // Логические
        case TokenKind::And:
        case TokenKind::Or:
            //Операции присваивания
        case TokenKind::Equal:
        case TokenKind::PlusAssign:
        case TokenKind::MinusAssign:
        case TokenKind::MultAssign:
        case TokenKind::DivAssign:
        case TokenKind::ModAssign:
        case TokenKind::AndAssign:
        case TokenKind::OrAssign:
        case TokenKind::XorAssign:
        case TokenKind::ShlAssign:
        case TokenKind::ShrAssign:
            return true;
        }
        return false;
    }

    bool static IsUnaryOperator(TokenKind Kind) {
        switch (Kind)
        {
        case TokenKind::Minus:
        case TokenKind::Plus:
        case TokenKind::Dec:
        case TokenKind::Inc:
        case TokenKind::Asterisk:
        case TokenKind::Exclamation:
        case TokenKind::Tilde:
            return true;
        }
        return false;
    }

    // Для бинарных операторов
    int static GetBinaryOperatorPriority(TokenKind Kind) {
        switch (Kind) {
            // Уровень 1: Присваивание (самый низкий приоритет)
        case TokenKind::Equal:
        case TokenKind::PlusAssign:
        case TokenKind::MinusAssign:
        case TokenKind::MultAssign:
        case TokenKind::DivAssign:
        case TokenKind::ModAssign:
        case TokenKind::AndAssign:
        case TokenKind::OrAssign:
        case TokenKind::XorAssign:
        case TokenKind::ShlAssign:
        case TokenKind::ShrAssign:
            return 1;

            // Уровень 2: Логическое ИЛИ
        case TokenKind::Or:
            return 2;

            // Уровень 3: Логическое И
        case TokenKind::And:
            return 3;

            // Уровень 4: Побитовое ИЛИ
        case TokenKind::Pipe:
            return 4;

            // Уровень 5: Побитовое XOR
        case TokenKind::Caret:
            return 5;

            // Уровень 6: Побитовое И
        case TokenKind::Ampersand:
            return 6;

            // Уровень 7: Равенство
        case TokenKind::Equals:
        case TokenKind::NotEqual:
            return 7;

            // Уровень 8: Сравнения
        case TokenKind::Less:
        case TokenKind::Greater:
        case TokenKind::LessEqual:
        case TokenKind::GreaterEqual:
            return 8;

            // Уровень 9: Сдвиги
        case TokenKind::Shl:
        case TokenKind::Shr:
            return 9;

            // Уровень 10: Сложение/вычитание
        case TokenKind::Plus:
        case TokenKind::Minus:
            return 10;

            // Уровень 11: Умножение/деление
        case TokenKind::Asterisk:
        case TokenKind::Slash:
        case TokenKind::Percent:
            return 11;

        default:
            return 0; // Нет приоритета
        }
    }

#define GENERATE_NAME(name) \
case TokenKind::name: return #name; \

    std::string getTokenName(TokenKind kind) {

        switch (kind)
        {
            GENERATE_NAME(Space);
            GENERATE_NAME(Exclamation);
            GENERATE_NAME(Quotation);
            GENERATE_NAME(Hash);
            GENERATE_NAME(Dollar);
            GENERATE_NAME(Percent);
            GENERATE_NAME(Ampersand);
            GENERATE_NAME(Apostrophe);
            GENERATE_NAME(LeftParen);
            GENERATE_NAME(RightParen);
            GENERATE_NAME(LeftBrace);
            GENERATE_NAME(RightBrace);
            GENERATE_NAME(LeftBracket);
            GENERATE_NAME(RightBracket);
            GENERATE_NAME(Asterisk);
            GENERATE_NAME(Plus);
            GENERATE_NAME(Comma);
            GENERATE_NAME(Minus);
            GENERATE_NAME(Dot);
            GENERATE_NAME(Slash);
            GENERATE_NAME(Colon);
            GENERATE_NAME(Semicolon);
            GENERATE_NAME(Less);
            GENERATE_NAME(Equal);
            GENERATE_NAME(Greater);
            GENERATE_NAME(Question);
            GENERATE_NAME(At);
            GENERATE_NAME(Backslash);
            GENERATE_NAME(Caret);
            GENERATE_NAME(Underscore);
            GENERATE_NAME(Backtick);
            GENERATE_NAME(Pipe);
            GENERATE_NAME(Tilde);
            GENERATE_NAME(Null);
            GENERATE_NAME(StartOfHeading);
            GENERATE_NAME(StartOfText);
            GENERATE_NAME(EndOfText);
            GENERATE_NAME(EndOfTransmission);
            GENERATE_NAME(Enquiry);
            GENERATE_NAME(Acknowledge);
            GENERATE_NAME(Bell);
            GENERATE_NAME(Backspace);
            GENERATE_NAME(Tab);
            GENERATE_NAME(LineFeed);
            GENERATE_NAME(VerticalTab);
            GENERATE_NAME(FormFeed);
            GENERATE_NAME(CarriageReturn);
            GENERATE_NAME(ShiftOut);
            GENERATE_NAME(ShiftIn);
            GENERATE_NAME(DataLinkEscape);
            GENERATE_NAME(DeviceControl1);
            GENERATE_NAME(DeviceControl2);
            GENERATE_NAME(DeviceControl3);
            GENERATE_NAME(DeviceControl4);
            GENERATE_NAME(NegativeAcknowledge);
            GENERATE_NAME(SynchronousIdle);
            GENERATE_NAME(EndOfTransmissionBlock);
            GENERATE_NAME(Cancel);
            GENERATE_NAME(EndOfMedium);
            GENERATE_NAME(Substitute);
            GENERATE_NAME(Escape);
            GENERATE_NAME(FileSeparator);
            GENERATE_NAME(GroupSeparator);
            GENERATE_NAME(RecordSeparator);
            GENERATE_NAME(UnitSeparator);
            GENERATE_NAME(Delete);
            GENERATE_NAME(FirstSpecialToken);
            GENERATE_NAME(neof);

            GENERATE_NAME(Unknown);

            GENERATE_NAME(Literal);
            GENERATE_NAME(IntegerLiteral);
            GENERATE_NAME(FloatLiteral);
            GENERATE_NAME(DoubleLiteral);
            GENERATE_NAME(LongDoubleLiteral);
            GENERATE_NAME(CharLiteral);
            GENERATE_NAME(WCharLiteral);
            GENERATE_NAME(StringLiteral);
            GENERATE_NAME(WStringLiteral);
            GENERATE_NAME(IdentifierLiteral);
            GENERATE_NAME(HexLiteral);
            GENERATE_NAME(BinaryLiteral);
            GENERATE_NAME(TrueLiteral);
            GENERATE_NAME(FalseLiteral);
            GENERATE_NAME(NullptrLiteral);

            GENERATE_NAME(Char);
            GENERATE_NAME(WChar_t);
            GENERATE_NAME(Short);
            GENERATE_NAME(Int);
            GENERATE_NAME(Long);
            GENERATE_NAME(Double);
            GENERATE_NAME(Float);
            GENERATE_NAME(Void);
            GENERATE_NAME(Signed);
            GENERATE_NAME(Unsigned);

            GENERATE_NAME(PlusAssign);
            GENERATE_NAME(MinusAssign);
            GENERATE_NAME(MultAssign);
            GENERATE_NAME(DivAssign);
            GENERATE_NAME(ModAssign);
            GENERATE_NAME(AndAssign);
            GENERATE_NAME(OrAssign);
            GENERATE_NAME(XorAssign);
            GENERATE_NAME(ShlAssign);
            GENERATE_NAME(ShrAssign);
            GENERATE_NAME(Equals);
            GENERATE_NAME(NotEqual);
            GENERATE_NAME(LessEqual);
            GENERATE_NAME(GreaterEqual);

            GENERATE_NAME(If);
            GENERATE_NAME(Else);
            GENERATE_NAME(While);
            GENERATE_NAME(Do);
            GENERATE_NAME(For);
            GENERATE_NAME(Try);
            GENERATE_NAME(Catch);
            GENERATE_NAME(Case);
            GENERATE_NAME(Return);
            GENERATE_NAME(Break);
            GENERATE_NAME(Continue);
            GENERATE_NAME(Switch);
            GENERATE_NAME(Default);
            GENERATE_NAME(New);
            GENERATE_NAME(Delete_);

            GENERATE_NAME(Class);
            GENERATE_NAME(Struct);
            GENERATE_NAME(Namespace);
            GENERATE_NAME(Enum);
            GENERATE_NAME(Override);
            GENERATE_NAME(Virtual);
            GENERATE_NAME(Constructor);
            GENERATE_NAME(Destructor);

            GENERATE_NAME(Template);
            GENERATE_NAME(Typename);

            GENERATE_NAME(Const);

            GENERATE_NAME(Private);
            GENERATE_NAME(Protected);
            GENERATE_NAME(Public);
            GENERATE_NAME(Static);
            GENERATE_NAME(Final);
            GENERATE_NAME(Auto);
            GENERATE_NAME(Operator);

            GENERATE_NAME(Property);
            GENERATE_NAME(Read);
            GENERATE_NAME(Write);
            GENERATE_NAME(Delegate);
            GENERATE_NAME(Var);
            GENERATE_NAME(Function);
            GENERATE_NAME(Lambda);
            GENERATE_NAME(Using);
            GENERATE_NAME(Pointer);

            GENERATE_NAME(LineComment);
            GENERATE_NAME(BlockComment);

            GENERATE_NAME(DefineDirective);
            GENERATE_NAME(IfDefDirective);
            GENERATE_NAME(IfNDefDirective);
            GENERATE_NAME(EndIfDirective);
            GENERATE_NAME(UndefDirective);
            GENERATE_NAME(IfDirective);
            GENERATE_NAME(ElseDirective);
            GENERATE_NAME(IncludeDirective);

            GENERATE_NAME(Inc);
            GENERATE_NAME(Dec);
            GENERATE_NAME(And);
            GENERATE_NAME(Or);
            GENERATE_NAME(Shl);
            GENERATE_NAME(Shr);
            GENERATE_NAME(ScResOp);
            GENERATE_NAME(Arrow);
        default:
            return "Unknow Token Name";
        }
    }
}

struct Token {
    TokenKind type;
    std::string value;
    size_t line = 0;
    size_t column = 0;
    
    bool operator == (const Token& other) const {
        return type == other.type;
    };
};
#endif // TOKEN_ID_HPP
