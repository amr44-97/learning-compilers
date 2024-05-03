const Type = @import("Type.zig");
pub const TokenIndex = u32;

pub const Node = struct {
    tag: Tag,
    main_token: TokenIndex,
    type: Type = .{ .specifier = .void },
    data: Data = .{ .none = {} },

    pub const Data = union {
        none: void,
        unary: Index,
        binary: struct {
            lhs: Index,
            rhs: Index,
        },
        range: SubRange,
        decl: struct {
            name: TokenIndex,
            node: Index,
        },
    };

    pub const Index = u32;
    pub const Tag = enum {
        root,
        identifier,
        var_decl,
        string_literal,
        number_literal,
        add,
        sub,
        div,
        mul,
        assign,
        array_type,
        ptr_type,
        typename,
        statement,
        block,
    };

    pub const SubRange = struct {
        start: Index,
        end: Index,
    };
};
