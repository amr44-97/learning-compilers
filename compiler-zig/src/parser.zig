const Parser = @This();

const std = @import("std");
const assert = std.debug.assert;
const Allocator = std.mem.Allocator;
const Token = @import("tokenizer.zig").Token;
const Tokenizer = @import("tokenizer.zig").Tokenizer;
const Ast = @import("Ast.zig");
const Node = Ast.Node;
const Type = @import("Type.zig");
const print = std.debug.print;
const Diagnostics = @import("Diagnostics.zig");
const CompError = Diagnostics.CompilationError;
const error_log = Diagnostics.error_log;
const warn_log = Diagnostics.warn_log;
const TokenIndex = @import("Ast.zig").TokenIndex;
const null_node: Node.Index = 0;

const Error = error.ParsingError;

allocator: std.mem.Allocator,
source: [:0]const u8,
tokens: std.ArrayListUnmanaged(Token),
tok_i: usize,
nodes: std.ArrayListUnmanaged(Node),
extra_data: std.ArrayListUnmanaged(Node.Index),
scratch: std.ArrayListUnmanaged(Node.Index),
errors: std.ArrayListUnmanaged(CompError),

fn addDiagnostic(self: *Parser, tag: CompError.Tag, comptime msg: []const u8, args: anytype) !void {
    const buffer = try self.allocator.alloc(u8, 512);
    _ = try std.fmt.bufPrint(buffer, msg, args);
    try self.errors.append(self.allocator, .{
        .msg = buffer,
        .tag = tag,
        .token = 0,
    });
}

pub fn init(allocator: Allocator, source: [:0]const u8) !Parser {
    var parser: Parser = .{
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
        try parser.tokens.append(allocator, token);
        if (token.tag == .eof) break;
    }
    _ = try parser.addNode(.{
        .tag = .root,
        .main_token = 0,
        .data = .{ .lhs = 0, .rhs = 0 },
    });

    return parser;
}

pub fn deinit(self: *Parser) !void {
    self.tokens.deinit(self.allocator);
    self.nodes.deinit(self.allocator);
    self.extra_data.deinit(self.allocator);
    self.errors.deinit(self.allocator);
}

fn listToSpan(self: *Parser, list: []const Node.Index) !Node.SubRange {
    try self.extra_data.appendSlice(self.allocator, list);
    return Node.SubRange{
        .start = @as(Node.Index, @intCast(self.extra_data.items.len - list.len)),
        .end = @as(Node.Index, @intCast(self.extra_data.items.len)),
    };
}

pub fn addNode(self: *Parser, node: Node) Allocator.Error!Node.Index {
    const index = @as(u32, @intCast(self.nodes.items.len));
    try self.nodes.append(self.allocator, node);
    return index;
}

pub inline fn nextToken(self: *Parser) TokenIndex {
    const token_index = self.tok_i;
    self.tok_i += 1;
    return @as(u23, @intCast(token_index));
}
pub inline fn advance_token(self: *Parser) void {
    _ = self.nextToken();
}

// equals nextToken with no return
pub inline fn advance(self: *Parser) void {
    self.tok_i += 1;
}

pub inline fn peekNext(p: *Parser) struct {
    token: Token,
    index: TokenIndex,
    buf: []const u8,

    pub inline fn tag(self: @This()) Token.Tag {
        return self.token.tag;
    }
} {
    const token = p.tokens.items[p.tok_i + 1];
    const buf = p.source[token.loc.start..token.loc.end];
    return .{
        .buf = buf,
        .index = @as(TokenIndex, @intCast(p.tok_i + 1)),
        .token = token,
    };
}

pub inline fn peekAt(p: *Parser, token_index: usize) struct {
    token: Token,
    index: TokenIndex,
    buf: []const u8,

    pub inline fn tag(self: @This()) Token.Tag {
        return self.token.tag;
    }
} {
    const token = p.tokens.items[token_index];
    const buf = p.source[token.loc.start..token.loc.end];
    return .{
        .buf = buf,
        .index = @as(TokenIndex, @intCast(token_index)),
        .token = token,
    };
}

pub inline fn current(p: *Parser) struct {
    token: Token,
    index: TokenIndex,
    buf: []const u8,

    pub inline fn tag(self: @This()) Token.Tag {
        return self.token.tag;
    }
} {
    const token = p.tokens.items[p.tok_i];
    const buf = p.source[token.loc.start..token.loc.end];
    return .{
        .buf = buf,
        .index = @as(TokenIndex, @intCast(p.tok_i)),
        .token = token,
    };
}

pub inline fn is_current(self: *Parser, tag: Token.Tag) bool {
    return self.current().token.tag == tag;
}

fn expectToken(self: *Parser, tag: Token.Tag) !TokenIndex {
    const token = self.tokens.items[self.tok_i];
    if (token.tag != tag) {
        try self.addDiagnostic(.expected_token, "{s}", .{@tagName(tag)});
    }
    return self.nextToken();
}

pub fn eatToken(self: *Parser, tag: Token.Tag) ?TokenIndex {
    if (self.tokens.items[self.tok_i].tag == tag) return self.nextToken() else return null;
}

pub fn parsePrimaryExpr(self: *Parser) !Node.Index {
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
fn expectExpr(p: *Parser) !Node.Index {
    const previous_tok = p.peekAt(p.tok_i - 1);
    const node = try p.parseExprPrecdence(0);
    if (node == 0) {
        const str = @tagName(p.tokens.items[p.tok_i].tag);
        const loc = p.tokens.items[p.tok_i].loc.start;
        error_log("expected primary expression after `{s}` but found `{s}` at [{d}]", .{ previous_tok.buf, str, loc });
        //try p.addDiagnostic(.expected_primary_expr, "but found {s} at [{d}]", .{ str, loc });
        return 0;
    }
    return node;
}
fn expectPrimaryExpr(p: *Parser) !Node.Index {
    const node = try p.parsePrimaryExpr();
    if (node == 0) {
        const str = @tagName(p.tokens.items[p.tok_i].tag);
        const loc = p.tokens.items[p.tok_i].loc.start;
        try p.addDiagnostic(.expected_primary_expr, "but found {s} at [{d}]", .{ str, loc });
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
pub fn parseExprPrecdence(self: *Parser, prev_prec: i8) !Node.Index {
    var lhs = try self.parsePrimaryExpr();
    if (lhs == null_node)
        return null_node;

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
pub fn parseAssign(self: *Parser) !Node.Index {
    const lhs = try self.parsePrimaryExpr();
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

pub fn parseStatement(self: *Parser) !Node.Index {
    const assign_stmt = try self.parseAssign();
    if (assign_stmt == null_node) {
        return null_node;
    }
    _ = try self.expectToken(.semicolon);
    return assign_stmt;
}

pub fn token_buf(self: *Parser, token: Token) []const u8 {
    return self.source[token.loc.start..token.loc.end];
}

const VarAttr = struct {
    is_typedef: bool,
    is_static: bool,
    is_extern: bool,
    is_inline: bool,
    is_tls: bool,
    _align: u32,
};

fn typeCreate(self: *Parser) Type {
    _ = self; // autofix

}
/// pointer : '*' pointer?
fn pointer_to(p: *Parser, base_ty: Type) !Node.Index {
    var ty = base_ty;
    while (p.eatToken(.asterisk)) |_| {
        const elem_ty = try p.allocator.create(Type);
        elem_ty.* = undefined; // &Type{ .  };
        ty = Type{
            .specifier = .pointer,
            .data = .{ .sub_type = elem_ty },
        };
    }
    return p.addNode(.{
        .tag = .pointer,
        .main_token = @as(u32, @intCast(p.tok_i)),
        .type = ty,
    });
}

pub fn getTypeName(p: *Parser, node: Node.Index) ![]const u8 {
    const real_node = p.nodes.items[node];
    const buf = try p.allocator.alloc(u8, 128);
    if (real_node.tag == .ptr_type) {
        const base_type = real_node.data.unary;
        const ftt = p.nodes.items[base_type];
        const bbb = p.tokens.items[ftt.main_token];
        const n = try std.fmt.bufPrint(buf, "*{s}", .{p.source[bbb.loc.start..bbb.loc.end]});
        return n;
    } else if (real_node.tag == .typename) {
        const base_type = real_node.main_token;
        const bbb = p.tokens.items[base_type];
        const n = try std.fmt.bufPrint(buf, "{s}", .{p.source[bbb.loc.start..bbb.loc.end]});
        return n;
    }
    return error.ExpectedTypeName;
}

pub fn parseTypeExpr(p: *Parser) !Node.Index {
    switch (p.current().tag()) {
        .open_bracket => {
            const l_bracket = p.nextToken();
            const elem_len = try p.parseExprPrecdence(0);
            _ = try p.expectToken(.close_bracket);
            return try p.addNode(.{
                .tag = .array_type,
                .main_token = l_bracket,
                .data = .{
                    .lhs = try p.parseTypeExpr(),
                    .rhs = elem_len,
                },
            });
        },
        .asterisk => {
            const astr = p.nextToken();
            return try p.addNode(.{
                .tag = .ptr_type,
                .main_token = astr,
                .data = .{
                    .lhs = try p.parseTypeExpr(),
                    .rhs = null_node,
                },
            });
        },
        .identifier => {
            const id = @as(u32, @intCast(p.tok_i));
            // const result = Type{ .name = id, .specifier = .user_defined_type };
            return try p.addNode(.{
                .tag = .typename,
                .main_token = id,
                //  .type = result,
                .data = .{},
            });
        },
        .keyword_char => {
            const id = @as(u32, @intCast(p.tok_i));
            //  const result = Type{ .name = id, .specifier = .char };
            return try p.addNode(.{
                .tag = .typename,
                .main_token = id,
                //.type = result,
                .data = .{},
            });
        },
        else => {
            error_log("Type not handled yet!! -> {s}", .{@tagName(p.current().tag())});
            return error.ParsingFailed;
        },
    }
}

// parse C-types : {char* , int var_name[len]}
// where the types comes before the declaration
pub fn parseCtypeExpr(p: *Parser, base_type: Node.Index) !Node.Index {
    switch (p.current().tag()) {
        .open_bracket => {
            std.debug.assert(base_type != 0);
            const o_brace = p.nextToken();
            const array_len = try p.parseExprPrecdence(0);
            _ = try p.expectToken(.close_brace);
            return try p.addNode(.{
                .tag = .array_type,
                .main_token = o_brace,
                .data = .{ .lhs = base_type, .rhs = array_len },
            });
        },
        // int**  nn[] -> new_type(T: ptr_type, base : int)
        .asterisk => {
            const asterisk = p.current().index;
            const ptr_t = try p.addNode(.{
                .tag = .ptr_type,
                .main_token = asterisk,
                .data = .{ .lhs = base_type },
            });
            _ = p.nextToken();
            switch (p.current().tag()) {
                .asterisk => {
                    return try p.addNode(.{
                        .tag = .ptr_type,
                        .main_token = p.nextToken(),
                        .data = .{ .lhs = try p.parseCtypeExpr(ptr_t) },
                    });
                },
                .open_bracket => {
                    return try p.parseCtypeExpr(ptr_t);
                },
                .identifier => {
                    p.advance();
                },
                .eof => {
                    return ptr_t;
                },
                else => {
                    warn_log("expected ptr or array but found -> {s} : {s}", .{ @tagName(p.current().tag()), p.current().buf });
                    return ptr_t;
                },
            }
            return try p.addNode(.{
                .tag = .ptr_type,
                .main_token = asterisk,
                .data = .{ .lhs = base_type },
            });
        },

        // base_t = int num[]
        //
        .identifier => {
            if (base_type != 0) {
                p.advance();
                return try p.parseCtypeExpr(base_type);
            }
            const t_name = @as(u32, @intCast(p.tok_i));
            const result = Type{ .name = t_name, .specifier = .user_defined_type };
            const base_ty = try p.addNode(.{
                .tag = .typename,
                .main_token = t_name,
                .type = result,
                .data = .{},
            });
            p.advance();

            if (p.current().tag() == .identifier) {
                _ = p.nextToken();
                var tmp: Node.Index = 0;
                if (p.eatToken(.open_bracket)) |tok| {
                    p.tok_i -= 1;
                    tmp = try p.parseCtypeExpr(base_ty);
                    _ = try p.expectToken(.close_bracket);
                    p.tok_i = tok - 1;
                    return tmp;
                }
                return base_ty;
            }
            return try p.parseCtypeExpr(base_ty);
        },

        .keyword_char => {
            const id = @as(u32, @intCast(p.tok_i));
            const result = Type{ .name = id, .specifier = .char };
            return try p.addNode(.{
                .tag = .typename,
                .main_token = id,
                .type = result,
                .data = .{},
            });
        },
        .semicolon, .comma, .equal => {
            warn_log("foun Semicolon ", .{});
            return base_type;
        },
        .eof => {
            return base_type;
        },
        else => {
            warn_log(" unexpected type : {s}", .{@tagName(p.current().tag())});
            return base_type;
        },
    }
}

pub fn expectTypeExpr(p: *Parser) !Node.Index {
    const node = try p.parseTypeExpr();
    if (node == 0) {
        const str = @tagName(p.tokens.items[p.tok_i].tag);
        const loc = p.tokens.items[p.tok_i].loc.start;
        try p.addDiagnostic(.expected_type_expr, "but found {s} at {d}", .{ str, loc });
    }
    return node;
}

pub fn parseBlock(self: *Parser) !Node.Index {
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

pub fn render_nodes(p: *Parser) void {
    var i: u32 = 0;
    for (p.nodes.items) |node| {
        const token = p.tokens.items[node.main_token];
        print("[{d}]:[{s} {s}]\n", .{ i, @tagName(node.tag), p.source[token.loc.start..token.loc.end] });
        i += 1;
    }
}

pub fn render_tree(self: *Parser, node_index: Node.Index) !void {
    try self.renderAstTree(node_index, "", false);
}

fn renderAstTree(p: *Parser, node: Node.Index, prefix: []const u8, isLeft: bool) !void {
    const nodes = p.nodes.items;
    const real_node = nodes[node];
    if (real_node.tag == .root) {
        return;
    }
    if (real_node.tag == .block) {
        const token = p.tokens.items[real_node.main_token];
        try std.io.tty.Config.setColor(.escape_codes, std.io.getStdOut(), .yellow);
        print("{s} {s}\n", .{ @tagName(real_node.tag), p.source[token.loc.start..token.loc.end] });
        const block_stmts = p.extra_data.items[real_node.data.lhs..real_node.data.rhs];
        for (block_stmts) |node_i| {
            try p.renderAstTree(node_i, "", true);
        }
        print("     {s}\n", .{"}"});
        return;
    }

    print("{s}{s}", .{ prefix, if (isLeft) "├──" else "└──" });

    // if (real_node.tag == .ptr_type) {
    //     const tokf = p.token_buf(p.tokens.items[real_node.type.name]);
    //     print("[{d}]- {s} {s}\n", .{ node, @tagName(real_node.tag), tokf });
    // } else {
    const token = p.tokens.items[real_node.main_token];
    print("[{d}]- {s} {s}\n", .{ node, @tagName(real_node.tag), p.source[token.loc.start..token.loc.end] });
    // }

    const new_prefix = try std.mem.concat(p.allocator, u8, &[_][]const u8{ prefix, if (isLeft) "│   " else "    " });

    try renderAstTree(p, real_node.data.lhs, new_prefix, true);
    try renderAstTree(p, real_node.data.rhs, new_prefix, false);
}
