const std = @import("std");

pub const Tag = enum{
    duplicate_decl_spec,
    expected_semicolon,
    expected_expr,
    expected_type_expr,
    expected_token,
    expected_primary_expr,
};
