const std = @import("std");
const Type = @This();
const Token = @import("tokenizer.zig").Token;
const NodeIndex = u32; //@import("Ast.zig").Node.Index;
const TokenIndex = u32; //@import("Ast.zig").TokenIndex;
const Parser = @import("parser.zig");

specifier: Specifier,
name: TokenIndex = undefined,
data: union {
    sub_type: *Type,
    array: *Array,
    expr: *Expr,
    none: void,
} = .{ .none = {} },

pub const Array = struct {
    elem: Type,
    len: u64,
};

pub const Expr = struct {
    node: NodeIndex,
    ty: Type,
};

pub const Specifier = enum {
    void,
    bool,
    int,
    signed_int,
    unsigned_int,
    long,
    signed_long,
    unsigned_long,
    char,
    signed_char,
    unsigned_char,
    short,
    signed_short,
    unsigned_short,
    user_defined_type,
    pointer,
    array,
    static_array,
    typeof_type,
    typeof_expr,
    @"struct",
    @"enum",
    @"union",
};

pub fn get(ty: *const Type, specifier: Specifier) ?*const Type {
    std.debug.assert(specifier != .typeof_type and specifier != .typeof_expr);
    return switch (ty.specifier) {
        .typeof_type => ty.data.sub_type.get(specifier),
        .typeof_expr => ty.data.expr.ty.get(specifier),
        else => if (ty.specifier == specifier) ty else null,
    };
}

/// Determine if type matches the given specifier, recursing into typeof
/// types if necessary.
pub fn is(ty: Type, specifier: Specifier) bool {
    std.debug.assert(specifier != .typeof_type and specifier != .typeof_expr);
    return ty.get(specifier) != null;
}

const type_names = std.StaticStringMap(Specifier).initComptime(.{
    .{ "void", .void },
    .{ "bool", .bool },
    .{ "char", .char },
    .{ "short", .short },
    .{ "int", .int },
    .{ "long", .long },
    .{ "struct", .@"struct" },
    .{ "union", .@"union" },
    .{ "enum", .@"enum" },
});

pub fn is_typename(tag: Token.Tag) bool {
    return switch (tag) {
        .keyword_int,
        .keyword_long,
        .keyword_void,
        .keyword_float,
        .keyword_short,
        .keyword_signed,
        .keyword_unsigned,
        .keyword_double,
        .keyword_bool,
        .keyword_struct,
        .keyword_enum,
        .keyword_union,
        .keyword_const,
        .keyword_const,
        .keyword_volatile,
        .keyword_restrict,
        .keyword_inline,
        .keyword_typedef,
        .keyword_typeof,
        .keyword_char,
        => true,
        else => false,
    };
}
