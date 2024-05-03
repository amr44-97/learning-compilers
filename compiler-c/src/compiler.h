#pragma once
#include <stdint.h>

typedef struct Type Type;
typedef struct Member Member;
typedef struct Token Token;

typedef struct File {
    char *name;
    char *content;
} File;

char *read_file(const char *path);
File *new_file(char *name, char *contents);

typedef enum TokenKind {
    Tok_Eof,
    Tok_Invalid,
    Tok_Identifier,
    Tok_StringLiteral,
    Tok_NumberLiteral,
    Tok_Equal,
    Tok_EqualEqual,
    Tok_Plus,
    Tok_PlusPlus,
    Tok_PlusEqual,
    Tok_Minus,
    Tok_MinusMinus,
    Tok_MinusEqual,
    Tok_Asterisk,
    Tok_AsteriskEqual,
    Tok_Slash,
    Tok_SlashEqual,
    Tok_LineComment,
    Tok_Arrow,               // ->
    Tok_EqualAngleBracketRt, // =>
    Tok_Colon,
    Tok_ColonColon,
    Tok_ColonEqual,
    Tok_Comma,
    Tok_Bang,
    Tok_Hash,
    Tok_Questionmark,
    Tok_AtSign,
    Tok_Ambersand,
    Tok_Dot,
    Tok_DollarSign,
    Tok_Semicolon,
    Tok_LBrace,
    Tok_RBrace,
    Tok_LBracket,
    Tok_RBracket,
    Tok_LParen,
    Tok_RParen,
    Tok_AngleBracketLeft,  // <
    Tok_AngleBracketRight, // >
    Tok_LessOrEqual,
    Tok_GreaterOrEqual,
    Tok_ShiftRight,
    Tok_ShiftLeft,
} TokenKind;

struct Token {
    TokenKind kind;
    Token *next;
    uint32_t line;
    uint32_t column;
    char *loc;
    int len;
    int64_t value;
    long double fvalue;
    char *str; // for string literal content
    File *file;
};

char *token_format(Token *);
char *token_to_str(Token *);
const char *token_kind_str(Token *);
const char *token_type_str(TokenKind type);

Token *tokenize(File *file);
void print_tokens(Token *);

//
// type.c
//

typedef enum TypeKind {
    Ty_Void,
    Ty_Bool,
    Ty_Char,
    Ty_Short,
    Ty_Int,
    Ty_Long,
    Ty_Float,
    Ty_Double,
    Ty_LongDouble,
    Ty_Enum,
    Ty_Struct,
    Ty_Union,
    Ty_Array,
    Ty_Ptr,
    Ty_Func,
} TypeKind;

typedef struct Struct Struct;
typedef struct Function Function;
typedef struct Pointer Pointer;
typedef struct Decl Decl;
struct Struct {
    // Struct
    Member *members;
    bool is_flexible;
    bool is_packed;
};

struct Function {
    // Function type
    Type *return_ty;
    Type *params;
    bool is_variadic;
    Type *next;
};

struct Pointer {
    // Pointer-to or array-of type. We intentionally use the same member
    // to represent pointer/array duality in C.
    //
    // In many contexts in which a pointer is expected, we examine this
    // member instead of "kind" member to determine whether a type is a
    // pointer or not. That means in many contexts "array of T" is
    // naturally handled as if it were "pointer to T", as required by
    // the C spec.
    Type *base;
};

struct Decl {

    // Declaration
    Token *name;
    Token *name_pos;
};

struct Type {
    TypeKind kind;
    int size;
    int align;
    bool is_unsigned;
    Type *origin;
    int array_len;
    union {
        Function *func;
        Struct *struc;
        Pointer *ptr;
        Decl *decl;
    };

};

// Struct member
struct Member {
    Member *next;
    Type *ty;
    Token *tok; // for error message
    Token *name;
    int idx;
    int align;
    int offset;

    // Bitfield
    bool is_bitfield;
    int bit_offset;
    int bit_width;
};

//
// hashmap.c
//

typedef struct HashEntry HashEntry;
typedef struct HashMap HashMap;

struct HashEntry {
    char *key;
    int keylen;
    void *val;
};

struct HashMap {
    HashEntry *buckets;
    int capacity;
    int used;
};
