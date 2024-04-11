use ansi_term::Color::Red;
use std::fmt::{self};

#[allow(dead_code)]
pub enum TokenKind {
    Eof,
    Invalid,
    Identifier,
    NumberLiteral,
    StringLiteral,
    Plus,
    PlusPlus,
    PlusEqual,
    Minus,
    MinusMinus,
    MinusEqual,
    Asterisk,
    AsteriskAsterisk,
    AsteriskEqual,
    Slash,
    SlashSlash,
    SlashEqual,
    SemiColon,
    Comma,
    ArrowOp,
    Dot,
    Colon,
    ColonEqual,
    ColonColon,
    Equal,
    EqualEqual,
    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
}

impl fmt::Display for TokenKind {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let output = match *self {
            TokenKind::Eof => "Eof",
            TokenKind::Invalid => "Invalid",
            TokenKind::Identifier => "Identifier",
            TokenKind::NumberLiteral => "NumberLiteral",
            TokenKind::StringLiteral => "StringLiteral",
            TokenKind::Plus => "Plus",
            TokenKind::PlusPlus => "PlusPlus",
            TokenKind::PlusEqual => "PlusEqual",
            TokenKind::Minus => "Minus",
            TokenKind::MinusMinus => "MinusMinus",
            TokenKind::MinusEqual => "MinusEqual",
            TokenKind::Asterisk => "Asterisk",
            TokenKind::AsteriskAsterisk => "AsteriskAsterisk",
            TokenKind::AsteriskEqual => "AsteriskEqual",
            TokenKind::Slash => "Slash",
            TokenKind::SlashSlash => "SlashSlash",
            TokenKind::SlashEqual => "SlashEqual",
            TokenKind::SemiColon => "SemiColon",
            TokenKind::Comma => "Comma",
            TokenKind::ArrowOp => "ArrowOp",
            TokenKind::Dot => "Dot",
            TokenKind::Colon => "Colon",
            TokenKind::ColonEqual => "ColonEqual",
            TokenKind::ColonColon => "ColonColon",
            TokenKind::Equal => "Equal",
            TokenKind::EqualEqual => "EqualEqual",
            TokenKind::LParen => "LParen",
            TokenKind::RParen => "RParen",
            TokenKind::LBrace => "LBrace",
            TokenKind::RBrace => "RBrace",
            TokenKind::LBracket => "LBracket",
            TokenKind::RBracket => "RBracket",
        };
        write!(f, "{}", output)
    }
}

pub struct Token {
    pub kind: TokenKind,
    pub start: u32,
    pub end: u32,
    pub line: i32,
    col: i32,
}

impl fmt::Display for Token {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "({}) -> [{}:{}]", self.kind, self.line, self.col)
    }
}

pub struct Lexer<'a> {
    pub source: &'a str,
    index: u32,
    column: i32,
    line: i32,
}

impl<'a> Lexer<'a> {
    pub fn new(src: &'a str) -> Self {
        Lexer {
            source: src,
            index: 0,
            column: 0,
            line: 1,
        }
    }
}

impl<'a> Lexer<'a> {
    pub fn next_token(&mut self) -> Token {
        enum State {
            Start,
            Identifier,
            NumberLiteral,
            StringLiteral,
            Equal,
            Plus,
            Minus,
            Asterisk,
            Slash,
            Colon,
        }

        let mut state: State = State::Start;
        let mut result: Token = Token {
            kind: TokenKind::Eof,
            start: self.index,
            end: 0,
            line: self.line,
            col: 0,
        };

        loop {
            if self.source.len() == (self.index as usize) {
                result.kind = TokenKind::Eof;
                break;
            }

            let c = self.source.chars().nth(self.index as usize).unwrap();
            match state {
                State::Start => match c {
                    'a'..='z' | 'A'..='Z' | '_' => {
                        state = State::Identifier;
                        result.col = self.column;
                    }
                    '0'..='9' => {
                        state = State::NumberLiteral;
                        result.col = self.column;
                    }
                    '"' => {
                        state = State::StringLiteral;
                        result.col = self.column;
                    }
                    ' ' | '\t' | '\r' => {
                        result.start += 1;
                        //self.column += 1;
                        result.col += 1;
                    }
                    '\n' => {
                        self.line += 1;
                        self.column = -1;
                        result.col = -1;
                        result.start += 1;
                    }
                    '=' => {
                        state = State::Equal;
                        result.col = self.column;
                    }
                    '+' => {
                        state = State::Plus;
                        result.col = self.column;
                    }
                    '-' => {
                        state = State::Minus;
                        result.col = self.column;
                    }
                    '*' => {
                        state = State::Asterisk;
                        result.col = self.column;
                    }
                    '/' => {
                        state = State::Slash;
                        result.col = self.column;
                    }
                    ':' => {
                        state = State::Colon;
                        result.col = self.column;
                    }
                    ';' => {
                        result.kind = TokenKind::SemiColon;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }

                    ',' => {
                        result.kind = TokenKind::Comma;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    '.' => {
                        result.kind = TokenKind::Dot;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    '(' => {
                        result.kind = TokenKind::LParen;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    ')' => {
                        result.kind = TokenKind::RParen;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    '{' => {
                        result.kind = TokenKind::LBrace;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    '}' => {
                        result.kind = TokenKind::RBrace;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    '[' => {
                        result.kind = TokenKind::LBracket;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    ']' => {
                        result.kind = TokenKind::RBracket;
                        result.col = self.column;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }

                    _ => {
                        println!(
                            "{}",
                            Red.paint(format!(
                                "[Error]:UnHandled Char =>  [{}:{}]  -> `{}`",
                                self.line, self.column, c
                            ))
                            .to_string()
                        );
                    }
                },

                State::Identifier => match c {
                    'a'..='z' | 'A'..='Z' | '_' => (),
                    _ => {
                        result.kind = TokenKind::Identifier;
                        break;
                    }
                },
                State::NumberLiteral => match c {
                    '0'..='9' | '_' => (),
                    _ => {
                        result.kind = TokenKind::NumberLiteral;
                        break;
                    }
                },
                State::StringLiteral => match c {
                    '"' => {
                        result.kind = TokenKind::StringLiteral;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    _ => (),
                },
                State::Plus => match c {
                    '+' => {
                        result.kind = TokenKind::PlusPlus;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }

                    '=' => {
                        result.kind = TokenKind::PlusEqual;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    _ => {
                        result.kind = TokenKind::Plus;
                        break;
                    }
                },

                State::Minus => match c {
                    '-' => {
                        result.kind = TokenKind::MinusMinus;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }

                    '>' => {
                        result.kind = TokenKind::ArrowOp;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    '=' => {
                        result.kind = TokenKind::MinusEqual;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    _ => {
                        result.kind = TokenKind::Minus;
                        break;
                    }
                },

                State::Asterisk => match c {
                    '*' => {
                        result.kind = TokenKind::AsteriskAsterisk;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }

                    '=' => {
                        result.kind = TokenKind::AsteriskEqual;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    _ => {
                        result.kind = TokenKind::Asterisk;
                        break;
                    }
                },
                State::Slash => match c {
                    '/' => {
                        result.kind = TokenKind::SlashSlash;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }

                    '=' => {
                        result.kind = TokenKind::SlashEqual;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    _ => {
                        result.kind = TokenKind::Slash;
                        break;
                    }
                },

                State::Equal => match c {
                    '=' => {
                        result.kind = TokenKind::EqualEqual;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    _ => {
                        result.kind = TokenKind::Equal;
                        break;
                    }
                },

                State::Colon => match c {
                    ':' => {
                        result.kind = TokenKind::ColonColon;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }

                    '=' => {
                        result.kind = TokenKind::ColonEqual;
                        self.index += 1;
                        self.column += 1;
                        break;
                    }
                    _ => {
                        result.kind = TokenKind::Colon;
                        break;
                    }
                },
            }
            self.index += 1;
            self.column += 1;
        } // end loop

        result.end = self.index;
        result.line = self.line;
        return result;
    }
}
