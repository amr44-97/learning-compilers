const std = @import("std");
const Tokenizer = @import("Tokenizer.zig");
const Parser = @import("Parser.zig");
const stdout = std.io.getStdOut().writer();
const stdin = std.io.getStdIn();

pub fn main() !void {
    var alloc = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    const arena = alloc.allocator();
    defer alloc.deinit();
    try stdout.print("> ", .{});
    const buffer = try stdin.reader().readUntilDelimiterAlloc(arena, '\n', 256);
    const result: [:0]const u8 = try std.mem.concatWithSentinel(arena, u8, &[_][]u8{buffer}, 0);

    var p = try Parser.init(arena, result);
    try p.render_ast(@intCast(p.nodes.len - 1));
}
