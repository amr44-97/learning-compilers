const Type = @import("Type.zig");
pub const TokenIndex = u32;

pub const Node = struct {
    tag: Tag,
    main_token: TokenIndex,
    type: Type = .{ .specifier = .void },
    data: Data,

    pub const Data = struct {
        lhs: Index = 0,
        rhs: Index = 0,
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
