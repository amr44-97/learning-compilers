const parser = @This();
const std = @import("std");
const assert = std.debug.assert;
const Allocator = std.mem.Allocator;
const Token = @import("tokenizer.zig").Token;
const Tokenizer = @import("tokenizer.zig").Tokenizer;
const Node = @import("Ast.zig").Node;
const print = std.debug.print;
const error_log = @import("error.zig").error_log;
const warn_log = @import("error.zig").warn_log;
const TokenIndex = @import("Ast.zig").TokenIndex;
const null_node: Node.Index = 0;

allocator: std.mem.Allocator,
source: [:0]const u8,
tokens: std.ArrayListUnmanaged(Token),
tok_i: usize,
nodes: std.ArrayListUnmanaged(Node),
extra_data: std.ArrayListUnmanaged(Node.Index),
scratch: std.ArrayListUnmanaged(Node.Index),
errors: std.ArrayListUnmanaged(ParseError),

pub const ParseError = struct {
    tag: Tag,
    pub const Tag = enum {
        expected_primaryExpression,
        expected_semicolon,
    };
};

pub fn init(allocator: Allocator, source: [:0]const u8) !parser {
    var Parser: parser = .{
        .allocator = allocator,
        .source = source,
        .tokens = .{},
        .tok_i = 0,
        .nodes = .{},
        .extra_data = .{},
        .scratch = .{},
        .errors = .{},
    };

    var tokenizer = Tokenizer.init(source);
    while (true) {
        const token = tokenizer.next();
        try Parser.tokens.append(allocator, token);
        if (token.tag == .eof) break;
    }

    _ = try Parser.addNode(.{
        .tag = .root,
        .main_token = 0,
        .data = .{
            .lhs = 0,
            .rhs = 0,
        },
    });

    return Parser;
}

pub fn deinit(self: *parser) !void {
    self.tokens.deinit(self.allocator);
    self.nodes.deinit(self.allocator);
    self.extra_data.deinit(self.allocator);
    self.errors.deinit(self.allocator);
}

fn listToSpan(self: *parser, list: []const Node.Index) !Node.SubRange {
    try self.extra_data.appendSlice(self.allocator, list);
    return Node.SubRange{
        .start = @as(Node.Index, @intCast(self.extra_data.items.len - list.len)),
        .end = @as(Node.Index, @intCast(self.extra_data.items.len)),
    };
}

pub fn addNode(self: *parser, node: Node) Allocator.Error!Node.Index {
    const index = @as(u32, @intCast(self.nodes.items.len));
    try self.nodes.append(self.allocator, node);
    return index;
}

pub fn nextToken(self: *parser) TokenIndex {
    const token_index = self.tok_i;
    self.tok_i += 1;
    return @as(u23, @intCast(token_index));
}

fn expectToken(self: *parser, tag: Token.Tag) !TokenIndex {
    const token = self.tokens.items[self.tok_i];
    if (token.tag != tag) {
        error_log("expected token {s} but found {s}", .{ @tagName(token.tag), @tagName(tag) });
    }
    return self.nextToken();
}

pub fn eatToken(self: *parser, tag: Token.Tag) ?TokenIndex {
    if (self.tokens.items[self.tok_i].tag == tag) return self.nextToken() else return null;
}

pub fn parserPrimaryTypeExpr(self: *parser) !Node.Index {
    const token_tags = self.tokens.items;
    switch (token_tags[self.tok_i].tag) {
        .identifier => return try self.addNode(.{
            .tag = .identifier,
            .main_token = self.nextToken(),
            .data = .{
                .lhs = 0,
                .rhs = 0,
            },
        }),
        .string_literal => return try self.addNode(.{
            .tag = .string_literal,
            .main_token = self.nextToken(),
            .data = .{
                .lhs = 0,
                .rhs = 0,
            },
        }),
        .number_literal => return try self.addNode(.{
            .tag = .number_literal,
            .main_token = self.nextToken(),
            .data = .{
                .lhs = 0,
                .rhs = 0,
            },
        }),
        else => {
            return null_node;
        },
    }
}

fn expectPrimaryTypeExpr(p: *parser) !Node.Index {
    const node = try p.parsePrimaryTypeExpr();
    if (node == 0) {
        error_log("expected PrimaryTypeExpr but found `{s}` at [{d}]", .{ @tagName(p.tokens.items[p.tok_i].tag), p.tokens.items[p.tok_i].loc.start });
    }
    return node;
}

const OpInfo = struct {
    tag: Node.Tag,
    prec: i8,
};

const operTable = std.enums.directEnumArrayDefault(Token.Tag, OpInfo, .{ .prec = -1, .tag = Node.Tag.root }, 0, .{
    .plus = .{ .tag = .add, .prec = 60 },
    .minus = .{ .tag = .sub, .prec = 60 },
    .slash = .{ .tag = .div, .prec = 70 },
    .asterisk = .{ .tag = .mul, .prec = 70 },
});

// 4 + 5
pub fn parseExprPrecdence(self: *parser, prev_prec: i8) !Node.Index {
    var lhs = try self.parserPrimaryExpr();
    if (lhs == null_node) {
        return null_node;
    }

    while (true) {
        const op_tag = self.tokens.items[self.tok_i].tag;
        const info = operTable[@as(usize, @intCast(@intFromEnum(op_tag)))];
        if (info.prec <= prev_prec) break;
        const op_token = self.nextToken();
        const rhs = try self.parseExprPrecdence(info.prec);
        if (rhs == null_node) {
            warn_log("expected expression", .{});
            return lhs;
        }
        lhs = try self.addNode(.{
            .tag = info.tag,
            .main_token = op_token,
            .data = .{
                .lhs = lhs,
                .rhs = rhs,
            },
        });
    }
    return lhs;
}

// identifier = expression | identifier ;
pub fn parseAssign(self: *parser) !Node.Index {
    const lhs = try self.parserPrimaryExpr();
    if (lhs == null_node) return null_node;
    const eql_token = self.nextToken();
    return try self.addNode(.{
        .tag = .assign,
        .main_token = eql_token,
        .data = .{
            .lhs = lhs,
            .rhs = try self.parseExprPrecdence(0),
        },
    });
}

pub fn parseStatement(self: *parser) !Node.Index {
    const assign_stmt = try self.parseAssign();
    if (assign_stmt == null_node) {
        return null_node;
    }
    _ = try self.expectToken(.semicolon);
    return assign_stmt;
}

// ptr_type(Type(* (ptr_type)*)) name([])
pub fn parseTypeExpr(self: *parser) !Node.Index {
    // parsePrimaryExpr -> Type
    // loop asterisk and add it
    // name_index = nextToken() -> name
    // switch(current_token)
    // if open_bracket
    const tokens = self.tokens.items;
    const elem_token = self.tok_i;
    const elem_type = try self.parserPrimaryTypeExpr();
    if (self.nodes.items[elem_type].tag != .identifier) {
        error_log("expected Type expression but found {s}", .{@tagName(self.nodes.items[elem_type].tag)});
    }

    switch (tokens[self.tok_i].tag) {
        .asterisk => {},
        else => {},
    }
}

pub fn expectTypeExpr(p: *parser) !Node.Index {
    const node = try p.parseTypeExpr();
    if (node == 0) {
        error_log("expected TypeExpr but found `{s}` at [{d}]", .{ @tagName(p.tokens.items[p.tok_i].tag), p.tokens.items[p.tok_i].loc.start });
    }
    return node;
}

pub fn parseBlock(self: *parser) !Node.Index {
    const open_brace = self.eatToken(.open_brace) orelse return null_node;
    const scratch_top = self.scratch.items.len;
    defer self.scratch.shrinkRetainingCapacity(scratch_top);
    while (true) {
        if (self.tokens.items[self.tok_i].tag == .close_brace) break;
        const stmt = try self.parseStatement();
        if (stmt == 0) break;
        // all their indices are in scratch
        // so scratch = [s1,s2,s3,s4,s5]
        try self.scratch.append(self.allocator, stmt);
    }
    _ = try self.expectToken(.close_brace);
    const statements = self.scratch.items[scratch_top..];
    switch (statements.len) {
        0 => return self.addNode(.{
            .tag = .block,
            .main_token = open_brace,
            .data = .{
                .lhs = 0,
                .rhs = 0,
            },
        }),
        1 => return self.addNode(.{
            .tag = .block,
            .main_token = open_brace,
            .data = .{
                .lhs = statements[0],
                .rhs = 0,
            },
        }),
        2 => return self.addNode(.{
            .tag = .block,
            .main_token = open_brace,
            .data = .{
                .lhs = statements[0],
                .rhs = statements[1],
            },
        }),
        else => {
            // the indices of the statements are in extra_data
            // so extra_data[span.start..] -> Node.Index..
            // e.g: node = nodes[extra_data[lhs]]
            const span = try self.listToSpan(statements);
            return self.addNode(.{
                .tag = .block,
                .main_token = open_brace,
                .data = .{
                    .lhs = span.start,
                    .rhs = span.end,
                },
            });
        },
    }
}

pub fn render_nodes(p: *parser) void {
    var i: u32 = 0;
    for (p.nodes.items) |node| {
        const token = p.tokens.items[node.main_token];
        print("[{d}]:[{s} {s}]\n", .{ i, @tagName(node.tag), p.source[token.loc.start..token.loc.end] });
        i += 1;
    }
}

pub fn render_tree(self: *parser, node_index: Node.Index) void {
    self.renderAstTree(node_index, "", false);
}

fn renderAstTree(p: *parser, node: Node.Index, prefix: []const u8, isLeft: bool) void {
    const nodes = p.nodes.items;
    const real_node = nodes[node];
    if (real_node.tag == .root) {
        return;
    }
    if (real_node.tag == .block) {
        const token = p.tokens.items[real_node.main_token];
        print("{s} {s}\n", .{ @tagName(real_node.tag), p.source[token.loc.start..token.loc.end] });
        // if (real_node.data.lhs == 0 and real_node.data.rhs != 0) {
        const block_stmts = p.extra_data.items[real_node.data.lhs..real_node.data.rhs];
        for (block_stmts) |node_i| {
            p.renderAstTree(node_i, "", true);
        }
        //} else {
        // p.renderAstTree(real_node.data.lhs, "", true);
        // p.renderAstTree(real_node.data.rhs, "", true);
        //}
        print("     {s}\n", .{"}"});
        return;
    }

    print("{s}", .{prefix});
    if (isLeft) {
        print("├──", .{});
    } else {
        print("└──", .{});
    }

    const token = p.tokens.items[real_node.main_token];
    print("{s} {s}\n", .{ @tagName(real_node.tag), p.source[token.loc.start..token.loc.end] });

    const new_prefix = std.mem.concat(p.allocator, u8, &[_][]const u8{ prefix, if (isLeft) "│   " else "    " }) catch |err| {
        error_log("failed to concat prefix at renderAstTree with error {s}", .{@errorName(err)});
    };
    renderAstTree(p, real_node.data.lhs, new_prefix, true);
    renderAstTree(p, real_node.data.rhs, new_prefix, false);
}
