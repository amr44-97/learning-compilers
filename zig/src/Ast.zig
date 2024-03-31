pub const TokenIndex = u32;

pub const Node = struct {
    tag: Tag,
    main_token: TokenIndex,
    data: Data,

    pub const Data = struct {
        lhs: Index,
        rhs: Index,
    };

    pub const Index = u32;
    pub const Tag = enum {
        root,
        identifier,
        var_decl,
        string_literal,
        number_literal,
        assign,
        ptr_type,
        statement,
        block,
        add,
        sub,
        div,
        mul,
    };

    pub const SubRange = struct {
        /// Index into sub_list.
        start: Index,
        /// Index into sub_list.
        end: Index,
    };
};
