#pragma once
#include "diagnostics.h"
#include "lexer.h"
#include <cstdio>
#include <stdlib.h>
#include <string.h>

enum NodeKind {
    Ast_None,
    Ast_Root,
    Ast_Identifier,
    Ast_StringLiteral,
    Ast_NumberLiteral,
    Ast_NullLiteral,
    Ast_IncludeStmt,
    Ast_MacroDefine,
    Ast_Mul,
    Ast_Add,
    Ast_Sub,
    Ast_Div,
    Ast_Bool_Not,
    Ast_Bool_Or,
    Ast_Bool_And,
    Ast_Bit_Not,
    Ast_Bit_Xor,
    Ast_Bit_And,
    Ast_Bit_Or,
    Ast_ShiftLeft,
    Ast_ShiftRight,
    Ast_AddressOf,
    Ast_Slice,
    Ast_SliceSentinel,
    Ast_SliceOpen,
    Ast_Assign,
    Ast_ArrayAccess,
    Ast_FieldAccess,
    Ast_Negation,
    Ast_FnProto,
    Ast_FnDecl,
    Ast_VarDecl,
    Ast_ConstDecl,
    Ast_Deref,
    Ast_Macro,
    Ast_ParamDecl,
    Ast_ParamDeclList,
    Ast_Block,
    Ast_If_Simple,
    Ast_If,
    Ast_WhileLoop,
    Ast_ForLoop,
    Ast_SimpleLoop,
    Ast_Struct,
    Ast_Union,
    Ast_TypeName,
    Ast_DeclType,
    Ast_Array,
    Ast_Pointer,
    Ast_Label,
    Ast_LessThan,
    Ast_GreaterThan,
    Ast_EqualEqual,
    Ast_NotEqual,
    Ast_CallOne,
    Ast_Call,
};

auto enum_to_str(NodeKind kind) -> const char*;

struct Node {
    NodeKind kind;
    ~Node() = default;
    virtual auto print(string prefix, bool isLeft) const -> void = 0;
};

struct Stmt : Node {
    Stmt() {}
    Stmt(NodeKind _kind) { this->kind = _kind; }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "├──" : "└──"));
        printf("%s\n", enum_to_str(this->kind));
    }
};

struct Expr : Stmt {
    Expr() {}
    Expr(NodeKind _kind) { this->kind = _kind; }

    virtual auto isLiteral() const -> bool { return false; }
    virtual auto isBinaryExpr() const -> bool { return false; }
    virtual auto get_value() const -> string { return ""; }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s"  "\n", enum_to_str(this->kind));
    }
};

// string_literal , Identifier, number_literal , char_literal , struct_literal
struct Literal : Expr {
    Token token;

    Literal(NodeKind _kind, Token tok) : Expr(_kind) { this->token = tok; }

    bool isLiteral() const override { return true; }

    virtual string get_value() const override { return token.buf; }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s "  ":: %s\n", enum_to_str(this->kind),
               token.buf.c_str());
    }
};

struct BinaryExpr : Expr {
    Token token; // useless , only for debugging
    Expr* lhs;
    Expr* rhs;

    BinaryExpr() {}
    BinaryExpr(NodeKind _kind, Token op_token, Expr* lhs, Expr* rhs) : Expr(_kind) {
        this->token = op_token;
        this->lhs = lhs;
        this->rhs = rhs;
    }

    bool isBinaryExpr() const override { return true; }

    virtual string get_value() const override { return ""; }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s"  ":: %s\n", enum_to_str(this->kind),
               token.buf.c_str());

        prefix += (isLeft ? "    " : "    ");

        if (lhs) lhs->print(prefix.c_str(), true);
        if (rhs) rhs->print(prefix.c_str(), false);
    }
};

struct Slice : Expr {
    Expr* ident;
    Expr* index;
    Expr* end;
    Expr* sentinel;
};

struct Type : Expr {
    Token token;
    Type() {}
    Type(const Type& type) : Expr(type.kind) { this->token = type.token; }

    Type(NodeKind _kind, Token name) : token(name) { this->kind = _kind; }

    virtual const string toStr() const { return token.buf; }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s "  ":: %s\n", enum_to_str(this->kind),
               token.buf.c_str());
    }
};

struct Pointer : Type {
    Type* base;

    Pointer() { this->kind = Ast_Pointer; }

    Pointer(Type* base, Token _name) {
        this->token = _name;
        this->kind = Ast_Pointer;
        this->base = base;
    }

    const string toStr() const override { return string(token.buf + base->toStr()); }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s "  ":: %s\n", enum_to_str(this->kind),
               token.buf.c_str());

        prefix += "    ";
       

        printf("%s{\n", prefix.c_str());
        if (base) base->print(prefix.c_str(), false);
        printf("%s}\n", prefix.c_str());
    }
};

struct Array : Type {
    Expr* len;
    Type* base;

    Array() { this->kind = Ast_Array; }

    Array(Type* base, Expr* len, Token _name) {
        this->token = _name;
        this->kind = Ast_Array;
        this->base = base;
        this->len = len;
    }

    const string toStr() const override {
        return string((len != nullptr ? len->get_value() : "") + "]" + base->toStr());
    }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s "  ":: %s\n", enum_to_str(this->kind),
               this->toStr().c_str());

        prefix += "    ";

        printf("%s{\n", prefix.c_str());
        if (len) len->print(prefix.c_str(), true);
        if (base) base->print(prefix.c_str(), false);
        printf("%s}\n", prefix.c_str());
    }
};

struct Decl : Stmt {
    Type* type;
    Decl() {}
    Decl(NodeKind _kind) : Decl(_kind, nullptr) {}
    Decl(NodeKind _kind, Type* type) {
        this->kind = _kind;
        this->type = type;
    }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "├──" : "└──"));
        printf( "%s"  "\n", enum_to_str(this->kind));


        prefix += "    ";

        printf("%s{\n", prefix.c_str());
        if (type) type->print(prefix.c_str(), false);
        printf("%s}\n", prefix.c_str());
    }
};

struct ParamDecl : Decl {
    Token token;
    ParamDecl(Token name, Type* _type) : Decl(Ast_ParamDecl, _type) {
        this->type = _type;
        this->kind = Ast_ParamDecl;
        this->token = name;
    }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s "  ":: %s\n", enum_to_str(this->kind),
               token.buf.c_str());

        prefix += "    ";
       
        printf("%s{\n", prefix.c_str());
        if (type) type->print(prefix.c_str(), false);
        printf("%s}\n", prefix.c_str());
    }
};

struct ParamList : Stmt {
    vector<Decl*> params;
    ParamList(vector<Decl*> list) : Stmt(Ast_ParamDeclList) { this->params = list; }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s"  "\n", enum_to_str(this->kind));

        prefix += "    ";
        printf("%s{\n", prefix.c_str());
        for (auto decl : params) {
            if (decl) decl->print(prefix.c_str(), true);
        }
        printf("%s}\n", prefix.c_str());
    }
};

struct CallExpr : Expr {
    Token fn_name;
    vector<Expr*> params;

    CallExpr() : Expr(Ast_Call) {}
    CallExpr(Token _name, vector<Expr*> parameters) : Expr(Ast_Call) {
        this->fn_name = _name;
        this->params = parameters;
    }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s "  ":: %s\n", enum_to_str(this->kind),
               fn_name.buf.c_str());


        prefix += "    ";


        printf("%s{\n", prefix.c_str());
        for (auto e : params) {
            if (e) e->print(prefix.c_str(), false);
        }
        printf("%s}\n", prefix.c_str());
    }
};

struct VarDecl : Decl {
    Literal* name;
    Expr* value_expr = nullptr;
    VarDecl(Literal* _name, Type* _type = nullptr, Expr* value = nullptr)
        : Decl(Ast_VarDecl, _type) {
        this->name = _name;
        this->value_expr = value;
    }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "VarDecl"  "\n");

        prefix += "    ";

        printf("%s{\n", prefix.c_str());
        if (name) name->print(prefix.c_str(), true);
        if (type) {
            type->print(prefix.c_str(), false);
        } else {
            printf("%sType :: inferred\n", prefix.c_str());
        }
        if (value_expr) value_expr->print(prefix.c_str(), false);
        printf("%s}\n", prefix.c_str());
    }
};

struct ConstDecl : Decl {
    Literal* name;
    Expr* value_expr = nullptr;
    ConstDecl(Literal* _name, Type* _type = nullptr, Expr* value = nullptr)
        : Decl(Ast_ConstDecl, _type) {
        this->name = _name;
        this->value_expr = value;
    }

    void print(string prefix = "", bool isLeft = false) const override {
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "ConstDecl"  "\n");

        prefix += "    ";

        printf("%s{\n", prefix.c_str());
        if (name) name->print(prefix.c_str(), true);
        if (type) {
            type->print(prefix.c_str(), false);
        } else {
            printf("%s   Type :: inferred\n", prefix.c_str());
        }
        if (value_expr) value_expr->print(prefix.c_str(), false);
        printf("%s}\n", prefix.c_str());
    }
};

struct Block : Stmt {
    vector<Stmt*> stmts;
    Block() : Stmt(Ast_Block) {}
    Block(vector<Stmt*> copy_list) : Stmt(Ast_Block) { this->stmts = copy_list; }

    void print(string prefix = "", bool isLeft = false) const override {
        fflush(stdout);
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s"  "\n", enum_to_str(this->kind));

        prefix += "    ";
        printf("%s{\n", prefix.c_str());
        for (auto statement : stmts) {
            if (statement) statement->print(prefix.c_str(), true);
        }
        printf("%s}\n", prefix.c_str());
    }
};

struct IfExpr : Expr {
    Expr* condition;
    Stmt* block;

    IfExpr(NodeKind _kind, Expr* cond, Stmt* _block) : Expr(_kind) {
        this->condition = cond;
        this->block = _block;
    }
    string get_value() const override { return ""; }
};

struct IfStmt : Stmt {
    Expr* condition;
    Stmt* block;

    IfStmt(NodeKind _kind, Expr* cond, Stmt* _block) : Stmt(_kind) {
        this->condition = cond;
        this->block = _block;
    }

    auto to_expr() const -> Expr* { return nullptr; }

    void print(string prefix = "", bool isLeft = false) const override {
        fflush(stdout);
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s "  "::\n", enum_to_str(this->kind));

        prefix += "    ";

        printf("%s{\n", prefix.c_str());
        if (condition) condition->print(prefix.c_str(), true);
        if (block) block->print(prefix.c_str(), true);
        printf("%s}\n", prefix.c_str());
    }
};

struct FnDecl : Decl {
    Token name;
    ParamList* params;
    Block* body;

    FnDecl() : Decl(Ast_FnDecl) {
        this->params = nullptr;
        this->body = nullptr;
    }
    FnDecl(Token _name, ParamList* parameter_decls, Type* ret_type, Block* body) {
        this->name = _name;
        this->params = parameter_decls;
        this->type = ret_type;
        this->body = body;
    }

    void print(string prefix = "", bool isLeft = false) const override {
        fflush(stdout);
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s"  ":: %s -> %s\n", enum_to_str(this->kind),
               name.buf.c_str(), type->toStr().c_str());

        prefix += "    ";

        printf("%s{\n", prefix.c_str());
        printf( "%s Params : "  "{"
                                   "\n",
               prefix.c_str());
        if (params) params->print(prefix.c_str(), true);
        printf("%s}\n", prefix.c_str());
        printf( "%s Body : "  "{"
                                   "\n",
               prefix.c_str());
        if (body) body->print(prefix.c_str(), true);
        printf("%s}\n", prefix.c_str());
    }
};

struct PayLoad : Decl {};

struct LoopStmt : Stmt {
    Expr* condition;
    Stmt* pattern;
    Expr* expression;
    Block* block;

    LoopStmt(NodeKind _kind) : Stmt(_kind) {}
    LoopStmt(NodeKind _kind, Expr* condition, Stmt* pattern, Expr* expr, Block* block)
        : Stmt(_kind) {
        this->condition = condition;
        this->pattern = pattern;
        this->expression = expr;
        this->block = block;
    }

    void print(string prefix = "", bool isLeft = false) const override {
        fflush(stdout);
        printf("%s%s", prefix.c_str(), (isLeft ? "   " : "   "));
        printf( "%s"  , enum_to_str(this->kind));

        prefix += "    ";
        printf(" {\n");
        if (condition) {
            printf( "%s Condition"  "\n", prefix.c_str());
            condition->print(prefix.c_str(), true);
        }
        if (pattern) {
            printf( "%s Pattern"  "\n", prefix.c_str());
            pattern->print(prefix.c_str(), true);
        }
        if (expression) {
            printf( "%s Expression"  "\n", prefix.c_str());
            expression->print(prefix.c_str(), true);
        }
        // printf("%s Body\n",prefix );
        if (block) block->print(prefix.c_str(), true);
        printf("%s}\n", prefix.c_str());
    }
};
