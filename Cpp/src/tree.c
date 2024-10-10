#include "tree.h"

auto enum_to_str(NodeKind kind) -> const char* {
#define case_to_str(T) case T:return &((#T)[4])
    switch (kind) {
        case_to_str(Ast_None);
        case_to_str(Ast_Root);
        case_to_str(Ast_Identifier);
        case_to_str(Ast_StringLiteral);
        case_to_str(Ast_NumberLiteral);
        case_to_str(Ast_NullLiteral);
		case_to_str(Ast_IncludeStmt);
		case_to_str(Ast_MacroDefine);
        case_to_str(Ast_Mul);
        case_to_str(Ast_Add);
        case_to_str(Ast_Sub);
        case_to_str(Ast_Div);
        case_to_str(Ast_Bool_Not);
        case_to_str(Ast_Bool_Or);
        case_to_str(Ast_Bool_And);
        case_to_str(Ast_Bit_Not);
        case_to_str(Ast_Bit_Xor);
        case_to_str(Ast_Bit_And);
        case_to_str(Ast_Bit_Or);
        case_to_str(Ast_ShiftLeft);
        case_to_str(Ast_ShiftRight);
        case_to_str(Ast_AddressOf);
        case_to_str(Ast_Slice);
        case_to_str(Ast_SliceSentinel);
        case_to_str(Ast_SliceOpen);
        case_to_str(Ast_Assign);
        case_to_str(Ast_ArrayAccess);
        case_to_str(Ast_FieldAccess);
        case_to_str(Ast_Negation);
        case_to_str(Ast_FnProto);
        case_to_str(Ast_FnDecl);
        case_to_str(Ast_VarDecl);
        case_to_str(Ast_ConstDecl);
        case_to_str(Ast_Deref);
        case_to_str(Ast_Macro);
        case_to_str(Ast_ParamDecl);
        case_to_str(Ast_ParamDeclList);
        case_to_str(Ast_Block);
        case_to_str(Ast_If_Simple);
        case_to_str(Ast_If);
        case_to_str(Ast_WhileLoop);
        case_to_str(Ast_ForLoop);
        case_to_str(Ast_SimpleLoop);
        case_to_str(Ast_Struct);
        case_to_str(Ast_Union);
        case_to_str(Ast_TypeName);
        case_to_str(Ast_DeclType);
        case_to_str(Ast_Array);
        case_to_str(Ast_Pointer);
        case_to_str(Ast_Label);
        case_to_str(Ast_LessThan);
        case_to_str(Ast_GreaterThan);
        case_to_str(Ast_EqualEqual);
        case_to_str(Ast_NotEqual);
        case_to_str(Ast_CallOne);
        case_to_str(Ast_Call);
    }
#undef case_to_str
    return "";
}
