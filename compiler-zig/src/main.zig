const std = @import("std");
const tokenizer = @import("tokenizer.zig").Tokenizer;
const Token = @import("tokenizer.zig").Token;
const parser = @import("parser.zig");
const stdout = std.io.getStdOut().writer();
const stdin = std.io.getStdIn();
const Todo =
    \\ [1]- Handle Types
    \\ [2]- parseVarDecl : you don't need to handle it like C
    \\ [3]- parseIf
    \\ [4]- parseForLoop
    \\ [5]- parseWhileLoop
    \\ [6]- parseFnDecl
;

pub fn main() !void {
    var alloc = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    const arena = alloc.allocator();
    defer alloc.deinit();
    try stdout.print("> ", .{});
    // const in_buffer = try arena.allocSentinel(u8, 256, 0);
    //   _ = in_buffer; // autofix
    const result = try stdin.reader().readUntilDelimiterAlloc(arena, '\n', 256);
    std.debug.print("{s}\n", .{result});
    const rss = try std.mem.concatWithSentinel(arena, u8, &[_][]u8{result}, 0);
    const buffer = [_][:0]const u8{
        \\ {
        \\  age  = 144;
        \\  long  = 244;
        \\  u64  = 344;
        \\  var  = 444;
        \\  Hello  = 544;
        \\  name  = 644;
        \\ }
        ,
        "char***;",
    };
    _ = buffer;
    var p = try parser.init(arena, rss);

    const node = try p.parseTypeExpr(0);
    //  p.render_nodes();
    try p.render_tree(node);
    // std.debug.print("{s}\n", .{try p.getTypeName(node)});

}
