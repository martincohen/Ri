#pragma once

#include <co-lib.h>

typedef struct Ri Ri;
typedef struct RiError RiError;
typedef struct RiStream RiStream;
typedef struct RiToken RiToken;
typedef struct RiNode RiNode;
typedef struct RiScope RiScope;

typedef enum RiErrorKind RiErrorKind;
typedef enum RiTokenKind RiTokenKind;
typedef enum RiNodeKind RiNodeKind;
typedef enum RiDeclState RiDeclState;

//
//
//

enum RiErrorKind {
    RiError_None,
    RiError_UnexpectedCharacter,
    RiError_UnexpectedToken,
    RiError_Declared,
    RiError_NotDeclared,
    RiError_CyclicReference,
};

struct RiError {
    RiErrorKind kind;
    CharArray message;
};

//
//
//

enum RiTokenKind
{
    RiToken_Unknown,

    RiToken_End,
    RiToken_Identifier,

    RiToken_Keyword_Func,
    RiToken_Keyword_Variable,
    RiToken_Keyword_Const,
    RiToken_Keyword_Type,
    RiToken_Keyword_Struct,
    RiToken_Keyword_Union,
    RiToken_Keyword_Enum,

    RiToken_Keyword_Return,
    RiToken_Keyword_If,
    RiToken_Keyword_Else,
    RiToken_Keyword_For,
    RiToken_Keyword_Switch,
    RiToken_Keyword_Case,
    RiToken_Keyword_Default,
    RiToken_Keyword_Break,
    RiToken_Keyword_Continue,
    RiToken_Keyword_Fallthrough,

    RiToken_LP,
    RiToken_RP,
    RiToken_LB,
    RiToken_RB,
    RiToken_Dot,
    RiToken_Comma,
    RiToken_Semicolon,

    RiToken_Plus,
    RiToken_Minus,
    RiToken_Star,
    RiToken_Slash,
    RiToken_Percent,

    RiToken_PlusPlus,
    RiToken_MinusMinus,

    RiToken_Assign_FIRST__,
        RiToken_Eq,
        RiToken_PlusEq,
        RiToken_MinusEq,
        RiToken_StarEq,
        RiToken_SlashEq,
        RiToken_PercentEq,
        RiToken_AmpEq,
        RiToken_PipeEq,
        RiToken_BeakEq,
    RiToken_Assign_LAST__,

    RiToken_Comparison_FIRST__,
        RiToken_Lt,
        RiToken_Gt,
        RiToken_LtEq,
        RiToken_GtEq,
        RiToken_EqEq,
        RiToken_BangEq,
    RiToken_Comparison_LAST__,

    RiToken_Bitwise_FIRST__,
        RiToken_Amp,
        RiToken_Pipe,
        RiToken_Beak,
        RiToken_Tilde,
        RiToken_LtLt,
        RiToken_GtGt,
    RiToken_Bitwise_LAST__,

    RiToken_Boolean_FIRST__,
        RiToken_Bang,
        RiToken_AmpAmp,
        RiToken_PipePipe,
    RiToken_Boolean_LAST__,

    RiToken_COUNT__
};

#define ri_tokenkind_in(TokenKind, Prefix) \
    ((TokenKind) > Prefix ## _FIRST__ && (TokenKind) < Prefix ## _LAST__)


struct RiStream {
    char* start;
    char* end;
    char* it;
};

struct RiToken {
    RiTokenKind kind;
    char* start;
    char* end;
    union {
        String id;
        uint64_t integer;
        double real;
    };
};

//
//
//

typedef Slice(RiNode*) RiNodeSlice;
typedef ArrayWithSlice(RiNodeSlice) RiNodeArray;

enum RiDeclState {
    RiDeclState_Unresolved,
    RiDeclState_Resolving,
    RiDeclState_Resolved,
};

//
//
//

// TODO: Collapse FIRST and LAST values.

enum RiNodeKind
{
    RiNode_Unknown,

    RiNode_Scope,
    RiNode_Ref,

    RiNode_Decl_FIRST__,
        RiNode_Decl_Func,
        // TODO: RiNode_Decl_Type,
        // TODO: RiNode_Decl_Type_Func,
        RiNode_Decl_Type_Func_Arg,
        RiNode_Decl_Variable,
    RiNode_Decl_LAST__,

    RiNode_Func,

    RiNode_Type_FIRST__,
        RiNode_Type_Func,
        RiNode_Type_Number_FIRST__,
            RiNode_Type_Number_Int64,
            RiNode_Type_Number_UInt64,
            RiNode_Type_Number_Int32,
            RiNode_Type_Number_UInt32,
            RiNode_Type_Number_Int16,
            RiNode_Type_Number_UInt16,
            RiNode_Type_Number_Int8,
            RiNode_Type_Number_UInt8,
            RiNode_Type_Number_Float64,
            RiNode_Type_Number_Float32,
        RiNode_Type_Number_LAST__,
    RiNode_Type_LAST__,

    RiNode_Expr_FIRST__,
        RiNode_Expr_Identifier,
        RiNode_Expr_Variable,
        RiNode_Expr_Call,
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
            // Arithmetic
            RiNode_Expr_Binary_Add,
            RiNode_Expr_Binary_Sub,
            RiNode_Expr_Binary_Mul,
            RiNode_Expr_Binary_Div,
            RiNode_Expr_Binary_Mod,
            // Bitwise
            RiNode_Expr_Binary_BXor,
            RiNode_Expr_Binary_BAnd,
            RiNode_Expr_Binary_BOr,
            RiNode_Expr_Binary_BShL,
            RiNode_Expr_Binary_BShR,
            // Boolean
            RiNode_Expr_Binary_And,
            RiNode_Expr_Binary_Or,
            // Syntax
            RiNode_Expr_Binary_Select,

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
    RiNode_St_Switch,
    RiNode_St_Switch_Case,

    RiNode_COUNT__
};

#define ri_nodekind_in(NodeKind, Prefix) \
    ((NodeKind) > Prefix ## _FIRST__ && (NodeKind) < Prefix ## _LAST__)

struct RiNode
{
    RiNodeKind kind;
    RiNode* owner;
    int index;
    union {
        struct {
            Map map;
            RiNodeArray decl;
            RiNodeArray statements;
        } scope;
        RiNode* ref;
        struct {
            String id;
            RiDeclState state;
            union {
                struct {
                    RiNode* type;
                    // NOTE: If NULL then considered to be a _function prototype_.
                    RiNode* scope;
                } func;
                struct {
                    RiNode* type;
                } variable;
                struct {
                    RiNode* type;
                } type_func_arg;
                struct {
                    RiNodeArray inputs;
                    RiNodeArray outputs;
                } type_func;
            };
        } decl;
        struct {
            // NOTE: Reference to the original declaration.
            RiNode* decl;
            // NOTE: Can be NULL if this is an unnamed function.
            String id;
            RiNode* type;
            // NOTE: All arguments are made to be declared here.
            // NOTE: Contains zero or one statement with the function body.
            RiNode* scope;
        } func;
        String expr_id;
        struct {
            // NOTE: Reference to the original declaration.
            RiNode* decl;
            String id;
            RiNode* type;
        } expr_variable;
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
        struct {
            RiNode* func;
            RiNodeArray arguments;
        } expr_call;
        struct {
            // TODO: Merge with expr_call member?
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
            RiNode* pre;
            RiNode* condition;
            RiNode* post;
            RiNode* scope;
        } st_for;
        RiNode* st_expr;
    };
};

//
//
//

struct Ri {
    Arena arena;
    Intern intern;
    RiError error;
    RiStream stream;
    RiToken token;
    RiNode* scope;
    RiNodeArray pending;

    int index;

    const char* id_func;
    const char* id_var;
    const char* id_const;
    const char* id_type;
    const char* id_struct;
    const char* id_union;
    const char* id_enum;

    const char* id_return;
    const char* id_if;
    const char* id_else;
    const char* id_for;
    const char* id_switch;
    const char* id_case;
    const char* id_default;
    const char* id_break;
    const char* id_continue;
    const char* id_fallthrough;
};

//
//
//


void ri_init(Ri* ri);
void ri_purge(Ri* ri);
void ri_log(Ri* ri, RiNode* node);
RiNode* ri_resolve(Ri* ri, RiNode* node);
RiNode* ri_parse(Ri* ri, String stream);