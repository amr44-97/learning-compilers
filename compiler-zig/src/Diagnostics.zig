const std = @import("std");
const TokenTag = @import("tokenizer.zig").Token.Tag;

const TokenIndex = u32;

pub const CompilationError = struct {
    msg: ?[]const u8,
    token: ?TokenIndex,
    tag: Tag,

    pub const Tag = enum {
        duplicate_decl_spec,
        expected_semicolon,
        expected_expr,
        expected_type_expr,
        expected_token,
        expected_primary_expr,
        storage_class_not_allowed,
        combine_typedef_with_decl_spec,
    };
};

pub fn error_log(comptime format: []const u8, args: anytype) void {
    std.debug.print(Color.get(.bold) ++ Color.get(.red) ++ "[ERROR]: " ++ format ++ Color.get(.reset) ++ "\n", args);
    std.process.exit(3);
}
pub fn warn_log(comptime format: []const u8, args: anytype) void {
    std.debug.print(Color.get(.yellow) ++ "[Warning]: " ++ format ++ Color.get(.reset) ++ "\n", args);
}

pub const Color = enum {
    black,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
    bright_black,
    bright_red,
    bright_green,
    bright_yellow,
    bright_blue,
    bright_magenta,
    bright_cyan,
    bright_white,
    bold,
    dim,
    reset,

    pub fn get(comptime color: Color) []const u8 {
        return switch (color) {
            .black => "\x1b[30m",
            .red => "\x1b[31m",
            .green => "\x1b[32m",
            .yellow => "\x1b[33m",
            .blue => "\x1b[34m",
            .magenta => "\x1b[35m",
            .cyan => "\x1b[36m",
            .white => "\x1b[37m",
            .bright_black => "\x1b[90m",
            .bright_red => "\x1b[91m",
            .bright_green => "\x1b[92m",
            .bright_yellow => "\x1b[93m",
            .bright_blue => "\x1b[94m",
            .bright_magenta => "\x1b[95m",
            .bright_cyan => "\x1b[96m",
            .bright_white => "\x1b[97m",
            .bold => "\x1b[1m",
            .dim => "\x1b[2m",
            .reset => "\x1b[0m",
        };
    }
};
