const std = @import("std");
const Allocator = std.mem.Allocator;
const Parser = @This();
const Tokenizer = @import("Tokenizer.zig");
const Token = Tokenizer.Token;
const Ast = @import("Ast.zig");
const TokenIndex = Ast.TokenIndex;
const Node = Ast.Node;
const Type = @import("Type.zig");
const ArrayList = std.ArrayList;
const assert = std.debug.assert;
const print = std.debug.print;

const null_node: Node.Index = 0;

const Error = error{ PasingError, OutOfMemory, UnexpectedToken };

gpa: Allocator,
source: [:0]const u8,
token_tags: []const Token.Tag,
token_starts: []const Ast.ByteOffset,
nodes: std.MultiArrayList(Node),
extra_data: std.ArrayListUnmanaged(Node.Index),
tok_i: u32 = 0,

pub fn nextToken(p: *Parser) TokenIndex {
    const result = p.tok_i;
    p.tok_i += 1;
    return result;
}

pub fn advance(p: *Parser) void {
    p.tok_i += 1;
}

pub fn expectToken(p: *Parser, tag: Token.Tag) Error!TokenIndex {
    if (p.token_tags[p.tok_i] != tag) {
        const token = p.token_starts[p.tok_i];
        const tok_buf = p.source[token..p.token_starts[p.tok_i + 1]];
        std.log.err("expected <{s}> but found <{s} : {s}>", .{
            @tagName(tag),
            @tagName(p.token_tags[p.tok_i]),
            tok_buf,
        });
        return Error.UnexpectedToken;
    }
    return p.nextToken();
}

fn print_token(p: *Parser, token_index: ?TokenIndex) void {
    const tok_i: TokenIndex = if (token_index == null) p.tok_i else token_index.?;
    const token = p.token_starts[tok_i];
    const tok_buf = p.source[token..p.token_starts[tok_i + 1]];
    print("{s} <=> {s}\n", .{ @tagName(p.token_tags[tok_i]), tok_buf });
}

fn eatToken(p: *Parser, tag: Token.Tag) ?TokenIndex {
    return if (p.token_tags[p.tok_i] == tag) p.nextToken() else null;
}

pub fn init(allocator: Allocator, source: [:0]const u8) !Parser {
    var tokens = Ast.TokenList{};

    var tkizer = Tokenizer.init(source);
    var token = tkizer.next();

    while (true) {
        try tokens.append(allocator, .{
            .tag = token.tag,
            .start = @intCast(token.loc.start),
        });
        //       print("{s} : `{s}`\n", .{ @tagName(token.tag), source[token.loc.start..token.loc.end] });
        if (token.tag == .eof) break;
        token = tkizer.next();
    }

    var parser: Parser = .{
        .gpa = allocator,
        .source = source,
        .token_tags = tokens.items(.tag),
        .token_starts = tokens.items(.start),
        .nodes = .{},
        .extra_data = .{},
        .tok_i = 0,
    };

    _ = try parser.parseRoot();

    return parser;
}

pub fn addNode(p: *Parser, elem: Node) !Node.Index {
    const result = @as(Node.Index, @intCast(p.nodes.len));
    try p.nodes.append(p.gpa, elem);
    return result;
}

//pub fn parse(p: *Parser, src: []u8, tag: ?Node.Tag) !Node.Index {}
pub fn parseRoot(p: *Parser) !void {
    _ = try p.addNode(.{
        .tag = .root,
        .main_token = 0,
        .data = undefined,
    });
    _ = try p.parseCTypeExpr();
    //   _ = try p.parseGlobalVarDecl();
}
//
//pub fn parsePrimaryExpr(p: *Parser, min: i32) !Node.Index {}
//pub fn parseBinaryExpr(p: *Parser, min: i32) !Node.Index {}
pub fn parseAssignExpr(p: *Parser) !Node.Index {
    const lhs = try p.parseExpr();
    const op_tok = try p.expectToken(.equal);
    p.advance();

    return p.addNode(.{
        .tag = .assign,
        .main_token = op_tok,
        .data = .{ .binary_expr = .{
            .lhs = lhs,
            .rhs = try p.parseExpr(),
        } },
    });
}

fn parsePrimaryTypeExpr(p: *Parser) Error!Node.Index {
    switch (p.token_tags[p.tok_i]) {
        .number_literal => return p.addNode(.{
            .tag = .number_literal,
            .main_token = p.nextToken(),
            .data = .{
                .binary_expr = .{
                    .lhs = null_node,
                    .rhs = null_node,
                },
            },
        }),
        .string_literal => {
            const main_token = p.nextToken();
            return p.addNode(.{
                .tag = .string_literal,
                .main_token = main_token,
                .data = .none,
            });
        },

        //.keyword_struct,
        //.keyword_enum,
        //.keyword_union,
        //=> return p.parseContainerDeclAuto(),

        .identifier => switch (p.token_tags[p.tok_i + 1]) {
            .colon_colon => {
                if (p.token_tags[p.tok_i + 2] == .l_paren) {
                    return p.parseFnProto();
                } else {
                    return null_node;
                }
            },

            else => return p.addNode(.{
                .tag = .identifier,
                .main_token = p.nextToken(),
                .data = .none,
            }),
        },

        else => return null_node,
    }
}

fn parseIdentifier(p: *Parser) Error!Node.Index {
    const tok = try p.expectToken(.identifier);
    return p.addNode(.{
        .tag = .identifier,
        .main_token = tok,
        .data = .none,
    });
}
fn expectPrimaryTypeExpr(p: *Parser) !Node.Index {
    const node = try p.parsePrimaryTypeExpr();
    if (node == 0) {
        std.log.err("expected primary_type expression", .{});
        return error.ParsingError;
    }
    return node;
}

pub fn parsePrimaryExpr(p: *Parser) Error!Node.Index {
    switch (p.token_tags[p.tok_i]) {
        .keyword_if => return p.parseIfExpr(),
        .keyword_return => {
            return p.addNode(.{
                .tag = .@"return",
                .main_token = p.nextToken(),
                .data = .{
                    .binary_expr = .{
                        .lhs = try p.parseExpr(),
                        .rhs = null_node,
                    },
                },
            });
        },
        .identifier => {
            return p.parsePrimaryTypeExpr();
        },
        .keyword_while => return p.parseWhileExpr(),
        .l_brace => return p.parseBlock(),
        else => return p.parsePrimaryTypeExpr(),
    }
}

pub fn parsePrefixExpr(p: *Parser) Error!Node.Index {
    const tag: Node.Tag = switch (p.token_tags[p.tok_i]) {
        .bang => .bool_not,
        .minus => .negation,
        .tilde => .bit_not,
        .ampersand => .address_of,
        else => return p.parsePrimaryExpr(),
    };
    return p.addNode(.{
        .tag = tag,
        .main_token = p.nextToken(),
        .data = .{
            .binary_expr = .{
                .lhs = try p.expectPrefixExpr(),
                .rhs = null_node,
            },
        },
    });
}

pub fn expectPrefixExpr(p: *Parser) Error!Node.Index {
    const node = try p.parsePrefixExpr();
    if (node == 0) {
        std.log.err("expected_prefix_expr", .{});
        return Error.PasingError;
    }
    return node;
}

const OperInfo = struct {
    prec: i8,
    tag: Node.Tag,
};

// A table of binary operator information. Higher precedence numbers are
// stickier. All operators at the same precedence level should have the same
// associativity.

pub inline fn operTable(tag: Token.Tag) OperInfo {
    switch (tag) {
        .pipe_pipe => return .{ .prec = 10, .tag = .bool_or },

        .ampersand_ampersand => return .{ .prec = 20, .tag = .bool_and },

        .equal_equal => return .{ .prec = 30, .tag = .equal_equal },
        .bang_equal => return .{ .prec = 30, .tag = .bang_equal },

        .ampersand => return .{ .prec = 40, .tag = .bit_and },
        .caret => return .{ .prec = 40, .tag = .bit_xor },
        .pipe => return .{ .prec = 40, .tag = .bit_or },

        .plus => return .{ .prec = 60, .tag = .add },
        .minus => return .{ .prec = 60, .tag = .sub },
        .plus_plus => return .{ .prec = 60, .tag = .array_cat },

        .asterisk => return .{ .prec = 70, .tag = .mul },
        .slash => return .{ .prec = 70, .tag = .div },
        .percent => return .{ .prec = 70, .tag = .mod },

        else => return .{ .prec = -1, .tag = Node.Tag.root },
    }
}

pub fn parseExprPrecedence(p: *Parser, min: i32) !Node.Index {
    var lhs = try p.parsePrefixExpr();
    //print("node tag = {s}\n", .{@tagName(p.nodes.items(.tag)[lhs])});
    if (lhs == 0) return null_node;

    while (true) {
        const tok_tag = p.token_tags[p.tok_i];

        const info = operTable(tok_tag);

        if (info.prec < min) break;

        const oper_token = p.nextToken();

        const rhs = try p.parseExprPrecedence(info.prec + 1);
        if (rhs == 0) {
            std.log.warn("expected_expr", .{});
            return lhs;
        }

        lhs = try p.addNode(.{
            .tag = info.tag,
            .main_token = oper_token,
            .data = .{
                .binary_expr = .{
                    .lhs = lhs,
                    .rhs = rhs,
                },
            },
        });
    }
    return lhs;
}

pub fn parseExpr(p: *Parser) Error!Node.Index {
    return p.parseExprPrecedence(0);
}
// name : String = "Hello World!";
// name :: "Hello World";
// name := "Hello World";
// if(a){ do this } else { do that}
// while(true){ do this}
// if(a == 1){ do this } elseif (a == 2){ do that} else{ do that}
// name = "New World";
// return 0;
// organize_all(name);

pub fn parseVarDecl(p: *Parser) !Node.Index {
    const var_name = try p.parseIdentifier();
    if (p.token_tags[p.tok_i] == .colon_equal) {
        p.advance();
        return p.addNode(.{
            .tag = .var_dec_inferred_type,
            .main_token = p.token_starts[p.nodes.items(.main_token)[var_name]],
            .data = .{
                .binary_expr = .{
                    .lhs = var_name,
                    .rhs = try p.parsePrimaryTypeExpr(),
                },
            },
        });
    }

    _ = p.eatToken(.colon);
    const vtype = try p.parseTypeExpr();

    const var_proto = try p.addNode(.{
        .tag = .var_decl_proto,
        .main_token = p.token_starts[p.nodes.items(.main_token)[var_name]],
        .data = .{
            .binary_expr = .{
                .lhs = var_name,
                .rhs = vtype,
            },
        },
    });

    switch (p.token_tags[p.tok_i]) {
        .semicolon => return var_proto,
        .equal => {
            const eql = p.nextToken();
            const rhs = try p.parsePrimaryTypeExpr();
            _ = try p.expectToken(.semicolon);
            return p.addNode(.{
                .tag = .var_decl,
                .main_token = eql,
                .data = .{
                    .binary_expr = .{
                        .lhs = var_proto,
                        .rhs = rhs,
                    },
                },
            });
        },

        else => return null_node,
    }
}

pub fn parseGlobalVarDecl(p: *Parser) !Node.Index {
    const var_name = try p.parseIdentifier();
    const desc = p.eatToken(.colon) orelse p.eatToken(.colon_colon) orelse 0;
    if (desc == 0) {
        std.log.err("expected colon or colon_colon after identifier but found <{s}>", .{@tagName(p.token_tags[p.tok_i])});
        return Error.UnexpectedToken;
    }

    if (p.token_tags[desc] == .colon) {
        const var_type = try p.parseTypeExpr();
        const var_proto = try p.addNode(.{
            .tag = .var_decl_proto,
            .main_token = desc - 1,
            .data = .{
                .binary_expr = .{ .lhs = var_name, .rhs = var_type },
            },
        });

        if (p.token_tags[p.tok_i] == .semicolon) {
            return var_proto;
        }
        const eql = try p.expectToken(.equal);

        const rhs = try p.parsePrimaryTypeExpr();

        _ = try p.expectToken(.semicolon);
        return p.addNode(.{
            .tag = .var_decl,
            .main_token = eql,
            .data = .{
                .binary_expr = .{
                    .lhs = var_proto,
                    .rhs = rhs,
                },
            },
        });
    }

    if (p.token_tags[desc] == .colon_colon) {
        const const_value = try p.parseExpr();
        const result = p.addNode(.{
            .tag = .const_decl,
            .main_token = desc - 1,
            .data = .{
                .binary_expr = .{ .lhs = var_name, .rhs = const_value },
            },
        });
        _ = try p.expectToken(.semicolon);
        return result;
    }

    return null_node;
}

//pub fn parseIfStmt(p: *Parser) !Node.Index {}
//pub fn parseForStmt(p: *Parser) !Node.Index {}

pub fn parseFnParams(p: *Parser) !Node.Index {
    var list = ArrayList(Node.Index).init(p.gpa);
    _ = p.eatToken(.l_paren);

    var param_name: Node.Index = null_node;
    var param_type: Node.Index = null_node;

    while (true) {
        if (p.token_tags[p.tok_i] == .r_paren) break;
        param_name = try p.parsePrimaryExpr();
        _ = try p.expectToken(.colon);
        param_type = try p.parseTypeExpr();

        const node = try p.addNode(.{
            .tag = .var_decl,
            .main_token = p.nodes.items(.main_token)[param_name],
            .data = .{
                .binary_expr = .{
                    .lhs = param_name,
                    .rhs = param_type,
                },
            },
        });

        try list.append(node);
        _ = p.eatToken(.comma);
    }
    if (list.items.len == 0) return null_node;
    return p.addNode(.{
        .tag = .params,
        .main_token = 0,
        .data = .{
            .params = try list.toOwnedSlice(),
        },
    });
}

pub fn parseFnProto(p: *Parser) Error!Node.Index {
    const fn_tok = p.tok_i;
    const fn_name = try p.parseIdentifier();
    _ = p.eatToken(.colon_colon);
    const params = try p.parseFnParams();
    const ret_type = try p.parseTypeExpr();
    const body = try p.parseBlock();

    return p.addNode(.{
        .tag = .fn_decl,
        .main_token = fn_tok,
        .data = .{
            .fn_decl = .{
                .name = fn_name,
                .params = params,
                .ret_type = ret_type,
                .body = body,
            },
        },
    });
}

fn expectExpr(p: *Parser) Error!Node.Index {
    const node = try p.parseExpr();
    if (node == 0) {
        //return p.fail(.expected_expr);
        std.log.err("expected expression", .{});
        return Error.PasingError;
    } else {
        return node;
    }
}

pub fn parseIfExpr(p: *Parser) !Node.Index {
    const if_token = p.eatToken(.keyword_if) orelse return null_node;
    _ = try p.expectToken(.l_paren);
    const condition = try p.expectExpr();
    _ = try p.expectToken(.r_paren);

    const then_expr = try p.expectExpr();
    assert(then_expr != 0);

    _ = p.eatToken(.keyword_else) orelse return p.addNode(.{
        .tag = .if_simple,
        .main_token = if_token,
        .data = .{ .binary_expr = .{
            .lhs = condition,
            .rhs = then_expr,
        } },
    });
    const else_expr = try p.expectExpr();
    assert(else_expr != 0);

    return p.addNode(.{
        .tag = .@"if",
        .main_token = if_token,
        .data = .{
            .if_expr = Ast.If{
                .condition = condition,
                .then_expr = then_expr,
                .else_expr = else_expr,
            },
        },
    });
}

pub fn parseWhileExpr(p: *Parser) !Node.Index {
    _ = p;
    return null_node;
}

pub fn parseContainer(p: *Parser) !Node.Index {
    _ = p;
    return null_node;
}

pub fn parseMacro(p: *Parser) !Node.Index {
    _ = p;
    return null_node;
}
pub fn parseIncludeDirective(p: *Parser) !Node.Index {
    _ = p;
    return null_node;
}

pub fn parseTopLevelDecl(p: *Parser) !void {
    var collect = ArrayList(Node.Index).init(p.gpa);
    const current = p.tokens[p.tok_i];
    while (true) {
        switch (current.tag) {
            .identifier => switch (p.token_tags[p.tok_i + 1]) {
                .colon_colon => switch (p.token_tags[p.tok_i + 2]) {
                    .l_paren => {
                        print("Function", .{});
                        const func = try p.parseFnProto();
                        try collect.append(func);
                    },
                    .keyword_struct => {
                        print("Struct", .{});
                        const struct_node = try p.parseContainer();
                        try collect.append(struct_node);
                    },
                    .keyword_enum => {
                        print("Enum", .{});
                        const enum_node = try p.parseContainer();
                        try collect.append(enum_node);
                    },
                    else => {},
                },
                .colon => {
                    print("Global variable", .{});
                    const var_decl = try p.parseGlobalVarDecl();
                    try collect.append(var_decl);
                },
                else => {},
            },

            .hash => switch (p.token_tags[p.tok_i]) {
                .keyword_define => {
                    print("macro definition", .{});
                    const macro_def = try p.parseMacro();
                    try collect.append(macro_def);
                },
                .keyword_include => {
                    print("include directive", .{});
                    const inc_ = try p.parseIncludeDirective();
                    try collect.append(inc_);
                },
                else => {},
            },

            .eof => break,

            else => {
                std.log.err("didn't expect {s}", .{@tagName(current.tag)});
                std.process.exit(0);
            },
        }
    }
}

pub fn parseBaseType(p: *Parser) !Node.Index {
    switch (p.token_tags[p.tok_i]) {
        .identifier,
        .keyword_int,
        .keyword_asm,
        => {},
        else => {},
    }
}

// pub fn parseCTypeExprWithBase(p: *Parser, base_type: Node.Index) !Node.Index {
//     switch (p.token_tags[p.tok_i]) {
//         .l_bracket => {
//             _ = p.nextToken();
//             const elem_len = try p.parseExprPrecedence(0);
//             _ = try p.expectToken(.r_bracket);
//             return try p.addNode(.{
//                 .tag = .array_type,
//                 .main_token = 0,
//                 .data = .{
//                     .binary_expr = .{
//                         .lhs = base_type, //try p.parseTypeExpr(),
//                         .rhs = elem_len,
//                     },
//                 },
//             });
//         },
//         .asterisk => {
//             return try p.addNode(.{
//                 .tag = .ptr_type,
//                 .main_token = p.nextToken(),
//                 .data = .{
//                     .binary_expr = .{
//                         .lhs = base_type, //try p.parseTypeExpr(),
//                         .rhs = null_node,
//                     },
//                 },
//             });
//         },
//         .identifier => {
//             return try p.addNode(.{
//                 .tag = .typename,
//                 .main_token = p.nextToken(),
//                 .data = .none,
//             });
//         },
//         .keyword_char => {
//             return try p.addNode(.{
//                 .tag = .typename,
//                 .main_token = p.nextToken(),
//                 .data = .none,
//             });
//         },
//         else => {
//             std.log.err("Type not handled yet!! -> {s}", .{@tagName(p.token_tags[p.tok_i])});
//             return Error.PasingError;
//         },
//     }
// }

pub fn parseCTypeExpr(p: *Parser) !Node.Index {
    const base = try p.parseBaseType();
    std.debug.print("{s}\n", .{@tagName(p.token_tags[p.tok_i])});
    return p.parseCTypeExprWithBase(base);
}

pub fn parseCTypeExprWithBase(p: *Parser, base_type_tmp: Node.Index) !Node.Index {
    var base_type = base_type_tmp;
    while (true) {
        switch (p.token_tags[p.tok_i]) {
            .l_bracket => {
                const l_bracket = p.nextToken();
                const elem_len = try p.parseExprPrecedence(0);
                _ = try p.expectToken(.r_bracket);
                base_type = try p.addNode(.{
                    .tag = .array_type,
                    .main_token = l_bracket,
                    .data = .{
                        .binary_expr = .{
                            .lhs = elem_len,
                            .rhs = base_type,
                        },
                    },
                });
            },
            .asterisk => {
                base_type = try p.addNode(.{
                    .tag = .ptr_type,
                    .main_token = p.nextToken(),
                    .data = .{
                        .binary_expr = .{
                            .lhs = null_node,
                            .rhs = base_type,
                        },
                    },
                });
            },
            .keyword_char,
            .keyword_int,
            .keyword_float,
            .keyword_long,
            .identifier,
            => {
                return base_type;
            },
            else => {
                return base_type;
                //std.log.err("Type not handled yet!! -> {s}", .{@tagName(p.token_tags[p.tok_i])});
                //return Error.PasingError;
            },
        }
    }
}

pub fn parseTypeExpr(p: *Parser) !Node.Index {
    switch (p.token_tags[p.tok_i]) {
        .l_bracket => {
            _ = p.nextToken();
            const elem_len = try p.parseExprPrecedence(0);
            _ = try p.expectToken(.r_bracket);
            return try p.addNode(.{
                .tag = .array_type,
                .main_token = 0,
                .data = .{
                    .binary_expr = .{
                        .lhs = try p.parseTypeExpr(),
                        .rhs = elem_len,
                    },
                },
            });
        },
        .asterisk => {
            return try p.addNode(.{
                .tag = .ptr_type,
                .main_token = p.nextToken(),
                .data = .{
                    .binary_expr = .{
                        .lhs = try p.parseTypeExpr(),
                        .rhs = null_node,
                    },
                },
            });
        },
        .identifier => {
            return try p.addNode(.{
                .tag = .typename,
                .main_token = p.nextToken(),
                .data = .none,
            });
        },
        .keyword_char => {
            return try p.addNode(.{
                .tag = .typename,
                .main_token = p.nextToken(),
                .data = .none,
            });
        },
        else => {
            std.log.err("Type not handled yet!! -> {s}", .{@tagName(p.token_tags[p.tok_i])});
            return Error.PasingError;
        },
    }
}

pub fn parseBlock(p: *Parser) !Node.Index {
    _ = p;
    return null_node;
}

pub fn print_nodes(p: *Parser) void {
    var i: u32 = 0;
    while (true) : (i += 1) {
        if (i == p.nodes.len) break;
        const node = p.nodes.get(@intCast(i));
        const token = p.token_starts[node.main_token];
        const token_buffer = p.source[token..p.token_starts[node.main_token + 1]];

        print("[{d}]- {s} {s}\n", .{ i, @tagName(node.tag), token_buffer });
    }
}

pub fn render_ast(p: *Parser, node_index: Node.Index) !void {
    return try p.render_node(node_index, "", false);
}

fn render_node(p: *Parser, node_index: Node.Index, prefix: []const u8, isLeft: bool) !void {
    const node = p.nodes.get(node_index);
    switch (node.tag) {
        .add, .mul, .div, .sub, .mod, .assign => try p.render_binary_expr(node_index, prefix, isLeft),

        .const_decl, .var_decl, .var_decl_proto, .var_dec_inferred_type => try p.render_binary_expr(node_index, prefix, isLeft),

        .ptr_type, .array_type => try p.render_binary_expr(node_index, prefix, isLeft),

        .params => try p.render_param_list(node_index, prefix, isLeft),
        .identifier, .string_literal, .number_literal, .typename => try p.render_simple_expr(node_index, prefix, isLeft),

        else => return,
    }
}

fn render_param_list(p: *Parser, node_index: Node.Index, prefix: []const u8, isLeft: bool) Allocator.Error!void {
    const node = p.nodes.get(node_index);
    if (node.tag == .root) return;
    assert(node.tag == .params);
    const node_list = node.data.params;
    for (node_list) |node_i| {
        print("\n", .{});
        try p.render_node(node_i, prefix, isLeft);
    }
}

fn render_simple_expr(p: *Parser, node_index: Node.Index, prefix: []const u8, isLeft: bool) Allocator.Error!void {
    const node = p.nodes.get(node_index);

    print("{s}{s}", .{ prefix, if (isLeft) "├──" else "└──" });

    const token = p.token_starts[node.main_token];
    const token_buffer = p.source[token..p.token_starts[node.main_token + 1]];
    // if (node.main_token == 0) {
    //     // token_buffer = "";
    // }

    print("[{d}]- {s} {s}\n", .{ node_index, @tagName(node.tag), token_buffer });
}

fn render_binary_expr(p: *Parser, node_index: Node.Index, prefix: []const u8, isLeft: bool) Allocator.Error!void {
    const node = p.nodes.get(node_index);

    if (node.tag == .root) return;
    if (node.data != .binary_expr) return p.render_node(node_index, prefix, isLeft);

    print("{s}{s}", .{ prefix, if (isLeft) "├──" else "└──" });

    const token = p.token_starts[node.main_token];
    var token_buffer = p.source[token..p.token_starts[node.main_token + 1]];
    if (node.main_token == 0) {
        token_buffer = "";
    }
    print("[{d}]- {s} {s}\n", .{ node_index, @tagName(node.tag), token_buffer });

    const new_prefix = try std.mem.concat(p.gpa, u8, &[_][]const u8{ prefix, if (isLeft) "│   " else "    " });

    if (node.data == .binary_expr) {
        try p.render_node(node.data.binary_expr.lhs, new_prefix, true);
        try p.render_node(node.data.binary_expr.rhs, new_prefix, false);
    }
}
