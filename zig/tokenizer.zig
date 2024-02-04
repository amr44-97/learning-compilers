const tokenizer = @This();
const std = @import("std");
const error_log = @import("error.zig").error_log;
const warn_log = @import("error.zig").warn_log;

pub const Token = struct {
    tag: Tag,
    loc: Loc,

    pub const Loc = struct {
        start: usize,
        end: usize,
    };

    pub const Tag = enum {
        eof,
        invalid,
        identifier,
        string_literal,
        number_literal,
        open_paren,
        close_paren,
        open_brace,
        close_brace,
        open_bracket,
        close_bracket,
        plus,
        minus,
        slash,
        equal,
        semicolon,
        colon,
        comma,
        asterisk,
        line_comment,
    };
};

pub const Tokenizer = struct {
    buffer: [:0]const u8,
    index: usize,

    pub fn dump(self: *Tokenizer, token: *const Token) void {
        std.debug.print("{s} \"{s}\"\n", .{ @tagName(token.tag), self.buffer[token.loc.start..token.loc.end] });
    }

    pub fn init(buffer: [:0]const u8) Tokenizer {
        // Skip the Byte-Order-Mark if present
        const src_start: usize = if (std.mem.startsWith(u8, buffer, "\xEF\xBB\xBF")) 3 else 0;
        return Tokenizer{
            .buffer = buffer,
            .index = src_start,
        };
    }

    const State = enum {
        start,
        identifier,
        number_literal,
        string_literal,
    };

    pub fn next(self: *Tokenizer) Token {
        var result: Token = .{
            .tag = .eof,
            .loc = .{
                .start = self.index,
                .end = undefined,
            },
        };
        var state: State = .start;
        while (true) : (self.index += 1) {
            const c = self.buffer[self.index];
            switch (state) {
                .start => switch (c) {
                    0 => {
                        if (self.buffer.len != self.index) {
                            warn_log("unexpected `0: eof` at index [{d}]", .{self.index});
                            self.index += 1;
                            result.loc.start = self.index;
                        }
                        break;
                    },

                    ' ', '\n', '\t', '\r' => {
                        result.loc.start = self.index + 1;
                    },
                    'a'...'z', 'A'...'Z', '_' => {
                        state = .identifier;
                    },
                    '"' => {
                        state = .string_literal;
                    },
                    '0'...'9' => {
                        state = .number_literal;
                    },
                    '+' => {
                        result.tag = .plus;
                        self.index += 1;
                        break;
                    },
                    '-' => {
                        result.tag = .minus;
                        self.index += 1;
                        break;
                    },
                    '/' => {
                        result.tag = .slash;
                        self.index += 1;
                        break;
                    },
                    '*' => {
                        result.tag = .asterisk;
                        self.index += 1;
                        break;
                    },
                    '=' => {
                        result.tag = .equal;
                        self.index += 1;
                        break;
                    },
                    ':' => {
                        result.tag = .colon;
                        self.index += 1;
                        break;
                    },
                    ';' => {
                        result.tag = .semicolon;
                        self.index += 1;
                        break;
                    },
                    ',' => {
                        result.tag = .comma;
                        self.index += 1;
                        break;
                    },

                    '{' => {
                        result.tag = .open_brace;
                        self.index += 1;
                        break;
                    },

                    '}' => {
                        result.tag = .close_brace;
                        self.index += 1;
                        break;
                    },

                    '[' => {
                        result.tag = .open_bracket;
                        self.index += 1;
                        break;
                    },
                    ']' => {
                        result.tag = .close_bracket;
                        self.index += 1;
                        break;
                    },
                    '(' => {
                        result.tag = .open_paren;
                        self.index += 1;
                        break;
                    },
                    ')' => {
                        result.tag = .open_paren;
                        self.index += 1;
                        break;
                    },
                    else => {
                        error_log("Unhandled char [{c}]", .{self.buffer[self.index]});
                    },
                },

                .identifier => switch (c) {
                    'a'...'z', 'A'...'Z', '0'...'9', '_' => {},
                    else => {
                        result.tag = .identifier;
                        break;
                    },
                },
                .number_literal => switch (c) {
                    '0'...'9', '_' => {},
                    else => {
                        result.tag = .number_literal;
                        break;
                    },
                },
                .string_literal => switch (c) {
                    else => {},
                    '"' => {
                        result.tag = .string_literal;
                        self.index += 1;
                        break;
                    },
                },
            }
        }

        result.loc.end = self.index;
        return result;
    }
};
