#include "lexer.h"
#include "lib.h"
#include <assert.h>
#include <cstring>
#include <stdio.h>

inline bool is_equal(const char* s1, const char* s2) {
    auto s1_len = strlen(s1);
    auto s2_len = strlen(s2);

    if (s1_len != s2_len) return false;
    return (memcmp(s1, s2, strlen(s1)) == 0);
}

//@Leak

//@Leak
string read_file(const char* file_name) {
    FILE* fp = fopen(file_name, "r");
    if (!fp) {
        fprintf(stderr, "Couldn't open file = %s\n", file_name);
        exit(0);
    }
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    string buf(fsize + 1, ' ');
    buf.resize(fsize + 1);
    auto read_bytes = fread(&buf[0], 1, fsize, fp);
    assert(read_bytes == fsize);
    fclose(fp);
    return buf;
}

// token index in a list
uint32_t token_index = 0;

char Lexer::advance_char() {
    location.column++;
    return source[index++];
}

char Lexer::peek(uint32_t offset = 0) { return source[index + offset]; }
bool Lexer::found_end() { return source[index] == '\0'; }
bool Lexer::match(char c) {
    if (source[index] == c) {
        advance_char();
        return true;
    }
    return false;
}
void Lexer::scan_ident() {
    auto start = location.start;
    while (1) {
        char c = peek();
        switch (c) {
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '_':
            break;
        default:
            goto EXIT;
        }
        advance_char();
    }
EXIT:;
    string buf = source.substr(start, index - start);
    this->token = Token(Tok_Identifier, token_index++, this->location, buf);
    switch (buf[0]) {
#define StrSlice(S) #S, sizeof(#S)
    case 'i': {
        if (buf == "if") this->token = Token(Tok_Keyword_if, token_index++, this->location, buf);
    } break;
    case 'p': {
        if (buf == "pub") this->token = Token(Tok_Keyword_pub, token_index++, this->location, buf);
    } break;
    case 'f': {
        if (buf == "fn") this->token = Token(Tok_Keyword_fn, token_index++, this->location, buf);
        if (buf == "for") this->token = Token(Tok_Keyword_for, token_index++, this->location, buf);
    } break;
    case 'c': {
        if (buf == "const")
            this->token = Token(Tok_Keyword_const, token_index++, this->location, buf);
    } break;
    case 'v': {
        if (buf == "var") this->token = Token(Tok_Keyword_var, token_index++, this->location, buf);
    } break;
    case 'w': {
        if (buf == "while")
            this->token = Token(Tok_Keyword_while, token_index++, this->location, buf);
    } break;
    case 'r': {
        if (buf == "return")
            this->token = Token(Tok_Keyword_return, token_index++, this->location, buf);
    } break;
    default: {
    } break;
    }
}
void Lexer::scan_string_literal() {
    auto start = location.start;

    while (1) {
        char c = peek();
        switch (c) {
        case '"':
            advance_char();
            goto EXIT;
        default:
            break;
        }
        c = advance_char();
    }
EXIT:;

    auto buf = source.substr(start, index - start);
    this->token = Token(Tok_StringLiteral, token_index++, this->location, buf);
}
void Lexer::scan_number_literal() {
    auto start = location.start;
    while (1) {
        char c = peek();
        switch (c) {
        case '0' ... '9':
            break;
        default:
            goto EXIT;
        }
        advance_char();
    }
EXIT:;
    auto buf = source.substr(start, index - start);
    this->token = Token(Tok_NumberLiteral, token_index++, this->location, buf);
}
void Lexer::scan_macro_or_preprocessor() {
    scan_ident();

    if (token.buf == "#define") {
        token.kind = Tok_Define;
    }

    if (token.buf == "#include") {
        token.kind = Tok_Include;
    }

    token.kind = Tok_Invalid;
}

void Lexer::skip_whitspaces() {
    while (true) {
        if (this->index >= source.size()) return;
        char c = peek();
        if (c == ' ' || c == '\t') {
            this->location.column++;
            c = advance_char();
        } else if (c == '\n') {
            this->location.line++;
            this->location.column = 1;
            c = advance_char();
            this->location.line_start = index;
        } else {
            return;
        }
    }
}

void Lexer::reset_location_start() { this->location.start = this->index; }

// it doesn't return Token but it change lexer.token
Token Lexer::next_token() {
    skip_whitspaces();
    reset_location_start();

    if (found_end()) {
        token.set(Tok_Eof, location, "<EOF>");
        return token;
    }

    char c = peek();
    advance_char();

    switch (c) {
    case '_':
    case 'a' ... 'z':
    case 'A' ... 'Z':
        scan_ident();
        break;

    case '0' ... '9':
        scan_number_literal();
        break;
    case '"':
        scan_string_literal();
        break;

    case '#':
        scan_macro_or_preprocessor();
        break;
    case '(':
        token.set(Tok_LParen, location, "(");
        break;

    case ')':
        token.set(Tok_RParen, location, ")");
        break;

    case '{':
        token.set(Tok_LBrace, location, "{");
        break;

    case '}':
        token.set(Tok_RBrace, location, "}");
        break;

    case '[':
        token.set(Tok_LBracket, location, "[");
        break;

    case ']':
        token.set(Tok_RBracket, location, "]");
        break;

    case ';':
        token.set(Tok_Semicolon, location, ";");
        break;

    case '&':
        token.set(Tok_Ambersand, location, "&");
        break;

    case '~':
        token.set(Tok_Tilde, location, "~");
        break;

    case '.':
        if (match('.')) token.set(Tok_DotDot, location, "..");
        token.set(Tok_Dot, location, ".");
        break;

    case ',':
        token.set(Tok_Comma, location, ",");
        break;

    case '?':
        token.set(Tok_Questionmark, location, "?");
        break;

    case ':':
        token.set(Tok_Colon, location, ":");
        break;

    case '^':
        token.set(Tok_Caret, location, "^");
        break;

    case '!':
        token.set(Tok_Bang, location, "!");
        break;

    case '=':
        token.set(Tok_Equal, location, "=");
        if (match('=')) token.set(Tok_EqualEqual, location, "==");
        break;
    case '+': {
        token.set(Tok_Plus, location, "+");
        if (match('+')) token.set(Tok_PlusPlus, location, "++");
        if (match('=')) token.set(Tok_PlusEqual, location, "+=");
        break;
    }
    case '-':
        token.set(Tok_Minus, location, "-");
        if (match('-')) token.set(Tok_MinusMinus, location, "--");
        if (match('=')) token.set(Tok_MinusEqual, location, "-=");
        if (match('>')) token.set(Tok_Arrow, location, "->");
        break;

    case '*':
        token.set(Tok_Asterisk, location, "*");
        if (match('=')) token.set(Tok_AsteriskEqual, location, "*=");
        break;

    case '/':
        token.set(Tok_Slash, location, "/");
        if (match('/')) token.set(Tok_LineComment, location, "//");
        if (match('=')) token.set(Tok_SlashEqual, location, "/=");
        break;

    case '<':
        token.set(Tok_Less, location, "<");
        if (match('=')) token.set(Tok_LessEqual, location, "<=");
        if (match('<')) token.set(Tok_ShiftLeft, location, "<<");
        break;

    case '>':
        token.set(Tok_Greater, location, ">");
        if (match('=')) token.set(Tok_GreaterEqual, location, ">=");
        if (match('<')) token.set(Tok_ShiftRight, location, ">>");
        break;

    case '|':
        token.set(Tok_Pipe, location, "|");
        if (match('|')) token.set(Tok_PipePipe, location, "||");
        break;

    default: {
        fprintf(stderr, "Unhandled char [%c] at %s", c,
                source.substr(index, source.size() - index).data());
        exit(0);
    }
    }
    return token;
}

#define case_to_str(T)                                                                             \
    case T:                                                                                        \
        return &((#T)[4])

auto enum_to_str(TokenKind kind) -> const char* {
    switch (kind) {
        case_to_str(Tok_Eof);
        case_to_str(Tok_Invalid);
        case_to_str(Tok_Identifier);
        case_to_str(Tok_StringLiteral);
        case_to_str(Tok_NumberLiteral);
        case_to_str(Tok_CharLiteral);
        case_to_str(Tok_Include);
        case_to_str(Tok_Define);
        case_to_str(Tok_Equal);
        case_to_str(Tok_EqualEqual);
        case_to_str(Tok_Plus);
        case_to_str(Tok_PlusPlus);
        case_to_str(Tok_PlusEqual);
        case_to_str(Tok_Minus);
        case_to_str(Tok_MinusMinus);
        case_to_str(Tok_MinusEqual);
        case_to_str(Tok_Asterisk);
        case_to_str(Tok_AsteriskEqual);
        case_to_str(Tok_Pipe);
        case_to_str(Tok_PipePipe);
        case_to_str(Tok_Tilde);
        case_to_str(Tok_Slash);
        case_to_str(Tok_SlashEqual);
        case_to_str(Tok_LineComment);
        case_to_str(Tok_Arrow);
        case_to_str(Tok_EqualAngleBracketRt);
        case_to_str(Tok_Colon);
        case_to_str(Tok_ColonColon);
        case_to_str(Tok_ColonEqual);
        case_to_str(Tok_Comma);
        case_to_str(Tok_Bang);
        case_to_str(Tok_BangEqual);
        case_to_str(Tok_Hash);
        case_to_str(Tok_Questionmark);
        case_to_str(Tok_AtSign);
        case_to_str(Tok_Ambersand);
        case_to_str(Tok_Ambersand_Ambersand);
        case_to_str(Tok_Dot);
        case_to_str(Tok_DotDot);
        case_to_str(Tok_Caret);
        case_to_str(Tok_DollarSign);
        case_to_str(Tok_Semicolon);
        case_to_str(Tok_LBrace);
        case_to_str(Tok_RBrace);
        case_to_str(Tok_LBracket);
        case_to_str(Tok_RBracket);
        case_to_str(Tok_LParen);
        case_to_str(Tok_RParen);
        case_to_str(Tok_Less);
        case_to_str(Tok_Greater);
        case_to_str(Tok_LessEqual);
        case_to_str(Tok_GreaterEqual);
        case_to_str(Tok_ShiftRight);
        case_to_str(Tok_ShiftLeft);
        case_to_str(Tok_Keyword_fn);
        case_to_str(Tok_Keyword_pub);
        case_to_str(Tok_Keyword_const);
        case_to_str(Tok_Keyword_var);
        case_to_str(Tok_Keyword_return);
        case_to_str(Tok_Keyword_if);
        case_to_str(Tok_Keyword_for);
        case_to_str(Tok_Keyword_while);
    };
#undef case_to_str
    return "";
}
