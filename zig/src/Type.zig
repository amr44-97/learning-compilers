const Type = @This();

specifier: Specifier,
name: []const u8 = undefined,
pub const Specifier = enum {
    void,
    int,
    uint,
    long,
    ulong,
    char,
    uchar,
    short,
    ushort,
    ptr,
    array,
    bool,
    @"struct",
    @"enum",
    @"union",
};

pub fn spec_from_tag(tag: anytype) Specifier {
    return switch (tag) {
        .keyword_int => .int,
    };
}
