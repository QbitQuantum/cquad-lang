#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP
#pragma once

#include <memory>
#include <vector>
#include <string>
#include "ParserError.hpp"
#include "Node.hpp"

enum class SymbolKind {
    Unknown,
    Variable,
    Parameter,
    Function,
    Class,
    Field,
    TemplateTypeParam,
    TemplateValueParam,
    Namespace
};

enum class ScopeKind {
    Global,
    Namespace,
    Class,
    Function,
    Block,
    For,
    Template
};

class Entity {
public:
    enum class Kind { Symbol, Scope };

    Kind        kind;
    std::string name;

    Entity(Kind k, std::string n)
        : kind(k), name(std::move(n)) {
    }
    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    bool isSymbol() const { return kind == Kind::Symbol; }
    bool isScope()  const { return kind == Kind::Scope; }

    virtual void dump(std::ostream& os, int depth) const = 0;

protected:
    static std::string indent(int depth) {
        return std::string(depth * 2, ' ');
    }
};

class Scope;
struct Symbol : Entity {
    SymbolKind   kind = SymbolKind::Unknown;
    Node* typeNode = nullptr;
    Node* declNode = nullptr;
    Scope* scope = nullptr;
    size_t       line = 0;
    size_t       column = 0;

    Symbol() : Entity(Entity::Kind::Symbol, {}) {}

    Symbol(std::string n, SymbolKind k, Node* t, Node* d,
        size_t ln, size_t col)
        : Entity(Entity::Kind::Symbol, std::move(n)),
        kind(k), typeNode(t), declNode(d),
        line(ln), column(col) {
    }

    void dump(std::ostream& os, int depth) const override {
        os << indent(depth)
            << symbolKindName(kind) << " " << name
            << " @" << line << ":" << column << "\n";
    }

    static std::string symbolKindName(SymbolKind k) {
        switch (k) {
        case SymbolKind::Unknown:            return "Unknown";
        case SymbolKind::Variable:           return "Var";
        case SymbolKind::Parameter:          return "Param";
        case SymbolKind::Function:           return "Func";
        case SymbolKind::Class:              return "Class";
        case SymbolKind::Field:              return "Field";
        case SymbolKind::TemplateTypeParam:  return "TType";
        case SymbolKind::TemplateValueParam: return "TValue";
        case SymbolKind::Namespace:          return "Namespace";
        }
        return "?";
    }
};

class Scope : public Entity {
public:
    ScopeKind                            kind;
    Scope* parent;
    std::vector<std::unique_ptr<Entity>> hierarchy;

    Scope(ScopeKind k, std::string n, Scope* p)
        : Entity(Entity::Kind::Scope, std::move(n)),
        kind(k), parent(p) {
    }

    Symbol* findLocal(const std::string& id) {
        for (auto& e : hierarchy)
            if (e->isSymbol() && e->name == id)
                return static_cast<Symbol*>(e.get());
        return nullptr;
    }

    const Symbol* findLocal(const std::string& id) const {
        for (const auto& e : hierarchy)
            if (e->isSymbol() && e->name == id)
                return static_cast<const Symbol*>(e.get());
        return nullptr;
    }

    std::vector<Symbol*> findLocalAll(const std::string& id) {
        std::vector<Symbol*> out;
        for (auto& e : hierarchy)
            if (e->isSymbol() && e->name == id)
                out.push_back(static_cast<Symbol*>(e.get()));
        return out;
    }

    void dump(std::ostream& os, int depth) const override {
        os << indent(depth) << "Scope(" << scopeKindName(kind);
        if (!name.empty()) os << " '" << name << "'";
        os << ")\n";

        for (const auto& e : hierarchy)
            e->dump(os, depth + 1);
    }

    static std::string scopeKindName(ScopeKind k) {
        switch (k) {
        case ScopeKind::Global:    return "Global";
        case ScopeKind::Namespace: return "Namespace";
        case ScopeKind::Class:     return "Class";
        case ScopeKind::Function:  return "Function";
        case ScopeKind::Block:     return "Block";
        case ScopeKind::For:       return "For";
        case ScopeKind::Template:  return "Template";
        }
        return "?";
    }
};

class SymbolTable {
public:
    SymbolTable()
    {
        auto global = std::make_unique<Scope>(ScopeKind::Global, "<global>", nullptr);
        current_ = global.get();
        root_ = std::move(global);
    }

    Scope* enterScope(ScopeKind kind, const std::string& name = {}) {
        auto child = std::make_unique<Scope>(kind, name, current_);
        Scope* raw = child.get();
        current_->hierarchy.push_back(std::move(child));
        current_ = raw;
        return current_;
    }

    void exitScope() {
        if (current_ && current_->parent)
            current_ = current_->parent;
    }

    Scope* currentScope()       noexcept { return current_; }
    const Scope* currentScope() const noexcept { return current_; }
    Scope* globalScope()        noexcept { return root_.get(); }
    const Scope* globalScope()  const noexcept { return root_.get(); }

    Symbol* declare(const std::string& name,
        SymbolKind kind,
        Node* typeNode = nullptr,
        Node* declNode = nullptr,
        size_t line = 0,
        size_t column = 0,
        bool throwOnRedeclare = true)
    {
        const bool allowOverload = (kind == SymbolKind::Function);

        if (!allowOverload) {
            Symbol* existing = current_->findLocal(name);
            if (existing) {
                if (throwOnRedeclare)
                    throwRedeclare(name, existing, line, column);
                return nullptr;
            }
        }

        auto sym = std::make_unique<Symbol>(name, kind, typeNode, declNode, line, column);
        Symbol* raw = sym.get();
        current_->hierarchy.push_back(std::move(sym));
        return raw;
    }

    Symbol* resolve(const std::string& name) {
        for (Scope* s = current_; s; s = s->parent)
            if (Symbol* sym = s->findLocal(name)) return sym;
        return nullptr;
    }

    const Symbol* resolve(const std::string& name) const {
        for (const Scope* s = current_; s; s = s->parent)
            if (const Symbol* sym = s->findLocal(name)) return sym;
        return nullptr;
    }

    std::vector<Symbol*> resolveAll(const std::string& name) {
        std::vector<Symbol*> out;
        for (Scope* s = current_; s; s = s->parent) {
            auto local = s->findLocalAll(name);
            if (!local.empty()) { out = std::move(local); break; }
        }
        return out;
    }

    void dump(std::ostream& os = std::cout) const {
        if (root_) root_->dump(os, 0);
    }

private:
    std::unique_ptr<Scope> root_;
    Scope* current_ = nullptr;

    static void throwRedeclare(const std::string& name,
        const Symbol* prev,
        size_t line, size_t column)
    {
        std::string msg = "Redeclaration of '" + name + "'";
        if (prev) {
            msg += " (previously declared at line " +
                std::to_string(prev->line) +
                ", column " + std::to_string(prev->column) + ")";
        }
        throw ParseError::ParseError(line, column, msg);
    }
};

#endif // SYMBOLTABLE_HPP