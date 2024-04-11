#include "tokenizer.h"
#include "utils.h"

//
// Token
//

// FIXME: replace start wiht column
auto Token::to_str() const -> std::string {
  auto ty = tok_type_str(this->type);
  std::string fmt;
  fmt = std::format("{} -> [{}:{}]", ty, this->line, this->start);
  return fmt;
}

//
//  Tokenizer
//
//

Tokenizer tokenizer_init(std::string *src) {
  return Tokenizer{
      .source = src,
      .index = 0,
  };
}

auto Tokenizer::token_buf(Token tok) const -> std::string {
  return this->source->substr(tok.start, tok.end - tok.start);
}
auto Tokenizer::next() -> Token {
  // auto T = this;
  enum State {
    S_start,
    S_identifier,
    S_numberliteral,
    S_stringliteral,
    S_equal,
    S_colon,
    S_plus,
    S_minus,
    S_asterisk,
    S_slash,
    S_less_than,
    S_greater_than,
  };

  enum State state = S_start;
  Token result = Token{
      .type = TokenType::Eof,
      .line = 1,
      .start = this->index,
      .column = 0,
      .end = 0,
  };

  while (true) {
    if (this->index == this->source->length()) {
      result.type = TokenType::Eof;
      return result;
    }
    char c = this->source->at(this->index);
    switch (state) {
    case S_start:
      {
        switch (c) {
        case 0:
          {
            result.type = TokenType::Eof;
            goto end_loop;
          }
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
          {
            state = S_identifier;
          }
          break;
        case ' ':
        case '\n':
        case '\t':
          {
            result.start += 1;
            break;
          }
        case '0' ... '9':
          {
            state = S_numberliteral;
          }
          break;
        case '"':
          {
            state = S_stringliteral;
          }
          break;
        case ':':
          {
            state = S_colon;
          }
          break;
        case '=':
          {
            state = S_equal;
          }
          break;
        case '<':
          {
            state = S_less_than;
          }
          break;

        case '>':
          {
            state = S_greater_than;
          }
          break;
        case '+':
          {
            state = S_plus;
          }
          break;
        case '-':
          {
            state = S_minus;
          }
          break;
        case '*':
          {
            state = S_asterisk;
          }
          break;
        case '/':
          {
            state = S_slash;
          }
          break;

        case ';':
          {
            result.type = TokenType::Semicolon;
            this->index += 1;
            goto end_loop;
          }
        case '.':
          {
            result.type = TokenType::Dot;
            this->index += 1;
            goto end_loop;
          }
        case '$':
          {
            result.type = TokenType::DollarSign;
            this->index += 1;
            goto end_loop;
          }
        case ',':
          {
            result.type = TokenType::Comma;
            this->index += 1;
            goto end_loop;
          }
        case '!':
          {
            result.type = TokenType::Bang;
            this->index += 1;
            goto end_loop;
          }
        case '?':
          {
            result.type = TokenType::Questionmark;
            this->index += 1;
            goto end_loop;
          }
        case '@':
          {
            result.type = TokenType::AtSign;
            this->index += 1;
            goto end_loop;
          }
        case '#':
          {
            result.type = TokenType::Hash;
            this->index += 1;
            goto end_loop;
          }
        case '&':
          {
            result.type = TokenType::Ambersand;
            this->index += 1;
            goto end_loop;
          }

        case '(':
          {
            result.type = TokenType::LParen;
            this->index += 1;
            goto end_loop;
          }

        case ')':
          {
            result.type = TokenType::RParen;
            this->index += 1;
            goto end_loop;
          }
        case '{':
          {

            result.type = TokenType::LBrace;
            this->index += 1;
            goto end_loop;
          }
        case '}':
          {

            result.type = TokenType::RBrace;
            this->index += 1;
            goto end_loop;
          }
        case '[':
          {

            result.type = TokenType::LBracket;
            this->index += 1;
            goto end_loop;
          }
        case ']':
          {
            result.type = TokenType::RBracket;
            this->index += 1;

            goto end_loop;
          }
        default:
          {
            fprintf(stderr, "%s[Error:Lexer]: Unhandled char `%c`%s\n",
                    Color::Red, c, Color::Reset);
            break;
          }
        }
      }
      break;
    case S_identifier:
      {
        switch (c) {
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '_':
          break;
        default:
          {
            result.type = TokenType::Identifier;
            goto end_loop;
          }
        }
      }
      break;

    case S_numberliteral:
      {
        switch (c) {
        case '0' ... '9':
          break;
        default:
          {
            result.type = TokenType::NumberLiteral;
            goto end_loop;
          }
        }
      }
      break;
    case S_stringliteral:
      {
        switch (c) {
        case '"':
          {
            result.type = TokenType::StringLiteral;
            this->index += 1;
            goto end_loop;
          }
        default:
          break;
        }
      }
      break;

    case S_less_than:
      {
        switch (c) {
        case '<':
          {

            result.type = TokenType::ShiftLeft;
            this->index += 1;
            goto end_loop;
          }
        case '=':
          {

            result.type = TokenType::LessOrEqual;
            this->index += 1;
            goto end_loop;
          }
        default:
          {

            result.type = TokenType::AngleBracketLeft;
            goto end_loop;
          }
        }
      }
      break;
    case S_greater_than:
      {
        switch (c) {
        case '>':
          {

            result.type = TokenType::ShiftRight;
            this->index += 1;
            goto end_loop;
          }
        case '=':
          {

            result.type = TokenType::GreaterOrEqual;
            this->index += 1;
            goto end_loop;
          }
        default:
          {

            result.type = TokenType::AngleBracketRight;
            goto end_loop;
          }
        }
      }
      break;

    case S_plus:
      {
        switch (c) {
        case '+':
          {

            result.type = TokenType::PlusPlus;
            this->index += 1;
            goto end_loop;
          }
        case '=':
          {

            result.type = TokenType::PlusEqual;
            this->index += 1;
            goto end_loop;
          }
        default:
          {

            result.type = TokenType::Plus;
            goto end_loop;
          }
        }
      }
      break;

    case S_minus:
      {
        switch (c) {
        case '-':
          {

            result.type = TokenType::MinusMinus;
            this->index += 1;
            goto end_loop;
          }
        case '=':
          {

            result.type = TokenType::MinusEqual;
            this->index += 1;
            goto end_loop;
          }
        default:
          {

            result.type = TokenType::Minus;
            goto end_loop;
          }
        }
      }
      break;

    case S_asterisk:
      {
        switch (c) {
        case '=':
          {

            result.type = TokenType::AsteriskEqual;
            this->index += 1;
            goto end_loop;
          }
        default:
          {

            result.type = TokenType::Asterisk;
            goto end_loop;
          }
        }
      }
      break;

    case S_slash:
      {
        switch (c) {
        case '/':
          {
            result.type = TokenType::LineComment;
            this->index += 1;
            goto end_loop;
          }
        case '=':
          {

            result.type = TokenType::SlashEqual;
            this->index += 1;
            goto end_loop;
          }
        default:
          {

            result.type = TokenType::Slash;
            goto end_loop;
          }
        }
      }
      break;

    case S_equal:
      {
        switch (c) {

        case '>':
          {
            result.type = TokenType::EqualAngleBracketRt;
            this->index += 1;
            goto end_loop;
          }
        case '=':
          {
            result.type = TokenType::EqualEqual;
            this->index += 1;
            goto end_loop;
          }
        default:
          {

            result.type = TokenType::Equal;
            goto end_loop;
          }
        }
      }
      break;

    case S_colon:
      {
        switch (c) {
        case ':':
          {
            result.type = TokenType::ColonColon;
            this->index += 1;
            goto end_loop;
          }

        case '=':
          {
            result.type = TokenType::ColonEqual;
            this->index += 1;
            goto end_loop;
          }
        default:
          {

            result.type = TokenType::Colon;
            goto end_loop;
          }
        }
      }
      break;
    }
    this->index++;
  } // end of loop

end_loop:
  result.end = this->index;
  return result;
}
