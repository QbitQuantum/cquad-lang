
#ifndef NODE_HPP
#define NODE_HPP
#pragma once

#include <string>
#include <memory>
#include <vector>

class Node
{
protected:
    int NodeType = -1;
    Node() {};
public:
    virtual std::string print() = 0;
    virtual ~Node() = default;
};

class NodeIdentifier : public Node
{
    Node* TemplateParameterInstantiationList = nullptr;
    std::string Name = "";
    Node* Scope = nullptr;
public:
    NodeIdentifier(Node* templateParameterInstantiationList, const std::string& name, Node* scope) :
        TemplateParameterInstantiationList(templateParameterInstantiationList), Name(name), Scope(scope) {
    };

    std::string print() override {
        std::string fprint = (Scope ? Scope->print() : "") + Name;
        if (TemplateParameterInstantiationList) return fprint += TemplateParameterInstantiationList->print();
        return fprint;
    };
    ~NodeIdentifier() override {
        delete Scope;
    };
};

class NodeType : public Node
{
public:
    enum class EType 
    {
        NONE, POINTER, REF, RVALUE
    };
    Node* Type = nullptr;
    Node* SizeArgCArray = nullptr;
    bool IsConst = false;
    EType eType = EType::NONE;
    std::string getSymbol() const {
        switch (eType)
        {
        case NodeType::EType::POINTER: return "*";
        case NodeType::EType::REF: return "&";
        case NodeType::EType::RVALUE: return "&&";
        default: return "";
        }
    }
public:
    NodeType(Node* type, Node* sizeArgCArray, bool isConst, EType etype) :
        Type(type), SizeArgCArray(sizeArgCArray), IsConst(isConst), eType(etype) {};

    std::string print() override {
        
        if (!Type) return "";
        std::string fprint = "[" + 
            (IsConst ? std::string("const ") : std::string("")) + 
            Type->print() + (SizeArgCArray ? "[" + SizeArgCArray->print() + "]" : "") +
            getSymbol() + "]";
        return fprint;
    };

    ~NodeType() {
        delete Type;
        delete SizeArgCArray;
    }
};

class NodeDeclaration : public Node
{
    Node* Identifier = nullptr;
    Node* Initializer = nullptr;
public:
    std::string print() override {
        if (!Identifier) return "";
        std::string fprint = Identifier->print();
        if (Initializer)
            fprint += " = " + Initializer->print();
        return fprint;
    };
    NodeDeclaration(Node* identifier, Node* initializer) :
        Identifier(identifier), Initializer(initializer) {
    };

    ~NodeDeclaration() override {
        delete Identifier; Identifier = nullptr;
        delete Initializer; Initializer = nullptr;
    };
};

class NodeDeclarationList : public Node
{
    std::vector<Node*> DeclarationList;
public:
    std::string print() override {
        std::string fprint;
        int size = DeclarationList.size();
        for (size_t i = 0; i < size; i++)
            if (auto Decl = DeclarationList[i]; Decl)
                fprint += Decl->print() + (i == size - 1 ? "" : ", ");
        return fprint;
    };
    NodeDeclarationList(const std::vector<Node*>& declarationList) : DeclarationList(declarationList) { };

    ~NodeDeclarationList() override {
        for (auto& decl : DeclarationList) {
            delete decl;
        }
    };
};

class NodeVarDeclarationList : public Node
{
    Node* TemplateParameterDeclarationList = nullptr;
    Node* Type = nullptr;
    Node* DeclarationList = nullptr;
public:
    std::string print() override {
        if (!Type) return "";
        std::string fprint = "var";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += Type->print();
        if (DeclarationList)
        {
            std::string Declaration = DeclarationList->print();
            fprint += Declaration.empty() ? "" : " " + DeclarationList->print();
        }
        return fprint;
    };
    NodeVarDeclarationList(Node* TemplateParameterDeclarationList, Node* type, Node* declarationList) :
        TemplateParameterDeclarationList(TemplateParameterDeclarationList), Type(type), DeclarationList(declarationList) {
    };

    ~NodeVarDeclarationList() override {
        delete TemplateParameterDeclarationList;
        delete Type;
        delete DeclarationList;
    };
};

class NodeMemberCall : public Node
{
    Node* Identifier = nullptr;
    Node* CallExpr = nullptr;
    bool IsArrow = false;

public:
    std::string print() override {
        std::string fprint = Identifier->print() + 
            (IsArrow ? "->" : + ".") + CallExpr->print();
        return fprint;
    };

    NodeMemberCall(Node* identifier, Node* callExpr, bool isArrow) :
        Identifier(identifier), CallExpr(callExpr), IsArrow(isArrow) {
    };

    ~NodeMemberCall() override {
        delete Identifier; Identifier = nullptr;
        delete CallExpr; CallExpr = nullptr;
    };
};

class NodeScope : public Node
{
    std::vector<std::string> Scope;
public:
    NodeScope(const std::vector<std::string>& scope) :
        Scope(scope) {
    };
    std::string print() override { 
        std::string fprint = "";
        int size = Scope.size();
        for (size_t i = 0; i < size; i++)
            fprint += Scope[i] + "::";
        return fprint;
    
    };
    ~NodeScope() {
    };
};

class NodeUsingSimple : public Node
{
public:
    Node* Name = nullptr;
    Node* ScopeType = nullptr;
public:
    NodeUsingSimple(Node* name, Node* scopeType) :
        Name(name), ScopeType(scopeType) {
    };

    std::string print() override {
        if (!Name) return "";
        std::string fprint = Name->print();
        if (ScopeType) fprint += " = " + ScopeType->print();
        return fprint;
    }

    ~NodeUsingSimple() {
        delete Name;
        delete ScopeType;
    };
};

class NodeUsing : public Node
{
public:
    Node* TemplateParameterDeclarationList = nullptr;
    Node* Name = nullptr;
    Node* ScopeType = nullptr;
    bool Simple = false;
public:
    NodeUsing(Node* TemplateParameterDeclarationList, Node* name, Node* scopeType) :
        TemplateParameterDeclarationList(TemplateParameterDeclarationList), Name(name), ScopeType(scopeType) {
    };

    std::string print() override {
        if (!Name) return "";
        std::string fprint = "using";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += " " + Name->print();
        if (ScopeType) fprint += " = " + ScopeType->print();
        return fprint;
    }
    
    ~NodeUsing() {
        delete TemplateParameterDeclarationList;
        delete Name;
        delete ScopeType;
    };
};

class NodePointerSignature : public Node
{
public:
    Node* ReturnType = nullptr;
    Node* ParameterList = nullptr;
public:
    NodePointerSignature(Node* returnType, Node* parameterList) :
        ReturnType(returnType), ParameterList(parameterList) {
    };

    std::string print() override {
        if (!ReturnType || !ParameterList) return "";
        std::string fprint = ReturnType->print() + ParameterList->print();
        return fprint;
    }

    ~NodePointerSignature() {
        delete ParameterList;
        delete ReturnType;
    };
};

class NodePointer : public Node
{
public:
    Node* TemplateParameterDeclarationList = nullptr;
    Node* Name = nullptr;
    Node* Declaration = nullptr;
public:
    NodePointer(Node* TemplateParameterDeclarationList, Node* name, Node* declartion) :
        TemplateParameterDeclarationList(TemplateParameterDeclarationList), Name(name), Declaration(declartion) {
    };

    std::string print() override {
        if (!Name || !Declaration) return "";
        std::string fprint = "pointer";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += " " + Name->print() + " = " + Declaration->print();
        return fprint;
    }

    ~NodePointer() {
        delete Declaration;
        delete Name;
        delete TemplateParameterDeclarationList;
    };
};

class NodeParameterList : public Node
{
    std::vector<Node*> ParameterList;
public:
    NodeParameterList(const std::vector<Node*>& parameterList) 
    : ParameterList(parameterList) { };

    std::string print() override {
        std::string fprint = "(";
        int size = ParameterList.size();
        for (size_t i = 0; i < size; i++)
            if (auto Decl = ParameterList[i]; Decl)
                fprint += Decl->print() + (i == size - 1 ? "" : ", ");
        fprint += ")";
        return fprint;
    };

    ~NodeParameterList() {
        for (auto& i : ParameterList) delete i;
    };
};

class NodeFunction : public Node
{
    Node* Type = nullptr;
    Node* TemplateParameterDeclarationList = nullptr;
    Node* Identifier = nullptr;
    Node* ParameterList = nullptr;
    Node* Body = nullptr;
public:
    NodeFunction(
        Node* type, Node* TemplateParameterDeclarationList, Node* name, Node* parameterList, Node* body) :
        Type(type), TemplateParameterDeclarationList(TemplateParameterDeclarationList), Identifier(name), ParameterList(parameterList), Body(body){ };

    std::string print() override {  
        if (!Type || !Identifier || !ParameterList) return "";

        std::string fprint = "function";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += (Type->print()) + " " + Identifier->print() + ParameterList->print();
        fprint += Body ? Body->print() : "";
        return fprint;
    };

    ~NodeFunction() {
        delete Type; Type = nullptr;
        delete TemplateParameterDeclarationList; TemplateParameterDeclarationList = nullptr;
        delete Identifier; Identifier = nullptr;
        delete ParameterList; ParameterList = nullptr;
        delete Body; Body = nullptr;
    };
};

class NodeLambda : public Node
{
    Node* Type = nullptr;
    Node* TemplateParameterDeclarationList = nullptr;
    Node* Identifier = nullptr;
    Node* ParameterList = nullptr;
    Node* Body = nullptr;
public:
    NodeLambda(
        Node* type, Node* TemplateParameterDeclarationList, Node* name, Node* parameterList, Node* body) :
        Type(type), TemplateParameterDeclarationList(TemplateParameterDeclarationList), Identifier(name), ParameterList(parameterList), Body(body) {
    };

    std::string print() override {
        if (!Type || !Identifier || !ParameterList) return "";

        std::string fprint = "lambda";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += (Type->print()) + " " + Identifier->print() + ParameterList->print();
        fprint += Body ? Body->print() : "";
        return fprint;
    };

    ~NodeLambda() {
        delete Type; Type = nullptr;
        delete TemplateParameterDeclarationList; TemplateParameterDeclarationList = nullptr;
        delete Identifier; Identifier = nullptr;
        delete ParameterList; ParameterList = nullptr;
        delete Body; Body = nullptr;
    };
};


class NodeConstructor : public Node 
{
    Node* TemplateParameterDeclarationList = nullptr;
    Node* ParameterList = nullptr;
    Node* Body = nullptr;
public:
    NodeConstructor(
        Node* TemplateParameterDeclarationList, Node* parameterList, Node* body) :
        TemplateParameterDeclarationList(TemplateParameterDeclarationList), ParameterList(parameterList), Body(body) {
    };

    std::string print() override {
        if (!ParameterList) return "";

        std::string fprint = "constructor";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += ParameterList->print();
        fprint += Body ? Body->print() : "";
        return fprint;
    };

    ~NodeConstructor() {
        delete TemplateParameterDeclarationList; TemplateParameterDeclarationList = nullptr;
        delete ParameterList; ParameterList = nullptr;
        delete Body; Body = nullptr;
    };
};

class NodeDestructor : public Node {
    Node* ParameterList = nullptr;
    Node* Body = nullptr;
public:
    NodeDestructor(
        Node* parameterList, Node* body) :
        ParameterList(parameterList), Body(body) {
    };

    std::string print() override {
        if (!ParameterList) return "";

        std::string fprint = "destructor";
        fprint += ParameterList->print();
        fprint += Body ? Body->print() : "";
        return fprint;
    };

    ~NodeDestructor() {
        delete ParameterList; ParameterList = nullptr;
        delete Body; Body = nullptr;
    };
};

class NodeTemplate : public Node
{
public:
    Node* TemplateParameterDeclarationList = nullptr;
    Node* Name = nullptr;
    Node* Declaration = nullptr;
public:
    NodeTemplate(Node* TemplateParameterDeclarationList, Node* name, Node* declartion) :
        TemplateParameterDeclarationList(TemplateParameterDeclarationList), Name(name), Declaration(declartion) {
    };

    std::string print() override {
        if (!Name || !Declaration) return "";
        std::string fprint = "template";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += " " + Name->print();
        if (Declaration) fprint += " = " + Declaration->print();
        return fprint;
    }

    ~NodeTemplate() {
        delete Declaration;
        delete Name;
        delete TemplateParameterDeclarationList;
    };
};

class NodeNew : public Node
{
    Node* Call = nullptr;
public:
    NodeNew(Node* call) : Call(call) {
    };

    std::string print() override {
        std::string fprint = "new " + (Call ? Call->print() : "");
        return fprint;
    };

    ~NodeNew() {
        delete Call;
    };
};

class NodeDelete : public Node
{
public:
    NodeDelete() {};

    std::string print() override {
        std::string fprint = "delete";
        return fprint;
    };

    ~NodeDelete() {};
};

class NodeCall : public Node
{
    Node* Name = nullptr;
    std::vector<Node*> ArgumentConcreticList;
public:
    NodeCall(
        Node* name, const std::vector<Node*> argumentConcreticList) :
        Name(name), ArgumentConcreticList(argumentConcreticList) {
    };

    std::string print() override {
        if (!Name) return "";
        std::string fprint = Name->print();
        fprint += "(";
        int size = ArgumentConcreticList.size();
        for (size_t i = 0; i < size; i++)
            if (auto Decl = ArgumentConcreticList[i]; Decl)
                fprint += Decl->print() + (i == size - 1 ? "" : ", ");

        fprint += ")";
        return fprint;
    };

    ~NodeCall() {
        delete Name;
        for (auto& i : ArgumentConcreticList) delete i;
    };
};

class NodeBlock : public Node
{
    std::vector<Node*> Statements;
public:
    NodeBlock() = default;

    void add(Node* stmt) {
        if (stmt) Statements.push_back(stmt);
    }

    std::string print() override {
        std::string fprint = "{\n ";
        int size = Statements.size();
        for (int i = 0; i < size; ++i) {
            if (auto stmt = Statements[i]) {
                fprint += stmt->print() + ";\n";
            }
        }
        fprint += " \n}";
        return fprint;
    }

    ~NodeBlock() override {
        for (auto* stmt : Statements) {
            delete stmt;
        }
    }
};

class NodeTemplateParameterDeclartionList : public Node {
    std::vector<Node*> Params;
public:
    NodeTemplateParameterDeclartionList(std::vector<Node*> params) : Params(params) { };

    std::string print() override {
        std::string fprint = "<";
        for (size_t i = 0; i < Params.size(); ++i) {
            fprint += Params[i]->print();
            if (i + 1 < Params.size()) fprint += ", ";
        }
        fprint += ">";
        return fprint;
    }

    ~NodeTemplateParameterDeclartionList() override {
        for (auto* p : Params) delete p;
    }
};

// Generic parameter list concretic: <int, 5, std::string>
class NodeTemplateParameterInstantiationList : public Node {
    std::vector<Node*> Params;
public:
    NodeTemplateParameterInstantiationList(std::vector<Node*> params) : Params(params) {};

    std::string print() override {
        std::string fprint = "<";
        for (size_t i = 0; i < Params.size(); ++i) {
            fprint += Params[i]->print();
            if (i + 1 < Params.size()) fprint += ", ";
        }
        fprint += ">";
        return fprint;
    }

    ~NodeTemplateParameterInstantiationList() override {
        for (auto* p : Params) delete p;
    }
};

class NodeBlockClass : public Node
{
public:
    // NONE - значит по умолчаниб небыло установлено
    enum class FieldType { NONE, PUBLIC, PRIVATE, STATIC };
private:

    std::vector<std::pair<FieldType, std::vector<Node*>>> FieldStatements;
    std::string getSymbol(FieldType Type) {
        switch (Type)
        {
        case NodeBlockClass::FieldType::NONE: return "";
        case NodeBlockClass::FieldType::PUBLIC: return "public:";
        case NodeBlockClass::FieldType::PRIVATE: return "private:";
        case NodeBlockClass::FieldType::STATIC: return "static:";
        default: return "";
        }
    }
public:
    NodeBlockClass(std::vector<std::pair<FieldType, std::vector<Node*>>> fieldStatements)
        : FieldStatements(fieldStatements) {
    };

    std::string print() override {
        std::string fprint = "{\n";
        for (auto& Statement : FieldStatements)
        {
            fprint += getSymbol(Statement.first) + "\n";
            for (auto& field : Statement.second)
                fprint += field->print() + "\n";
        }
        fprint += "\n}";
        return fprint;
    }

    ~NodeBlockClass() override {
        for (auto& Statement : FieldStatements) {
            for (auto& field : Statement.second)
                delete field;
        }
    }
};

class NodeBaseClass : public Node {
public:
    enum class InheritanceType { NONE, PUBLIC, PRIVATE };
private:
    Node* Identifier = nullptr;
    InheritanceType Type = InheritanceType::NONE;
    std::string getSymbol() {
        switch (Type)
        {
        case NodeBaseClass::InheritanceType::NONE: return "";
        case NodeBaseClass::InheritanceType::PUBLIC: return "public";
        case NodeBaseClass::InheritanceType::PRIVATE: return "privte";
        default: return "";
        }
    }
public:
    NodeBaseClass(
        Node* identifier,
        InheritanceType type = InheritanceType::NONE
    )
        : Identifier(identifier), Type(type) {
    }

    std::string print() override {

        if (!Identifier) return "";
        return getSymbol() + " " + Identifier->print();
    }

    ~NodeBaseClass() override {
        delete Identifier;
    }
};

class NodeClass : public Node {
public:
    enum class FIELD_TYPE { PUBLIC, PRIVATE, STATIC };
private:
    Node* Identifier = nullptr;
    Node* TemplateParameterDeclarationList = nullptr;
    Node* BaseClass = nullptr;
    Node* Body = nullptr;
public:
    NodeClass(
        Node* identifier,
        Node* templateParameterDeclarationList,
        Node* baseClass,
        Node* body
    )
        : Identifier(identifier), TemplateParameterDeclarationList(templateParameterDeclarationList), BaseClass(baseClass), Body(body) {
    }

    std::string print() override {
        if (!Identifier) return "";
        std::string fprint = "class";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += " " + Identifier->print();
        if (BaseClass) fprint += " : " + BaseClass->print();
        if (Body) fprint += " " + Body->print();
        return fprint;
    }

    ~NodeClass() override {
        delete Identifier;
        delete TemplateParameterDeclarationList;
        delete BaseClass;
        delete Body;
    }
};

class NodeBlockStruct : public Node
{
public:
    // NONE - значит по умолчаниб небыло установлено
    enum class FieldType { NONE, PUBLIC, STATIC };
private:

    std::vector<std::pair<FieldType, std::vector<Node*>>> FieldStatements;
    std::string getSymbol(FieldType Type) {
        switch (Type)
        {
        case NodeBlockStruct::FieldType::NONE: return "";
        case NodeBlockStruct::FieldType::PUBLIC: return "public:";
        case NodeBlockStruct::FieldType::STATIC: return "static:";
        default: return "";
        }
    }
public:
    NodeBlockStruct(std::vector<std::pair<FieldType, std::vector<Node*>>> fieldStatements)
        : FieldStatements(fieldStatements) {
    };

    std::string print() override {
        std::string fprint = "{\n";
        for (auto& Statement : FieldStatements)
        {
            fprint += getSymbol(Statement.first) + "\n";
            for (auto& field : Statement.second)
                fprint += field->print() + "\n";
        }
        fprint += "\n}";
        return fprint;
    }

    ~NodeBlockStruct() override {
        for (auto& Statement : FieldStatements) {
            for (auto& field : Statement.second)
                delete field;
        }
    }
};

class NodeStruct : public Node {
public:
    enum class INHERITANCE_TYPE { PUBLIC, STATIC };

private:
    Node* Identifier = nullptr;
    Node* TemplateParameterDeclarationList = nullptr;
    Node* Body = nullptr;
public:
    NodeStruct(
        Node* identifier,
        Node* generics,
        Node* body
    )
        : Identifier(identifier), TemplateParameterDeclarationList(generics), Body(body) {
    }

    std::string print() override {
        if (!Identifier) return "";

        std::string fprint = "struct";
        if (TemplateParameterDeclarationList) fprint += TemplateParameterDeclarationList->print();
        fprint += " " + Identifier->print();
        if (Body) fprint += " " + Body->print();
        return fprint;
    }

    ~NodeStruct() override {
        delete Identifier;
        delete TemplateParameterDeclarationList;
        delete Body;
    }
};

class NodePropertyBlock : public Node {
    Node* Getter = nullptr, * Setter = nullptr;
public:
    NodePropertyBlock(
        Node* getter,
        Node* setter) :
        Getter(getter), Setter(setter) {
    }

    std::string print() override {
        return "{\n" +
            (!Getter ? "" : "write = " + Getter->print() + ",") +
            (!Setter ? "" : "read = " + Setter->print()) + "\n}";
    }
    ~NodePropertyBlock() {
        delete Setter;
        delete Getter;
    }
};

class NodeProperty : public Node {
    Node* Name = nullptr;
    Node* Type = nullptr;
    Node* Block = nullptr;
public:
    NodeProperty(Node* name, Node* type, Node* block) :
        Name(name), Type(type), Block(block) {
    }

    std::string print() override {
        if (!Name || !Type || !Block) return "";
        return "__property" + Type->print() + " " + Name->print() + " " + Block->print() + ";";
    }
    ~NodeProperty() {
        delete Block;
        delete Type;
        delete Name;
    }
};


class NodeNamespace : public Node {
private:
    Node* Name = nullptr;
    Node* Body = nullptr;
public:
    NodeNamespace(Node* name, Node* body)
        : Name(name), Body(body) {
    }

    std::string print() override {
        std::string fprint = "namespace " + Name->print();
        if (Body) fprint += " " + Body->print();
        return fprint;
    }

    ~NodeNamespace() override {
        delete Body;
        delete Name;
    }
};

class NodeInteger : public Node {
private:
    std::string raw_value;
public:
    NodeInteger(const std::string& val) : raw_value(val) {}

    std::string print() override {
        return raw_value;
    }
};

class NodeFloating : public Node {
private:
    std::string raw_value;
public:
    NodeFloating(const std::string& val) : raw_value(val) {}

    std::string print() override {
        return raw_value;
    }
};

class NodeString : public Node {
private:
    std::string raw_value;
public:
    NodeString(const std::string& val) : raw_value(val) {}

    std::string print() override {
        return "\"" + raw_value + "\"";
    }
};

class NodeCharacter : public Node {
private:
    std::string raw_value;
public:
    NodeCharacter(const std::string& val) : raw_value(val) {}

    std::string print() override {
        return "'" + std::string(1, raw_value[0]) + "'";
    }
};

class NodeBoolean : public Node {
private:
    std::string raw_value;
public:
    NodeBoolean(const std::string& val) : raw_value(val) {}

    std::string print() override {
        return raw_value;
    }
};

class NodeNullptr : public Node {
public:
    std::string print() override {
        return "nullptr";
    }
};

class NodeDefault : public Node {
public:
    std::string print() override {
        return "default";
    }
};

class NodeWhile : public Node {
    Node* Condition = nullptr;
    Node* Body = nullptr;
    bool IsDoWhile = false;
public:
    NodeWhile(Node* condition, Node* body, bool isDoWhile) :
        Condition(condition), Body(body), IsDoWhile(isDoWhile) {}
    
    std::string print() override {
        if (!Condition || !Body) return "";
        std::string fprint = "while ";
        fprint += "(" + Condition->print() + ") ";
        fprint += (IsDoWhile ? "do " : "");
        fprint += Body->print();
        return fprint;
    }
    ~NodeWhile() {
        delete Body;
        delete Condition;
        IsDoWhile = false;
    }
};

class NodeTryCatch : public Node {
    Node* BodyTry = nullptr;
    Node* BodyCatch = nullptr;
    Node* Declaration = nullptr;
public:
    NodeTryCatch(Node* bodytry, Node* bodycatch, Node* declaration) :
        BodyTry(bodytry), BodyCatch(bodycatch), Declaration(declaration) {
    }
    std::string print() override {
        if (!BodyTry) return "BodyTry empty";
        std::string fprint = "try " + BodyTry->print();
        if (BodyCatch)
        {
            fprint += "\ncatch " + (Declaration ? "(" + Declaration->print() + ")" : "") + BodyCatch->print();
        }
        return fprint;
    }
    ~NodeTryCatch() {
        delete Declaration;
        delete BodyCatch;
        delete BodyTry;
    }
};

class NodeBinaryOp : public Node {
public:
    enum class BinaryOp
    {
        Unknown, Plus, Minus, Asterisk, Slash
    };
private:
    BinaryOp Op = BinaryOp::Unknown;
    Node* Left = nullptr;
    Node* Right = nullptr;
    std::string getSymbol()
    {
        switch (Op) {
        case BinaryOp::Plus: return "+";
        case BinaryOp::Minus: return "-";
        case BinaryOp::Asterisk: return "*";
        case BinaryOp::Slash: return "/";
        default: return "";
        }
    }
public:
    NodeBinaryOp(const BinaryOp& op, Node* left, Node* right)
        : Op(op), Left(left), Right(right) {
    }

    std::string print() override {
        if (Op == BinaryOp::Unknown)
            return "";
        if (!Left || !Right)
            return "";
        return Left->print() + " " + getSymbol() + " " + Right->print();
    }
    ~NodeBinaryOp() {
        delete Left;
        delete Right;
    }
};

class NodeUnaryOp : public Node {
public:
    enum class UnaryOp
    {
        Unknown, Minus
    };
private:
    UnaryOp Op = UnaryOp::Unknown;
    Node* Right = nullptr;
    std::string getSymbol()
    {
        switch (Op) {
        case UnaryOp::Minus: return "-";
        default: return "";
        }
    }
public:
    NodeUnaryOp(const UnaryOp& op, Node* right)
        : Op(op), Right(right) {
    }

    std::string print() override {
        if (!Right)
            return "";
        return getSymbol() + Right->print();
    }
    ~NodeUnaryOp() {
        delete Right;
    }
};

#endif // NODE_HPP