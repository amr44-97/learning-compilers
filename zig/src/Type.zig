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
    pointer,
    array,
    bool,
    @"struct",
    @"enum",
    @"union",
};
