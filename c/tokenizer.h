#pragma once

#include <stdint.h>

typedef struct Token Token;

enum TokenType {
  Tok_eof,
  Tok_invalid,
  Tok_identifier,
  Tok_stringliteral,
  Tok_numberliteral,
  Tok_plus,
  Tok_minus,
  Tok_asterisk,
  Tok_slash,
  Tok_colon,
  Tok_comma,
  Tok_l_brace,
  Tok_r_brace,
  Tok_l_bracket,
  Tok_r_bracket,
  Tok_l_paren,
  Tok_r_paren,
};

struct Token {
  TokenType type;
  uint32_t line;
  uint32_t start;
  uint32_t end;
};

struct Tokenizer {
  char *source;
  uint32_t index;
};

Tokenizer tokenizer_init(char *src);
Token tokenizer_next(Tokenizer *T);
