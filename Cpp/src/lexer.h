#pragma once
#include "lib.h"
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

using string = std::string;
using std::vector;

string read_file(const char* file_name);

enum TokenKind {
    Tok_Eof,
    Tok_Invalid,
    Tok_Identifier,
    Tok_StringLiteral,
    Tok_NumberLiteral,
    Tok_CharLiteral,
    Tok_Include,
    Tok_Define,
    Tok_Equal,
    Tok_EqualEqual,
    Tok_Plus,
    Tok_PlusPlus,
    Tok_PlusEqual,
    Tok_Minus,
    Tok_MinusMinus,
    Tok_MinusEqual,
    Tok_Asterisk,
    Tok_AsteriskEqual,
    Tok_Pipe,
    Tok_PipePipe,
    Tok_Tilde,
    Tok_Slash,
    Tok_SlashEqual,
    Tok_LineComment,
    Tok_Arrow,               // ->
    Tok_EqualAngleBracketRt, // =>
    Tok_Colon,
    Tok_ColonColon,
    Tok_ColonEqual,
    Tok_Comma,
    Tok_Bang,
    Tok_BangEqual,
    Tok_Hash,
    Tok_Questionmark,
    Tok_AtSign,
    Tok_Ambersand,
    Tok_Ambersand_Ambersand,
    Tok_Dot,
    Tok_DotDot,
    Tok_Caret,
    Tok_DollarSign,
    Tok_Semicolon,
    Tok_LBrace,
    Tok_RBrace,
    Tok_LBracket,
    Tok_RBracket,
    Tok_LParen,
    Tok_RParen,
    Tok_Less,    // <
    Tok_Greater, // >
    Tok_LessEqual,
    Tok_GreaterEqual,
    Tok_ShiftRight,
    Tok_ShiftLeft,
    Tok_Keyword_fn,
    Tok_Keyword_pub,
    Tok_Keyword_const,
    Tok_Keyword_var,
    Tok_Keyword_return,
    Tok_Keyword_if,
    Tok_Keyword_for,
    Tok_Keyword_while,
};

auto enum_to_str(TokenKind type) -> const char*;

struct Location {
    uint32_t start;
    uint32_t line_start = 1;
    uint32_t line = 1;
    uint32_t column = 1;

    ~Location() = default;
    Location() {}
    Location(uint32_t start, uint32_t line_start, uint32_t line, uint32_t column) {
        this->start = start;
        this->line_start = line_start;
        this->line = line;
        this->column = column;
    }

    Location(const Location& location) {
        this->start = location.start;
        this->line_start = location.line_start;
        this->line = location.line;
        this->column = location.column;
    }

    Location& operator=(const Location& location) {
        if (this != &location) {
            this->start = location.start;
            this->line_start = location.line_start;
            this->line = location.line;
            this->column = location.column;
        }
        return (*this);
    }
};

struct Token {
    TokenKind kind;
    uint32_t index;
    Location loc;
    string buf;

    ~Token() = default;

    Token() {}

    Token(const Token& _token) {
        this->kind = _token.kind;
        this->index = _token.index;
        this->loc = _token.loc;
        this->buf = _token.buf;
    }

    Token(TokenKind _kind, uint32_t _index, Location& _loc, string _buf)
        : index(_index), loc(_loc), buf(_buf) {
        this->kind = _kind;
    }

    Token& operator=(const Token& _token) {
        if (this != &_token) {
            this->kind = _token.kind;
            this->index = _token.index;
            this->loc = _token.loc;
            this->buf = _token.buf;
        }
        return *this;
    }

    void set(TokenKind _kind, Location& _loc, string _buf) {
        this->kind = _kind;
        this->loc = _loc;
        this->buf = _buf;
        this->index = 0;
    }
};

struct Lexer {
     string& source;
    ~Lexer() = default;
    Lexer( string& _source) : source(_source) {
        this->index = 0;
        this->location = {0, 0, 1, 1};
    }

  private:
    uint32_t index;
    Location location;
    Token token;

    char advance_char();
    char peek(uint32_t offset);
    bool found_end();
    bool match(char c);
    void scan_ident();
    void scan_string_literal();
    void scan_number_literal();
    void scan_macro_or_preprocessor();

    void skip_whitspaces();
    void reset_location_start();
    // it doesn't return Token but it change lexer.token
  public:
    Token next_token();

    static void print_token(Token& t) {
        printf("{ %s | `%s`}\n", enum_to_str(t.kind), t.buf.data());
    }

    static void print_tokens(vector<Token>& tokens) {
        for (int i = 0; i < tokens.size(); i++) {
            Token token = tokens[i];
            Lexer::print_token(token);
        }
    }
};
