#include "compiler.h"
#include "utils.h"
#include <ctype.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define is_digit                                                               \
    '0' : case '1':                                                            \
    case '2':                                                                  \
    case '3':                                                                  \
    case '4':                                                                  \
    case '5':                                                                  \
    case '6':                                                                  \
    case '7':                                                                  \
    case '8':                                                                  \
    case '9'

#define is_alpha                                                               \
    'a' : case 'b':                                                            \
    case 'c':                                                                  \
    case 'd':                                                                  \
    case 'e':                                                                  \
    case 'f':                                                                  \
    case 'g':                                                                  \
    case 'h':                                                                  \
    case 'i':                                                                  \
    case 'j':                                                                  \
    case 'k':                                                                  \
    case 'l':                                                                  \
    case 'm':                                                                  \
    case 'n':                                                                  \
    case 'o':                                                                  \
    case 'p':                                                                  \
    case 'q':                                                                  \
    case 'r':                                                                  \
    case 's':                                                                  \
    case 't':                                                                  \
    case 'u':                                                                  \
    case 'v':                                                                  \
    case 'w':                                                                  \
    case 'x':                                                                  \
    case 'y':                                                                  \
    case 'z':                                                                  \
    case 'A':                                                                  \
    case 'B':                                                                  \
    case 'C':                                                                  \
    case 'D':                                                                  \
    case 'E':                                                                  \
    case 'F':                                                                  \
    case 'G':                                                                  \
    case 'H':                                                                  \
    case 'I':                                                                  \
    case 'J':                                                                  \
    case 'K':                                                                  \
    case 'L':                                                                  \
    case 'M':                                                                  \
    case 'N':                                                                  \
    case 'O':                                                                  \
    case 'P':                                                                  \
    case 'Q':                                                                  \
    case 'R':                                                                  \
    case 'S':                                                                  \
    case 'T':                                                                  \
    case 'U':                                                                  \
    case 'V':                                                                  \
    case 'W':                                                                  \
    case 'X':                                                                  \
    case 'Y':                                                                  \
    case 'Z'

static File *current_file;

char *read_file(const char *path) {
    FILE *fp = fopen(path, "r");
	if(!fp)
		return NULL;
	
	fseek(fp, 0, SEEK_END);
	auto flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* buffer = malloc(flen+1);
	if(!buffer)
	{
		fprintf(stderr, "Error:Couldn't allocate memory for file buffer\n");
		exit(1);
	}
	fread(buffer, 1, flen, fp);

    return buffer;
}

File *new_file(char *name, char *contents) {
    File *file = (File *)calloc(1, sizeof(File));
    file->name = name;
    file->content = contents;
    return file;
}

static Token *new_token(TokenKind kind, char *start, char *end) {
    Token *tok = (Token *)calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = start;
    tok->len = end - start;
    tok->file = current_file;
    return tok;
}

char *read_file2(const char *file_name) {
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
char *token_format(Token *token) {
    const char *ty = token_type_str(token->kind);
    char *fmt = (char *)calloc(265, sizeof(char));
    sprintf(fmt, "{%s} -> [{%d}:{%d}]", ty, token->line, token->column);
    return fmt;
}

char *token_to_str(Token *token) {
    char *fmt = (char *)calloc(token->len + 1, sizeof(char));
    memcpy(fmt, token->loc, token->len);
    fmt[token->len] = '\0';
    return fmt;
}

void print_tokens(Token *tokens) {
    Token *temp = tokens;
    while (true) {
        if (temp->kind == Tok_Eof)
            break;
        const char *kind = token_kind_str(temp);
        printf("%s"
               "(%s)"
               "%s"
               " => `%s`\n",
               Color_Yellow, kind, Color_Reset, token_to_str(temp));
        temp = temp->next;
    }
}
const char *token_type_str(TokenKind type) {
    switch (type) {
    case Tok_Eof:
        return "Eof";
    case Tok_Invalid:
        return "Invalid";
    case Tok_Identifier:
        return "Identifier";
    case Tok_StringLiteral:
        return "StringLiteral";
    case Tok_NumberLiteral:
        return "NumberLiteral";
    case Tok_Equal:
        return "Equal";
    case Tok_EqualEqual:
        return "EqualEqual";
    case Tok_Plus:
        return "Plus";
    case Tok_PlusPlus:
        return "PlusPlus";
    case Tok_PlusEqual:
        return "PlusEqual";
    case Tok_Minus:
        return "Minus";
    case Tok_MinusMinus:
        return "MinusMinus";
    case Tok_MinusEqual:
        return "MinusEqual";
    case Tok_Asterisk:
        return "Asterisk";
    case Tok_AsteriskEqual:
        return "AsteriskEqual";
    case Tok_Slash:
        return "Slash";
    case Tok_SlashEqual:
        return "SlashEqual";
    case Tok_LineComment:
        return "Linecomment";
    case Tok_Arrow:
        return "Arrow";
    case Tok_EqualAngleBracketRt:
        return "EqualAngleBracketRt";
    case Tok_Colon:
        return "Colon";
    case Tok_ColonColon:
        return "ColonColon";
    case Tok_ColonEqual:
        return "ColonEqual";
    case Tok_Comma:
        return "Comma";
    case Tok_Bang:
        return "Bang";
    case Tok_Hash:
        return "Hash";
    case Tok_Questionmark:
        return "Questionmark";
    case Tok_AtSign:
        return "AtSign";
    case Tok_Ambersand:
        return "Ambersand";
    case Tok_Dot:
        return "Dot";
    case Tok_DollarSign:
        return "DollarSign";
    case Tok_Semicolon:
        return "Semicolon";
    case Tok_LBrace:
        return "LBrace";
    case Tok_RBrace:
        return "RBrace";
    case Tok_LBracket:
        return "LBracket";
    case Tok_RBracket:
        return "RBracket";
    case Tok_LParen:
        return "LParen";
    case Tok_RParen:
        return "RParen";
    case Tok_AngleBracketLeft:
        return "AngleBracketLeft";
    case Tok_AngleBracketRight:
        return "AngleBracketRight";
    case Tok_LessOrEqual:
        return "LessOrEqual";
    case Tok_GreaterOrEqual:
        return "GreaterOrEqual";
    case Tok_ShiftRight:
        return "ShiftRight";
    case Tok_ShiftLeft:
        return "ShiftLeft";

    default:
        {
            //	char msg[] = "Error: Unknown TokenKind!!" ;
            //		constexpr size_t s =  20 + sizeof(msg);
            return (Color_Red "Error: Unknown TokenKind" Color_Reset);
        }
    };
}

const char *token_kind_str(Token *token) {
    char *buf = (char *)malloc(21);
    memset(buf, ' ', 21);
    sprintf(buf, "%s", token_type_str(token->kind));
    buf[20] = 0;
    return (const char *)buf;
}

//
//  Tokenizer
//
//

// static bool startswith(char *p, char *q) {
//   return strncmp(p, q, strlen(q)) == 0;
// }

Token *tokenize(File *file) {
    current_file = file;

    char *p = current_file->content;
    Token head = {};
    Token *current = &head;
    int line = 1;
    int col = 0;

    while (*p) {
        switch (*p) {
        case '\n':
            {
                p++;
                col = 0;
                line++;
                continue;
            }
            break;

        case ' ':
            {
                p++;
                col++;
                continue;
            }
            break;
        case is_digit:
            {
                char *start = p++;
                while (isdigit(*p) || *p == '_') {
                    p++;
                    col++;
                }
                current = current->next =
                    new_token(Tok_NumberLiteral, start, p);
                current->column = col;
                current->line = line;
                continue;
            }
            break;

        case is_alpha:
        case '_':
            {
                char *start = p++;
                while (isalnum(*p) || *p == '_') {
                    p++;
                    col++;
                }
                current = current->next = new_token(Tok_Identifier, start, p);
                current->column = col;
                current->line = line;
                continue;
            }
            break;
        case ';':
            {

                current = current->next = new_token(Tok_Semicolon, p, p + 1);
                p++;
                col++;
                continue;
            }
            break;

        case ',':
            {

                current = current->next = new_token(Tok_Comma, p, p + 1);
                p++;
                col++;
                continue;
            }
            break;

        case '.':
            {

                current = current->next = new_token(Tok_Dot, p, p + 1);
                p++;
                col++;
                continue;
            }
            break;

            case '+':{
                auto begin = p;
                if(*(++p) == '+'){
                    current = current->next = new_token(Tok_PlusPlus,begin,p);
                }
            }break;
        case '{':
            {

                current = current->next = new_token(Tok_LBrace, p, p + 1);
                p++;
                col++;
                continue;
            }
            break;

        case '}':
            {

                current = current->next = new_token(Tok_RBrace, p, p + 1);
                p++;
                col++;
                continue;
            }
            break;
        case '[':
            {

                current = current->next = new_token(Tok_LBracket, p, p + 1);
                p++;
                col++;
                continue;
            }
            break;
        case ']':
            {

                current = current->next = new_token(Tok_RBracket, p, p + 1);
                p++;
                col++;
                continue;
            }
            break;

        case '(':
            {

                current = current->next = new_token(Tok_LParen, p, p + 1);
                p++;
                col++;
                continue;
            }
            break;
        case ')':
            {

                current = current->next = new_token(Tok_RParen, p, p + 1);
                p++;
                col++;
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

    current = current->next = new_token(Tok_Eof, p, p);

    return head.next;
}
