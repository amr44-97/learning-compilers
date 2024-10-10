const Ast = @This();
const std = @import("std");
const Token = @import("Tokenizer.zig").Token;
const Allocator = std.mem.Allocator;

const ArrayList = std.ArrayList;

pub const TokenIndex = u32;
pub const ByteOffset = u32;

pub const TokenList = std.MultiArrayList(struct {
    tag: Token.Tag,
    start: ByteOffset,
});

pub const Node = struct {
    tag: Tag,
    main_token: TokenIndex,
    data: Data,

    pub const Index = u32;

    pub const Tag = enum {
        root,
        identifier,
        string_literal,
        number_literal,
        add,
        sub,
        div,
        mul,
        mod,
        array_cat,
        bool_not,
        bool_or,
        bool_and,
        negation,
        bit_xor,
        bit_not,
        bit_or,
        bit_and,
        address_of,
        equal_equal,
        bang_equal,
        binary_expr,
        container,
        @"return",
        @"struct",
        @"enum",
        assign,
        array_type,
        array_type_sentinel,
        ptr_type,
        primitive_type,
        typename,
        statement,
        block,
        params,
        fn_decl,
        if_simple,
        @"if",
        const_decl,
        var_decl,
        var_dec_inferred_type,
        var_decl_proto,
        simple_var_decl,
    };

    const DataType = enum {
        none,
        binary_expr,
        block,
        params,
        fn_decl,
        if_expr,
        //    typeinfo,
    };
    const Data = union(DataType) {
        none: void,
        binary_expr: BinaryExpr,
        block: Block,
        params: Params,
        fn_decl: FnDecl,
        if_expr: If,
        //     typeinfo: TypeInfo,
    };
};

pub const Block = ArrayList(Node.Index);

pub const Params = []Node.Index;

pub const FnDecl = struct {
    name: Node.Index,
    params: Node.Index,
    ret_type: Node.Index,
    body: Node.Index,
};

pub const BinaryExpr = struct {
    lhs: Node.Index,
    rhs: Node.Index,
};

pub const If = struct {
    condition: Node.Index,
    then_expr: Node.Index,
    else_expr: Node.Index,
};
