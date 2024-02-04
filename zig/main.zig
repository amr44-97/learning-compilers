const std = @import("std");
const tokenizer = @import("tokenizer.zig").Tokenizer;
const Token = @import("tokenizer.zig").Token;
const parser = @import("parser.zig");

const Todo =
    \\ [1]- Handle Types
    \\ [2]- parseVarDecl : you don't need to handle it like C
    \\ [3]- parseIf
    \\ [4]- parseForLoop
    \\ [5]- parseWhileLoop
    \\ [6]- parseFnDecl
;

pub fn main() !void {
    std.debug.print("{s}\n", .{Todo});

    // var alloc = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    // const arena = alloc.allocator();
    // defer alloc.deinit();
    // const buffer: [:0]const u8 =
    //     \\ {
    //     \\  age  = 344;
    //     \\  long  = 344;
    //     \\  u64  = 344;
    //     \\  var  = 344;
    //     \\  Hello  = 344;
    //     \\  name  = 344;
    //     \\ }
    // ;

    // var p = try parser.init(arena, buffer);
    // const node = try p.parseBlock();
    // p.render_tree(node);
    //  p.render_nodes();
}
