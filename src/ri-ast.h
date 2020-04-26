#pragma once

#include "ri-common.h"

//
//
//

typedef enum RiNodeKind RiNodeKind;
typedef enum RiVarKind RiVarKind;
typedef enum RiSpecState RiSpecState;
typedef enum RiTypeCompleteness RiTypeCompleteness;
typedef struct RiNode RiNode;
typedef union RiLiteral RiLiteral;

typedef Slice(RiNode*) RiNodeSlice;
typedef ArrayWithSlice(RiNodeSlice) RiNodeArray;

//
//
//

enum RiSpecState {
    RiSpec_Unresolved,
    RiSpec_Resolving,
    RiSpec_Resolved,
};

//
//
//

enum RiTypeCompleteness {
    RiType_Incomplete,
    RiType_Completing,
    RiType_Completed,
};

//
//
//

// TODO: Collapse FIRST and LAST values.

enum RiNodeKind
{
    RiNode_Unknown,

    RiNode_Module,
    RiNode_Scope_FIRST__,
        RiNode_Scope_Root,
        RiNode_Scope_Module,
        RiNode_Scope_Struct,
        RiNode_Scope_Union,
        RiNode_Scope_Function_Root,
        RiNode_Scope_Function_Body,
        RiNode_Scope_If_Root,
        RiNode_Scope_If_Body,
        RiNode_Scope_Switch_Root,
        RiNode_Scope_Switch_Body,
        RiNode_Scope_For_Root,
        RiNode_Scope_For_Body,
        RiNode_Scope_Nested,
        RiNode_Scope_Statement,
    RiNode_Scope_LAST__,

    RiNode_Decl,

    RiNode_Spec_FIRST__,
        RiNode_Spec_Var,
        RiNode_Spec_Constant,
        RiNode_Spec_Func,
        RiNode_Spec_Module,
        RiNode_Spec_Type_FIRST__,
            // Used to denote a node that returns void (like statements).
            RiNode_Spec_Type_None,
            // Used with `var a = 1` to set a's type to Infer.
            RiNode_Spec_Type_Infer,
            RiNode_Spec_Type_Func,
            RiNode_Spec_Type_Struct,
            RiNode_Spec_Type_Union,
            RiNode_Spec_Type_Pointer,
            RiNode_Spec_Type_Number_FIRST__,
                RiNode_Spec_Type_Number_None_FIRST__,
                    RiNode_Spec_Type_Number_None_Int,
                    RiNode_Spec_Type_Number_None_Real,
                RiNode_Spec_Type_Number_None_LAST__,
                RiNode_Spec_Type_Number_Bool,
                RiNode_Spec_Type_Number_Int_FIRST__,
                    RiNode_Spec_Type_Number_Int_Signed_FIRST__,
                        RiNode_Spec_Type_Number_Int64,
                        RiNode_Spec_Type_Number_Int32,
                        RiNode_Spec_Type_Number_Int16,
                        RiNode_Spec_Type_Number_Int8,
                    RiNode_Spec_Type_Number_Int_Signed_LAST__,

                    RiNode_Spec_Type_Number_Int_Unsigned_FIRST__,
                        RiNode_Spec_Type_Number_UInt64,
                        RiNode_Spec_Type_Number_UInt32,
                        RiNode_Spec_Type_Number_UInt16,
                        RiNode_Spec_Type_Number_UInt8,
                    RiNode_Spec_Type_Number_Int_Unsigned_LAST__,
                RiNode_Spec_Type_Number_Int_LAST__,

                RiNode_Spec_Type_Number_Float_FIRST__,
                    RiNode_Spec_Type_Number_Float64,
                    RiNode_Spec_Type_Number_Float32,
                RiNode_Spec_Type_Number_Float_LAST__,

                RiNode_Spec_Type_Number_Enum,
            RiNode_Spec_Type_Number_LAST__,
        RiNode_Spec_Type_LAST__,
    RiNode_Spec_LAST__,

    RiNode_Expr_FIRST__,
        RiNode_Expr_Symbol,
        RiNode_Expr_Select,
        RiNode_Expr_Field,
        RiNode_Expr_Constant,
        RiNode_Expr_Call,
        RiNode_Expr_Cast,

        // TODO: As macro
        RiNode_Expr_AddrOf,

        RiNode_Expr_Unary_FIRST__,
            // Arithmetic
            RiNode_Expr_Unary_Positive,
            RiNode_Expr_Unary_Negative,
            RiNode_Expr_Unary_IncPost,
            RiNode_Expr_Unary_DecPost,
            RiNode_Expr_Unary_IncPre,
            RiNode_Expr_Unary_DecPre,
            // Bitwise
            RiNode_Expr_Unary_BNeg,
            // Boolean
            RiNode_Expr_Unary_Not,
        RiNode_Expr_Unary_LAST__,

        RiNode_Expr_Binary_FIRST__,
            RiNode_Expr_Binary_Numeric_FIRST__,
                // Arithmetic
                RiNode_Expr_Binary_Numeric_Arithmetic_FIRST__,
                    RiNode_Expr_Binary_Numeric_Arithmetic_Add,
                    RiNode_Expr_Binary_Numeric_Arithmetic_Sub,
                    RiNode_Expr_Binary_Numeric_Arithmetic_Mul,
                    RiNode_Expr_Binary_Numeric_Arithmetic_Div,
                    RiNode_Expr_Binary_Numeric_Arithmetic_Mod,
                RiNode_Expr_Binary_Numeric_Arithmetic_LAST__,
                // Bitwise
                RiNode_Expr_Binary_Numeric_Bitwise_FIRST__,
                    RiNode_Expr_Binary_Numeric_Bitwise_BXor,
                    RiNode_Expr_Binary_Numeric_Bitwise_BAnd,
                    RiNode_Expr_Binary_Numeric_Bitwise_BOr,
                    RiNode_Expr_Binary_Numeric_Bitwise_BShL,
                    RiNode_Expr_Binary_Numeric_Bitwise_BShR,
                RiNode_Expr_Binary_Numeric_Bitwise_LAST__,
                // Boolean
                RiNode_Expr_Binary_Numeric_Boolean_FIRST__,
                    RiNode_Expr_Binary_Numeric_Boolean_And,
                    RiNode_Expr_Binary_Numeric_Boolean_Or,
                RiNode_Expr_Binary_Numeric_Boolean_LAST__,
            RiNode_Expr_Binary_Numeric_LAST__,

            RiNode_Expr_Binary_Comparison_FIRST__,
                RiNode_Expr_Binary_Comparison_Lt,
                RiNode_Expr_Binary_Comparison_Gt,
                RiNode_Expr_Binary_Comparison_LtEq,
                RiNode_Expr_Binary_Comparison_GtEq,
                RiNode_Expr_Binary_Comparison_Eq,
                RiNode_Expr_Binary_Comparison_NotEq,
            RiNode_Expr_Binary_Comparison_LAST__,
        RiNode_Expr_Binary_LAST__,
    RiNode_Expr_LAST__,

    RiNode_St_Assign_FIRST__,
        RiNode_St_Assign,
        RiNode_St_Assign_Add,
        RiNode_St_Assign_Sub,
        RiNode_St_Assign_Mul,
        RiNode_St_Assign_Div,
        RiNode_St_Assign_Mod,
        RiNode_St_Assign_And,
        RiNode_St_Assign_Or,
        RiNode_St_Assign_Xor,
    RiNode_St_Assign_LAST__,

    RiNode_St_Expr,
    RiNode_St_Return,
    RiNode_St_If,
    RiNode_St_For,
    RiNode_St_Break,
    RiNode_St_Continue,
    RiNode_St_Switch,
    RiNode_St_Switch_Case,
    RiNode_St_Switch_Default,
    RiNode_St_Switch_Fallthrough,

    RiNode_COUNT__
};

static inline ri_is_in_(RiNodeKind kind, RiNodeKind first, RiNodeKind last) {
    return (kind > first) && (kind < last);
}

#define ri_is_in(NodeKind, Prefix) \
    ri_is_in_(NodeKind, Prefix ## _FIRST__, Prefix ## _LAST__)

#define ri_is_expr(NodeKind) \
    (ri_is_in(NodeKind, RiNode_Expr))

#define ri_is_spec(NodeKind) \
    (ri_is_in(NodeKind, RiNode_Spec))

#define ri_is_type(NodeKind) \
    (ri_is_in(NodeKind, RiNode_Spec_Type))

//
//
//

enum RiVarKind {
    RiVar_Local,
    RiVar_Input,
    RiVar_Output,
    RiVar_Field,
};

//
//
//

union RiLiteral {
    uint64_t integer;
    double real;
    String string;
    bool boolean;
    void* pointer;
};

//
//
//

struct RiNode
{
    RiNodeKind kind;
    // Scope that owns this node.
    // We can get rid of this pointer but we'll have to maintain a scope stack for every walk.
    RiNode* owner;
    RiPos pos;
    union {
        struct {
            String name;
            // When resolved, spec is filled, otherwise it's NULL.
            RiNode* spec;
        } symbol;

        struct {
            RiNode* scope;
        } module;

        struct {
            Map map;
            // TODO: Doesn't seem to be essential.
            RiNodeArray decl;
            RiNodeArray statements;
        } scope;

        struct {
            RiNode* spec;
            String id;
        } decl;

        struct {
            RiSpecState state;
            RiNode* decl;
            union {
                // When module is loaded, here we keep the reference to RiNode_Module node.
                RiNode* module;

                struct {
                    RiNode* type;
                    RiNode* scope;
                    // Used by compiler.
                    // RI_INVALID_SLOT by default.
                    uint32_t slot;
                } func;

                struct {
                    RiNode* type;
                    RiVarKind kind;
                    // Used by compiler.
                    // RI_INVALID_SLOT by default.
                    uint32_t slot;
                } var;

                struct {
                    RiNode* type;
                    RiLiteral value;
                } constant;

                struct {
                    RiTypeCompleteness completeness;
                    // NOTE: This is set to another type declared earlier that is the same as this one.
                    // It is used for comparing whether two types are identical.
                    // We cannot just merge entire node to the base one, since that would make the
                    // AST useless for other applications (like editor).
                    RiNode* identity;
                    union {
                        struct {
                            RiNodeArray inputs;
                            RiNodeArray outputs;
                        } func;
                        struct {
                            RiNode* scope;
                        } compound;
                        struct {
                            RiNode* base;
                        } pointer;
                    };
                } type;
            };
        } spec;

        // Statement/Expression operators.

        struct {
            RiNode* parent;
            RiNode* child;
        } field;

        struct {
            // NOTE: Data used by all RiNode_Expr_Binary_* types.
            // NOTE: Data used by all RiNode_St_Assign_* types.
            RiNode* argument0;
            RiNode* argument1;
        } binary;
        struct {
            // NOTE: Data used by all RiNode_Expr_Unary_* types.
            RiNode* argument;
        } unary;

        // Expressions

        struct {
            RiNode* func;
            RiNodeArray arguments;
        } call;

        struct {
            RiNode* type;
            // Used for inline constants, otherwise spec is not NULL.
            // All literals are resolved to their final type in resolve phase.
            // For untyped constants, spec == NULL and type == NULL.
            RiLiteral literal;
        } constant;

        // Statements

        struct {
            // NOTE: Can be NULL.
            RiNode* argument;
        } st_return;

        struct {
            // NOTE: Can be NULL.
            RiNode* pre;
            // NOTE: Can be NULL.
            RiNode* condition;
            // NOTE: Has one or two statements referencing scopes.
            // One for `if` block.
            // One for `else` scope or `if`.
            RiNode* scope;
        } st_if;

        struct {
            // NOTE: Can be NULL.
            RiNode* pre;
            // NOTE: Can be NULL.
            RiNode* condition;
            // NOTE: Can be NULL.
            RiNode* post;
            // NOTE: Always present.
            RiNode* scope;
        } st_for;

        struct {
            // NOTE: Can be NULL.
            RiNode* pre;
            // NOTE: Always present.
            RiNode* expr;
            // NOTE: Always present.
            RiNode* scope;
        } st_switch;

        struct {
            RiNode* expr;
        } st_switch_case;

        RiNode* st_expr;
    };
};

RiNode* ri_make_node_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind);
RiNode* ri_make_scope_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind);
RiNode* ri_make_spec_var_(Arena* arena, RiNode* owner, RiPos pos, RiNode* type, RiVarKind kind);
RiNode* ri_make_spec_constant_(Arena* arena, RiNode* owner, RiPos pos, RiNode* type, RiLiteral value);
RiNode* ri_make_spec_module_(Arena* arena, RiNode* owner, RiPos pos);
RiNode* ri_make_spec_func_(Arena* arena, RiNode* owner, RiPos pos, RiNode* type, RiNode* scope);
RiNode* ri_make_spec_type_func_(Arena* arena, RiNode* owner, RiPos pos, RiNodeArray inputs, RiNodeArray outputs);
RiNode* ri_make_spec_type_number_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind);
RiNode* ri_make_spec_type_struct_(Arena* arena, RiNode* owner, RiPos pos, RiNode* scope);
RiNode* ri_make_decl_(Arena* arena, RiNode* owner, RiPos pos, String id, RiNode* spec);
RiNode* ri_make_expr_symbol_(Arena* arena, RiNode* owner, RiPos pos, String name);
RiNode* ri_make_expr_field_(Arena* arena, RiPos pos, RiNode* parent, RiNode* child);
RiNode* ri_make_expr_call_(Arena* arena, RiNode* owner, RiPos pos, RiNode* func);
RiNode* ri_make_expr_binary_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind, RiNode* argument0, RiNode* argument1);
RiNode* ri_make_expr_unary_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind, RiNode* argument);
RiNode* ri_make_st_assign_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind, RiNode* argument0, RiNode* argument1);
RiNode* ri_make_st_expr_(Arena* arena, RiNode* owner, RiPos pos, RiNode* expr);
RiNode* ri_make_st_return_(Arena* arena, RiNode* owner, RiPos pos, RiNode* argument);
RiNode* ri_make_st_if_(Arena* arena, RiNode* owner, RiPos pos, RiNode* pre, RiNode* condition, RiNode* scope);
RiNode* ri_make_st_for_(Arena* arena, RiNode* owner, RiPos pos, RiNode* pre, RiNode* condition, RiNode* post, RiNode* scope);
RiNode* ri_make_st_switch_(Arena* arena, RiNode* owner, RiPos pos, RiNode* pre, RiNode* expr, RiNode* scope);
RiNode* ri_make_st_switch_case_(Arena* arena, RiNode* owner, RiPos pos, RiNode* expr);
RiNode* ri_make_st_switch_default_(Arena* arena, RiNode* owner, RiPos pos);
RiNode* ri_make_st_break_(Arena* arena, RiNode* owner, RiPos pos);
RiNode* ri_make_st_continue_(Arena* arena, RiNode* owner, RiPos pos);

// Extracts Spec from Symbol, Field or Decl.
RiNode* ri_get_spec_(RiNode* node);
void ri_type_get_title_(RiNode* type, CharArray* o_buffer);
