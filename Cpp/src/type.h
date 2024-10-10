#pragma once
#include <stdint.h>

enum Typekind {
    Int,
    Char,
    Int8_t,
    Uint8_t,
    Int16_t,
    Uint16_t,
    Int32_t,
    Uint32_t,
    Int64_t,
    Uint64_t,
    Float,
    Double,
    LongDouble,

    Ptr,
    Array,
    Slice,

    Struct,
    Enum,
    Union,

    TypeName,
};

struct Type {
    Typekind kind;
    Type* base = nullptr; // in case of Ptr
    union {
        unsigned long long Integer;
        float Float;
        double Double;
        long double LongDouble;

        struct {
            uint32_t len;
        } array_info;

        struct {
            int index;
            int end;
            int sentinel;
        } slice_info;
    };
};
