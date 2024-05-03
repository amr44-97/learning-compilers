"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.tokenize = exports.TokenType = void 0;
var TokenType;
(function (TokenType) {
    TokenType["eof"] = "eof";
    TokenType["numberliteral"] = "numberliteral";
    TokenType["stringliteral"] = "stringliteral";
    TokenType["identifier"] = "identifier";
    TokenType["semicolon"] = "semicolon";
    TokenType["hash"] = "hash";
    TokenType["plus"] = "plus";
    TokenType["plus_plus"] = "plus_plus";
    TokenType["plus_equal"] = "plus_equal";
    TokenType["equal"] = "equal";
    TokenType["equal_equal"] = "equal_equal";
    TokenType["minus"] = "minus";
    TokenType["minus_minus"] = "minus_minus";
    TokenType["minus_equal"] = "minus_equal";
    TokenType["slash"] = "slash";
    TokenType["slash_equal"] = "slash_equal";
    TokenType["line_comment"] = "line_comment";
    TokenType["asterisk"] = "asterisk";
    TokenType["asterisk_equal"] = "asterisk_equal";
    TokenType["questionmark"] = "questionmark";
})(TokenType || (exports.TokenType = TokenType = {}));
function new_token(value, type, loc) {
    return {
        type,
        value,
        loc,
    };
}
function tokenize(source) {
    const tokens = new Array();
    let State;
    (function (State) {
        State[State["start"] = 0] = "start";
        State[State["numberliteral"] = 1] = "numberliteral";
        State[State["stringliteral"] = 2] = "stringliteral";
        State[State["identifier"] = 3] = "identifier";
        State[State["plus"] = 4] = "plus";
        State[State["minus"] = 5] = "minus";
        State[State["slash"] = 6] = "slash";
        State[State["asterisk"] = 7] = "asterisk";
    })(State || (State = {}));
    let state = State.start;
    let is_running = true;
    let i = 0;
    let line = 1;
    while (is_running) {
        if (i >= source.length) {
            tokens.push(new_token("<Eof>", TokenType.eof, { offset: source.length, line: line }));
            break;
        }
        let c = source[i];
        switch (c) {
            case ' ':
            case '\n':
                {
                    i++;
                }
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'm':
            case 'n':
            case 'o':
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'w':
            case 'x':
            case 'y':
            case 'z':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
            case '_':
                {
                    let start = i;
                    c = source[++i];
                    while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                        i++;
                        c = source[i];
                    }
                    tokens.push(new_token(source.substring(start, i), TokenType.identifier, { offset: start, line: line }));
                }
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                {
                    let start = i;
                    c = source[++i];
                    while (c > '0' && c < '9') {
                        i++;
                        c = source[i];
                    }
                    tokens.push(new_token(source.substring(start, i), TokenType.numberliteral, { offset: start, line: line }));
                }
                break;
            case ';':
                {
                    tokens.push(new_token(";", TokenType.semicolon, { offset: i, line: line }));
                    i++;
                }
                break;
            case '#':
                {
                    tokens.push(new_token("#", TokenType.hash, { offset: i, line: line }));
                    i++;
                }
                break;
            case '?':
                {
                    tokens.push(new_token("?", TokenType.questionmark, { offset: i, line: line }));
                    i++;
                }
                break;
            case '+':
                {
                    c = source[++i];
                    switch (c) {
                        case '+':
                            tokens.push(new_token("++", TokenType.plus, { offset: i, line: line }));
                            break;
                        case '=':
                            tokens.push(new_token("+=", TokenType.plus_equal, { offset: i, line: line }));
                            break;
                        default:
                            tokens.push(new_token("+", TokenType.plus, { offset: i, line: line }));
                            break;
                    }
                    i++;
                }
                break;
            case '-':
                {
                    c = source[++i];
                    switch (c) {
                        case '-':
                            tokens.push(new_token("--", TokenType.minus_minus, { offset: i, line: line }));
                            break;
                        case '=':
                            tokens.push(new_token("-=", TokenType.minus_equal, { offset: i, line: line }));
                            break;
                        default:
                            tokens.push(new_token("-", TokenType.minus, { offset: i, line: line }));
                            break;
                    }
                    i++;
                }
                break;
            default:
                {
                    console.log("Unhandled char [" + source.charAt(i) + "]");
                    is_running = false;
                }
                break;
        }
        i += 1;
    }
    return tokens;
}
exports.tokenize = tokenize;
