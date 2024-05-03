export enum TokenType {
	eof = "eof",
	numberliteral = "numberliteral",
	stringliteral = "stringliteral",
	identifier = "identifier",
	semicolon = "semicolon",
	hash = "hash",
	plus = "plus",
	plus_plus = "plus_plus",
	plus_equal = "plus_equal",
	equal = "equal",
	equal_equal = "equal_equal",
	minus = "minus",
	minus_minus = "minus_minus",
	minus_equal = "minus_equal",
	slash = "slash",
	slash_equal = "slash_equal",
	line_comment = "line_comment", // "// comment line"
	asterisk = "asterisk",
	asterisk_equal = "asterisk_equal",
	questionmark = "questionmark",
}

export interface Token {
	type: TokenType;
	value: string;
	loc: { offset: number, line: number };
}

function new_token(value: string, type: TokenType, loc: { offset: number, line: number }): Token {
	return {
		type,
		value,
		loc,
	}
}
export function tokenize(source: string): Token[] {
	const tokens = new Array<Token>();
	enum State {
		start,
		numberliteral,
		stringliteral,
		identifier,
		plus,
		minus,
		slash,
		asterisk,
	}
	let state: State = State.start;
	let is_running: boolean = true;
	let i = 0;
	let line: number = 1;
	while (is_running) {
		if (i >= source.length) {
			tokens.push(new_token("<Eof>", TokenType.eof, { offset: source.length, line: line }))
			break;
		}
		let c = source[i];
		switch (c) {
			case ' ':
			case '\n': {
				i++;
			} break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': case 'A': case 'B': case 'C': case 'D':
			case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
			case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
			case 'Y': case 'Z': case '_': {
				let start = i;
				c = source[++i];
				while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
					i++;
					c = source[i];
				}
				tokens.push(new_token(source.substring(start, i), TokenType.identifier, { offset: start, line: line }))
			} break;
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
			case '8': case '9':
				{
					let start = i;
					c = source[++i];
					while (c > '0' && c < '9') {
						i++;
						c = source[i];
					}
					tokens.push(new_token(source.substring(start, i), TokenType.numberliteral, { offset: start, line: line }))
				} break;

			case ';': {
				tokens.push(new_token(";", TokenType.semicolon, { offset: i, line: line }))
				i++;
			} break;

			case '#': {
				tokens.push(new_token("#", TokenType.hash, { offset: i, line: line }))
				i++;
			} break;

			case '?': {
				tokens.push(new_token("?", TokenType.questionmark, { offset: i, line: line }))
				i++;
			} break;

			case '+': {
				c = source[++i];
				switch (c) {
					case '+':
						tokens.push(new_token("++", TokenType.plus, { offset: i, line: line }))
						break;
					case '=':
						tokens.push(new_token("+=", TokenType.plus_equal, { offset: i, line: line }))
						break;
					default:
						tokens.push(new_token("+", TokenType.plus, { offset: i, line: line }))
						break;
				}
				i++;
			} break;
			case '-': {
				c = source[++i];
				switch (c) {
					case '-':
						tokens.push(new_token("--", TokenType.minus_minus, { offset: i, line: line }))
						break;
					case '=':
						tokens.push(new_token("-=", TokenType.minus_equal, { offset: i, line: line }))
						break;
					default:
						tokens.push(new_token("-", TokenType.minus, { offset: i, line: line }))
						break;
				}
				i++;
			} break;
			default: {
				console.log("Unhandled char [" + source.charAt(i) + "]");
				is_running = false;
			} break;
		}
		i += 1;
	}
	return tokens;
}
