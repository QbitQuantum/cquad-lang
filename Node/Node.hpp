#ifndef NODE_HPP
#define NODE_HPP
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <NodeBinaryOperand.h>
#include <NodeUnaryOperand.h>

class Node
{
public:
    enum class EDeclType {
        None = -1,
        Identifier,
        Type,
        Declaration,
        DeclarationList,
        VarDeclarationList,
        TemplateVarDeclarationList,
        MemberCall,
        Scope,
        Using,
        PointerSignature,
        Pointer,
        ParameterList,
        Function,
        FunctionTemplate,
        Lambda,
        Constructor,
        Destructor,
        New,
        Delete,
        Call,
        Block,
        TemplateParameterDeclarationList,
        TemplateParameterInstantiationList,
        BlockClass,
        BaseClass,
        Class,
        ClassTemplate,
        BlockStruct,
        Struct,
        PropertyBlock,
        Property,
        Namespace,
        Integer,
        Floating,
        String,
        Character,
        Boolean,
        Nullptr,
        Default,
        While,
        TryCatch,
        BinaryOp,
        UnaryOp,
        Condition,
        If,
        Else,
        Return,
        InitializerList,
        Template,
        TemplateParameterList,
        TemplateTypeParam,
        TemplateValueParam,
    };

protected:
    Node() {}
    explicit Node(EDeclType declType) : DeclType(declType) {}
public:
    const EDeclType DeclType = EDeclType::None;
    virtual std::string print() = 0;
    virtual ~Node() = default;
};

class NodeIdentifier : public Node
{
    Node* TemplateArgs = nullptr;
    std::string Name;
    Node* Scope = nullptr;
public:
    NodeIdentifier(Node* templateArgs, const std::string& name, Node* scope)
        : Node(EDeclType::Identifier),
        TemplateArgs(templateArgs), Name(name), Scope(scope) {
    }

    std::string print() override {
        std::string out = (Scope ? Scope->print() : "") + Name;
        if (TemplateArgs) out += TemplateArgs->print();
        return out;
    }

    ~NodeIdentifier() override {
        delete TemplateArgs;
        delete Scope;
    }
};

class NodeType : public Node
{
public:
    enum class EType { None, Pointer, Ref, RValue };

    Node* Type = nullptr;
    Node* SizeArgCArray = nullptr;
    bool IsConst = false;
    EType Kind = EType::None;
    bool IsAuto = false;

    std::string getSymbol() const {
        switch (Kind) {
        case EType::Pointer: return "*";
        case EType::Ref:     return "&";
        case EType::RValue:  return "&&";
        default:             return "";
        }
    }

    NodeType(Node* type, Node* sizeArgCArray, bool isConst, EType kind, bool isAuto)
        : Node(EDeclType::Type),
        Type(type), SizeArgCArray(sizeArgCArray),
        IsConst(isConst), Kind(kind), IsAuto(isAuto) {
    }

    std::string print() override {
        if (IsAuto) return "auto";
        if (!Type) return "";
        return (IsConst ? "const " : "") + Type->print() +
            (SizeArgCArray ? "[" + SizeArgCArray->print() + "]" : "") +
            getSymbol();
    }

    ~NodeType() override {
        delete Type;
        delete SizeArgCArray;
    }
};

class NodeDeclaration : public Node
{
    Node* Identifier = nullptr;
    Node* Initializer = nullptr;
    int InitKind = -1;
public:
    std::string print() override {
        if (!Identifier) return "";
        std::string out = Identifier->print();

        switch (InitKind) {
        case 0: break;                                                  // Default
        case 1: out += "{  }"; break;                                   // Value
        case 2: out += " = " + (Initializer ? Initializer->print() : ""); break; // Copy
        case 3: out += (Initializer ? Initializer->print() : ""); break;         // DirectList
        case 4: out += " = " + (Initializer ? Initializer->print() : ""); break; // CopyList
        default:
            if (Initializer) out += " = " + Initializer->print();
            break;
        }
        return out;
    }

    NodeDeclaration(Node* identifier, Node* initializer, int initKind = -1)
        : Node(EDeclType::Declaration),
        Identifier(identifier), Initializer(initializer), InitKind(initKind) {
    }

    ~NodeDeclaration() override {
        delete Identifier;
        delete Initializer;
    }
};

class NodeDeclarationList : public Node
{
    std::vector<Node*> DeclarationList;
public:
    std::string print() override {
        std::string out;
        size_t size = DeclarationList.size();
        for (size_t i = 0; i < size; ++i)
            if (auto decl = DeclarationList[i]; decl)
                out += decl->print() + (i + 1 == size ? "" : ", ");
        return out;
    }

    NodeDeclarationList(const std::vector<Node*>& list)
        : Node(EDeclType::DeclarationList), DeclarationList(list) {
    }

    ~NodeDeclarationList() override {
        for (auto* d : DeclarationList) delete d;
    }
};

class NodeVarDeclarationList : public Node
{
    Node* Type = nullptr;
    Node* DeclarationList = nullptr;
public:
    std::string print() override {
        if (!Type) return "";
        std::string out = Type->print();
        if (DeclarationList) {
            std::string decl = DeclarationList->print();
            if (!decl.empty()) out += " " + decl;
        }
        return out;
    }

    NodeVarDeclarationList(Node* type, Node* declarationList)
        : Node(EDeclType::VarDeclarationList),
        Type(type), DeclarationList(declarationList) {
    }

    ~NodeVarDeclarationList() override {
        delete Type;
        delete DeclarationList;
    }
};

class NodeVarDeclarationListTemplate : public Node
{
    Node* Template = nullptr;
    Node* VarDeclarationList = nullptr;
public:
    NodeVarDeclarationListTemplate(Node* tmpl, Node* varDeclList)
        : Node(EDeclType::TemplateVarDeclarationList),
        Template(tmpl), VarDeclarationList(varDeclList) {
    }

    std::string print() override {
        if (!Template || !VarDeclarationList) return "";
        return Template->print() + "\n" + VarDeclarationList->print();
    }

    ~NodeVarDeclarationListTemplate() override {
        delete Template;
        delete VarDeclarationList;
    }
};

class NodeMemberCall : public Node
{
    Node* Object = nullptr;
    Node* Member = nullptr;
    bool IsArrow = false;
public:
    std::string print() override {
        if (!Object || !Member) return "";
        return Object->print() + (IsArrow ? "->" : ".") + Member->print();
    }

    NodeMemberCall(Node* object, Node* member, bool isArrow)
        : Node(EDeclType::MemberCall),
        Object(object), Member(member), IsArrow(isArrow) {
    }

    ~NodeMemberCall() override {
        delete Object;
        delete Member;
    }
};

class NodeScope : public Node
{
    std::vector<std::string> Scope;
public:
    NodeScope(const std::vector<std::string>& scope)
        : Node(EDeclType::Scope), Scope(scope) {
    }

    std::string print() override {
        std::string out;
        for (auto& s : Scope) out += s + "::";
        return out;
    }
};

class NodeUsing : public Node
{
    Node* Name = nullptr;
    Node* ScopeType = nullptr;
    bool Simple = false;
public:
    NodeUsing(Node* name, Node* scopeType)
        : Node(EDeclType::Using), Name(name), ScopeType(scopeType) {
    }

    std::string print() override {
        if (!Name) return "";
        std::string out = "using " + Name->print();
        if (ScopeType) out += " = " + ScopeType->print();
        return out;
    }

    ~NodeUsing() override {
        delete Name;
        delete ScopeType;
    }
};

class NodePointerSignature : public Node
{
    Node* ReturnType = nullptr;
    Node* ParameterList = nullptr;
public:
    NodePointerSignature(Node* returnType, Node* parameterList)
        : Node(EDeclType::PointerSignature),
        ReturnType(returnType), ParameterList(parameterList) {
    }

    std::string print() override {
        if (!ReturnType || !ParameterList) return "";
        return ReturnType->print() + ParameterList->print();
    }

    ~NodePointerSignature() override {
        delete ReturnType;
        delete ParameterList;
    }
};

class NodePointer : public Node
{
    Node* Name = nullptr;
    Node* Declaration = nullptr;
public:
    NodePointer(Node* name, Node* declaration)
        : Node(EDeclType::Pointer), Name(name), Declaration(declaration) {
    }

    std::string print() override {
        if (!Name || !Declaration) return "";
        return "pointer " + Name->print() + " = " + Declaration->print();
    }

    ~NodePointer() override {
        delete Name;
        delete Declaration;
    }
};

class NodeParameterList : public Node
{
    std::vector<Node*> Params;
public:
    NodeParameterList(const std::vector<Node*>& params)
        : Node(EDeclType::ParameterList), Params(params) {
    }

    std::string print() override {
        std::string out = "(";
        size_t size = Params.size();
        for (size_t i = 0; i < size; ++i)
            if (auto p = Params[i]; p)
                out += p->print() + (i + 1 == size ? "" : ", ");
        out += ")";
        return out;
    }

    ~NodeParameterList() override {
        for (auto* p : Params) delete p;
    }
};

class NodeFunction : public Node
{
    Node* Type = nullptr;
    Node* Identifier = nullptr;
    Node* ParameterList = nullptr;
    Node* Body = nullptr;
public:
    NodeFunction(Node* type, Node* name, Node* parameterList, Node* body)
        : Node(EDeclType::Function),
        Type(type), Identifier(name),
        ParameterList(parameterList), Body(body) {
    }

    std::string print() override {
        if (!Type || !Identifier || !ParameterList) return "";
        std::string out = Type->print() + " " + Identifier->print() + ParameterList->print();
        if (Body) out += Body->print();
        return out;
    }

    ~NodeFunction() override {
        delete Type;
        delete Identifier;
        delete ParameterList;
        delete Body;
    }
};

class NodeFunctionTemplate : public Node
{
    Node* Template = nullptr;
    Node* Function = nullptr;
public:
    NodeFunctionTemplate(Node* tmpl, Node* function)
        : Node(EDeclType::FunctionTemplate),
        Template(tmpl), Function(function) {
    }

    std::string print() override {
        if (!Template || !Function) return "";
        return Template->print() + "\n" + Function->print();
    }

    ~NodeFunctionTemplate() override {
        delete Template;
        delete Function;
    }
};

class NodeLambda : public Node
{
    Node* Type = nullptr;
    Node* Identifier = nullptr;
    Node* ParameterList = nullptr;
    Node* Body = nullptr;
public:
    NodeLambda(Node* type, Node* name, Node* parameterList, Node* body)
        : Node(EDeclType::Lambda),
        Type(type), Identifier(name),
        ParameterList(parameterList), Body(body) {
    }

    std::string print() override {
        if (!Type || !Identifier || !ParameterList) return "";
        std::string out = "lambda " + Type->print() + " " +
            Identifier->print() + ParameterList->print();
        if (Body) out += Body->print();
        return out;
    }

    ~NodeLambda() override {
        delete Type;
        delete Identifier;
        delete ParameterList;
        delete Body;
    }
};

class NodeConstructor : public Node
{
    Node* ParameterList = nullptr;
    Node* Body = nullptr;
public:
    NodeConstructor(Node* parameterList, Node* body)
        : Node(EDeclType::Constructor),
        ParameterList(parameterList), Body(body) {
    }

    std::string print() override {
        if (!ParameterList) return "";
        std::string out = "constructor" + ParameterList->print();
        if (Body) out += Body->print();
        return out;
    }

    ~NodeConstructor() override {
        delete ParameterList;
        delete Body;
    }
};

class NodeDestructor : public Node
{
    Node* ParameterList = nullptr;
    Node* Body = nullptr;
public:
    NodeDestructor(Node* parameterList, Node* body)
        : Node(EDeclType::Destructor),
        ParameterList(parameterList), Body(body) {
    }

    std::string print() override {
        if (!ParameterList) return "";
        std::string out = "destructor" + ParameterList->print();
        if (Body) out += Body->print();
        return out;
    }

    ~NodeDestructor() override {
        delete ParameterList;
        delete Body;
    }
};

class NodeNew : public Node
{
    Node* Expression = nullptr;
    Node* SizeArgCArray = nullptr;
public:
    NodeNew(Node* expression, Node* sizeArgCArray)
        : Node(EDeclType::New),
        Expression(expression), SizeArgCArray(sizeArgCArray) {
    }

    std::string print() override {
        if (!Expression) return "";
        return "new " + (SizeArgCArray ? "[" + SizeArgCArray->print() + "] " : " ") +
            Expression->print();
    }

    ~NodeNew() override {
        delete Expression;
        delete SizeArgCArray;
    }
};

class NodeDelete : public Node
{
    Node* Expression = nullptr;
    Node* SizeArgCArray = nullptr;
public:
    NodeDelete(Node* expression, Node* sizeArgCArray)
        : Node(EDeclType::Delete),
        Expression(expression), SizeArgCArray(sizeArgCArray) {
    }

    std::string print() override {
        if (!Expression) return "";
        return "delete " + (SizeArgCArray ? "[" + SizeArgCArray->print() + "] " : " ") +
            Expression->print();
    }

    ~NodeDelete() override {
        delete Expression;
        delete SizeArgCArray;
    }
};

class NodeCall : public Node
{
    Node* Name = nullptr;
    std::vector<Node*> Args;
public:
    NodeCall(Node* name, const std::vector<Node*>& args)
        : Node(EDeclType::Call), Name(name), Args(args) {
    }

    std::string print() override {
        if (!Name) return "";
        std::string out = Name->print() + "(";
        size_t size = Args.size();
        for (size_t i = 0; i < size; ++i)
            if (auto a = Args[i]; a)
                out += a->print() + (i + 1 == size ? "" : ", ");
        out += ")";
        return out;
    }

    ~NodeCall() override {
        delete Name;
        for (auto* a : Args) delete a;
    }
};

class NodeBlock : public Node
{
    std::vector<Node*> Statements;
public:
    NodeBlock() : Node(EDeclType::Block) {}

    void add(Node* stmt) {
        if (stmt) Statements.push_back(stmt);
    }

    std::string print() override {
        std::string out = "{\n";
        for (auto* stmt : Statements)
            if (stmt) out += " " + stmt->print() + ";\n";
        out += "}";
        return out;
    }

    ~NodeBlock() override {
        for (auto* stmt : Statements) delete stmt;
    }
};

class NodeTemplateParameterDeclarationList : public Node
{
    std::vector<Node*> Params;
public:
    NodeTemplateParameterDeclarationList(std::vector<Node*> params)
        : Node(EDeclType::TemplateParameterDeclarationList),
        Params(std::move(params)) {
    }

    std::string print() override {
        std::string out = "<";
        for (size_t i = 0; i < Params.size(); ++i) {
            out += Params[i]->print();
            if (i + 1 < Params.size()) out += ", ";
        }
        out += ">";
        return out;
    }

    ~NodeTemplateParameterDeclarationList() override {
        for (auto* p : Params) delete p;
    }
};

class NodeTemplateParameterInstantiationList : public Node
{
    std::vector<Node*> Params;
public:
    NodeTemplateParameterInstantiationList(std::vector<Node*> params)
        : Node(EDeclType::TemplateParameterInstantiationList),
        Params(std::move(params)) {
    }

    std::string print() override {
        std::string out = "<";
        for (size_t i = 0; i < Params.size(); ++i) {
            out += Params[i]->print();
            if (i + 1 < Params.size()) out += ", ";
        }
        out += ">";
        return out;
    }

    ~NodeTemplateParameterInstantiationList() override {
        for (auto* p : Params) delete p;
    }
};

class NodeBlockClass : public Node
{
public:
    enum class FieldType { None, Public, Private, Static };
private:
    std::vector<std::pair<FieldType, std::vector<Node*>>> FieldStatements;

    std::string getSymbol(FieldType type) {
        switch (type) {
        case FieldType::None:    return "";
        case FieldType::Public:  return "public:";
        case FieldType::Private: return "private:";
        case FieldType::Static:  return "static:";
        }
        return "";
    }
public:
    NodeBlockClass(std::vector<std::pair<FieldType, std::vector<Node*>>> fields)
        : Node(EDeclType::BlockClass), FieldStatements(std::move(fields)) {
    }

    std::string print() override {
        std::string out = "{\n";
        for (auto& [type, stmts] : FieldStatements) {
            out += getSymbol(type) + "\n";
            for (auto* field : stmts) out += field->print() + "\n";
        }
        out += "}";
        return out;
    }

    ~NodeBlockClass() override {
        for (auto& [type, stmts] : FieldStatements)
            for (auto* field : stmts) delete field;
    }
};

class NodeBaseClass : public Node
{
public:
    enum class InheritanceType { None, Public, Private };
private:
    Node* Identifier = nullptr;
    InheritanceType Type = InheritanceType::None;

    std::string getSymbol() const {
        switch (Type) {
        case InheritanceType::None:    return "";
        case InheritanceType::Public:  return "public";
        case InheritanceType::Private: return "private";
        }
        return "";
    }
public:
    NodeBaseClass(Node* identifier, InheritanceType type = InheritanceType::None)
        : Node(EDeclType::BaseClass), Identifier(identifier), Type(type) {
    }

    std::string print() override {
        if (!Identifier) return "";
        std::string sym = getSymbol();
        return (sym.empty() ? "" : sym + " ") + Identifier->print();
    }

    ~NodeBaseClass() override {
        delete Identifier;
    }
};

class NodeClass : public Node
{
    Node* Identifier = nullptr;
    Node* TemplateParameterDeclarationList = nullptr;
    Node* BaseClass = nullptr;
    Node* Body = nullptr;
public:
    NodeClass(Node* identifier,
        Node* templateParameterDeclarationList,
        Node* baseClass,
        Node* body)
        : Node(EDeclType::Class),
        Identifier(identifier),
        TemplateParameterDeclarationList(templateParameterDeclarationList),
        BaseClass(baseClass), Body(body) {
    }

    std::string print() override {
        if (!Identifier) return "";
        std::string out = "class";
        if (TemplateParameterDeclarationList) out += TemplateParameterDeclarationList->print();
        out += " " + Identifier->print();
        if (BaseClass) out += " : " + BaseClass->print();
        if (Body) out += " " + Body->print();
        return out;
    }

    ~NodeClass() override {
        delete Identifier;
        delete TemplateParameterDeclarationList;
        delete BaseClass;
        delete Body;
    }
};

class NodeClassTemplate : public Node
{
    Node* Template = nullptr;
    Node* Class = nullptr;
public:
    NodeClassTemplate(Node* tmpl, Node* cls)
        : Node(EDeclType::ClassTemplate), Template(tmpl), Class(cls) {
    }

    std::string print() override {
        if (!Template || !Class) return "";
        return Template->print() + "\n" + Class->print();
    }

    ~NodeClassTemplate() override {
        delete Template;
        delete Class;
    }
};

class NodeBlockStruct : public Node
{
public:
    enum class FieldType { None, Public, Static };
private:
    std::vector<std::pair<FieldType, std::vector<Node*>>> FieldStatements;

    std::string getSymbol(FieldType type) {
        switch (type) {
        case FieldType::None:   return "";
        case FieldType::Public: return "public:";
        case FieldType::Static: return "static:";
        }
        return "";
    }
public:
    NodeBlockStruct(std::vector<std::pair<FieldType, std::vector<Node*>>> fields)
        : Node(EDeclType::BlockStruct), FieldStatements(std::move(fields)) {
    }

    std::string print() override {
        std::string out = "{\n";
        for (auto& [type, stmts] : FieldStatements) {
            out += getSymbol(type) + "\n";
            for (auto* field : stmts) out += field->print() + "\n";
        }
        out += "}";
        return out;
    }

    ~NodeBlockStruct() override {
        for (auto& [type, stmts] : FieldStatements)
            for (auto* field : stmts) delete field;
    }
};

class NodeStruct : public Node
{
    Node* Identifier = nullptr;
    Node* TemplateParameterDeclarationList = nullptr;
    Node* Body = nullptr;
public:
    NodeStruct(Node* identifier, Node* generics, Node* body)
        : Node(EDeclType::Struct),
        Identifier(identifier),
        TemplateParameterDeclarationList(generics),
        Body(body) {
    }

    std::string print() override {
        if (!Identifier) return "";
        std::string out = "struct";
        if (TemplateParameterDeclarationList) out += TemplateParameterDeclarationList->print();
        out += " " + Identifier->print();
        if (Body) out += " " + Body->print();
        return out;
    }

    ~NodeStruct() override {
        delete Identifier;
        delete TemplateParameterDeclarationList;
        delete Body;
    }
};

class NodePropertyBlock : public Node
{
    Node* Getter = nullptr;
    Node* Setter = nullptr;
public:
    NodePropertyBlock(Node* getter, Node* setter)
        : Node(EDeclType::PropertyBlock), Getter(getter), Setter(setter) {
    }

    std::string print() override {
        return "{\n" +
            (Getter ? "read = " + Getter->print() : "") +
            (Getter && Setter ? ",\n" : "") +
            (Setter ? "write = " + Setter->print() : "") +
            "\n}";
    }

    ~NodePropertyBlock() override {
        delete Getter;
        delete Setter;
    }
};

class NodeProperty : public Node
{
    Node* Name = nullptr;
    Node* Type = nullptr;
    Node* Block = nullptr;
public:
    NodeProperty(Node* name, Node* type, Node* block)
        : Node(EDeclType::Property),
        Name(name), Type(type), Block(block) {
    }

    std::string print() override {
        if (!Name || !Type || !Block) return "";
        return "__property " + Type->print() + " " + Name->print() + " " + Block->print() + ";";
    }

    ~NodeProperty() override {
        delete Name;
        delete Type;
        delete Block;
    }
};

class NodeNamespace : public Node
{
    Node* Name = nullptr;
    Node* Body = nullptr;
public:
    NodeNamespace(Node* name, Node* body)
        : Node(EDeclType::Namespace), Name(name), Body(body) {
    }

    std::string print() override {
        if (!Name) return "";
        std::string out = "namespace " + Name->print();
        if (Body) out += " " + Body->print();
        return out;
    }

    ~NodeNamespace() override {
        delete Name;
        delete Body;
    }
};

class NodeInteger : public Node
{
    std::string Raw;
public:
    NodeInteger(const std::string& val)
        : Node(EDeclType::Integer), Raw(val) {
    }

    std::string print() override { return Raw; }
};

class NodeFloating : public Node
{
    std::string Raw;
public:
    NodeFloating(const std::string& val)
        : Node(EDeclType::Floating), Raw(val) {
    }

    std::string print() override { return Raw; }
};

class NodeString : public Node
{
    std::string Raw;
public:
    NodeString(const std::string& val)
        : Node(EDeclType::String), Raw(val) {
    }

    std::string print() override { return "\"" + Raw + "\""; }
};

class NodeCharacter : public Node
{
    std::string Raw;
public:
    NodeCharacter(const std::string& val)
        : Node(EDeclType::Character), Raw(val) {
    }

    std::string print() override {
        return "'" + std::string(1, Raw.empty() ? '\0' : Raw[0]) + "'";
    }
};

class NodeBoolean : public Node
{
    std::string Raw;
public:
    NodeBoolean(const std::string& val)
        : Node(EDeclType::Boolean), Raw(val) {
    }

    std::string print() override { return Raw; }
};

class NodeNullptr : public Node
{
public:
    NodeNullptr() : Node(EDeclType::Nullptr) {}
    std::string print() override { return "nullptr"; }
};

class NodeDefault : public Node
{
public:
    NodeDefault() : Node(EDeclType::Default) {}
    std::string print() override { return "default"; }
};

class NodeWhile : public Node
{
    Node* Condition = nullptr;
    Node* Body = nullptr;
    bool IsDoWhile = false;
public:
    NodeWhile(Node* condition, Node* body, bool isDoWhile)
        : Node(EDeclType::While),
        Condition(condition), Body(body), IsDoWhile(isDoWhile) {
    }

    std::string print() override {
        if (!Condition || !Body) return "";
        std::string out = "while (" + Condition->print() + ") ";
        if (IsDoWhile) out += "do ";
        out += Body->print();
        return out;
    }

    ~NodeWhile() override {
        delete Condition;
        delete Body;
    }
};

class NodeTryCatch : public Node
{
    Node* BodyTry = nullptr;
    Node* BodyCatch = nullptr;
    Node* Declaration = nullptr;
public:
    NodeTryCatch(Node* bodyTry, Node* bodyCatch, Node* declaration)
        : Node(EDeclType::TryCatch),
        BodyTry(bodyTry), BodyCatch(bodyCatch), Declaration(declaration) {
    }

    std::string print() override {
        if (!BodyTry) return "BodyTry empty";
        std::string out = "try " + BodyTry->print();
        if (BodyCatch)
            out += "\ncatch " + (Declaration ? "(" + Declaration->print() + ")" : "") +
            BodyCatch->print();
        return out;
    }

    ~NodeTryCatch() override {
        delete BodyTry;
        delete BodyCatch;
        delete Declaration;
    }
};

class NodeBinaryOp : public Node
{
    BinaryOperand Operand = BinaryOperand::Unknown;
    Node* Left = nullptr;
    Node* Right = nullptr;
public:
    NodeBinaryOp(const BinaryOperand& operand, Node* left, Node* right)
        : Node(EDeclType::BinaryOp),
        Operand(operand), Left(left), Right(right) {
    }

    std::string print() override {
        if (!Left || !Right || Operand == BinaryOperand::Unknown) return "";
        return Left->print() + " " + BinOparand::opToString(Operand) + " " + Right->print();
    }

    ~NodeBinaryOp() override {
        delete Left;
        delete Right;
    }
};

class NodeUnaryOp : public Node
{
    UnaryOperand Operand = UnaryOperand::Unknown;
    Node* Right = nullptr;
public:
    NodeUnaryOp(const UnaryOperand& operand, Node* right)
        : Node(EDeclType::UnaryOp), Operand(operand), Right(right) {
    }

    std::string print() override {
        if (!Right) return "";
        return UnOparand::opToString(Operand) + Right->print();
    }

    ~NodeUnaryOp() override {
        delete Right;
    }
};

class NodeCondition : public Node
{
    Node* If = nullptr;
    Node* Else = nullptr;
public:
    NodeCondition(Node* ifNode, Node* elseNode)
        : Node(EDeclType::Condition), If(ifNode), Else(elseNode) {
    }

    std::string print() override {
        if (!If) return "";
        std::string out = If->print();
        if (Else) out += Else->print();
        return out;
    }

    ~NodeCondition() override {
        delete If;
        delete Else;
    }
};

class NodeIf : public Node
{
    Node* Condition = nullptr;
    Node* Body = nullptr;
public:
    NodeIf(Node* cond, Node* body)
        : Node(EDeclType::If), Condition(cond), Body(body) {
    }

    std::string print() override {
        if (!Condition || !Body) return "";
        return "if (" + Condition->print() + ")" + Body->print();
    }

    ~NodeIf() override {
        delete Condition;
        delete Body;
    }
};

class NodeElse : public Node
{
    Node* Body = nullptr;
public:
    NodeElse(Node* body)
        : Node(EDeclType::Else), Body(body) {
    }

    std::string print() override {
        return Body ? Body->print() : "";
    }

    ~NodeElse() override {
        delete Body;
    }
};

class NodeReturn : public Node
{
    Node* Expression = nullptr;
public:
    explicit NodeReturn(Node* expression)
        : Node(EDeclType::Return), Expression(expression) {
    }

    std::string print() override {
        return "return " + (Expression ? Expression->print() : "");
    }

    ~NodeReturn() override {
        delete Expression;
    }
};

class NodeInitializerList : public Node
{
    std::vector<Node*> Elements;
public:
    NodeInitializerList(const std::vector<Node*>& elements)
        : Node(EDeclType::InitializerList), Elements(elements) {
    }

    std::string print() override {
        std::string out = "{ ";
        size_t size = Elements.size();
        for (size_t i = 0; i < size; ++i) {
            if (auto* e = Elements[i]; e) {
                out += e->print();
                if (i + 1 != size) out += ", ";
            }
        }
        out += " }";
        return out;
    }

    const std::vector<Node*>& getElements() const { return Elements; }

    ~NodeInitializerList() override {
        for (auto* e : Elements) delete e;
    }
};

class NodeTemplate : public Node
{
    Node* Params;
public:
    explicit NodeTemplate(Node* params)
        : Node(EDeclType::Template), Params(params) {
    }

    std::string print() override {
        if (!Params) return "";
        return "template" + Params->print();
    }

    ~NodeTemplate() override {
        delete Params;
    }
};

class NodeTemplateParameterList : public Node
{
    std::vector<Node*> Params;
public:
    explicit NodeTemplateParameterList(std::vector<Node*> params)
        : Node(EDeclType::TemplateParameterList), Params(std::move(params)) {
    }

    std::string print() override {
        std::string out = "<";
        size_t size = Params.size();
        for (size_t i = 0; i < size; ++i) {
            if (auto* p = Params[i]; p) {
                out += p->print();
                if (i + 1 != size) out += ", ";
            }
        }
        out += ">";
        return out;
    }

    const std::vector<Node*>& getParams() const noexcept { return Params; }

    ~NodeTemplateParameterList() override {
        for (auto* p : Params) delete p;
    }
};

class NodeTemplateTypeParam : public Node
{
    Node* Name = nullptr;
    Node* DefaultArg = nullptr;
public:
    NodeTemplateTypeParam(Node* name, Node* defaultArg)
        : Node(EDeclType::TemplateTypeParam),
        Name(name), DefaultArg(defaultArg) {
    }

    std::string print() override {
        if (!Name) return "";
        std::string out = "typename " + Name->print();
        if (DefaultArg) out += " = " + DefaultArg->print();
        return out;
    }

    ~NodeTemplateTypeParam() override {
        delete Name;
        delete DefaultArg;
    }
};

class NodeTemplateValueParam : public Node
{
    Node* Type = nullptr;
    Node* Name = nullptr;
    Node* DefaultArg = nullptr;
public:
    NodeTemplateValueParam(Node* type, Node* name, Node* defaultArg)
        : Node(EDeclType::TemplateValueParam),
        Type(type), Name(name), DefaultArg(defaultArg) {
    }

    std::string print() override {
        if (!Type) return "";
        std::string out = Type->print();
        if (Name) out += Name->print();
        if (DefaultArg) out += " = " + DefaultArg->print();
        return out;
    }

    ~NodeTemplateValueParam() override {
        delete Type;
        delete Name;
        delete DefaultArg;
    }
};

#endif // NODE_HPP