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

enum RiTokenKind {
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

    RiToken_LP,
    RiToken_RP,
    RiToken_LB,
    RiToken_RB,
    RiToken_Comma,
    RiToken_Semicolon,

    RiToken_Plus,
    RiToken_Minus,

    RiToken_Eq_FIRST__,
        RiToken_Eq,
        RiToken_PlusEq,
        RiToken_MinusEq,
    RiToken_Eq_LAST__,

    RiToken_EqEq,

    RiToken_
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
        RiNode_Expr_Unary,
        RiNode_Expr_Binary_FIRST__,
            RiNode_Expr_Binary_Plus,
            RiNode_Expr_Binary_Minus,
        RiNode_Expr_Binary_LAST__,
        RiNode_Expr_Call,
    RiNode_Expr_LAST__,

    RiNode_St_Expr,
    RiNode_St_Return,
    RiNode_St_If,
    RiNode_St_For,
    RiNode_St_Switch,
    RiNode_St_Switch_Case,
    RiNode_St_Assign_FIRST__,
        RiNode_St_Assign,
        RiNode_St_Assign_Plus,
        RiNode_St_Assign_Minus,
    RiNode_St_Assign_LAST__,
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
            RiNode* argument0;
            RiNode* argument1;
        } expr_binary;
        struct {
            // NOTE: Data used by all RiNode_Expr_Unary_* types.
            RiNode* argument;
        } expr_unary;
        struct {
            RiNode* func;
            RiNodeArray arguments;
        } expr_call;
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
            RiNode* pre;
            RiNode* condition;
            RiNode* post;
            RiNode* scope;
        } st_for;
        struct {
            // NOTE: Can be either a variable or ref to declaration of a variable.
            RiNode* argument0;
            RiNode* argument1;
        } st_assign;
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
};

//
//
//


void ri_init(Ri* ri);
void ri_purge(Ri* ri);
void ri_log(Ri* ri, RiNode* node);
RiNode* ri_resolve(Ri* ri, RiNode* node);
RiNode* ri_parse(Ri* ri, String stream);