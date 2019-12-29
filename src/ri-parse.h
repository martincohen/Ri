#pragma once

#include "ri-common.h"
#include "ri-ast.h"

typedef enum RiTokenKind RiTokenKind;
typedef struct RiToken RiToken;
typedef struct RiStream RiStream;
typedef struct RiGlobals RiGlobals;
typedef struct RiParser RiParser;

enum RiTokenKind
{
    RiToken_Unknown,

    RiToken_End,
    RiToken_Identifier,

    RiToken_Integer,
    RiToken_Real,
    RiToken_String,

    RiToken_Keyword_Func,
    RiToken_Keyword_Variable,
    RiToken_Keyword_Const,
    RiToken_Keyword_Type,
    RiToken_Keyword_Struct,
    RiToken_Keyword_Union,
    RiToken_Keyword_Enum,

    RiToken_Keyword_Import,

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

    RiToken_Keyword_True,
    RiToken_Keyword_False,
    RiToken_Keyword_Nil,

    RiToken_LP,
    RiToken_RP,
    RiToken_LB,
    RiToken_RB,
    RiToken_Dot,
    RiToken_Comma,
    RiToken_Semicolon,
    RiToken_Colon,

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

struct RiToken {
    RiTokenKind kind;
    RiPos pos;
    char* start;
    char* end;
    union {
        String id;
        uint64_t integer;
        double real;
    };
};

struct RiStream
{
    iptr line_index;
    char* line;
    char* start;
    char* end;
    char* it;
};

struct RiParser
{
    Ri* ri;
    RiStream stream;
    Arena* arena;

    // State
    RiToken token;
    RiNode* scope;
};

RiNode* ri_parse_(Ri* ri, ByteSlice stream);
