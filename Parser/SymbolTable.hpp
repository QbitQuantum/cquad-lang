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

struct Scope;

struct Symbol {
    std::string  name;
    SymbolKind   kind = SymbolKind::Unknown;
    Node* typeNode = nullptr;
    Node* declNode = nullptr;
    Scope* scope = nullptr;
    size_t       line = 0;
    size_t       column = 0;
    bool         isConst = false;

    Symbol() = default;
    Symbol(std::string n, SymbolKind k, Node* t, Node* d,
        size_t ln, size_t col, bool c)
        : name(std::move(n)), kind(k), typeNode(t), declNode(d),
        line(ln), column(col), isConst(c) {
    }
};

class Scope {
public:
    ScopeKind                             kind;
    std::string                           name;
    Scope* parent;
    std::vector<std::unique_ptr<Scope>>   children;
    std::vector<std::unique_ptr<Symbol>>  symbols;

    Scope(ScopeKind k, std::string n, Scope* p)
        : kind(k), name(std::move(n)), parent(p) {
    }

    Symbol* findLocal(const std::string& id) {
        for (auto& s : symbols)
            if (s->name == id) return s.get();
        return nullptr;
    }

    const Symbol* findLocal(const std::string& id) const {
        for (auto& s : symbols)
            if (s->name == id) return s.get();
        return nullptr;
    }

    std::vector<Symbol*> findLocalAll(const std::string& id) {
        std::vector<Symbol*> out;
        for (auto& s : symbols)
            if (s->name == id) out.push_back(s.get());
        return out;
    }
};

class SymbolTable {
public:
    SymbolTable() 
    {
        auto global = std::make_unique<Scope>(ScopeKind::Global, "<global>", nullptr);
        current_ = global.get();
        root_ = std::move(global);
    };

    Scope* enterScope(ScopeKind kind, const std::string& name = {}) {
        auto child = std::make_unique<Scope>(kind, name, current_);
        Scope* raw = child.get();
        current_->children.push_back(std::move(child));
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
        bool isConst = false,
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

        auto sym = std::make_unique<Symbol>(name, kind, typeNode, declNode, line, column, isConst);
        Symbol* raw = sym.get();
        current_->symbols.push_back(std::move(sym));
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
        if (root_) dumpScope(root_.get(), os, 0);
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

    static void dumpScope(const Scope* s, std::ostream& os, int depth) {
        std::string pad(depth * 2, ' ');
        os << pad << "Scope(" << scopeKindName(s->kind);
        if (!s->name.empty()) os << " '" << s->name << "'";
        os << ")\n";
        for (const auto& sym : s->symbols) {
            os << pad << "  " << symbolKindName(sym->kind)
                << " " << sym->name
                << " @" << sym->line << ":" << sym->column;
            if (sym->isConst) os << " const";
            os << "\n";
        }
        for (const auto& c : s->children)
            dumpScope(c.get(), os, depth + 1);
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

#endif // SYMBOLTABLE_HPP