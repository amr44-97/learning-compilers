#pragma once

#include "diagnostics.h"
#include "lexer.h"
#include "tree.h"
#include <stdint.h>

enum ErrorKind {
    error_unexpected_token,
    error_expected_expression,
    error_expected_type_expr,
    error_expected_var_decl,
    error_expected_fnuction_call,
};
struct Error {
    ErrorKind kind;
    const char* msg;
    const char* caller_fn;
    Token token;   // Location of error
    bool is_fatal; // Determines if parsing should stop
};

auto enum_to_str(ErrorKind kind) -> const char*;

struct Parser {
    Lexer lexer;
    string& source;
    vector<Token> tokens;
    vector<Error> errors;
    Token current;
    uint32_t index = 0;

    Parser(string& _source) : lexer(_source), source(_source) {
        Token tok = lexer.next_token();

        tokens.reserve(source.size() / 4);
        while (true) {
            tokens.push_back(tok);
            if (tok.kind == Tok_Eof) break;
            tok = lexer.next_token();
        }
        current = tokens[0];
    }

    auto fail(ErrorKind kind, const char* msg, Token token, bool is_fatal = true) {
        errors.push_back({kind, msg, __func__, token, is_fatal});
        if (is_fatal) {
            fprintf(stderr, "[ParsingError]: %s at [%d,%d] { %s : `%s` }\nMessage: %s\n",
                    enum_to_str(kind), token.loc.line, token.loc.column, enum_to_str(token.kind),
                    token.buf.c_str(), msg);

            exit(1); // Fatal error, exit
        }
    }

    auto fail(string error_msg, Token token) -> void {
        fprintf(stderr, Color_Bright_red "Error -> " Color_Reset "at [line = %d, column = %d]: %s\n",
                token.loc.line, token.loc.column, error_msg.c_str());
        fprintf(stderr, Color_Bright_red "error line => " Color_Reset);
        fprintf(stderr, "%s\n", get_line_of(current).c_str());
		exit(0);
    }

    auto warn(ErrorKind kind, const char* msg, Token token) {
        errors.push_back({kind, msg, __func__, token, false});
        fprintf(stderr, "[ParsingError]: %s at [%d,%d] { %s : `%s` }\nMessage: %s\n",
                enum_to_str(kind), token.loc.line, token.loc.column, enum_to_str(token.kind),
                token.buf.c_str(), msg);
    }

    auto printErrors() {
        for (const auto& error : errors) {
            fprintf(stderr, Color_Red "Error" Color_Reset " in %s: %s\n", error.caller_fn,
                    error.msg);
            // Include token position or other information
        }
    }
    auto has_errors() const -> bool { return !errors.empty(); }

    auto next_token() -> Token;
    auto expectToken(TokenKind kind) -> Token;
    auto get_line_of(Token token) -> string;

    auto parseTypeExpr() -> Type*;
    auto parseExpr() -> Expr*;
    auto expectExpr() -> Expr*;
    auto parsePrimaryExpr() -> Expr*;
    auto parsePrefixExpr() -> Expr*;
    auto parsePrecedenceExpr(int min) -> Expr*;
    auto parseAssignExpr() -> Expr*;
    auto parseIfExpr() -> Expr*;
    auto parseParamDecl() -> Decl*;
    auto parseFnDeclParams() -> ParamList*;
    auto parseFnProto() -> Stmt*;
    auto parseTopLevelStmts() -> vector<Stmt*>;
    auto parseFnDecl() -> Decl*;
    auto parseFnCall() -> Expr*;
    auto parseVarDecl() -> Decl*;
    auto parseConstDecl() -> Decl*;
    auto parseMacroFn() -> Decl*;
    auto parsePayLoad() -> Decl*;
    auto parseBlock() -> Stmt*;
    auto parseStatement() -> Stmt*;
    auto parseIfStmt() -> Stmt*;
    auto parseLoop() -> Stmt*;
    auto parseForLoop() -> Stmt*;
    auto parseWhileLoop() -> Stmt*;
};
