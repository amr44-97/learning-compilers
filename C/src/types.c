#pragma once
#include "compiler.h"
#include <stdlib.h>

Type *ty_void = &(Type){Ty_Void, 1, 1};
Type *ty_bool = &(Type){Ty_Bool, 1, 1};

Type *ty_char = &(Type){Ty_Char, 1, 1};
Type *ty_short = &(Type){Ty_Short, 2, 2};
Type *ty_int = &(Type){Ty_Int, 4, 4};
Type *ty_long = &(Type){Ty_Long, 8, 8};

Type *ty_uchar = &(Type){Ty_Char, 1, 1, true};
Type *ty_ushort = &(Type){Ty_Short, 2, 2, true};
Type *ty_uint = &(Type){Ty_Int, 4, 4, true};
Type *ty_ulong = &(Type){Ty_Long, 8, 8, true};

Type *ty_float = &(Type){Ty_Float, 4, 4};
Type *ty_double = &(Type){Ty_Double, 8, 8};
Type *ty_ldouble = &(Type){Ty_LongDouble, 16, 16};

Type *new_type(TypeKind kind, int size, int align) {
    Type *ty = (Type *)calloc(1, sizeof(Type));
    ty->kind = kind;
    ty->size = size;
    ty->align = align;
    return ty;
}
