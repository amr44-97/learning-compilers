const tokenizer = @This();
const std = @import("std");
const error_log = @import("Diagnostics.zig").error_log;
const warn_log = @import("Diagnostics.zig").warn_log;

pub const Token = struct {
    tag: Tag,
    loc: Loc,

    pub const Loc = struct {
        start: usize,
        end: usize,
    };

    pub const keywords = std.StaticStringMap(Tag).initComptime(.{
        .{ "int", .keyword_int },
        .{ "long", .keyword_long },
        .{ "void", .keyword_void },
        .{ "float", .keyword_float },
        .{ "short", .keyword_short },
        .{ "signed", .keyword_signed },
        .{ "unsigned", .keyword_unsigned },
        .{ "double", .keyword_double },
        .{ "char", .keyword_char },
        .{ "true", .keyword_true },
        .{ "false", .keyword_false },
        .{ "bool", .keyword_bool },
        .{ "asm", .keyword_asm },
        .{ "break", .keyword_break },
        .{ "catch", .keyword_catch },
        .{ "const", .keyword_const },
        .{ "continue", .keyword_continue },
        .{ "defer", .keyword_defer },
        .{ "else", .keyword_else },
        .{ "enum", .keyword_enum },
        .{ "extern", .keyword_extern },
        .{ "for", .keyword_for },
        .{ "if", .keyword_if },
        .{ "inline", .keyword_inline },
        .{ "pub", .keyword_pub },
        .{ "return", .keyword_return },
        .{ "struct", .keyword_struct },
        .{ "switch", .keyword_switch },
        .{ "try", .keyword_try },
        .{ "union", .keyword_union },
        .{ "volatile", .keyword_volatile },
        .{ "restrict", .keyword_restrict },
        .{ "static", .keyword_static },
        .{ "typedef", .keyword_typedef },
        .{ "typeof", .keyword_typeof },
        .{ "auto", .keyword_auto },
        .{ "while", .keyword_while },
    });

    pub fn getKeyword(bytes: []const u8) ?Tag {
        return keywords.get(bytes);
    }

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
        colon_colon,
        colon_equal,
        comma,
        asterisk,
        line_comment,
        keyword_int,
        keyword_long,
        keyword_void,
        keyword_float,
        keyword_short,
        keyword_signed,
        keyword_unsigned,
        keyword_double,
        keyword_char,
        keyword_true,
        keyword_false,
        keyword_bool,
        keyword_asm,
        keyword_break,
        keyword_catch,
        keyword_const,
        keyword_continue,
        keyword_defer,
        keyword_else,
        keyword_enum,
        keyword_extern,
        keyword_for,
        keyword_if,
        keyword_inline,
        keyword_pub,
        keyword_return,
        keyword_struct,
        keyword_switch,
        keyword_try,
        keyword_union,
        keyword_volatile,
        keyword_restrict,
        keyword_while,
        keyword_static,
        keyword_typedef,
        keyword_typeof,
        keyword_auto,
    };

    pub fn is(self: *Token, ntag: Tag) bool {
        return self.tag == ntag;
    }
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
        colon,
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

                    ':' => {
                        state = .colon;
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
                        if (Token.getKeyword(self.buffer[result.loc.start..self.index])) |tag| {
                            result.tag = tag;
                        }
                        break;
                    },
                },

                .colon => switch (c) {
                    ':' => {
                        result.tag = .colon_colon;
                        break;
                    },
                    '=' => {
                        result.tag = .colon_equal;
                        break;
                    },
                    else => {
                        result.tag = .colon;
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
