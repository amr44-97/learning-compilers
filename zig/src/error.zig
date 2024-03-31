const std = @import("std");

pub fn error_log(comptime format: []const u8, args: anytype) void {
    std.debug.print("ERROR:" ++ format ++ "\n", args);
    std.process.exit(3);
}

pub fn warn_log(comptime format: []const u8, args: anytype) void {
    std.debug.print("Warning:" ++ format ++ "\n", args);
}
