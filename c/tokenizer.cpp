#include "tokenizer.h"

Tokenizer tokenizer_init(char *src) {
  return Tokenizer{
      .source = src,
      .index = 0,
  };
}
Token tokenizer_next(Tokenizer *T) {
  enum State {
    S_start,
    S_identifier,
    S_numberliteral,
    S_stringliteral,
  };

  enum State state = S_start;
  Token result = Token{Tok_eof, 1, T->index, 0};

  while (true) {
    char c = T->source[T->index];
    switch (state) {
    case S_start: {
      switch (c) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '_': {
        state = S_identifier;
      } break;
      case '0' ... '9': {
        state = S_numberliteral;
      } break;
      case '"': {
        state = S_stringliteral;
      } break;
	  case '(':{
				   result.type = Tok_l_paren;
				   T->index +=1;
				   goto end_loop;
			   }
	  case ')':{

				   result.type = Tok_r_paren;
				   T->index +=1;
				   goto end_loop;
			   }
	  case '{':{

				   result.type = Tok_l_brace;
				   T->index +=1;
				   goto end_loop;
			   }
	  case '}':{

				   result.type = Tok_r_brace;
				   T->index +=1;
				   goto end_loop;
			   }
	  case '[':{

				   result.type = Tok_l_bracket;
				   T->index +=1;
				   goto end_loop;
			   }
	  case ']':{

				   result.type = Tok_r_bracket;
				   T->index +=1;
				   goto end_loop;
			   }
	  }
    } break;
    case S_identifier: {
      switch (c) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '0' ... '9':
      case '_':
        break;
      default: {
        result.type = Tok_identifier;
        goto end_loop;
      }
      }
    } break;

    case S_numberliteral: {
      switch (c) {
      case '0' ... '9':
        break;
      default: {
        result.type = Tok_numberliteral;
        goto end_loop;
      }
      }
    } break;
    case S_stringliteral: {
      switch (c) {
      case '"': {
        result.type = Tok_stringliteral;
        T->index += 1;
        goto end_loop;
      }
      default:
        break;
      }
    } break;
    }
    T->index++;
  } // end of loop

end_loop:
  return result;
}
