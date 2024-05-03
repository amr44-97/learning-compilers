#pragma once
#include <stdint.h>
#include <stdlib.h>

namespace compiler {

// Memory managemnet

template <typename T> struct LinkedList {
  private:
    struct Node {
        T data;
        Node *next = nullptr;

        static Node *create(T _data, Node *_next = nullptr) {
            auto n = (Node *)malloc(sizeof(Node));
            n->next = _next;
            n->data = _data;
            return n;
        }
    };

  private:
    Node *head = nullptr;
    int size = 0;

  public:
    LinkedList() { head = (Node *)calloc(1, sizeof(Node)); }

    ~LinkedList() {
        Node *tmp = head;
        while (tmp) {
            auto n = tmp->next;
            free(tmp);
            tmp = n;
        }
    }
        Node* get_head(){
            return this->head;
        }
    void insert(T value, int index) {
        Node *node = head;
        if (index == 0) {
            auto nxt = head->next;
            head->next = Node::create(value, nxt);
            size++;
            return;
        }
        if (index >= size) {
            auto node = get(size - 1);
            node->next = Node::create(value);
            size++;
            return;
        }

        for (int i = 0; i < index; i++) {
            node = node->next;
        }
        node->next = Node::create(value, node->next);
        size++;
    }

    T remove(int index) {
        Node *prev = get(index - 1);
        Node *_cur = get(index);
        printf("remove index = %d: value =%d\n", index, _cur->data);
        auto nxt = _cur->next;
        free(_cur);
        prev->next = nxt;
        size--;
        return prev->next->data;
    }

    inline Node *get(int index) {
        auto node = head->next;
        for (int i = 0; i < index; ++i) {
            if (index - 1 == size) {
                return node;
            }
            node = node->next;
        }
        return node;
    }
    void append(T value) {
        Node *tmp = head;
        while (tmp->next != nullptr) {
            tmp = tmp->next;
        }
        tmp->next = Node::create(value);
        size++;
    }

    void display() {
        Node *tmp = head->next;
        int i = 0;
        while (tmp) {
            printf("[%d]%d\n", i, tmp->data);
            i++;
            tmp = tmp->next;
        }
    }
	void operator+=(T value){
		append(value);
	}
    const T &operator[](size_t index) const {
        auto tmp = head->next;
        int count = 0;
        while (tmp) {
            if (index == count)
                return tmp->data;
            tmp = tmp->next;
            count++;
        }
    }
};


struct Type;
struct Member;

typedef struct File {
    char *name;
    char *content;
} File;

char *read_file(const char *path);
File *new_file(char *name, char *contents);

enum class TokenKind {
    Eof,
    Invalid,
    Identifier,
    StringLiteral,
    NumberLiteral,
    Equal,
    EqualEqual,
    Plus,
    PlusPlus,
    PlusEqual,
    Minus,
    MinusMinus,
    MinusEqual,
    Asterisk,
    AsteriskEqual,
    Slash,
    SlashEqual,
    LineComment,
    Arrow,               // ->
    EqualAngleBracketRt, // =>
    Colon,
    ColonColon,
    ColonEqual,
    Comma,
    Bang,
    Hash,
    Questionmark,
    AtSign,
    Ambersand,
    Dot,
    DollarSign,
    Semicolon,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    LParen,
    RParen,
    AngleBracketLeft,  // <
    AngleBracketRight, // >
    LessOrEqual,
    GreaterOrEqual,
    ShiftRight,
    ShiftLeft,
};

struct Token {
    TokenKind kind;
    Token *next;
    uint32_t line;
    uint32_t column = 0;
    char *loc;
    int64_t len;
    int64_t value;
    long double fvalue;
    char *str; // for string literal content
    File *file;

    // Methods

    // [type: line:col]
    char *format();
    char *to_str();
    const char *kind_str();
};

const char *token_type_str(TokenKind type);

LinkedList<Token> tokenize(File *file);
void print_tokens(Token *);

//
// type.c
//
enum class TypeKind {
    Void,
    Bool,
    Char,
    Short,
    Int,
    Long,
    Float,
    Double,
    Enum,
    Struct,
    Union,
    Array,
    Ptr,
    Func,
};

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

    static Type *create(TypeKind kind, int size, int align,
                        bool is_unsigned = false) {
        Type *ty = (Type *)calloc(1, sizeof(Type));
        ty->kind = kind;
        ty->size = size;
        ty->align = align;
        return ty;
    }
    union data {
        Function func;
        Struct struc;
        Pointer ptr;
        Decl decl;
    };

    struct primitive {
        static Type *t_void() { return Type::create(TypeKind::Void, 1, 1); }
        static Type *t_bool() { return Type::create(TypeKind::Bool, 1, 1); }
        static Type *t_char() { return Type::create(TypeKind::Char, 1, 1); }
        static Type *t_short() { return Type::create(TypeKind::Short, 2, 2); }
        static Type *t_int() { return Type::create(TypeKind::Int, 4, 4); }
        static Type *t_long() { return Type::create(TypeKind::Long, 8, 8); }
        static Type *t_uchar() {
            return Type::create(TypeKind::Char, 1, 1, true);
        }
        static Type *t_uint() {
            return Type::create(TypeKind::Int, 4, 4, true);
        }
        static Type *t_ulong() {
            return Type::create(TypeKind::Long, 8, 8, true);
        }
        static Type *t_ushort() {
            return Type::create(TypeKind::Short, 2, 2, true);
        }
        static Type *t_float() { return Type::create(TypeKind::Float, 4, 4); }
        static Type *t_double() { return Type::create(TypeKind::Double, 8, 8); }
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
} // namespace compiler
