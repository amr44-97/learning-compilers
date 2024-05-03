#include "compiler.h"
#include "utils.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

namespace compiler {

static File *current_file;

File *new_file(char *name, char *contents) {
    File *file = (File *)calloc(1, sizeof(File));
    file->name = name;
    file->content = contents;
    return file;
}
static Token *new_token_ptr(TokenKind kind, char *start, char *end) {
    Token *tok = (Token *)calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = start;
    tok->len = end - start;
    tok->file = current_file;
    return tok;
}

static Token new_token(TokenKind kind, char *start, char *end) {
    return Token{
        .kind = kind,
        .next = nullptr,
        .line = 1,
        .column = 0,
        .loc = start,
        .len = end - start,
        .file = current_file,
    };
}

char *read_file(const char *file_name) {
    FILE *fp = NULL;
    size_t file_size = 0;
    char *buffer = NULL;
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: failed to open file {%s}\n", file_name);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer = (char *)utils_alloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr,
                "Error: failed to allocate memory for reading the file!\n");
        return NULL;
    }
    fread(buffer, file_size, 1, fp);
    fclose(fp);
    return buffer;
}

//
// Token
//

// FIXME: replace start wiht column
char *Token::format() {
    const char *ty = token_type_str(this->kind);
    char *fmt = (char *)calloc(265, sizeof(char));
    sprintf(fmt, "{%s} -> [{%d}:{%d}]", ty, this->line, this->column);
    return fmt;
}

char *Token::to_str() {
    char *fmt = (char *)calloc(this->len + 1, sizeof(char));
    memcpy(fmt, this->loc, this->len);
    fmt[this->len] = '\0';
    return fmt;
}

void print_tokens(Token *tokens) {
    Token *temp = tokens;
    while (true) {
        if (temp->kind == TokenKind::Eof)
            break;
        const char *kind = temp->kind_str();
        printf("%s"
               "(%s)"
               "%s"
               " => `%s`\n",
               Color_Yellow, kind, Color_Reset, temp->to_str());
        temp = temp->next;
    }
}
const char *token_type_str(TokenKind type) {
    switch (type) {
    case TokenKind::Eof:
        return "Eof";
    case TokenKind::Invalid:
        return "Invalid";
    case TokenKind::Identifier:
        return "Identifier";
    case TokenKind::StringLiteral:
        return "StringLiteral";
    case TokenKind::NumberLiteral:
        return "NumberLiteral";
    case TokenKind::Equal:
        return "Equal";
    case TokenKind::EqualEqual:
        return "EqualEqual";
    case TokenKind::Plus:
        return "Plus";
    case TokenKind::PlusPlus:
        return "PlusPlus";
    case TokenKind::PlusEqual:
        return "PlusEqual";
    case TokenKind::Minus:
        return "Minus";
    case TokenKind::MinusMinus:
        return "MinusMinus";
    case TokenKind::MinusEqual:
        return "MinusEqual";
    case TokenKind::Asterisk:
        return "Asterisk";
    case TokenKind::AsteriskEqual:
        return "AsteriskEqual";
    case TokenKind::Slash:
        return "Slash";
    case TokenKind::SlashEqual:
        return "SlashEqual";
    case TokenKind::LineComment:
        return "Linecomment";
    case TokenKind::Arrow:
        return "Arrow";
    case TokenKind::EqualAngleBracketRt:
        return "EqualAngleBracketRt";
    case TokenKind::Colon:
        return "Colon";
    case TokenKind::ColonColon:
        return "ColonColon";
    case TokenKind::ColonEqual:
        return "ColonEqual";
    case TokenKind::Comma:
        return "Comma";
    case TokenKind::Bang:
        return "Bang";
    case TokenKind::Hash:
        return "Hash";
    case TokenKind::Questionmark:
        return "Questionmark";
    case TokenKind::AtSign:
        return "AtSign";
    case TokenKind::Ambersand:
        return "Ambersand";
    case TokenKind::Dot:
        return "Dot";
    case TokenKind::DollarSign:
        return "DollarSign";
    case TokenKind::Semicolon:
        return "Semicolon";
    case TokenKind::LBrace:
        return "LBrace";
    case TokenKind::RBrace:
        return "RBrace";
    case TokenKind::LBracket:
        return "LBracket";
    case TokenKind::RBracket:
        return "RBracket";
    case TokenKind::LParen:
        return "LParen";
    case TokenKind::RParen:
        return "RParen";
    case TokenKind::AngleBracketLeft:
        return "AngleBracketLeft";
    case TokenKind::AngleBracketRight:
        return "AngleBracketRight";
    case TokenKind::LessOrEqual:
        return "LessOrEqual";
    case TokenKind::GreaterOrEqual:
        return "GreaterOrEqual";
    case TokenKind::ShiftRight:
        return "ShiftRight";
    case TokenKind::ShiftLeft:
        return "ShiftLeft";

    default:
        {
            return (Color_Red "Error: Unknown TokenKind!!" Color_Reset);
        }
    };
}

const char *Token::kind_str() {
    char *buf = (char *)malloc(64);
    memset(buf, 0, 64);
    auto n = token_type_str(this->kind);
    sprintf(buf, "%s", n);
    return (const char *)buf;
}

//
//  Tokenizer
//
//

// static bool startswith(char *p, char *q) {
//   return strncmp(p, q, strlen(q)) == 0;
// }

LinkedList<Token> tokenize(File *file) {
    current_file = file;
    char *p = current_file->content;
    LinkedList<Token> token_list;
    int line = 1;

    while (*p) {
        switch (*p) {
        case '\n':
            {
                p++;
                line++;
                continue;
            }
            break;

        case ' ':
            {
                p++;
                continue;
            }
            break;
        case '0' ... '9':
            {
                char *start = p++;
                while (isdigit(*p) or *p == '_') {
                    p++;
                }
                auto tk = new_token(TokenKind::NumberLiteral, start, p);
                tk.line = line;
				token_list += tk;
				continue;
            }
            break;

        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            {
                char *start = p++;
                while (isalnum(*p) or *p == '_') {
                    p++;
                }
                auto tk = new_token(TokenKind::Identifier, start, p);
				tk.line = line;
				token_list += tk;
                continue;
            }
            break;

        case '+':
            {
                auto start = p++;
                switch (*p) {
                case '+':
                    {
                    auto tk = new_token(TokenKind::PlusPlus, start, p);
                    tk.line = line;
                    token_list += tk;
                    }
                    break;
                case '=':
                    {

                    }
                    break;
                default:
                }
            }
            break;

        case '-':
            {
            }
            break;
        case '*':
            {
            }
            break;
        case '/':
            {
            }
            break;
        case '#':
            {
                current = current->next = new_token(TokenKind::Hash, p, p + 1);
                p++;
                continue;
            }
            break;
        case ';':
            {
                current = current->next =
                    new_token(TokenKind::Semicolon, p, p + 1);
                p++;
                continue;
            }
            break;

        case ',':
            {
                current = current->next = new_token(TokenKind::Comma, p, p + 1);
                p++;
                continue;
            }
            break;

        case '.':
            {
                current = current->next = new_token(TokenKind::Dot, p, p + 1);
                p++;
                continue;
            }
            break;

        case '{':
            {

                current = current->next =
                    new_token(TokenKind::LBrace, p, p + 1);
                p++;
                continue;
            }
            break;

        case '}':
            {

                current = current->next =
                    new_token(TokenKind::RBrace, p, p + 1);
                p++;
                continue;
            }
            break;
        case '[':
            {

                current = current->next =
                    new_token(TokenKind::LBracket, p, p + 1);
                p++;
                continue;
            }
            break;
        case ']':
            {

                current = current->next =
                    new_token(TokenKind::RBracket, p, p + 1);
                p++;
                continue;
            }
            break;

        case '(':
            {

                current = current->next =
                    new_token(TokenKind::LParen, p, p + 1);
                p++;
                continue;
            }
            break;
        case ')':
            {

                current = current->next =
                    new_token(TokenKind::RParen, p, p + 1);
                p++;
                continue;
            }
            break;

        default:
            {
                fprintf(stderr,
                        Color_Red "[Error]: Unhandled char %c\n" Color_Reset,
                        *p);
                exit(6);
            }
        }
    }

    current = current->next = new_token(TokenKind::Eof, p, p);

    return head.next;
}

} // namespace compiler
