#include "parser.h"
#include "diagnostics.h"
#include "lexer.h"
#include "tree.h"
#include <stdio.h>
#include <string.h>

#define case_to_str(T)                                                                             \
    case T:                                                                                        \
        return &((#T)[6])
auto enum_to_str(ErrorKind kind) -> const char* {
    switch (kind) {
        case_to_str(error_unexpected_token);
        case_to_str(error_expected_expression);
        case_to_str(error_expected_type_expr);
        case_to_str(error_expected_var_decl);
        case_to_str(error_expected_fnuction_call);
    }
    return "";
}

auto Parser::next_token() -> Token {
    Token tok = current;
    current = tokens[++index];
    return tok;
}

auto Parser::get_line_of(Token token) -> string {
    return source.substr(token.loc.line_start,
                         source.find("\n", token.loc.line_start) - token.loc.line_start);
}

auto Parser::expectToken(TokenKind kind) -> Token {
    if (current.kind != kind) {
        fprintf(stderr, "expected token %s but found { %s: %s } \n", enum_to_str(kind),
                enum_to_str(current.kind), current.buf.c_str());
        fprintf(stderr, Color_Bright_red "error line =>  %s" Color_Reset "\n",
                get_line_of(current).c_str());
        exit(1); // Fatal error, exit
    }
    return next_token();
}

auto Parser::parsePrimaryExpr() -> Expr* {
    switch (current.kind) {
    case Tok_Identifier: {
        Token ident = next_token();
        Literal* ident_literal = new Literal(Ast_Identifier, ident);
        switch (current.kind) {
        case Tok_Dot: {
            Token dot = next_token();
            BinaryExpr* expr = new BinaryExpr(Ast_FieldAccess, dot, ident_literal, nullptr);
            expr->rhs = parsePrimaryExpr();
            return expr;
        }
        case Tok_LParen: {
            next_token();
        } break;
        default: {
            return ident_literal;
        }
        }
    } break;
    case Tok_NumberLiteral: {
        Literal* lit = new Literal(Ast_NumberLiteral, current);
        next_token();
        return lit;
    } break;
    case Tok_StringLiteral: {
        Literal* lit = new Literal(Ast_StringLiteral, current);
        next_token();
        return lit;
    } break;
    default: {
        return nullptr;
    }
    }
    return nullptr;
}

struct OpInfo {
    int prec;
    NodeKind tag;
};

static inline auto get_binary_op_info(TokenKind kind) -> OpInfo {
    switch (kind) {
    case Tok_PipePipe:
        return {.prec = 10, .tag = Ast_Bool_Or};
    case Tok_Ambersand_Ambersand:
        return {.prec = 20, .tag = Ast_Bool_And};
    case Tok_EqualEqual:
        return {.prec = 30, .tag = Ast_EqualEqual};
    case Tok_BangEqual:
        return {.prec = 30, .tag = Ast_NotEqual};
    case Tok_Less:
        return {.prec = 30, .tag = Ast_LessThan};
    case Tok_Greater:
        return {.prec = 30, .tag = Ast_GreaterThan};
    case Tok_Ambersand:
        return {.prec = 40, .tag = Ast_Bit_And};
    case Tok_Pipe:
        return {.prec = 40, .tag = Ast_Bit_Or};
    case Tok_Caret:
        return {.prec = 40, .tag = Ast_Bit_Xor};
    case Tok_ShiftLeft:
        return {.prec = 50, .tag = Ast_ShiftLeft};
    case Tok_ShiftRight:
        return {.prec = 50, .tag = Ast_ShiftRight};
    case Tok_Plus:
        return {.prec = 60, .tag = Ast_Add};
    case Tok_Minus:
        return {.prec = 60, .tag = Ast_Sub};
    case Tok_Asterisk:
        return {.prec = 70, .tag = Ast_Mul};
    case Tok_Slash:
        return {.prec = 70, .tag = Ast_Div};
    default:
        return {.prec = -1, .tag = Ast_None};
    }
}

auto Parser::parsePrefixExpr() -> Expr* {
    NodeKind tag;
    switch (current.kind) {
    case Tok_Bang:
        tag = Ast_Bool_Not;
        next_token();
        break;
    case Tok_Minus:
        tag = Ast_Negation;
        next_token();
        break;
    case Tok_Tilde:
        tag = Ast_Bit_Not;
        next_token();
        break;
    case Tok_Ambersand:
        tag = Ast_AddressOf;
        next_token();
        break;
    default:
        return parsePrimaryExpr();
    }
    Token op = next_token();
    return new Literal(tag, op);
}

auto Parser::parsePrecedenceExpr(int min) -> Expr* {
    Expr* left = parsePrefixExpr();
    if (!left) return nullptr;

    while (true) {
        auto op_info = get_binary_op_info(current.kind);
        if (op_info.prec < min) break;

        if (op_info.prec == -1) {
            printf("[ParsingError]: Chained comparison operator!!\n");
        }
        auto op_token = next_token();

        Expr* right = parsePrecedenceExpr(op_info.prec);
        if (!right) {
            fprintf(stderr, "[ParsingError]: expected primary expression but found -> %s\n",
                    tokens[op_token.index + 1].buf.c_str());

            exit(1); // Fatal error, exit
                     //  return nullptr;
        }
        left = new BinaryExpr(op_info.tag, op_token, left, right);
    }
    return left;
}

auto Parser::parseExpr() -> Expr* { return parsePrecedenceExpr(0); }

auto Parser::expectExpr() -> Expr* {
    auto expr = parseExpr();
    if (expr == nullptr) {
        fail(error_expected_expression, "<Expr>", current);
    }
    return expr;
}

auto is_assignable(Expr* expr) -> bool { return true; }

auto Parser::parseAssignExpr() -> Expr* {
    auto id = parseExpr();
    if (!is_assignable(id)) {
        fprintf(stderr,
                Color_Red "[ParsingError]:" Color_Reset "you can't assign to this expressiong ");
        fprintf(stderr, "%s", enum_to_str(id->kind));
        exit(1); // Fatal error, exit
    }

    auto eql_op = expectToken(Tok_Equal);
    auto val = parseExpr();
    return new BinaryExpr(Ast_Assign, eql_op, id, val);
}

auto Parser::parseTypeExpr() -> Type* {
    switch (current.kind) {
    case Tok_Identifier: {
        Token id = next_token();
        return new Type(Ast_Identifier, id);
    }
    case Tok_Asterisk: {
        Token astr = next_token();
        Type* base = parseTypeExpr();
        return new Pointer(base, astr);
    }
    case Tok_LBracket: {
        auto l_brace = next_token();
        Token tok_rep = l_brace;
        auto len_expr = parseExpr();
        if (len_expr == nullptr) {
            tok_rep.buf += "]";
        }
        expectToken(Tok_RBracket);
        Type* base = parseTypeExpr();
        return new Array(base, len_expr, l_brace);
    }
    default: {
        fprintf(stderr, "[ParsingError]: expected type expression ");
        fprintf(stderr, "but found %s\n", enum_to_str(current.kind));
        exit(0);
    }
    }
    return nullptr;
}

// Call( name: str, id : int)
auto Parser::parseParamDecl() -> Decl* {
    const auto id = expectToken(Tok_Identifier);
    expectToken(Tok_Colon);
    const auto type = parseTypeExpr();
    return new ParamDecl(id, type);
}

auto Parser::parseFnDeclParams() -> ParamList* {
    auto l_paren = next_token();
    vector<Decl*> list;
    list.reserve(3);

    while (true) {
        if (current.kind == Tok_RParen) {
            next_token();
            break;
        }
        auto param = parseParamDecl();
        list.push_back(param);

        if (current.kind == Tok_RParen) {
            next_token();
            break;
        }
        if (current.kind == Tok_Comma) next_token();
    }
    return new ParamList(list);
}
auto Parser::parseFnCall() -> Expr* {
    auto fn_name = next_token();
    auto l_paren = expectToken(Tok_LParen);
    vector<Expr*> list;
    list.reserve(3);

    while (true) {
        if (current.kind == Tok_RParen) {
            next_token();
            break;
        }
        auto param = parseExpr();
        list.push_back(param);
        if (current.kind == Tok_RParen) {
            next_token();
            break;
        }
        if (current.kind == Tok_Comma) next_token();
    }
    return new CallExpr(fn_name, list);
}

auto Parser::parseFnDecl() -> Decl* {
    next_token(); // eat fn keyword
    Token name = expectToken(Tok_Identifier);
    auto params = parseFnDeclParams();
    expectToken(Tok_Arrow);
    auto ret_type = parseTypeExpr();
    auto body_s = parseBlock();
    auto blk = static_cast<Block*>(body_s);
    return new FnDecl(name, params, ret_type, blk);
}

auto Parser::parseVarDecl() -> Decl* {
    auto var_or_const = next_token(); // eat var keyword
    auto name_token = next_token();
    auto var_name = new Literal(Ast_Identifier, name_token);
    if (current.kind == Tok_Semicolon) {
        fprintf(stderr, "[ParsingError]: expected variable declaration\n");
        exit(0);
    }
    if (current.kind != Tok_Colon) {
        Token eql_tok = expectToken(Tok_Equal);
        Expr* value_expr = parseExpr();

        return new VarDecl(var_name, nullptr, value_expr);
    }
    auto colon = next_token();
    Type* decl_type = parseTypeExpr();
    if (current.kind == Tok_Semicolon) {
        next_token();
        return new VarDecl(var_name, decl_type, nullptr);
    }

    Token eql_tok = expectToken(Tok_Equal);
    Expr* value_expr = parseExpr();
    return new VarDecl(var_name, decl_type, value_expr);
}

auto Parser::parseConstDecl() -> Decl* {
    auto const_tok = next_token(); // eat const keyword
    auto name_token = next_token();
    auto var_name = new Literal(Ast_Identifier, name_token);
    if (current.kind == Tok_Semicolon) {
        fprintf(stderr, "[ParsingError]: expected variable declaration\n");
        exit(0);
    }
    if (current.kind != Tok_Colon) {
        Token eql_tok = expectToken(Tok_Equal);
        Expr* value_expr = parseExpr();

        return new ConstDecl(var_name, nullptr, value_expr);
    }
    auto colon = next_token();
    Type* decl_type = parseTypeExpr();
    if (current.kind == Tok_Semicolon) {
        next_token();
        return new ConstDecl(var_name, decl_type, nullptr);
    }

    Token eql_tok = expectToken(Tok_Equal);
    Expr* value_expr = parseExpr();
    return new ConstDecl(var_name, decl_type, value_expr);
}

// auto Parser::parseStatement() -> Stmt* {
//     Stmt* result = nullptr;
//     switch (current.kind) {
//     case Tok_Keyword_var: {
//         result = parseVarDecl();
//         //expectToken(Tok_Semicolon);
//     }
//     case Tok_Keyword_const: {
//         result = parseConstDecl();
//       //  expectToken(Tok_Semicolon);
//     }
//     case Tok_Keyword_if: {
//         result = parseIfStmt();
//     }
//     case Tok_Identifier: {
//         result = parseAssignExpr();
//         expectToken(Tok_Semicolon);
//     }
//     case Tok_Keyword_for:
//     case Tok_Keyword_while: {
//         fail(error_unexpected_token, "for and while loops are not implmented yet", current);
//         return nullptr;
//     }
//     default:
//         return nullptr;
//     }
//     return result;
// }

auto Parser::parseStatement() -> Stmt* {
    Stmt* result = nullptr;
    switch (current.kind) {
    case Tok_Keyword_var: {
        result = parseVarDecl();
        expectToken(Tok_Semicolon);
    } break;
    case Tok_Keyword_const: {
        return parseConstDecl();
        expectToken(Tok_Semicolon);
    } break;
    case Tok_Keyword_if: {
        result = parseIfStmt();
    } break;
    case Tok_Identifier: {
        switch (tokens[index + 1].kind) {
        case Tok_LParen: {
            result = parseFnCall();
            expectToken(Tok_Semicolon);
        } break;
        default: {
            result = parseAssignExpr();
            expectToken(Tok_Semicolon);
        } break;
        }
    } break;
    case Tok_Keyword_for: {
        result = parseLoop();
    } break;
    case Tok_Keyword_while: {
        fail(error_unexpected_token, "for and while loops are not implmented yet", current);
        return nullptr;
    } break;
    default:
        return nullptr;
    }
    return result;
}

auto Parser::parseIfStmt() -> Stmt* {
    auto if_tok = expectToken(Tok_Keyword_if);
    auto cond_expr = parseExpr();
    auto then_expr = parseBlock();
    return new IfStmt(Ast_If_Simple, cond_expr, then_expr);
}

auto Parser::parseBlock() -> Stmt* {
    auto l_brace = expectToken(Tok_LBrace);
    auto blk = new Block;
    while (true) {
        if (current.kind == Tok_RBrace || current.kind == Tok_Semicolon) {
            break;
        }
        auto stmt = parseStatement();
        blk->stmts.push_back(stmt);
    }
    expectToken(Tok_RBrace);
    return blk;
}

// for{ } loop until break;
auto Parser::parseLoop() -> Stmt* {
    Token for_tok = expectToken(Tok_Keyword_for);
	auto expr  = parseExpr();
    auto body_stmt = parseBlock();
    Block* body = static_cast<Block*>(body_stmt);
    return new LoopStmt(Ast_SimpleLoop, nullptr, nullptr, expr, body);
}

auto Parser::parseTopLevelStmts() -> vector<Stmt*> {
    vector<Stmt*> list = {};
    Stmt* result = nullptr;

    while (true) {
        switch (current.kind) {
        case Tok_Eof: {
            return list;
        }

        case Tok_Keyword_fn: {
            result = parseFnDecl();
            // expectToken(Tok_Semicolon);
            list.push_back(result);
        } break;
        case Tok_Keyword_var: {
            result = parseVarDecl();
            expectToken(Tok_Semicolon);
            list.push_back(result);
        } break;
        case Tok_Keyword_const: {
            result = parseConstDecl();
            expectToken(Tok_Semicolon);
            list.push_back(result);
        } break;
        case Tok_Keyword_if: {
            result = parseIfStmt();
            list.push_back(result);
        } break;
        case Tok_Identifier: {
            switch (tokens[index + 1].kind) {
            case Tok_LParen: {
                result = parseFnCall();
                expectToken(Tok_Semicolon);
                list.push_back(result);
            } break;
            default: {
                result = parseAssignExpr();
                expectToken(Tok_Semicolon);
                list.push_back(result);
            } break;
            }
        } break;
        case Tok_Keyword_for: {
            result = parseLoop();
            list.push_back(result);
        } break;
        case Tok_Keyword_while: {
            fail(error_unexpected_token, "for and while loops are not implmented yet", current);
            return {};
        } break;
        default:
            fail("parsing top level statements", current);
            break;
        }
    }
}
auto Parser::parseWhileLoop() -> Stmt* { return nullptr; }
