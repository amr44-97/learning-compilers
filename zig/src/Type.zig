const Type = @This();
const std = @import("std");
const ArrayList = std.ArrayList;

pub const TypeKind = enum {
    void_type,
    boolean,
    unsigned_int,
    signed_int,
    unsigned_int_bits,
    signed_int_bits,
    float,
    double,
    unsigned_8,
    signed_8,
    unsigned_16,
    signed_16,
    unsigned_32,
    signed_32,
    unsigned_64,
    signed_64,
};

pub const TypeInfo = struct {
    kind: TypeKind,
};
