#pragma once

#include "utils.h"
#include <cstdio>
#include <format>
#include <stdint.h>
#include <string>

typedef struct Token Token;

enum class TokenType {
  Eof,
  Invalid,
  Identifier,
  StringLiteral,
  NumberLiteral,
  Equal,
  EqualEqual,
  Plus,
  PlusPlus,
  PlusEqual,
  Minus,
  MinusMinus,
  MinusEqual,
  Asterisk,
  AsteriskEqual,
  Slash,
  SlashEqual,
  LineComment,
  Arrow,               // ->
  EqualAngleBracketRt, // =>
  Colon,
  ColonColon,
  ColonEqual,
  Comma,
  Bang,
  Hash,
  Questionmark,
  AtSign,
  Ambersand,
  Dot,
  DollarSign,
  Semicolon,
  LBrace,
  RBrace,
  LBracket,
  RBracket,
  LParen,
  RParen,
  AngleBracketLeft,  // <
  AngleBracketRight, // >
  LessOrEqual,
  GreaterOrEqual,
  ShiftRight,
  ShiftLeft,
};

static auto tok_type_str(TokenType type) -> const char * {
  switch (type) {
  case TokenType::Eof:
    return "Eof";
  case TokenType::Invalid:
    return "Invalid";
  case TokenType::Identifier:
    return "Identifier";
  case TokenType::StringLiteral:
    return "StringLiteral";
  case TokenType::NumberLiteral:
    return "NumberLiteral";
  case TokenType::Equal:
    return "Equal";
  case TokenType::EqualEqual:
    return "EqualEqual";
  case TokenType::Plus:
    return "Plus";
  case TokenType::PlusPlus:
    return "PlusPlus";
  case TokenType::PlusEqual:
    return "PlusEqual";
  case TokenType::Minus:
    return "Minus";
  case TokenType::MinusMinus:
    return "MinusMinus";
  case TokenType::MinusEqual:
    return "MinusEqual";
  case TokenType::Asterisk:
    return "Asterisk";
  case TokenType::AsteriskEqual:
    return "AsteriskEqual";
  case TokenType::Slash:
    return "Slash";
  case TokenType::SlashEqual:
    return "SlashEqual";
  case TokenType::LineComment:
    return "Linecomment";
  case TokenType::Arrow:
    return "Arrow";
  case TokenType::EqualAngleBracketRt:
    return "EqualAngleBracketRt";
  case TokenType::Colon:
    return "Colon";
  case TokenType::ColonColon:
    return "ColonColon";
  case TokenType::ColonEqual:
    return "ColonEqual";
  case TokenType::Comma:
    return "Comma";
  case TokenType::Bang:
    return "Bang";
  case TokenType::Hash:
    return "Hash";
  case TokenType::Questionmark:
    return "Questionmark";
  case TokenType::AtSign:
    return "AtSign";
  case TokenType::Ambersand:
    return "Ambersand";
  case TokenType::Dot:
    return "Dot";
  case TokenType::DollarSign:
    return "DollarSign";
  case TokenType::Semicolon:
    return "Semicolon";
  case TokenType::LBrace:
    return "LBrace";
  case TokenType::RBrace:
    return "RBrace";
  case TokenType::LBracket:
    return "LBracket";
  case TokenType::RBracket:
    return "RBracket";
  case TokenType::LParen:
    return "LParen";
  case TokenType::RParen:
    return "RParen";
  case TokenType::AngleBracketLeft:
    return "AngleBracketLeft";
  case TokenType::AngleBracketRight:
    return "AngleBracketRight";
  case TokenType::LessOrEqual:
    return "LessOrEqual";
  case TokenType::GreaterOrEqual:
    return "GreaterOrEqual";
  case TokenType::ShiftRight:
    return "ShiftRight";
  case TokenType::ShiftLeft:
    return "ShiftLeft";

  default:
    printf("%s\n",
           (std::format("{}Error: Unknown TokenType!!", Color::Red)).c_str());
  };
}

struct Token {
  TokenType type;
  uint32_t line;
  uint32_t start;
  uint32_t column;
  uint32_t end;

  // FIXME: replace start wiht column
  auto to_str()const -> std::string ;
};

struct Tokenizer {
  std::string *source;
  uint32_t index;

  ~Tokenizer() { delete source; }

  static auto Init(std::string *src) -> Tokenizer {
    return Tokenizer{
        .source = src,
        .index = 0,
    };
  }

  auto next() -> Token;
  auto token_buf(Token tok) const -> std::string;
};
