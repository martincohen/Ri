#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "ri.h"
#include "ri-print.c"

#define RI_TODO FAIL("todo")
#define RI_POS_OUTSIDE (RiPos){ -1, -1 }
#define RI_ID_NULL (String){ NULL, 0 }

static const struct {
    RiNodeKind unary;
    RiNodeKind binary;
    RiNodeKind assign;
} RI_TOKEN_TO_OP_[RiToken_COUNT__] = {
    [RiToken_Dot]           = { .binary = RiNode_Expr_Binary_Select },
    [RiToken_Plus]          = { .unary = RiNode_Expr_Unary_Positive,
                                .binary = RiNode_Expr_Binary_Numeric_Arithmetic_Add },
    [RiToken_Minus]         = { .unary = RiNode_Expr_Unary_Negative,
                                .binary = RiNode_Expr_Binary_Numeric_Arithmetic_Sub },
    [RiToken_Star]          = { .binary = RiNode_Expr_Binary_Numeric_Arithmetic_Mul },
    [RiToken_Slash]         = { .binary = RiNode_Expr_Binary_Numeric_Arithmetic_Div },
    [RiToken_Percent]       = { .binary = RiNode_Expr_Binary_Numeric_Arithmetic_Mod },

    // [RiToken_PlusPlus]      = { .unary = RiNode_Expr_Unary_Inc },
    // [RiToken_MinusMinus]    = { .unary = RiNode_Expr_Unary_Dec },

    [RiToken_Eq]            = { .assign = RiNode_St_Assign },
    [RiToken_PlusEq]        = { .assign = RiNode_St_Assign_Add },
    [RiToken_MinusEq]       = { .assign = RiNode_St_Assign_Sub },
    [RiToken_StarEq]        = { .assign = RiNode_St_Assign_Mul },
    [RiToken_SlashEq]       = { .assign = RiNode_St_Assign_Div },
    [RiToken_PercentEq]     = { .assign = RiNode_St_Assign_Mod },
    [RiToken_AmpEq]         = { .assign = RiNode_St_Assign_And },
    [RiToken_PipeEq]        = { .assign = RiNode_St_Assign_Or },
    [RiToken_BeakEq]        = { .assign = RiNode_St_Assign_Xor },

    [RiToken_Lt]            = { .binary = RiNode_Expr_Binary_Comparison_Lt },
    [RiToken_Gt]            = { .binary = RiNode_Expr_Binary_Comparison_Gt },
    [RiToken_LtEq]          = { .binary = RiNode_Expr_Binary_Comparison_LtEq },
    [RiToken_GtEq]          = { .binary = RiNode_Expr_Binary_Comparison_GtEq },
    [RiToken_EqEq]          = { .binary = RiNode_Expr_Binary_Comparison_Eq },
    [RiToken_BangEq]        = { .binary = RiNode_Expr_Binary_Comparison_NotEq },

    [RiToken_Amp]           = { .unary = RiNode_Expr_AddrOf,
                                .binary = RiNode_Expr_Binary_Numeric_Bitwise_BAnd },
    [RiToken_Pipe]          = { .binary = RiNode_Expr_Binary_Numeric_Bitwise_BOr },
    [RiToken_Beak]          = { .binary = RiNode_Expr_Binary_Numeric_Bitwise_BXor },
    [RiToken_Tilde]         = { .unary = RiNode_Expr_Unary_BNeg },
    [RiToken_LtLt]          = { .binary = RiNode_Expr_Binary_Numeric_Bitwise_BShL },
    [RiToken_GtGt]          = { .binary = RiNode_Expr_Binary_Numeric_Bitwise_BShR },

    [RiToken_Bang]          = { .unary = RiNode_Expr_Unary_Not },
    [RiToken_AmpAmp]        = { .binary = RiNode_Expr_Binary_Numeric_Boolean_And },
    [RiToken_PipePipe]      = { .binary = RiNode_Expr_Binary_Numeric_Boolean_Or },
};

const char* RI_OP_NAMES_[] = {
    [RiNode_Expr_Unary_Positive] = "+",
    [RiNode_Expr_Unary_Negative] = "-",
    [RiNode_Expr_Unary_IncPost] = "_++",
    [RiNode_Expr_Unary_DecPost] = "_--",
    [RiNode_Expr_Unary_IncPre] = "++_",
    [RiNode_Expr_Unary_DecPre] = "--_",
    [RiNode_Expr_Unary_BNeg] = "~",
    [RiNode_Expr_Unary_Not] = "!",

    [RiNode_Expr_Binary_Numeric_Arithmetic_Add] = "+",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Sub] = "-",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Mul] = "*",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Div] = "/",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Mod] = "%",

    [RiNode_Expr_Binary_Numeric_Bitwise_BXor] = "^",
    [RiNode_Expr_Binary_Numeric_Bitwise_BAnd] = "&",
    [RiNode_Expr_Binary_Numeric_Bitwise_BOr] = "|",
    [RiNode_Expr_Binary_Numeric_Bitwise_BShL] = "<<",
    [RiNode_Expr_Binary_Numeric_Bitwise_BShR] = ">>",

    [RiNode_Expr_Binary_Numeric_Boolean_And] = "&&",
    [RiNode_Expr_Binary_Numeric_Boolean_Or] = "||",
};

//
//
//

inline String ri_make_id_0(Ri* ri, char* start, char* end);
inline String ri_make_id_(Ri* ri, String string);

//
//
//

#define ri_error_check_(Ri) \
    RI_CHECK((Ri)->error.kind == RiError_None)

static inline void
ri_error_set_(Ri* ri, RiErrorKind kind, RiPos pos, const char* format, ...)
{
    RI_CHECK(ri->error.kind == RiError_None);
    RI_CHECK(kind != RiError_None);
    ri->error.kind = kind;
    ri->error.pos = pos;

    va_list args;
    va_start(args, format);
    chararray_push_fv(&ri->error.message, format, args);
    va_end(args);

    // TODO: Use current file's name/path.
    RI_LOG("error %S:%d:%d: %S", ri->path.slice, pos.row + 1, pos.col + 1, ri->error.message.slice);
    // __debugbreak();
}

static inline void
ri_error_set_unexpected_token_(Ri* ri, RiToken* token)
{
    ri_error_set_(ri, RiError_UnexpectedToken, token->pos, "unexpected token '%.*s' (%d)",
        token->end - token->start,
        token->start,
        token->kind
    );
}

static inline void
ri_error_set_mismatched_types_(Ri* ri, RiPos pos, RiNode* type0, RiNode* type1, const char* op)
{
    RI_CHECK(ri_is_in(type0->kind, RiNode_Spec_Type));
    RI_CHECK(ri_is_in(type1->kind, RiNode_Spec_Type));

    if (op == NULL)  {
        op = "and";
    }

    // TODO: Get proper names.
    // TODO: Merge same types.
    ri_error_set_(ri, RiError_Type, pos, "mismatched types %S %s %S",
        // ri->node_meta[type0->kind].node->spec.id,
        type0->spec.id,
        op,
        // ri->node_meta[type1->kind].node->spec.id
        type1->spec.id
    );
}

//
//
//

static inline String
ri_make_id_r_(Ri* ri, char* start, char* end) {
    RI_CHECK(start);
    RI_CHECK(end);
    RI_CHECK(start <= end);
    return (String){
        .items = (char*)intern_put_r(&ri->intern, start, end),
        .count = end - start
    };
}

static inline String
ri_make_id_(Ri* ri, String string) {
    RI_CHECK(string.items);
    RI_CHECK(string.count >= 0);
    return (String){
        .items = (char*)intern_put_c(&ri->intern, string.items, string.count),
        .count = string.count
    };
}

//
//
//

static inline bool
ri_rune_is_id_(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static inline bool
ri_rune_is_number_(char c) {
    return (c >= '0' && c <= '9');
}

static inline RiTokenKind
ri_lex_one_or_two_(Ri* ri, char** it, char* end, enum RiTokenKind op0, char ch, enum RiTokenKind op1)
{
    ++*it;
    if (*it < end && **it == ch) {
        ++*it;
        return op1;
    } else {
        return op0;
    }
}

static inline bool
ri_lex_real_decimal_(char** it_, char* end)
{
    char* it = *it_;
    if (it < end && ri_rune_is_number_(*it)) {
        ++it;
        while (it < end && ri_rune_is_number_(*it)) ++it;
        *it_ = it;
        return true;
    }
    return false;
}

static inline bool
ri_lex_real_exponent_(char** it_, char* end)
{
    return false;
}

// NOTE: Expects the stream to end with 0.
static inline RiTokenKind
ri_lex_number_(Ri* ri, char** it_, char* end, RiToken* token)
{
    // <digit+>?.<digit+>([eE][+-]<digit+>)?
    char* it = *it_;
    token->kind = RiToken_Integer;
    while (ri_rune_is_number_(*it)) ++it;

    if (*it == '.') {
        token->kind = RiToken_Real;
        ++it;
        while (ri_rune_is_number_(*it)) ++it;
    }

    if (*it == 'e' || *it == 'E') {
        token->kind = RiToken_Real;
        ++it;
        if (*it == '+' || *it == '-') ++it;
        if (!ri_rune_is_number_(*it)) {
            token->pos.col += *it_ - it;
            ri_error_set_(ri, RiError_UnexpectedCharacter, token->pos, "digit or '+' or '-' expected");
            return false;
        } else {
            ++it;
            while (ri_rune_is_number_(*it)) ++it;
        }
    }

    switch (token->kind)
    {
        case RiToken_Integer: {
            uint64_t i = 0;
            char* start = *it_;
            while (start < it && ri_rune_is_number_(*start)) {
                // TODO: Check for overflow.
                i = i * 10 + (*start - '0');
                ++start;
            }
            token->integer = i;
        } break;

        case RiToken_Real: {
            double r = strtod(*it_, NULL);
            if (r == HUGE_VAL) {
                ri_error_set_(ri, RiError_UnexpectedToken, token->pos, "real constant too large");
            }
            token->real = r;
        } break;

        default:
            RI_UNREACHABLE;
    }

    *it_ = it;
    return true;
}

static bool
ri_lex_next_(Ri* ri)
{
    ri_error_check_(ri);

    RiStream* stream = &ri->stream;
    RiToken* token = &ri->token;
    char* it = stream->it;
    char* end = stream->end;

next:
    stream->it = it;

    ri->token.kind = RiToken_Unknown;
    ri->token.start = it;
    ri->token.end = it;
    ri->token.pos.row = ri->stream.line_index;
    ri->token.pos.col = it - ri->stream.line;

    if (stream->it == stream->end) {
        token->kind = RiToken_End;
        return true;
    }

    switch (*it)
    {
        case ' ': case '\t':
            while (it < end && (it[0] == ' ' || it[0] == '\t')) {
                ++it;
            }
            goto next;

        case '\r':
            ++it;
            if (it < end && it[0] == '\n') {
                ++it;
            }
            ++stream->line_index;
            stream->line = it;
            goto next;

        case '\n':
            ++it;
            ++stream->line_index;
            stream->line = it;
            goto next;

        case '(': ++it; token->kind = RiToken_LP; break;
        case ')': ++it; token->kind = RiToken_RP; break;
        case '{': ++it; token->kind = RiToken_LB; break;
        case '}': ++it; token->kind = RiToken_RB; break;
        case ',': ++it; token->kind = RiToken_Comma; break;
        case ';': ++it; token->kind = RiToken_Semicolon; break;
        case ':': ++it; token->kind = RiToken_Colon; break;

        case '~': ++it; token->kind = RiToken_Tilde; break;

        case '=': token->kind = ri_lex_one_or_two_(ri, &it, end, RiToken_Eq, '=', RiToken_EqEq); break;
        case '*': token->kind = ri_lex_one_or_two_(ri, &it, end, RiToken_Star, '=', RiToken_StarEq); break;
        case '%': token->kind = ri_lex_one_or_two_(ri, &it, end, RiToken_Percent, '=', RiToken_PercentEq); break;
        case '^': token->kind = ri_lex_one_or_two_(ri, &it, end, RiToken_Beak, '=', RiToken_BeakEq); break;

        case '!': token->kind = ri_lex_one_or_two_(ri, &it, end, RiToken_Bang, '=', RiToken_BangEq); break;

        // TODO: Simplify following with call:
        // ri_lex_map_(ri, &it, end, "=&", RiToken_Amp, RiToken_AmpEq, RiToken_AmpAmp)

        // & &= &&
        case '&':
            ++it;
            token->kind = RiToken_Amp;
            if (it < stream->end) {
                switch (it[0]) {
                    case '=': token->kind = RiToken_AmpEq; ++it; break;
                    case '&': token->kind = RiToken_AmpAmp; ++it; break;
                }
            }
            break;
        // | |= ||
        case '|':
            ++it;
            token->kind = RiToken_Pipe;
            if (it < stream->end) {
                switch (it[0]) {
                    case '=': token->kind = RiToken_PipeEq; ++it; break;
                    case '|': token->kind = RiToken_PipePipe; ++it; break;
                }
            }
            break;

        // + += ++
        case '+':
            ++it;
            token->kind = RiToken_Plus;
            if (it < stream->end) {
                switch (it[0]) {
                    case '=': token->kind = RiToken_PlusEq; ++it; break;
                    case '+': token->kind = RiToken_PlusPlus; ++it; break;
                }
            }
            break;
        // - -= --
        case '-':
            ++it;
            token->kind = RiToken_Minus;
            if (it < stream->end) {
                switch (it[0]) {
                    case '=': token->kind = RiToken_MinusEq; ++it; break;
                    case '-': token->kind = RiToken_MinusMinus; ++it; break;
                }
            }
            break;

        // < <= <<
        case '<':
            ++it;
            token->kind = RiToken_Lt;
            if (it < stream->end) {
                switch (it[0]) {
                    case '=': token->kind = RiToken_LtEq; ++it; break;
                    case '<': token->kind = RiToken_LtLt; ++it; break;
                }
            }
            break;

        // < <= <<
        case '>':
            ++it;
            token->kind = RiToken_Gt;
            if (it < stream->end) {
                switch (it[0]) {
                    case '>': token->kind = RiToken_GtGt; ++it; break;
                    case '=': token->kind = RiToken_GtEq; ++it; break;
                }
            }
            break;

        case '/':
            ++it;
            if (it < end)
            {
                switch (it[0])
                {
                    case '/':
                        while (it < end && it[0] != '\r' && it[0] != '\n') {
                            ++it;
                        }
                        goto next;
                    case '*':
                        // TODO: Block comments.
                        RI_TODO;
                        break;
                    case '=':
                        ++it;
                        token->kind = RiToken_SlashEq;
                        break;
                    default:
                        ++it;
                        token->kind = RiToken_Slash;
                        break;
                }
            }
            break;


        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_': {
            token->kind = RiToken_Identifier;
            ++it;
            while (it < end && ri_rune_is_id_(*it)) {
                ++it;
            }
            token->id = ri_make_id_r_(ri, token->start, it);
            if (token->id.items == ri->id_func) {
                token->kind = RiToken_Keyword_Func;
            } else if (token->id.items == ri->id_var) {
                token->kind = RiToken_Keyword_Variable;
            } else if (token->id.items == ri->id_const) {
                token->kind = RiToken_Keyword_Const;
            } else if (token->id.items == ri->id_type) {
                token->kind = RiToken_Keyword_Type;
            } else if (token->id.items == ri->id_struct) {
                token->kind = RiToken_Keyword_Struct;
            } else if (token->id.items == ri->id_union) {
                token->kind = RiToken_Keyword_Union;
            } else if (token->id.items == ri->id_enum) {
                token->kind = RiToken_Keyword_Enum;
            } else if (token->id.items == ri->id_return) {
                token->kind = RiToken_Keyword_Return;
            } else if (token->id.items == ri->id_if) {
                token->kind = RiToken_Keyword_If;
            } else if (token->id.items == ri->id_else) {
                token->kind = RiToken_Keyword_Else;
            } else if (token->id.items == ri->id_for) {
                token->kind = RiToken_Keyword_For;
            } else if (token->id.items == ri->id_switch) {
                token->kind = RiToken_Keyword_Switch;
            } else if (token->id.items == ri->id_case) {
                token->kind = RiToken_Keyword_Case;
            } else if (token->id.items == ri->id_default) {
                token->kind = RiToken_Keyword_Default;
            } else if (token->id.items == ri->id_break) {
                token->kind = RiToken_Keyword_Break;
            } else if (token->id.items == ri->id_continue) {
                token->kind = RiToken_Keyword_Continue;
            } else if (token->id.items == ri->id_fallthrough) {
                token->kind = RiToken_Keyword_Fallthrough;
            } else if (token->id.items == ri->id_true) {
                token->kind = RiToken_Keyword_True;
            } else if (token->id.items == ri->id_false) {
                token->kind = RiToken_Keyword_False;
            } else if (token->id.items == ri->id_nil) {
                token->kind = RiToken_Keyword_Nil;
            }
        } break;

        case '.':
            ++it;
            if (ri_rune_is_number_(*it)) {
                --it;
                if (ri_lex_number_(ri, &it, end, &ri->token) == false) {
                    return false;
                }
                RI_CHECK(ri->token.kind == RiToken_Real);
            } else {
                token->kind = RiToken_Dot;
            }
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            if (ri_lex_number_(ri, &it, end, &ri->token) == false) {
                return false;
            }
        } break;

        default: {
            ri_error_set_(ri, RiError_UnexpectedCharacter, ri->token.pos, "unexpected character '%c' (%d)", *it, *it);
            return false;
        } break;
    }

    token->end = it;
    stream->it = it;

    if (ri->debug_tokens) {
        RI_LOG_DEBUG("'%.*s' (%d:%d)",
            token->end - token->start,
            token->start,
            token->pos.row + 1,
            token->pos.col + 1
        );
    }

    return true;
}

static bool
ri_lex_assert_token_(Ri* ri, RiTokenKind expected_kind)
{
    ri_error_check_(ri);

    if (ri->token.kind != expected_kind) {
        ri_error_set_unexpected_token_(ri, &ri->token);
        return false;
    }

    return true;
}

typedef enum RiLexNextResult_ {
    RiLexNextIf_NoMatch = -1,
    RiLexNextIf_Error = 0,
    RiLexNextIf_Match = 1
} RiLexNextResult_;

static RiLexNextResult_
ri_lex_next_if_(Ri* ri, RiTokenKind expected_kind)
{
    ri_error_check_(ri);

    if (ri->token.kind != expected_kind) {
        return RiLexNextIf_NoMatch;
    }

    if (!ri_lex_next_(ri)) {
        return RiLexNextIf_Error;
    }

    return RiLexNextIf_Match;
}

static bool
ri_lex_expect_token_(Ri* ri, RiTokenKind expected_kind)
{
    ri_error_check_(ri);

    if (!ri_lex_assert_token_(ri, expected_kind)) {
        return false;
    }

    if (!ri_lex_next_(ri)) {
        return false;
    }

    return true;
}


static bool
ri_stream_set_(Ri* ri, String stream)
{
    ri_error_check_(ri);

    ri->stream.start = stream.items;
    ri->stream.end = stream.items + stream.count;
    ri->stream.it = stream.items;
    ri->stream.line = stream.items;

    return ri_lex_next_(ri);
}

//
//
//

static RiNode*
ri_get_spec_(Ri* ri, RiNode* node)
{
    RiNode* spec = node;
    if (ri_is_in(spec->kind, RiNode_Value)) {
        spec = node->value.spec;
    }
    RI_CHECK(ri_is_in(spec->kind, RiNode_Spec));
    return spec;
}

//
// Comparing types
//

static bool ri_compare_types_(Ri* ri, RiNode* t0, RiNode* t1);

static bool
ri_compare_function_arg_types_(Ri* ri, RiNodeArray* a0, RiNodeArray* a1)
{
    if (a0->count != a1->count) {
        return false;
    }

    for (int i = 0; i < a0->count; ++i) {
        RiNode* it0 = a0->items[i];
        RiNode* it1 = a1->items[i];
        RI_CHECK(it0->kind == RiNode_Decl);
        RI_CHECK(it0->decl.spec->kind == RiNode_Spec_Var);
        RI_CHECK(it1->kind == RiNode_Decl);
        RI_CHECK(it1->decl.spec->kind == RiNode_Spec_Var);
        if (ri_compare_types_(ri, ri_get_spec_(ri, it0->decl.spec->spec.var.type), ri_get_spec_(ri, it1->decl.spec->spec.var.type)) == false) {
            return false;
        }
    }

    return true;
}

static bool
ri_compare_types_(Ri* ri, RiNode* t0, RiNode* t1)
{
    RI_CHECK(t0 && ri_is_in(t0->kind, RiNode_Spec_Type));
    RI_CHECK(t1 && ri_is_in(t1->kind, RiNode_Spec_Type));

    if (t0->kind != t1->kind) {
        return false;
    }

    switch (t0->kind)
    {
        case RiNode_Spec_Type_Func:
            return (
                ri_compare_function_arg_types_(ri, &t0->spec.type.func.inputs, &t1->spec.type.func.inputs) &&
                ri_compare_function_arg_types_(ri, &t0->spec.type.func.outputs, &t1->spec.type.func.outputs)
            );

        case RiNode_Spec_Type_Pointer:
            return ri_compare_types_(ri, t0->spec.type.pointer.base, t1->spec.type.pointer.base);

        case RiNode_Spec_Type_None:
            return true;

        case RiNode_Spec_Type_Infer:
        default:
            RI_UNREACHABLE;
            return false;
    }
}

static void
ri_mark_identical_type_(Ri* ri, RiNode* node)
{
    RI_CHECK(ri_is_in(node->kind, RiNode_Spec_Type));
    RiNode* it;
    array_eachi(&ri->types, i, &it) {
        if (ri_compare_types_(ri, it, node)) {
            node->spec.type.identical = it;
            return;
        }
    }
    array_push(&ri->types, node);
    node->spec.type.identical = node;
}

//
// Completing types
//

static RiNode*
ri_complete_type_(Ri* ri, RiPos pos, RiNode* type)
{
    ri_error_check_(ri);
    RI_CHECK(type);
    RI_CHECK(ri_is_in(type->kind, RiNode_Spec_Type));

    if (type->spec.type.completeness == RiType_Completing) {
        ri_error_set_(ri, RiError_CyclicType, pos, "cyclic type completion");
        return 0;
    } else if (type->spec.type.completeness == RiType_Completed) {
        return type;
    }
    RI_CHECK(type->spec.type.completeness == RiType_Incomplete);
    type->spec.type.completeness = RiType_Completing;
    switch (type->kind)
    {
        case RiNode_Spec_Type_Struct: {
            RI_TODO;
            // uint64_t offset = 0;
            // RiNodeArray* declarations = &type->type.structure.scope->scope.declarations;
            // for (iptr i = 0; i < declarations->count; ++i) {
            //     RiNode* field = declarations->items[i];

            //     RI_ASSERT(ri_is_value(field->declaration.node));
            //     // NOTE: sizeof will complete the type as well.
            //     iptr align = ri_alignof(ri, field->pos, field->declaration.node->value.type);
            //     offset = iptr_align_forward(offset, align);
            //     field->declaration.offset = offset;
            //     iptr size = ri_sizeof(ri, field->pos, field->declaration.node->value.type);
            //     offset += size;
            // }
            // type->type.structure.size = offset;
        } break;
        default:
            // ri_error_set_(ri, RiError_CompletingType, pos, "could not complete type");
            RI_ABORT("invalid type to complete");
            return 0;
    }
    type->spec.type.completeness = RiType_Completed;

    return type;
}

//
// Getting types
//

static RiNode*
ri_retof_(Ri* ri, RiNode* node)
{
    ri_error_check_(ri);

    if (ri_is_in(node->kind, RiNode_Expr_Binary_Comparison)) {
        // Always bool
        return ri->node_meta[RiNode_Spec_Type_Number_Bool].node;
    } else if (ri_is_in(node->kind, RiNode_Expr_Binary_Numeric)) {
        // Same return value as zero argument.
        return ri_retof_(ri, node->binary.argument0);
    } else {
        // RI_ASSERT(ri_is_rt(node));
        switch (node->kind)
        {
            case RiNode_Value_Var:
                if (node->value.spec->kind == RiNode_Spec_Var) {
                    return node->value.type;
                } else {
                    ri_error_set_(ri, RiError_UnexpectedValue, node->pos, "variable expected");
                    return NULL;
                }
                break;
            case RiNode_Value_Constant:
                RI_CHECK(node->value.type);
                return node->value.type;
            case RiNode_Value_Func:
                RI_CHECK(node->value.type);
                return node->value.type;

            case RiNode_Expr_Cast:
                return array_at(&node->call.arguments, 0);
            case RiNode_Expr_Call: {
                RI_CHECK(node->call.func);
                RI_CHECK(node->call.func->kind == RiNode_Value_Func);
                RiNode* func_spec = node->call.func->value.spec;
                RiNode* func_type = func_spec->spec.func.type;
                RI_CHECK(func_spec->spec.func.type);
                RiNodeArray* outputs = &func_type->spec.type.func.outputs;
                switch (outputs->count)
                {
                    case 1: {
                        RiNode* output = array_at(outputs, 0);
                        RI_CHECK(output->kind == RiNode_Decl);
                        RI_CHECK(output->decl.spec->kind == RiNode_Spec_Var);
                        return ri_get_spec_(ri, output->decl.spec->spec.var.type);
                    }
                    case 0:
                        return ri->node_meta[RiNode_Spec_Type_None].node;

                    default: {
                        // TODO: Multiple return arguments.
                        RI_UNREACHABLE;
                    }
                }
            } break;

            default:
                if (ri_is_in(node->kind, RiNode_Expr_Unary)) {
                    return ri_retof_(ri, node->unary.argument);
                // } else if (ri_is_in(node->kind, RiNode_Expr_Binary)) {
                //     RI_ABORT("todo");
                }
                RI_CHECK(ri_is_in(node->kind, RiNode_Expr));

            // TODO:
            // case RiNode_Field:
            //     RI_CHECK(expr->field.child);
            //     return ri_retof_(ri, expr->field.child);
        }
        RI_ABORT("unknown expr type");
        return NULL;
    }
}

//
// Size and align
//

static uint64_t
ri_sizeof_(Ri* ri, RiPos pos, RiNode* type)
{
    switch (type->kind)
    {
        case RiNode_Spec_Type_Pointer:
        case RiNode_Spec_Type_Number_Int64:
        case RiNode_Spec_Type_Number_UInt64:
        case RiNode_Spec_Type_Number_Float64:
            return 8;
        case RiNode_Spec_Type_Number_Enum:
        case RiNode_Spec_Type_Number_Int32:
        case RiNode_Spec_Type_Number_UInt32:
        case RiNode_Spec_Type_Number_Float32:
            return 4;
        case RiNode_Spec_Type_Number_Int16:
        case RiNode_Spec_Type_Number_UInt16:
            return 2;
        case RiNode_Spec_Type_Number_Bool:
        case RiNode_Spec_Type_Number_Int8:
        case RiNode_Spec_Type_Number_UInt8:
            return 1;
        case RiNode_Spec_Type_Struct:
        case RiNode_Spec_Type_Union:
            type = ri_complete_type_(ri, pos, type);
            return type->spec.type.compound.size;
            break;
    }

    ri_error_set_(ri, RiError_UnexpectedType, pos, "unexpected type");
    return 0;
}

static uint64_t
ri_alignof_(Ri* ri, RiPos pos, RiNode* type)
{
    switch (type->kind)
    {
        case RiNode_Spec_Type_Struct:
        case RiNode_Spec_Type_Union:
        case RiNode_Spec_Type_Pointer:
        case RiNode_Spec_Type_Number_Int64:
        case RiNode_Spec_Type_Number_UInt64:
        case RiNode_Spec_Type_Number_Float64:
            return 8;
        case RiNode_Spec_Type_Number_Enum:
        case RiNode_Spec_Type_Number_Int32:
        case RiNode_Spec_Type_Number_UInt32:
        case RiNode_Spec_Type_Number_Float32:
            return 4;
        case RiNode_Spec_Type_Number_Int16:
        case RiNode_Spec_Type_Number_UInt16:
            return 2;
        case RiNode_Spec_Type_Number_Bool:
        case RiNode_Spec_Type_Number_Int8:
        case RiNode_Spec_Type_Number_UInt8:
            return 1;
    }

    ri_error_set_(ri, RiError_UnexpectedType, pos, "unexpected type");
    return 0;
}

//
//
//

static RiNode*
ri_make_node_(Ri* ri, RiPos pos, RiNodeKind kind)
{
    RiNode* node = arena_push_t(&ri->arena, RiNode);
    memset(node, 0, sizeof(RiNode));
    node->kind = kind;
    node->owner = ri->scope;
    node->index = ++ri->index;
    node->pos = pos;
    return node;
}

static RiNode*
ri_make_scope_(Ri* ri, RiPos pos)
{
    RiNode* node = ri_make_node_(ri, pos, RiNode_Scope);
    return node;
}

static RiNode*
ri_make_identifier_(Ri* ri, RiPos pos, String name)
{
    CHECK(name.items != 0);
    CHECK(name.count > 0);
    // TODO: Checks.
    RiNode* node = ri_make_node_(ri, pos, RiNode_Id);
    node->id.name = name;
    return node;
}

//
// Variable
//

static RiNode*
ri_make_spec_var_(Ri* ri, RiPos pos, String id, RiNode* type, RiVarKind kind)
{
    RiNode* spec = ri_make_node_(ri, pos, RiNode_Spec_Var);
    spec->spec.id = id;
    spec->spec.var.type = type;
    spec->spec.var.kind = kind;
    spec->spec.var.slot = RI_INVALID_SLOT;
    return spec;
}

//
// Constant
//

static RiNode*
ri_make_spec_constant_(Ri* ri, RiPos pos, String id, RiNode* type, RiLiteral value)
{
    RI_CHECK(type);
    RI_CHECK(ri_is_in(type->kind, RiNode_Spec_Type_Number));
    RiNode* node = ri_make_node_(ri, pos, RiNode_Spec_Constant);
    node->spec.id = id;
    node->spec.constant.type = type;
    node->spec.constant.value = value;
    return node;
}

//
// Functions
//

static RiNode*
ri_make_spec_func_(Ri* ri, RiPos pos, String id, RiNode* type, RiNode* scope)
{
    RI_CHECK(type);
    RI_CHECK(type->kind == RiNode_Spec_Type_Func);
    RI_CHECK(scope);
    RI_CHECK(scope->kind == RiNode_Scope);

    RiNode* node = ri_make_node_(ri, pos, RiNode_Spec_Func);
    node->spec.id = id;
    node->spec.func.type = type;
    node->spec.func.scope = scope;
    node->spec.func.slot = RI_INVALID_SLOT;
    return node;
}

//
// Types
//

static RiNode*
ri_make_spec_type_func_(Ri* ri, RiPos pos, String id, RiNodeArray inputs, RiNodeArray outputs) {
    RiNode* node = ri_make_node_(ri, pos, RiNode_Spec_Type_Func);
    node->spec.id = id;
    node->spec.type.func.inputs = inputs;
    node->spec.type.func.outputs = outputs;
    return node;
}

static RiNode*
ri_make_spec_type_number_(Ri* ri, RiPos pos, String id, RiNodeKind kind)
{
    RI_CHECK(ri_is_in(kind, RiNode_Spec_Type_Number));
    RiNode* node = ri_make_node_(ri, pos, kind);
    node->spec.id = id;
    return node;
}

//
//
//

static RiNode*
ri_make_decl_(Ri* ri, RiPos pos, RiNode* spec)
{
    RI_CHECK(spec);
    RI_CHECK(ri_is_in(spec->kind, RiNode_Spec) || spec->kind == RiNode_Id);

    RiNode* node = ri_make_node_(ri, pos, RiNode_Decl);
    node->decl.spec = spec;
    return node;
}

//
// Expressions
//

static RiNode*
ri_make_expr_call_(Ri* ri, RiPos pos, RiNode* func)
{
    RI_CHECK(func);
    // TODO: Check func node type?

    RiNode* node = ri_make_node_(ri, pos, RiNode_Expr_Call);
    node->call.func = func;
    return node;
}

static RiNode*
ri_make_expr_binary_(Ri* ri, RiPos pos, RiNodeKind kind, RiNode* argument0, RiNode* argument1)
{
    RI_CHECK(ri_is_in(kind, RiNode_Expr_Binary));
    RI_CHECK(argument0);
    RI_CHECK(ri_is_expr_like(argument0->kind));
    RI_CHECK(argument1);
    RI_CHECK(ri_is_expr_like(argument1->kind));

    RiNode* node = ri_make_node_(ri, pos, kind);
    node->binary.argument0 = argument0;
    node->binary.argument1 = argument1;
    return node;
}

static RiNode*
ri_make_expr_unary_(Ri* ri, RiPos pos, RiNodeKind kind, RiNode* argument)
{
    RI_CHECK(ri_is_in(kind, RiNode_Expr_Unary));
    RI_CHECK(argument);
    RI_CHECK(ri_is_expr_like(argument->kind));

    RiNode* node = ri_make_node_(ri, pos, kind);
    node->unary.argument = argument;
    return node;
}

RiNode*
ri_make_expr_cast_(Ri* ri, RiPos pos, RiNode* expr, RiNode* type_to)
{
    RI_CHECK(type_to);
    RiNode* type_from = ri_retof_(ri, expr);
    if (type_from != type_to) {
        // TODO: Check if we can cast.
        // TODO: Check if we should cast.
        RiNode* node = ri_make_node_(ri, pos, RiNode_Expr_Cast);
        array_push(&node->call.arguments, type_to);
        array_push(&node->call.arguments, expr);
        return node;
    }
    return expr;
}

//
// Statements
//

static RiNode*
ri_make_st_assign_(Ri *ri, RiPos pos, RiNodeKind kind, RiNode* argument0, RiNode* argument1)
{
    RI_CHECK(ri_is_in(kind, RiNode_St_Assign));
    RI_CHECK(argument0);
    RI_CHECK(
        ri_is_expr_like(argument0->kind) ||
        argument0->kind == RiNode_Spec_Var ||
        (argument0->kind == RiNode_Decl && argument0->decl.spec->kind == RiNode_Spec_Var)
    );
    RI_CHECK(argument1);
    RI_CHECK(ri_is_expr_like(argument1->kind));

    RiNode* node = ri_make_node_(ri, pos, kind);
    node->binary.argument0 = argument0;
    node->binary.argument1 = argument1;
    return node;
}

static RiNode*
ri_make_st_expr_(Ri* ri, RiPos pos, RiNode* expr)
{
    RI_CHECK(ri_is_expr_like(expr->kind));

    RiNode* node = ri_make_node_(ri, pos, RiNode_St_Expr);
    node->st_expr = expr;
    return node;
}

static RiNode*
ri_make_st_return_(Ri* ri, RiPos pos, RiNode* argument)
{
    RI_CHECK(argument == NULL || ri_is_expr_like(argument->kind));
    RiNode* node = ri_make_node_(ri, pos, RiNode_St_Return);
    node->st_return.argument = argument;
    return node;
}

static RiNode*
ri_make_st_if_(Ri* ri, RiPos pos, RiNode* pre, RiNode* condition, RiNode* scope)
{
    RI_CHECK(condition);
    RI_CHECK(scope);
    RI_CHECK(scope->scope.statements.count >= 0 && scope->scope.statements.count <= 2);

    RiNode* node = ri_make_node_(ri, pos, RiNode_St_If);
    node->st_if.pre = pre;
    node->st_if.condition = condition;
    node->st_if.scope = scope;
    return node;
}

static RiNode*
ri_make_st_for_(Ri* ri, RiPos pos, RiNode* pre, RiNode* condition, RiNode* post, RiNode* scope)
{
    RiNode* node = ri_make_node_(ri, pos, RiNode_St_For);
    node->st_for.pre = pre;
    node->st_for.condition = condition;
    node->st_for.post = post;
    node->st_for.scope = scope;
    return node;
}

static RiNode*
ri_make_st_switch_(Ri* ri, RiPos pos, RiNode* pre, RiNode* expr, RiNode* scope)
{
    RiNode* node = ri_make_node_(ri, pos, RiNode_St_Switch);
    node->st_switch.pre = pre;
    node->st_switch.expr = expr;
    node->st_switch.scope = scope;
    return node;
}

static RiNode*
ri_make_st_switch_case_(Ri* ri, RiPos pos, RiNode* expr)
{
    RiNode* node = ri_make_node_(ri, pos, RiNode_St_Switch_Case);
    node->st_switch_case.expr = expr;
    return node;
}

static RiNode*
ri_make_st_switch_default_(Ri* ri, RiPos pos)
{
    RiNode* node = ri_make_node_(ri, pos, RiNode_St_Switch_Default);
    return node;
}

static RiNode*
ri_make_st_break_(Ri* ri, RiPos pos)
{
    RiNode* node = ri_make_node_(ri, pos, RiNode_St_Break);
    return node;
}

static RiNode*
ri_make_st_continue_(Ri* ri, RiPos pos)
{
    RiNode* node = ri_make_node_(ri, pos, RiNode_St_Continue);
    return node;
}

//
//
//

static bool
ri_scope_set_(Ri* ri, RiNode* decl)
{
    RI_CHECK(decl);
    RI_CHECK(decl->decl.spec);
    RI_CHECK(ri_is_in(decl->decl.spec->kind, RiNode_Spec));
    ValueScalar id_ = { .ptr = decl->decl.spec->spec.id.items };
    RiNode* decl_found = map_get(&ri->scope->scope.map, id_).ptr;
    if (decl_found != NULL) {
        ri_error_set_(ri, RiError_Declared, decl->pos, "'%S' is already declared", decl->decl.spec->spec.id);
        return false;
    }
    map_put(&ri->scope->scope.map, id_, (ValueScalar){ .ptr = decl });
    return decl;
}

//
// Expressions parser.
//

static RiNode* ri_parse_expr_(Ri* ri);
static RiNode* ri_parse_expr_operator_(Ri* ri, int precedence);

RiNode*
ri_parse_expr_operand_(Ri* ri)
{
    switch (ri->token.kind)
    {
        case RiToken_Identifier: {
            RiToken token = ri->token;
            if (ri_lex_next_(ri)) {
                return ri_make_identifier_(ri, token.pos, token.id);
            }
            return NULL;
        } break;

        case RiToken_Integer: {
            RiNode* node = ri_make_node_(ri, ri->token.pos, RiNode_Value_Constant);
            node->value.constant.integer = ri->token.integer;
            node->value.type = ri->node_meta[RiNode_Spec_Type_Number_None_Int].node;
            if (ri_lex_next_(ri)) {
                return node;
            }
            return NULL;
        } break;

        case RiToken_Real: {
            RiNode* node = ri_make_node_(ri, ri->token.pos, RiNode_Value_Constant);
            node->value.constant.real = ri->token.real;
            node->value.type = ri->node_meta[RiNode_Spec_Type_Number_None_Real].node;
            if (ri_lex_next_(ri)) {
                return node;
            }
            return NULL;
        } break;

        // TODO: case RiToken_Real:
        // TODO: case RiToken_String:

        case RiToken_Keyword_False:
        case RiToken_Keyword_True: {
            RiNode* node = ri_make_node_(ri, ri->token.pos, RiNode_Value_Constant);
            node->value.constant.boolean = (ri->token.kind == RiToken_Keyword_True);
            node->value.type = ri->node_meta[RiNode_Spec_Type_Number_Bool].node;
            if (ri_lex_next_(ri)) {
                return node;
            }
            return NULL;
        }

        case RiToken_LP: {
            if (ri_lex_next_(ri)) {
                RiNode* node = ri_parse_expr_(ri);
                if (node && ri_lex_expect_token_(ri, RiToken_RP)) {
                    return node;
                }
            }
            return NULL;
        } break;
    }

    ri_error_set_unexpected_token_(ri, &ri->token);
    return NULL;
}

RiNode*
ri_parse_expr_dot_(Ri* ri)
{
    RiNode* L = ri_parse_expr_operand_(ri);
    while (L && ri->token.kind == RiToken_Dot) {
        RiToken token = ri->token;
        if (ri_lex_next_(ri)) {
            RiNode* R = ri_parse_expr_operand_(ri);
            if (R) {
                L = ri_make_expr_binary_(ri, token.pos, RiNode_Expr_Binary_Select, L, R);
                continue;
            }
        }
        return NULL;
    }
    return L;
}

static bool
ri_parse_expr_call_arguments_(Ri* ri, RiNode* call)
{
    while (ri_lex_next_if_(ri, RiToken_RP) == RiLexNextIf_NoMatch) {
        RiNode* expr = ri_parse_expr_(ri);
        if (!expr) {
            return false;
        }
        array_push(&call->call.arguments, expr);
        if (ri_lex_next_if_(ri, RiToken_Comma) == RiLexNextIf_Error) {
            return false;
        }
    }
    if (ri->error.kind) {
        return false;
    }
    return true;
}

RiNode*
ri_parse_expr_call_(Ri* ri)
{
    RiNode* L = ri_parse_expr_dot_(ri);
    while (ri->token.kind == RiToken_LP) {
        if (ri_lex_next_(ri)) {
            L = ri_make_expr_call_(ri, L->pos, L);
            if (ri_parse_expr_call_arguments_(ri, L)) {
                continue;
            }
        }
        return NULL;
    }
    return L;
}

static RiNode*
ri_parse_expr_operator_unary_(Ri* ri)
{
    switch (ri->token.kind) {
        case RiToken_Bang:
        case RiToken_Tilde:
        case RiToken_Plus:
        case RiToken_Minus:
        case RiToken_Amp: {
            RiToken token = ri->token;
            if (ri_lex_next_(ri)) {
                RiNode* R = ri_parse_expr_operator_unary_(ri);
                if (R) {
                    return ri_make_expr_unary_(ri, token.pos, RI_TOKEN_TO_OP_[token.kind].unary, R);
                }
            }
        } return NULL;
    }
    return ri_parse_expr_call_(ri);
}

static RiNode*
ri_parse_expr_operator_binary_(Ri* ri, RiNodeKind op, RiNode* L, int super_precedence)
{
    RI_CHECK(ri_is_in(op, RiNode_Expr_Binary));
    RiToken token = ri->token;
    if (ri_lex_next_(ri)) {
        RiNode* R = ri_parse_expr_operator_(ri, super_precedence);
        if (R) {
            return ri_make_expr_binary_(ri, token.pos, op, L, R);
        }
    }
    return NULL;
}

static RiNode*
ri_parse_expr_operator_(Ri* ri, int precedence)
{
    RI_CHECK(precedence >= 0);
    if (precedence == 0) {
        return ri_parse_expr_operator_unary_(ri);
    }

    RiNode* L = ri_parse_expr_operator_(ri, precedence - 100);
    RiTokenKind token_kind;
    while (L)
    {
        token_kind = ri->token.kind;
        switch (precedence)
        {
            case 0:
                RI_ABORT("not reached");
                return L;

            // * / %
            case 500:
                switch (token_kind) {
                    case RiToken_Star: case RiToken_Slash: case RiToken_Percent:
                        L = ri_parse_expr_operator_binary_(
                            ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_ABORT("not reached");

            // + -
            case 600:
                switch (token_kind) {
                    case RiToken_Minus: case RiToken_Plus:
                        L = ri_parse_expr_operator_binary_(
                            ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_ABORT("not reached");

            // << >>
            case 700:
                switch (token_kind) {
                    case RiToken_LtLt: case RiToken_GtGt:
                        L = ri_parse_expr_operator_binary_(
                            ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_ABORT("not reached");

            // > >= < <=
            case 800:
                switch (token_kind) {
                    case RiToken_Gt: case RiToken_GtEq:
                    case RiToken_Lt: case RiToken_LtEq:
                        L = ri_parse_expr_operator_binary_(
                            ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_ABORT("not reached");

            // == !=
            case 900:
                switch (token_kind) {
                    case RiToken_EqEq: case RiToken_BangEq:
                        L = ri_parse_expr_operator_binary_(
                            ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_ABORT("not reached");

            // &
            case 1000: if (token_kind == RiToken_Amp) {
                L = ri_parse_expr_operator_binary_(
                    ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // ^
            case 1100: if (token_kind == RiToken_Beak) {
                L = ri_parse_expr_operator_binary_(
                    ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // |
            case 1200: if (token_kind == RiToken_Pipe) {
                L = ri_parse_expr_operator_binary_(
                    ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // &&
            case 1300: if (token_kind == RiToken_AmpAmp) {
                L = ri_parse_expr_operator_binary_(
                    ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // ||
            case 1400: if (token_kind == RiToken_PipePipe) {
                L = ri_parse_expr_operator_binary_(
                    ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // =
            // case 1600: if (token_kind == RiToken_Eq) {
            //     L = ri_parse_expr_operator_binary_(
            //         ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence);
            // } return L;

            // :
            // case 1800: if (token_kind == RiToken_Colon) {
            //     L = ri_parse_expr_operator_binary_(
            //         ri, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence);
            // } return L;

            default:
                return L;
        }
    }
    return L;
}

static RiNode*
ri_parse_expr_(Ri* ri)
{
    return ri_parse_expr_operator_(ri, 2000);
}

//
// Declarations parser.
//

static RiNode* ri_parse_decl_(Ri* ri);
static RiNode* ri_parse_scope_(Ri* ri, RiTokenKind end, RiNodeKind scope_kind);
static RiNode* ri_parse_spec_partial_func_type_(Ri* ri, RiPos pos);

static RiNode*
ri_parse_type_(Ri* ri)
{
    // TODO: Parse Id, or parse type spec (func..., struct...)
    ri_error_check_(ri);

    RiToken token = ri->token;
    switch (ri->token.kind)
    {
        case RiToken_Keyword_Func:
            if (!ri_lex_next_(ri)) {
                return NULL;
            }
            return ri_parse_spec_partial_func_type_(ri, token.pos);
        case RiToken_Identifier:
            if (!ri_lex_next_(ri)) {
                return NULL;
            }
            return ri_make_identifier_(ri, token.pos, token.id);
        default:
            ri_error_set_unexpected_token_(ri, &token);
            return NULL;
    }
}

static RiNode*
ri_parse_decl_variable_(Ri* ri)
{
    ri_error_check_(ri);
    RI_CHECK(ri->token.kind == RiToken_Keyword_Variable);
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    RiToken token = ri->token;
    if (!ri_lex_expect_token_(ri, RiToken_Identifier)) {
        return NULL;
    }

    RiNode* type;
    // `=` or `type-spec`
    if (ri->token.kind == RiToken_Eq) {
        // if (!ri_lex_next_(ri)) {
        //     return NULL;
        // }
        type = ri->node_meta[RiNode_Spec_Type_Infer].node;
    } else {
        type = ri_parse_type_(ri);
        if (!type) {
            return NULL;
        }
    }

    RiNode* spec = ri_make_spec_var_(ri, token.pos, token.id, type, RiVar_Local);
    RiNode* decl = ri_make_decl_(ri, token.pos, spec);
    return decl;
}

static RiNode*
ri_parse_decl_type_func_input_arg_(Ri* ri)
{
    // <id> <type-spec>
    // type-spec: <id> | _ | function(...)(...) | struct {...} ...

    // TODO: Implement short syntax for arguments of same type `function (a, b int32)`.
    // TODO: Implement `...int32` for variable arity.

    RiToken token = ri->token;
    if (!ri_lex_expect_token_(ri, RiToken_Identifier)) {
        return NULL;
    }

    RiNode* type = ri_parse_type_(ri);
    if (!type) {
        return NULL;
    }

    RiNode* spec = ri_make_spec_var_(ri, token.pos, token.id, type, RiVar_Input);
    RiNode* decl = ri_make_decl_(ri, token.pos, spec);
    return decl;
}

static bool
ri_parse_decl_type_func_input_arg_list_(Ri* ri, RiNodeArray* list)
{
    if (!ri_lex_expect_token_(ri, RiToken_LP)) {
        return false;
    }

    RiNode* decl_arg;
    while (ri->token.kind != RiToken_RP)
    {
        decl_arg = ri_parse_decl_type_func_input_arg_(ri);
        if (!decl_arg) {
            return false;
        }
        array_push(list, decl_arg);
        if (ri_lex_next_if_(ri, RiToken_Comma) == RiLexNextIf_Error) {
            return false;
        }
    }
    if (!ri_lex_next_(ri)) {
        return false;
    }

    return true;
}

static bool
ri_parse_decl_type_func_output_arg_(Ri* ri, RiNodeArray* outputs)
{
    if (ri->token.kind == RiToken_Semicolon) {
        return true;
    }

    RiNode* type = ri_parse_type_(ri);
    if (type) {
        RiNode* spec = ri_make_spec_var_(ri, type->pos, (String){0}, type, RiVar_Output);
        RiNode* decl = ri_make_decl_(ri, type->pos, spec);
        array_push(outputs, decl);
        return true;
    }

    return false;
}

static RiNode*
ri_parse_spec_partial_func_type_(Ri* ri, RiPos pos)
{
    ri_error_check_(ri);

    RiNodeArray inputs = {0};
    RiNodeArray outputs = {0};
    if (!ri_parse_decl_type_func_input_arg_list_(ri, &inputs)) {
        return NULL;
    }

    if (!ri_parse_decl_type_func_output_arg_(ri, &outputs)) {
        return NULL;
    }

    RiNode* type = ri_make_spec_type_func_(ri, pos, RI_ID_NULL, inputs, outputs);
    return type;
}

static RiNode*
ri_parse_spec_partial_func_(Ri* ri, RiPos pos, String id)
{
    ri_error_check_(ri);
    RI_CHECK(id.items);
    RI_CHECK(id.count);

    // NOTE: Starting and setting scope here, so all the nodes
    // created within have owner set to the scope.
    RiNode* scope = ri_make_scope_(ri, pos);
    ri->scope = scope;
    // RI_LOG_DEBUG("function '%S' scope %d", id, scope->index);

    RiNode* type = ri_parse_spec_partial_func_type_(ri, pos);
    if (!type) {
        return NULL;
    }

    RiNode* scope_body = NULL;
    switch (ri_lex_next_if_(ri, RiToken_LB))
    {
        case RiLexNextIf_Match: {
            scope_body = ri_parse_scope_(ri, RiToken_RB, RiNode_Spec_Func);
            if (!scope_body) {
                return NULL;
            }

            // TODO: Make parsing arguments actually generate variable decls, so
            // this won't be needed.

            RiNode* it;
            array_each(&type->spec.type.func.inputs, &it) {
                map_put(&scope->scope.map,
                    (ValueScalar){ .ptr = it->decl.spec->spec.id.items },
                    (ValueScalar){ .ptr = it }
                );
            }
            // TODO: Multiple return values.
            // TODO: Named return variables.
            // array_each(&type->spec.type.func.outputs, &it) {
            //     map_put(&scope->scope.map,
            //         (ValueScalar){ .ptr = it->decl.spec->spec.id.items },
            //         (ValueScalar){ .ptr = it }
            //     );
            // }
            array_push(&scope->scope.statements, scope_body);
        } break;
        case RiLexNextIf_Error:
            return NULL;
    }

    RI_CHECK(ri->scope == scope);
    ri->scope = scope->owner;

    RiNode* spec = ri_make_spec_func_(ri, pos, id, type, scope_body ? scope : NULL);
    RiNode* decl = ri_make_decl_(ri, pos, spec);
    return decl;
}

static RiNode*
ri_parse_spec_func_or_func_type_(Ri* ri)
{
    ri_error_check_(ri);
    RI_CHECK(ri->token.kind == RiToken_Keyword_Func);
    RiPos pos = ri->token.pos;
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    RiNode* node = NULL;
    String id = ri->token.id;
    switch (ri_lex_next_if_(ri, RiToken_Identifier))
    {
        case RiLexNextIf_Match:
            node = ri_parse_spec_partial_func_(ri, pos, id);
            break;
        case RiLexNextIf_NoMatch:
            node = ri_parse_spec_partial_func_type_(ri, pos);
            break;
        case RiLexNextIf_Error:
            node = NULL;
            break;
    }

    return node;
}

static RiNode*
ri_parse_decl_type_(Ri* ri)
{
    ri_error_check_(ri);
    RI_CHECK(ri->token.kind == RiToken_Keyword_Type);
    RiPos pos = ri->token.pos;
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    String id = ri->token.id;
    if (!ri_lex_expect_token_(ri, RiToken_Identifier)) {
        return NULL;
    }

    RiNode* type = ri_parse_type_(ri);
    if (!type) {
        return NULL;
    }
    RI_CHECK(type->spec.id.items == 0);
    type->spec.id = id;

    RiNode* decl = ri_make_decl_(ri, pos, type);
    return decl;
}

static RiNode*
ri_parse_decl_(Ri* ri)
{
    ri_error_check_(ri);

    RiNode* node = NULL;
    switch (ri->token.kind)
    {
        // TODO: Type is not expected here, only function declaration.
        case RiToken_Keyword_Func: node = ri_parse_spec_func_or_func_type_(ri); break;
        case RiToken_Keyword_Variable: node = ri_parse_decl_variable_(ri); break;
        case RiToken_Keyword_Type: node = ri_parse_decl_type_(ri); break;
        default:
            ri_error_set_unexpected_token_(ri, &ri->token);
            break;
    }

    if (node) {
        if (!ri_scope_set_(ri, node)) {
            return NULL;
        }
    }
    return node;
}

//
// Statements parser.
//

static RiNode* ri_parse_st_(Ri* ri, RiNodeKind scope_kind);
static RiNode* ri_parse_st_simple_(Ri* ri);

static RiNode*
ri_parse_st_expr_(Ri* ri)
{
    ri_error_check_(ri);

    RiNode* expr = ri_parse_expr_(ri);
    if (!expr) {
        return NULL;
    }

    RiTokenKind token_kind = ri->token.kind;
    if (ri_tokenkind_in(token_kind, RiToken_Assign)) {
        if (!ri_lex_next_(ri)) {
            return NULL;
        }
        RiNode* right = ri_parse_expr_(ri);
        if (!right) {
            return NULL;
        }
        return ri_make_st_assign_(ri, expr->pos, RI_TOKEN_TO_OP_[token_kind].assign, expr, right);
    } else {
        return ri_make_st_expr_(ri, expr->pos, expr);
    }
}

static RiNode*
ri_parse_st_return_(Ri* ri)
{
    ri_error_check_(ri);

    RiPos pos = ri->token.pos;
    RI_CHECK(ri->token.kind == RiToken_Keyword_Return);
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    RiNode* argument = NULL;
    if (ri->token.kind != RiToken_Semicolon) {
        argument = ri_parse_expr_(ri);
        if (!argument) {
            return NULL;
        }
    }

    if (!ri_lex_expect_token_(ri, RiToken_Semicolon)) {
        return NULL;
    }

    RiNode* statement = ri_make_st_return_(ri, pos, argument);
    return statement;
}

static RiNode*
ri_parse_st_if_(Ri* ri)
{
    ri_error_check_(ri);

    RiPos pos = ri->token.pos;
    RI_CHECK(ri->token.kind == RiToken_Keyword_If);
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    RiNode* scope = ri_make_scope_(ri, pos);
    ri->scope = scope;

    RiNode* condition = NULL;
    RiNode* pre = ri_parse_st_simple_(ri);
    if (!pre) {
        return NULL;
    }

    switch (ri_lex_next_if_(ri, RiToken_Semicolon))
    {
        case RiLexNextIf_Match: {
            condition = ri_parse_expr_(ri);
            if (!condition) {
                return NULL;
            }
        } break;

        case RiLexNextIf_NoMatch: {
            condition = pre;
            if (condition->kind != RiNode_St_Expr) {
                ri_error_set_(ri, RiError_UnexpectedStatement, condition->pos, "conditional expression expected");
                return NULL;
            }
            condition = condition->st_expr;
            RI_CHECK(ri_is_expr_like(condition->kind));
            pre = NULL;
        } break;

        case RiLexNextIf_Error:
            return NULL;
    }


    if (!ri_lex_expect_token_(ri, RiToken_LB)) {
        return NULL;
    }

    RiNode* scope_else = NULL;
    RiNode* scope_then = ri_parse_scope_(ri, RiToken_RB, RiNode_St_If);
    if (!scope_then) {
        return NULL;
    }
    array_push(&scope->scope.statements, scope_then);

    switch (ri_lex_next_if_(ri, RiToken_Keyword_Else))
    {
        case RiLexNextIf_Match:
            if (ri->token.kind == RiToken_Keyword_If) {
                RiNode* else_if = ri_parse_st_if_(ri);
                if (!else_if) {
                    return NULL;
                }
                array_push(&scope->scope.statements, else_if);
            } else {
                if (!ri_lex_expect_token_(ri, RiToken_LB)) {
                    return NULL;
                }
                scope_else = ri_parse_scope_(ri, RiToken_RB, RiNode_St_If);
                if (!scope_else) {
                    return NULL;
                }
                array_push(&scope->scope.statements, scope_else);
            }
            break;
        case RiLexNextIf_Error:
            return NULL;
    }

    RI_CHECK(ri->scope == scope);
    ri->scope = scope->owner;

    RiNode* node = ri_make_st_if_(ri, pos, pre, condition, scope);
    return node;
}

static RiNode*
ri_parse_st_for_(Ri* ri)
{
    ri_error_check_(ri);
    RiPos pos = ri->token.pos;
    RI_CHECK(ri->token.kind == RiToken_Keyword_For);
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    RiNode* scope = ri_make_scope_(ri, pos);
    ri->scope = scope;

    RiNode* pre = NULL;
    RiNode* condition = NULL;
    RiNode* post = NULL;
    RiNode* scope_block = NULL;

    if (ri_lex_next_if_(ri, RiToken_LB) == RiLexNextIf_NoMatch) {
        // for pre?; condition?; post? { ... }
        if (ri_lex_next_if_(ri, RiToken_Semicolon) == RiLexNextIf_NoMatch) {
            pre = ri_parse_st_simple_(ri);
            if (!pre) {
                return NULL;
            }
            if (ri_lex_next_if_(ri, RiToken_Semicolon) == RiLexNextIf_NoMatch) {
                // for condition { ... }
                condition = pre;
                if (condition->kind != RiNode_St_Expr) {
                    ri_error_set_(ri, RiError_UnexpectedStatement, condition->pos, "conditional expression expected");
                    return NULL;
                }
                condition = condition->st_expr;
                RI_CHECK(ri_is_expr_like(condition->kind));
                pre = NULL;
                goto skip;
            }
        }

        if (ri->error.kind) {
            return NULL;
        }

        // for ; condition?; post? { ... }
        if (ri_lex_next_if_(ri, RiToken_Semicolon) == RiLexNextIf_NoMatch) {
            // for ; condition; post?
            condition = ri_parse_expr_(ri);
            if (!condition) {
                return NULL;
            }
            if (!ri_lex_expect_token_(ri, RiToken_Semicolon)) {
                return NULL;
            }
        } else {
            // for ; ; post?
        }

        if (ri->error.kind) {
            return NULL;
        }

        if (ri->token.kind != RiToken_LB) {
            // for ; condition?; post { ... }
            post = ri_parse_st_simple_(ri);
            if (!post) {
                return NULL;
            }
        } else {
            // for ; condition?; { ... }
        }

skip:;
        if (!ri_lex_expect_token_(ri, RiToken_LB)) {
            return NULL;
        }
    }


    scope_block = ri_parse_scope_(ri, RiToken_RB, RiNode_St_For);
    if (!scope_block) {
        return NULL;
    }
    array_push(&scope->scope.statements, scope_block);

    RI_CHECK(ri->scope == scope);
    ri->scope = scope->owner;

    RiNode* node = ri_make_st_for_(ri, pos, pre, condition, post, scope);
    return node;
}

static RiNode*
ri_parse_st_switch_(Ri* ri)
{
    ri_error_check_(ri);

    RiPos pos = ri->token.pos;
    RI_CHECK(ri->token.kind == RiToken_Keyword_Switch);
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    RiNode* scope = ri_make_scope_(ri, pos);
    ri->scope = scope;

    RiNode* pre = NULL;
    RiNode* condition = NULL;

    if (ri_lex_next_if_(ri, RiToken_Semicolon) == RiLexNextIf_NoMatch)
    {
        // switch pre; condition
        // switch condition
        pre = ri_parse_st_simple_(ri);
        if (!pre) {
            return NULL;
        }

        if (ri_lex_next_if_(ri, RiToken_Semicolon) == RiLexNextIf_NoMatch)
        {
            // switch condition { ... }
            condition = pre;
            if (condition->kind != RiNode_St_Expr) {
                ri_error_set_(ri, RiError_UnexpectedStatement, condition->pos, "conditional expression expected");
                return NULL;
            }
            condition = condition->st_expr;
            RI_CHECK(ri_is_expr_like(condition->kind));
            pre = NULL;
            goto skip;
        }
    }

    if (ri->error.kind) {
        return NULL;
    }

    condition = ri_parse_expr_(ri);
    if (!condition) {
        return NULL;
    }

skip:;

    if (!ri_lex_expect_token_(ri, RiToken_LB)) {
        return NULL;
    }

    RiNode* scope_block = ri_parse_scope_(ri, RiToken_RB, RiNode_St_Switch);
    if (!scope_block) {
        return NULL;
    }

    array_push(&scope->scope.statements, scope_block);

    RI_CHECK(ri->scope == scope);
    ri->scope = scope->owner;

    RiNode* node = ri_make_st_switch_(ri, pos, pre, condition, scope);
    return node;
}

static RiNode*
ri_parse_st_switch_case_(Ri* ri)
{
    ri_error_check_(ri);

    RiPos pos = ri->token.pos;
    RI_CHECK(ri->token.kind == RiToken_Keyword_Case);
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    RiNode* expr = ri_parse_expr_(ri);
    if (!ri_lex_expect_token_(ri, RiToken_Colon)) {
        return NULL;
    }

    RiNode* node = ri_make_st_switch_case_(ri, pos, expr);
    return node;
}

static RiNode*
ri_parse_st_switch_default_(Ri* ri)
{
    ri_error_check_(ri);

    RiPos pos = ri->token.pos;
    RI_CHECK(ri->token.kind == RiToken_Keyword_Default);
    if (!ri_lex_next_(ri)) {
        return NULL;
    }
    if (!ri_lex_expect_token_(ri, RiToken_Colon)) {
        return NULL;
    }

    RiNode* node = ri_make_st_switch_default_(ri, pos);
    return node;
}

static RiNode*
ri_parse_st_simple_(Ri* ri)
{
    ri_error_check_(ri);

    RiPos pos = ri->token.pos;
    RiNode* node = NULL;
    switch (ri->token.kind)
    {
        case RiToken_Keyword_Variable:
            node = ri_parse_decl_(ri);
            if (!node) {
                return NULL;
            }

            // Assignment.
            switch (ri_lex_next_if_(ri, RiToken_Eq))
            {
                case RiLexNextIf_Match: {
                    RiNode* right = ri_parse_expr_(ri);
                    if (right) {
                        node = ri_make_st_assign_(ri, pos, RiNode_St_Assign, node, right);
                    }
                } break;
                // case RiLexNextIf_NoMatch: {
                //     if (node->decl.spec->spec.var.type->kind == RiNode_Spec_Type_Infer) {
                //         ri_error_set_unexpected_token_(ri, &ri->token);
                //         return NULL;
                //     }
                // }
                case RiLexNextIf_Error:
                    // Assumes node == NULL.
                    break;
            }
            break;

        default:
            node = ri_parse_st_expr_(ri);
            break;
    }

    return node;
}

static RiNode*
ri_parse_st_(Ri* ri, RiNodeKind scope_kind)
{
    ri_error_check_(ri);

    RiNode* node = NULL;
    switch (ri->token.kind)
    {
        case RiToken_Keyword_Type:
            node = ri_parse_decl_(ri);
            if (!node) {
                return NULL;
            }
            if (ri_lex_next_if_(ri, RiToken_Semicolon) == RiLexNextIf_Error) {
                return NULL;
            }
            break;

        case RiToken_Keyword_Func:
            node = ri_parse_decl_(ri);
            if (!node) {
                return NULL;
            }
            RI_CHECK(node->kind == RiNode_Decl);
            if (node->decl.spec->kind == RiNode_Spec_Func) {
                if (ri_lex_next_if_(ri, RiToken_Semicolon) == RiLexNextIf_Error) {
                    return NULL;
                }
            } else if (!ri_lex_expect_token_(ri, RiToken_Semicolon)) {
                return NULL;
            }
            break;

        case RiToken_Keyword_Return:
            node = ri_parse_st_return_(ri);
            break;

        case RiToken_Keyword_If:
            node = ri_parse_st_if_(ri);
            break;

        case RiToken_Keyword_For:
            node = ri_parse_st_for_(ri);
            break;

        case RiToken_Keyword_Switch:
            node = ri_parse_st_switch_(ri);
            break;

        case RiToken_Keyword_Case:
            if (scope_kind != RiNode_St_Switch) {
                ri_error_set_unexpected_token_(ri, &ri->token);
                return NULL;
            }
            node = ri_parse_st_switch_case_(ri);
            break;

        case RiToken_Keyword_Default:
            if (scope_kind != RiNode_St_Switch) {
                ri_error_set_unexpected_token_(ri, &ri->token);
                return NULL;
            }
            node = ri_parse_st_switch_default_(ri);
            break;

        case RiToken_Keyword_Break:
            if (scope_kind != RiNode_St_Switch && scope_kind != RiNode_St_For) {
                ri_error_set_unexpected_token_(ri, &ri->token);
                return NULL;
            }
            RiPos pos = ri->token.pos;
            if (ri_lex_next_(ri)) {
                node = ri_make_st_break_(ri, pos);
                if (ri_lex_expect_token_(ri, RiToken_Semicolon)) {
                    break;
                }
            }
            return NULL;

        // TODO: Merge with `break`?
        // TODO: Continue/break is allowed if one of the parents is switch or for.
        // - We can do this with counted allow_break++, allow_continue++.
        // - We will also need to assign break and continue with the correct parent.
        case RiToken_Keyword_Continue: {
            if (scope_kind != RiNode_St_For) {
                ri_error_set_unexpected_token_(ri, &ri->token);
                return NULL;
            }
            RiPos pos = ri->token.pos;
            if (ri_lex_next_(ri)) {
                node = ri_make_st_continue_(ri, pos);
                if (ri_lex_expect_token_(ri, RiToken_Semicolon)) {
                    break;
                }
            }
        } return NULL;

        default:
            node = ri_parse_st_simple_(ri);
            if (!node || !ri_lex_expect_token_(ri, RiToken_Semicolon)) {
                return NULL;
            }
            break;
    }

    // RI_CHECK(node);
    return node;
}

static RiNode*
ri_parse_scope_(Ri* ri, RiTokenKind end, RiNodeKind scope_kind)
{
    RiNode* scope = ri_make_scope_(ri, ri->token.pos);
    ri->scope = scope;
    while (ri->token.kind != end) {
        RiNode* statement = ri_parse_st_(ri, scope_kind);
        if (statement == NULL) {
            return NULL;
        }
        array_push(&ri->scope->scope.statements, statement);
    }

    if (end == RiToken_RB && !ri_lex_next_(ri)) {
        return NULL;
    }
    RI_ASSERT(ri->scope == scope);
    ri->scope = scope->owner;

    return scope;
}

RiNode*
ri_parse(Ri* ri, String stream, String path)
{
    ri_error_check_(ri);

    array_clear(&ri->path);
    chararray_push(&ri->path, path);

    if (!ri_stream_set_(ri, stream)) {
        return NULL;
    }

    // TODO: Block scope kind.
    RiNode* block = ri_parse_scope_(ri, RiToken_End, RiNode_Unknown);

    return block;
}

//
//
//

// NOTE: `expected_type` is used to cast constants if possible.
#define RI_RESOLVE_F_(Name) bool Name(Ri* ri, RiNode** node)
typedef RI_RESOLVE_F_(RiResolveF_);

static RI_RESOLVE_F_(ri_resolve_identifier_);
static RI_RESOLVE_F_(ri_resolve_node_);

static bool
ri_resolve_slice_with_(Ri* ri, RiNodeSlice nodes, RiResolveF_* f)
{
    RiNode* it;
    slice_eachi(&nodes, i, &it) {
        if (!f(ri, &it)) {
            return false;
        }
        slice_at(&nodes, i) = it;
    }
    return true;
}

static RI_RESOLVE_F_(ri_resolve_unary_)
{
    RiNode* n = *node;

    if (!ri_resolve_node_(ri, &n->unary.argument)) {
        return false;
    }

#if 0
    // RiNode* ret_type = 0;
    RiNode* arg_type = ri_retof_(ri, n->unary.argument);

    switch (n->kind)
    {
    case RiNode_Expr_Unary_Positive:
    case RiNode_Expr_Unary_Negative:
        // TODO: Warn if we're using `-` with unsigned types, the type doesn't change.
        // warning C4146: unary minus operator applied to unsigned type, result still unsigned

        if (ri_is_in(arg_type->kind, RiNode_Spec_Type_Number_Int)) {
            // Allow
        } else if (ri_is_in(arg_type->kind, RiNode_Spec_Type_Number_Float)) {
            // Allow
        } else {
            ri_error_set_(ri, RiError_Type, n->pos, "unary %s is not defined for %S",
                RI_OP_NAMES_[n->kind],
                ri->node_meta[arg_type->kind].node->spec.id
            );
            return false;
        }
        break;
    case RiNode_Expr_Unary_BNeg:
        if (ri_is_in(arg_type->kind, RiNode_Spec_Type_Number_Int)) {
            // Allow
        } else {
            ri_error_set_(ri, RiError_Type, n->pos, "%s is not defined for %s",
                RI_OP_NAMES_[n->kind],
                ri->node_meta[arg_type->kind].node->spec.id
            );
            return false;
        }
        break;
    case RiNode_Expr_Unary_Not:
        if (arg_type->kind == RiNode_Spec_Type_Number_Bool) {
            // Allow
        } else {
            ri_error_set_(ri, RiError_Type, n->pos, "%s is not defined for %s",
                RI_OP_NAMES_[n->kind],
                ri->node_meta[arg_type->kind].node->spec.id
            );
            return false;
        }
        break;
    default:
        RI_TODO;
    }
#endif

    return true;
}

static RI_RESOLVE_F_(ri_resolve_binary_)
{
    RiNode* n = *node;

    return (
        ri_resolve_node_(ri, &n->binary.argument0) &&
        ri_resolve_node_(ri, &n->binary.argument1)
    );
}

static RI_RESOLVE_F_(ri_resolve_assign_)
{
    RiNode* n = *node;

    return (
        ri_resolve_node_(ri, &n->binary.argument0) &&
        ri_resolve_node_(ri, &n->binary.argument1)
    );
}

static RI_RESOLVE_F_(ri_resolve_expr_call_func_)
{
    RiNode* n = *node;
    RiNode* func = n->call.func;
    // TODO: Check number of arguments.
    // TOOD: Check if the argument types match.
    if (!ri_resolve_slice_with_(ri, n->call.arguments.slice, &ri_resolve_node_)) {
        return false;
    }
    return true;
}

// TODO: Move type checking to typecheck phase because here
// we cannot continue processing after error.
static RI_RESOLVE_F_(ri_resolve_expr_call_type_)
{
    RiNode* n = *node;
    RI_CHECK(n->kind == RiNode_Expr_Call);
    RiNode* type_to = ri_get_spec_(ri, n->call.func);
    RI_CHECK(ri_is_in(type_to->kind, RiNode_Spec_Type));

    if (n->call.arguments.count != 1) {
        ri_error_set_(ri, RiError_Argument, n->pos, "1 argument expected");
        return false;
    }

    // Turn from type(expr) to cast(type_to, expr).
    n->kind = RiNode_Expr_Cast;
    array_insert(&n->call.arguments, 0, type_to);
    if (!ri_resolve_slice_with_(ri, n->call.arguments.slice, &ri_resolve_node_)) {
        return false;
    }

    RiNode* expr = array_at(&n->call.arguments, 1);
    RiNode* type_expr = ri_retof_(ri, expr);
    if (!type_expr) {
        return false;
    }

    RiNodeKind type_from_kind = type_expr->kind;
    RiNodeKind type_to_kind = type_to->kind;

    bool from_bool = type_from_kind == RiNode_Spec_Type_Number_Bool;
    bool from_int = ri_is_in(type_from_kind, RiNode_Spec_Type_Number_Int);
    bool from_float = ri_is_in(type_from_kind, RiNode_Spec_Type_Number_Float);

    bool to_bool = type_to_kind == RiNode_Spec_Type_Number_Bool;
    bool to_int = ri_is_in(type_to_kind, RiNode_Spec_Type_Number_Int);
    bool to_float = ri_is_in(type_to_kind, RiNode_Spec_Type_Number_Float);

    // TODO: Allow casts for bool to pointers too?

    if (type_from_kind == type_to_kind) {
        // TODO: Warn of unnecessary cast.
    } else if (from_int && to_int) {
        // Allow from any int to any int.
    } else if (from_int && to_float) {
        // Allow from any int to any float.
    } else if (from_float && to_float) {
        // Allow from any float to any float.
    } else if (from_float && to_int) {
        // Allow cast from any float to any int.
    } else if (from_bool && to_int) {
        // Allow bool to any int.
    } else if (from_bool && to_float) {
        // Allow bool to any float.
    } else {
        ri_error_set_(ri, RiError_Type, n->pos, "cannot cast from %S to %S",
            ri->node_meta[type_from_kind].node->spec.id,
            ri->node_meta[type_to_kind].node->spec.id
        );
        return false;
    }

    return true;
}

static RI_RESOLVE_F_(ri_resolve_st_if_)
{
    RiNode* n = *node;

    if (n->st_if.pre && !ri_resolve_node_(ri, &n->st_if.pre)) {
        return false;
    }

    // No expected type, as we expect it'll be bool right away.
    // So `if 1 { ... }` won't pass and result in error.
    if (!ri_resolve_node_(ri, &n->st_if.condition)) {
        return false;
    }
    RiNode* condition_type = ri_retof_(ri, n->st_if.condition);
    if (condition_type == NULL) {
        return false;
    }
    if (condition_type->kind != RiNode_Spec_Type_Number_Bool) {
        ri_error_set_(ri, RiError_Type, n->st_if.condition->pos, "'bool' expected");
        return false;
    }

    if (!ri_resolve_node_(ri, &n->st_if.scope)) {
        return false;
    }

    return true;
}

static RI_RESOLVE_F_(ri_resolve_st_for_)
{
    RiNode* n = *node;

    if (n->st_for.pre && !ri_resolve_node_(ri, &n->st_for.pre)) {
        return false;
    }
    if (n->st_for.condition) {
        if (!ri_resolve_node_(ri, &n->st_for.condition)) {
            return false;
        }
        RiNode* condition_type = ri_retof_(ri, n->st_for.condition);
        if (condition_type == NULL) {
            return false;
        }
        if (condition_type->kind != RiNode_Spec_Type_Number_Bool) {
            ri_error_set_(ri, RiError_Type, n->st_for.condition->pos, "'bool' expected");
            return false;
        }
    }
    if (n->st_for.post && !ri_resolve_node_(ri, &n->st_for.post)) {
        return false;
    }
    if (!ri_resolve_node_(ri, &n->st_for.scope)) {
        return false;
    }

    return true;
}

static RI_RESOLVE_F_(ri_resolve_identifier_)
{
    RiNode* id = *node;

    RI_CHECK(id->kind == RiNode_Id);
    RI_CHECK(id->id.name.items != NULL);

    RiNode* decl = NULL;
    RiNode* scope = id->owner;
    while (scope) {
        decl = map_get(&scope->scope.map, (ValueScalar){ .ptr = id->id.name.items }).ptr;
        if (decl) {
            break;
        }
        scope = scope->owner;
    }

    if (decl == NULL) {
        ri_error_set_(ri, RiError_NotDeclared, id->pos, "%S was not declared", id->id.name);
        return false;
    }

    RI_CHECK(decl->owner == scope);

    RI_CHECK(decl->kind == RiNode_Decl);

    if (decl->decl.state == RiDecl_Resolving)
    {
        ri_error_set_(ri, RiError_CyclicDeclaration, decl->pos, "cyclic declaration");
        return false;
    }
    else if (decl->decl.state != RiDecl_Resolved)
    {
        if (!ri_resolve_node_(ri, &decl)) {
            return false;
        }
    } else {
        RI_CHECK(decl->decl.state == RiDecl_Resolved);
    }


    switch (decl->decl.spec->kind)
    {
        case RiNode_Spec_Var:
            id->kind = RiNode_Value_Var;
            id->value.spec = decl->decl.spec;
            id->value.type = ri_get_spec_(ri, decl->decl.spec->spec.var.type);
            break;
        case RiNode_Spec_Constant:
            id->kind = RiNode_Value_Constant;
            id->value.spec = decl->decl.spec;
            id->value.type = ri_get_spec_(ri, decl->decl.spec->spec.constant.type);
            id->value.constant = decl->decl.spec->spec.constant.value;
            break;
        case RiNode_Spec_Func:
            id->kind = RiNode_Value_Func;
            id->value.spec = decl->decl.spec;
            id->value.type = ri_get_spec_(ri, decl->decl.spec->spec.func.type);
            break;
        default:
            RI_CHECK(ri_is_in(decl->decl.spec->kind, RiNode_Spec_Type));
            id->kind = RiNode_Value_Type;
            id->value.spec = decl->decl.spec;
    }
    RI_CHECK(ri_is_in(id->value.spec->kind, RiNode_Spec));

    return true;
}

static bool
ri_resolve_func_args_(Ri* ri, RiNode* func_type, RiNodeSlice* args)
{
    RiNode* it;
    slice_each(args, &it) {
        RI_CHECK(it->kind == RiNode_Decl);
        RI_CHECK(it->decl.spec->kind == RiNode_Spec_Var);
        if (!ri_resolve_node_(ri, &it->decl.spec->spec.var.type)) {
            return false;
        }
    }
    return true;
}

static
RI_RESOLVE_F_(ri_resolve_node_)
{
    RiNode* n = *node;

    if (n->kind == RiNode_Decl)
    {
        // TODO: 1. Move out to ri_resolve_decl_().
        // TODO: 2. Here only resolve variables and functions if we're in root scope.

        RI_CHECK(n->decl.state == RiDecl_Unresolved);
        n->decl.state = RiDecl_Resolving;

        switch (n->decl.spec->kind) {
            case RiNode_Spec_Func:
                if (!ri_resolve_node_(ri, &n->decl.spec)) {
                    return false;
                }
                break;
            case RiNode_Spec_Var:
                if (!ri_resolve_node_(ri, &n->decl.spec->spec.var.type)) {
                    return false;
                }
                break;
        }
        n->decl.state = RiDecl_Resolved;
        array_push(&n->owner->scope.decl, n);
        return true;
    } else if (ri_is_in(n->kind, RiNode_St_Assign)) {
        return ri_resolve_assign_(ri, node);
    } else if (ri_is_in(n->kind, RiNode_Expr_Binary)) {
        return ri_resolve_binary_(ri, node);
    } else if (ri_is_in(n->kind, RiNode_Expr_Unary)) {
        return ri_resolve_unary_(ri, node);
    } else if (ri_is_in(n->kind, RiNode_Spec_Type_Number)) {
        // Nothing to do.
        return true;
    } else if (n->kind == RiNode_Spec_Type_Infer) {
        return true;
    } else {
        switch (n->kind)
        {
            case RiNode_Scope:
                return ri_resolve_slice_with_(ri, n->scope.statements.slice, &ri_resolve_node_);

            case RiNode_Id:
                return ri_resolve_identifier_(ri, node);

            case RiNode_Spec_Type_Func:
                return (
                    ri_resolve_func_args_(ri, n, &n->spec.type.func.inputs.slice) &&
                    ri_resolve_func_args_(ri, n, &n->spec.type.func.outputs.slice)
                );

            case RiNode_Spec_Func:
                if (!ri_resolve_node_(ri, &n->spec.func.type)) {
                    return false;
                }
                array_push(&ri->pending, n->spec.func.scope);
                return true;

            case RiNode_Value_Constant:
            case RiNode_Value_Type:
            case RiNode_Value_Func:
            case RiNode_Value_Var: {
                // Nothing to do (these come resolved from ri_resolve_identifer_).
                return true;
            } break;

            case RiNode_Expr_Call: {
                if (!ri_resolve_node_(ri, &n->call.func)) {
                    return false;
                }
                RiNode* callable = n->call.func->value.spec;
                if (callable->kind == RiNode_Spec_Func) {
                    if (!ri_resolve_expr_call_func_(ri, &n)) {
                        return false;
                    }
                } else if (ri_is_in(callable->kind, RiNode_Spec_Type)) {
                    if (!ri_resolve_expr_call_type_(ri, &n)) {
                        return false;
                    }
                }
            } break;

            case RiNode_St_Expr:
                // Statement-level expression won't get any `expected_type`.
                return ri_resolve_node_(ri, &n->st_expr);

            case RiNode_St_Return:
                // TODO: Use func's return value type to infer return's argument.
                // TODO: Use the return type as `expected_type` too.
                if (n->st_return.argument) {
                    return ri_resolve_node_(ri, &n->st_return.argument);
                }
                return true;

            case RiNode_St_If:
                return ri_resolve_st_if_(ri, &n);

            case RiNode_St_For:
                return ri_resolve_st_for_(ri, &n);

            default: {
                RI_ABORT("unexpected node");
                return false;
            }
        }

        *node = n;
        return true;
    }
}

RiNode*
ri_resolve(Ri* ri, RiNode* node)
{
    array_push(&ri->pending, node);

    RiNode* it;
    array_each(&ri->pending, &it) {
        if (!ri_resolve_node_(ri, &it)) {
            return NULL;
        }
    }
    return node;
}

//
//
//

static void
ri_typecheck_cast_const_(Ri* ri, RiNode* expr, RiNode* type)
{
    RI_CHECK(ri_is_in(type->kind, RiNode_Spec_Type));
    if (ri_is_in(expr->kind, RiNode_Expr_Binary)) {
        ri_typecheck_cast_const_(ri, expr->binary.argument0, type);
        ri_typecheck_cast_const_(ri, expr->binary.argument1, type);
    } else if (ri_is_in(expr->kind, RiNode_Expr_Unary)) {
        ri_typecheck_cast_const_(ri, expr->unary.argument, type);
    } else if (expr->kind == RiNode_Value_Constant) {
        // TODO: We need to be sure that the const type can actually be implicitly cast to `type`.
        RI_ASSERT(ri_is_in(expr->value.type->kind, RiNode_Spec_Type_Number_None));
        expr->value.type = type;
    }
}

static RiNode*
ri_typecheck_get_untyped_default_type_(Ri* ri, RiNodeKind untyped)
{
    switch (untyped)
    {
        case RiNode_Spec_Type_Number_None_Int:
            return ri->node_meta[RiNode_Spec_Type_Number_Int64].node;
            break;
        case RiNode_Spec_Type_Number_None_Real:
            return ri->node_meta[RiNode_Spec_Type_Number_Float32].node;
            break;
    }
    RI_UNREACHABLE;
    return NULL;
}

RiNode*
ri_typecheck_node_(Ri* ri, RiNode* node)
{
    RiNode* type_none = ri->node_meta[RiNode_Spec_Type_None].node;

    if (ri_is_in(node->kind, RiNode_Expr_Binary_Comparison))
    {
        // Here we have to check for dominance, but
        // we'd always return bool. Here we'd need to call `set`
        // because we want the types to be concrete.
        RiNode* t0 = ri_typecheck_node_(ri, node->binary.argument0);
        RiNode* t1 = ri_typecheck_node_(ri, node->binary.argument1);
        bool d0 = !ri_is_in(t0->kind, RiNode_Spec_Type_Number_None);
        bool d1 = !ri_is_in(t1->kind, RiNode_Spec_Type_Number_None);
        if (t0 == NULL || t1 == NULL) {
            // Error.
            return NULL;
        } else if (t0 == t1) {
            // Types are the same, so return one.
            if (d0 == false) {
                RI_CHECK(d1 == false);
                // Both are not-dominant, so we'll set their respective defaults.
                t0 = t1 = ri_typecheck_get_untyped_default_type_(ri, t0->kind);
                ri_typecheck_cast_const_(ri, node, t0);
            }
        } else {
            // Types are different, so return the more dominant one.
            if ((d0 && d1) || (!d0 && !d1)) {
                // Error: Both types have same dominance, and we
                // know that they are different now.
                // TODO: We can probably allow for Int-to-Real cast.
                ri_error_set_mismatched_types_(ri,
                    node->pos, t0, t1, RI_OP_NAMES_[node->kind]);
                return NULL;
            } else if (d0) {
                // Left it dominant.
                // Set right to t0?
                t1 = t0;
                ri_typecheck_cast_const_(ri, node->binary.argument1, t0);
            } else {
                RI_CHECK(d0 == false && d1 == true);
                // Right is dominant.
                t0 = t1;
                ri_typecheck_cast_const_(ri, node->binary.argument0, t1);
            }
        }

        // We always return bool.
        return ri->node_meta[RiNode_Spec_Type_Number_Bool].node;
    }
    else if (ri_is_in(node->kind, RiNode_Expr_Binary))
    {
        RiNode* t0 = ri_typecheck_node_(ri, node->binary.argument0);
        RiNode* t1 = ri_typecheck_node_(ri, node->binary.argument1);
        if (t0 == NULL || t1 == NULL) {
            // Error.
            return NULL;
        } else if (t0 == t1) {
            // Types are the same, so return one.
        } else {
            // Types are different, so return the more dominant one.
            bool d0 = !ri_is_in(t0->kind, RiNode_Spec_Type_Number_None);
            bool d1 = !ri_is_in(t1->kind, RiNode_Spec_Type_Number_None);
            if ((d0 && d1) || (!d0 && !d1)) {
                // Error: Both types have same dominance, and we
                // know that they are different now.
                // TODO: We can probably allow for Int-to-Real cast.
                ri_error_set_mismatched_types_(ri,
                    node->pos, t0, t1, RI_OP_NAMES_[node->kind]);
                return NULL;
            } else if (d0) {
                // Left it dominant.
                // Set right to t0.
                t1 = t0;
                ri_typecheck_cast_const_(ri, node->binary.argument1, t0);
            } else {
                RI_CHECK(d0 == false && d1 == true);
                // Right is dominant.
                // Set left to t1.
                t0 = t1;
                ri_typecheck_cast_const_(ri, node->binary.argument0, t1);
            }
        }

        return t0;
    } else if (ri_is_in(node->kind, RiNode_Expr_Unary)) {
        return ri_typecheck_node_(ri, node->unary.argument);
    } else {
        switch (node->kind)
        {
            case RiNode_Module: {
                return ri_typecheck_node_(ri, node->module.scope);
            }

            case RiNode_Scope: {
                RiNode* it;
                array_each(&node->scope.statements, &it) {
                    if (!ri_typecheck_node_(ri, it)) {
                        return NULL;
                    }
                }
                return type_none;
            } break;

            case RiNode_Decl: {
                switch (node->decl.spec->kind) {
                    case RiNode_Spec_Func:
                        if (!ri_typecheck_node_(ri, node->decl.spec->spec.func.scope)) {
                            return NULL;
                        }
                        break;
                    case RiNode_Spec_Var:
                        return ri_get_spec_(ri, node->decl.spec->spec.var.type);
                }
                return type_none;
            } break;

            case RiNode_Expr_Call: {
                RiNode* func_spec = ri_get_spec_(ri, node->call.func);
                RiNode* func_type = ri_get_spec_(ri, func_spec->spec.func.type);
                RiNodeArray* inputs = &func_type->spec.type.func.inputs;
                RiNode* it;
                array_eachi(&node->call.arguments, i, &it)
                {
                    RiNode* t = ri_typecheck_node_(ri, it);
                    if (!t) {
                        return NULL;
                    }
                    if (ri_is_in(t->kind, RiNode_Spec_Type_Number_None)) {
                        // TODO: Cast constant to type required by input argument at `i`.
                        RiNode* arg = array_at(inputs, i);
                        RI_CHECK(arg->kind == RiNode_Decl);
                        RI_CHECK(arg->decl.spec->kind == RiNode_Spec_Var);
                        ri_typecheck_cast_const_(ri, it, ri_get_spec_(ri, arg->decl.spec->spec.var.type));
                    }
                }
                return ri_retof_(ri, node);
            } break;

            case RiNode_St_Expr: {
                if (!ri_typecheck_node_(ri, node->st_expr)) {
                    return NULL;
                }
                return type_none;
            } break;

            case RiNode_St_Return: {
                if (node->st_return.argument) {
                    if (!ri_typecheck_node_(ri, node->st_return.argument)) {
                        return NULL;
                    }
                }
                return type_none;
            } break;

            case RiNode_St_Assign: {
                RiNode* type0 = ri_typecheck_node_(ri, node->binary.argument0);
                RiNode* type1 = ri_typecheck_node_(ri, node->binary.argument1);
                if (!type0 || !type1) {
                    return NULL;
                }
                if (type0->kind == RiNode_Spec_Type_Infer) {
                    RI_CHECK(type1->kind != RiNode_Spec_Type_Infer);
                    if (ri_is_in(type1->kind, RiNode_Spec_Type_Number_None)) {
                        // If right is untyped, set it to default.
                        type1 = ri_typecheck_get_untyped_default_type_(ri, type1->kind);
                        ri_typecheck_cast_const_(ri, node->binary.argument1, type1);
                    }

                    // Set left type to same type as we now have for right.
                    RiNode* var = node->binary.argument0;
                    RI_CHECK(var->kind == RiNode_Decl);
                    RI_CHECK(var->decl.spec->kind == RiNode_Spec_Var);
                    var->decl.spec->spec.var.type = type1;
                } else if (type0 != type1) {
                    if (
                        (
                            type0->kind == RiNode_Spec_Type_Number_Float64 ||
                            type0->kind == RiNode_Spec_Type_Number_Float32
                        )
                        &&
                        type1->kind == RiNode_Spec_Type_Number_None_Real
                    ) {
                        ri_typecheck_cast_const_(ri, node->binary.argument1, type0);
                    } else if (ri_is_in(type0->kind, RiNode_Spec_Type_Number_Int) &&
                        type1->kind == RiNode_Spec_Type_Number_None_Int
                    ) {
                        ri_typecheck_cast_const_(ri, node->binary.argument1, type0);
                    } else {
                        ri_error_set_mismatched_types_(ri, node->pos, type0, type1, "=");
                        return NULL;
                    }
                }
                return type_none;
            } break;

            case RiNode_St_If: {
                if (node->st_if.pre && !ri_typecheck_node_(ri, node->st_if.pre)) {
                    return NULL;
                }
                if (!ri_typecheck_node_(ri, node->st_if.condition)) {
                    return NULL;
                }
                return type_none;
            } break;

            default:
                return ri_retof_(ri, node);
        }
    }

    return NULL;
}

bool
ri_typecheck(Ri* ri, RiNode* node)
{
    return ri_typecheck_node_(ri, node);
}

//
//
//

RiNode*
ri_build(Ri* ri, String stream, String path)
{
    RiNode* module = ri_make_node_(ri, (RiPos){0}, RiNode_Module);
    RiNode* scope = ri_parse(ri, stream, path);
    if (scope) {
        scope = ri_resolve(ri, scope);
        if (scope) {
            if (ri_typecheck(ri, scope)) {
                module->module.scope = scope;
                return module;
            }
        }
    }
    return NULL;
}

//
// Dump.
//

typedef struct RiDump_ {
    Ri* ri;
    RiPrinter printer;
    Map logged;
} RiDump_;

static void ri_dump_(RiDump_* dump, RiNode* node);

static void
ri_dump_slice_(RiDump_* D, RiNodeSlice* nodes, const char* block)
{
    if (nodes->count > 0) {
        if (block) {
            riprinter_print(&D->printer, "(%s", block);
        }
        riprinter_print(&D->printer, "\n\t");
        RiNode* it;
        array_each(nodes, &it) {
            ri_dump_(D, it);
        }
        riprinter_print(&D->printer, "\b");
        if (block) {
            riprinter_print(&D->printer, ")\n");
        }
    } else if (block) {
        riprinter_print(&D->printer, "(%s)\n", block);
    }
}

static void
ri_dump_block_(RiDump_* D, RiNode* node, const char* block)
{
    if (!node && block) {
        riprinter_print(&D->printer, "(%s)\n", block);
    } else if (node) {
        if (block) {
            riprinter_print(&D->printer, "(%s\n\t", block);
        }
        ri_dump_(D, node);
        if (block) {
            riprinter_print(&D->printer, "\b)\n");
        }
    }
}

static const char* RI_NODEKIND_NAMES_[RiNode_COUNT__] = {
    [RiNode_Value_Constant] = "const",
    [RiNode_Expr_Cast] = "expr-cast",
    [RiNode_Expr_Unary_Positive] = "expr-positive",
    [RiNode_Expr_Unary_Negative] = "expr-negative",
    [RiNode_Expr_Unary_IncPre] = "expr-inc-prefix",
    [RiNode_Expr_Unary_DecPre] = "expr-dec-prefix",
    [RiNode_Expr_Unary_IncPost] = "expr-inc-postfix",
    [RiNode_Expr_Unary_DecPost] = "expr-dec-postfix",
    [RiNode_Expr_Unary_BNeg] = "expr-bneg",
    [RiNode_Expr_Unary_Not] = "expr-not",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Add] = "expr-add",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Sub] = "expr-sub",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Mul] = "expr-mul",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Div] = "expr-div",
    [RiNode_Expr_Binary_Numeric_Arithmetic_Mod] = "expr-mod",
    [RiNode_Expr_Binary_Numeric_Bitwise_BXor] = "expr-bxor",
    [RiNode_Expr_Binary_Numeric_Bitwise_BAnd] = "expr-band",
    [RiNode_Expr_Binary_Numeric_Bitwise_BOr] = "expr-bor",
    [RiNode_Expr_Binary_Numeric_Bitwise_BShL] = "expr-bshl",
    [RiNode_Expr_Binary_Numeric_Bitwise_BShR] = "expr-bshr",
    [RiNode_Expr_Binary_Numeric_Boolean_And] = "expr-and",
    [RiNode_Expr_Binary_Numeric_Boolean_Or] = "expr-or",
    [RiNode_Expr_Binary_Select] = "expr-select",
    [RiNode_Expr_Binary_Comparison_Lt] = "expr-lt",
    [RiNode_Expr_Binary_Comparison_Gt] = "expr-gt",
    [RiNode_Expr_Binary_Comparison_LtEq] = "expr-lt-eq",
    [RiNode_Expr_Binary_Comparison_GtEq] = "expr-gt-eq",
    [RiNode_Expr_Binary_Comparison_Eq] = "expr-eq",
    [RiNode_Expr_Binary_Comparison_NotEq] = "expr-not-eq",
    [RiNode_St_Assign] = "st-assign",
    [RiNode_St_Assign_Add] = "st-assign-add",
    [RiNode_St_Assign_Sub] = "st-assign-sub",
    [RiNode_St_Assign_Mul] = "st-assign-mul",
    [RiNode_St_Assign_Div] = "st-assign-div",
    [RiNode_St_Assign_Mod] = "st-assign-mod",
    [RiNode_St_Assign_And] = "st-assign-and",
    [RiNode_St_Assign_Or] = "st-assign-or",
    [RiNode_St_Assign_Xor] = "st-assign-xor",
    [RiNode_Value_Var] = "value-var",
    [RiNode_Value_Func] = "value-func",
    [RiNode_Value_Type] = "value-type",
};

static void
ri_dump_(RiDump_* D, RiNode* node)
{
    RI_CHECK(node);

    int is_logged = map_get(&D->logged, (ValueScalar){ .ptr = node }).i32;
    if (!is_logged) {
        map_put(&D->logged, (ValueScalar){ .ptr = node }, (ValueScalar){ .i32 = 1 });
    }

    if (ri_is_in(node->kind, RiNode_Expr_Binary) || ri_is_in(node->kind, RiNode_St_Assign)) {
        riprinter_print(&D->printer, "(%s\n\t", RI_NODEKIND_NAMES_[node->kind]);
        ri_dump_(D, node->binary.argument0);
        ri_dump_(D, node->binary.argument1);
        riprinter_print(&D->printer, "\b)\n");
    } else if (ri_is_in(node->kind, RiNode_Expr_Unary)) {
        riprinter_print(&D->printer, "(%s\n\t", RI_NODEKIND_NAMES_[node->kind]);
        ri_dump_(D, node->unary.argument);
        riprinter_print(&D->printer, "\b)\n");
    } else if (ri_is_in(node->kind, RiNode_Spec_Type_Number)) {
        riprinter_print(&D->printer, "(spec-type-number '%S')\n", node->spec.id);
    } else {
        RiNode* it;
        switch (node->kind)
        {
            case RiNode_Module: {
                riprinter_print(&D->printer, "(module\n\t");
                ri_dump_(D, node->module.scope);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Scope: {
                riprinter_print(&D->printer, "(scope %d\n\t", node->index);

                if (node->scope.decl.count) {
                    array_each(&node->scope.decl, &it) {
                        ri_dump_(D, it);
                    }
                }
                riprinter_print(&D->printer, "(code");
                if (is_logged) {
                    riprinter_print(&D->printer, " (recursive)");
                } else {
                    ri_dump_slice_(D, &node->scope.statements.slice, NULL);
                }
                riprinter_print(&D->printer, ")\n");

                riprinter_print(&D->printer, "\b)\n");
            } break;


            case RiNode_Decl: {
                riprinter_print(&D->printer, "(decl\n\t");
                ri_dump_(D, node->decl.spec);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Id: {
                riprinter_print(&D->printer, "(id '%S')\n", node->id.name);
            } break;

            case RiNode_Spec_Type_Number_None_Int: {
                riprinter_print(&D->printer, "(spec-type-untyped-int)\n");
            } break;

            case RiNode_Spec_Type_Number_None_Real: {
                riprinter_print(&D->printer, "(spec-type-untyped-real)\n");
            } break;

            case RiNode_Expr_Call: {
                riprinter_print(&D->printer, "(expr-call\n\t");
                ri_dump_(D, node->call.func);
                ri_dump_slice_(D, &node->call.arguments.slice, "arguments");
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Expr_Cast: {
                riprinter_print(&D->printer, "(expr-cast\n\t");
                ri_dump_slice_(D, &node->call.arguments.slice, "arguments");
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_St_Expr: {
                riprinter_print(&D->printer, "(st-expr\n\t");
                ri_dump_(D, node->st_expr);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_St_Return: {
                riprinter_print(&D->printer, "(st-return");
                if (node->st_return.argument) {
                    riprinter_print(&D->printer, "\n\t");
                    ri_dump_(D, node->st_return.argument);
                    riprinter_print(&D->printer, "\b");
                }
                riprinter_print(&D->printer, ")\n");
            } break;

            case RiNode_St_If: {
                riprinter_print(&D->printer, ("(st-if\n\t"));
                ri_dump_block_(D, node->st_if.pre, "pre");
                ri_dump_block_(D, node->st_if.condition, "condition");
                ri_dump_(D, node->st_if.scope);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_St_For: {
                riprinter_print(&D->printer, ("(st-for\n\t"));
                ri_dump_block_(D, node->st_for.pre, "pre");
                ri_dump_block_(D, node->st_for.condition, "condition");
                ri_dump_block_(D, node->st_for.post, "post");
                ri_dump_(D, node->st_for.scope);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_St_Switch: {
                riprinter_print(&D->printer, ("(st-switch\n\t"));
                ri_dump_block_(D, node->st_switch.pre, "pre");
                ri_dump_block_(D, node->st_switch.expr, "expr");
                ri_dump_(D, node->st_switch.scope);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_St_Switch_Case: {
                riprinter_print(&D->printer, ("(st-switch-case\n\t"));
                ri_dump_(D, node->st_switch_case.expr);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_St_Switch_Default: {
                riprinter_print(&D->printer, ("(st-switch-default)\n"));
            } break;

            case RiNode_St_Break: {
                riprinter_print(&D->printer, ("(st-break)\n"));
            } break;

            case RiNode_St_Continue: {
                riprinter_print(&D->printer, ("(st-continue)\n"));
            } break;

            case RiNode_Value_Func:
            case RiNode_Value_Type:
            case RiNode_Value_Var: {
                riprinter_print(&D->printer, "(%s\n\t", RI_NODEKIND_NAMES_[node->kind]);
                ri_dump_(D, node->value.spec);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Value_Constant: {
                riprinter_print(&D->printer, "(const ");
                switch (node->value.type->kind) {
                    case RiNode_Spec_Type_Number_Int8:
                    case RiNode_Spec_Type_Number_Int16:
                    case RiNode_Spec_Type_Number_Int32:
                    case RiNode_Spec_Type_Number_UInt8:
                    case RiNode_Spec_Type_Number_UInt16:
                    case RiNode_Spec_Type_Number_UInt32:
                        riprinter_print(&D->printer, "%d\n\t", node->value.constant.integer);
                        break;

                    case RiNode_Spec_Type_Number_UInt64:
                    case RiNode_Spec_Type_Number_None_Int:
                        riprinter_print(&D->printer, "%"PRIu64"\n\t", node->value.constant.integer);
                        break;
                    case RiNode_Spec_Type_Number_Float32:
                    case RiNode_Spec_Type_Number_Float64:
                    case RiNode_Spec_Type_Number_None_Real:
                        riprinter_print(&D->printer, "%f\n\t", node->value.constant.real);
                        break;
                    case RiNode_Spec_Type_Number_Bool:
                        riprinter_print(&D->printer, "%s\n\t",
                            node->value.constant.boolean ? "true" : "false");
                        break;

                    case RiNode_Spec_Type_Number_Int64:
                        riprinter_print(&D->printer, "%"PRIi64"\n\t", node->value.constant.integer);
                        break;

                    default:
                        riprinter_print(&D->printer, "UNKNOWN\n\t");
                        break;
                }
                ri_dump_(D, node->value.type);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Spec_Var: {
                riprinter_print(&D->printer, "(spec-var '%S'\n\t", node->spec.id);
                ri_dump_(D, node->spec.var.type);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Spec_Func: {
                riprinter_print(&D->printer, "(spec-func '%S'\n\t", node->spec.id);
                ri_dump_(D, node->spec.func.type);
                ri_dump_(D, node->spec.func.scope);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Spec_Type_Func: {
                riprinter_print(&D->printer, "(spec-type-func '%S'\n\t", node->spec.id);
                ri_dump_slice_(D, &node->spec.type.func.inputs.slice, "in");
                ri_dump_slice_(D, &node->spec.type.func.outputs.slice, "out");
                riprinter_print(&D->printer, "\b)\n");
            } break;

            default: {
                riprinter_print(&D->printer, "(UNKNOWN)\n");
            } break;
        }
    }
}

void
ri_dump(Ri* ri, RiNode* node, CharArray* buffer)
{
    RI_CHECK(node);
    RI_CHECK(buffer);

    RiDump_ dump = {
        .printer.out = buffer
    };
    ri_dump_(&dump, node);
    map_purge(&dump.logged);
}

void
ri_log(Ri* ri, RiNode* node)
{
    CharArray buffer = {0};
    ri_dump(ri, node, &buffer);
    RI_LOG("%S", buffer.slice);
    array_purge(&buffer);
}

//
//
//

void
ri_init(Ri* ri, void* host)
{
    // RI_LOG_DEBUG("node %d bytes", sizeof(RiNode));

    memset(ri, 0, sizeof(Ri));
    arena_init(&ri->arena, MEGABYTES(1));
    intern_init(&ri->intern);

    ri->id_func        = ri_make_id_(ri, S("func")).items;
    ri->id_var         = ri_make_id_(ri, S("var")).items;
    ri->id_const       = ri_make_id_(ri, S("const")).items;
    ri->id_type        = ri_make_id_(ri, S("type")).items;
    ri->id_struct      = ri_make_id_(ri, S("struct")).items;
    ri->id_union       = ri_make_id_(ri, S("union")).items;
    ri->id_enum        = ri_make_id_(ri, S("enum")).items;

    ri->id_return      = ri_make_id_(ri, S("return")).items;
    ri->id_if          = ri_make_id_(ri, S("if")).items;
    ri->id_else        = ri_make_id_(ri, S("else")).items;
    ri->id_for         = ri_make_id_(ri, S("for")).items;
    ri->id_switch      = ri_make_id_(ri, S("switch")).items;
    ri->id_case        = ri_make_id_(ri, S("case")).items;
    ri->id_default     = ri_make_id_(ri, S("default")).items;
    ri->id_break       = ri_make_id_(ri, S("break")).items;
    ri->id_continue    = ri_make_id_(ri, S("continue")).items;
    ri->id_fallthrough = ri_make_id_(ri, S("fallthrough")).items;

    ri->id_true        = ri_make_id_(ri, S("true")).items;
    ri->id_false       = ri_make_id_(ri, S("false")).items;
    ri->id_nil         = ri_make_id_(ri, S("nil")).items;

    ri->id_underscore  = ri_make_id_(ri, S("_")).items;

    ri->scope = ri_make_scope_(ri, RI_POS_OUTSIDE);

    ri->node_meta[RiNode_Spec_Type_None].node =
        ri_make_node_(ri, RI_POS_OUTSIDE, RiNode_Spec_Type_None);
    ri->node_meta[RiNode_Spec_Type_None].node->spec.id = ri_make_id_(ri, S("none"));

    ri->node_meta[RiNode_Spec_Type_Number_None_Int].node =
        ri_make_node_(ri, RI_POS_OUTSIDE, RiNode_Spec_Type_Number_None_Int);
    ri->node_meta[RiNode_Spec_Type_Number_None_Int].node->spec.id = ri_make_id_(ri, S("untyped-int"));

    ri->node_meta[RiNode_Spec_Type_Number_None_Real].node =
        ri_make_node_(ri, RI_POS_OUTSIDE, RiNode_Spec_Type_Number_None_Real);
    ri->node_meta[RiNode_Spec_Type_Number_None_Real].node->spec.id = ri_make_id_(ri, S("untyped-real"));

    ri->node_meta[RiNode_Spec_Type_Infer].node =
        ri_make_node_(ri, RI_POS_OUTSIDE, RiNode_Spec_Type_Infer);
    ri->node_meta[RiNode_Spec_Type_Infer].node->spec.id = ri_make_id_(ri, S("infer"));

    #define DECL_TYPE(Name, Type) { \
        String name = ri_make_id_(ri, S(Name)); \
        RiNode* spec = ri_make_spec_type_number_(ri, \
            RI_POS_OUTSIDE, \
            name, \
            RiNode_Spec_Type_ ## Type \
        ); \
        ri->node_meta[RiNode_Spec_Type_ ## Type] = (RiNodeMeta){ \
            .node = spec, \
        }; \
        ri_scope_set_(ri, ri_make_decl_(ri, spec->pos, spec)); \
    }

        DECL_TYPE("bool",       Number_Bool);
        DECL_TYPE("int64",      Number_Int64);
        DECL_TYPE("uint64",     Number_UInt64);
        DECL_TYPE("int32",      Number_Int32);
        DECL_TYPE("uint32",     Number_UInt32);
        DECL_TYPE("int16",      Number_Int16);
        DECL_TYPE("uint16",     Number_UInt16);
        DECL_TYPE("int8",       Number_Int8);
        DECL_TYPE("uint8",      Number_UInt8);
        DECL_TYPE("float32",    Number_Float32);
        DECL_TYPE("float64",    Number_Float64);

    #undef DECL_TYPE

    ri_scope_set_(ri,
        ri_make_decl_(ri, RI_POS_OUTSIDE,
            ri_make_spec_constant_(ri,
                RI_POS_OUTSIDE,
                ri_make_id_(ri, S("host")),
                ri->node_meta[RiNode_Spec_Type_Number_UInt64].node,
                (RiLiteral){ .pointer = host }
            )
        )
    );
}

void
ri_purge(Ri* ri)
{
    RI_LOG_DEBUG("types %d", ri->types.count);
    RI_LOG_DEBUG("memory %d bytes", ri->arena.head);
    RI_LOG_DEBUG("nodes %d", ri->index);
    arena_purge(&ri->arena);
    intern_purge(&ri->intern);
}