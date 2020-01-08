#include "ri-parse.h"

static const struct {
    RiNodeKind unary;
    RiNodeKind binary;
    RiNodeKind assign;
} RI_TOKEN_TO_OP_[RiToken_COUNT__] = {
    [RiToken_Dot]           = { .binary = RiNode_Expr_Select },
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

//
// Errors
//

static void
ri_error_set_unexpected_token_(RiParser* parser, RiToken* token)
{
    ri_error_set_(parser->ri, RiError_UnexpectedToken, token->pos, "unexpected token '%.*s' (%d)",
        token->end - token->start,
        token->start,
        token->kind
    );
}

//
// Lexer
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
ri_lex_one_or_two_(RiParser* parser, char** it, char* end, enum RiTokenKind op0, char ch, enum RiTokenKind op1)
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
ri_lex_number_(RiParser* parser, char** it_, char* end, RiToken* token)
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
            ri_error_set_(parser->ri, RiError_UnexpectedCharacter, token->pos, "digit or '+' or '-' expected");
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
                ri_error_set_(parser->ri, RiError_UnexpectedToken, token->pos, "real constant too large");
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
ri_lex_next_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiStream* stream = &parser->stream;
    RiToken* token = &parser->token;
    char* it = stream->it;
    char* end = stream->end;

next:
    stream->it = it;

    token->kind = RiToken_Unknown;
    token->start = it;
    token->end = it;
    token->pos.row = stream->line_index;
    token->pos.col = it - stream->line;

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

        case '=': token->kind = ri_lex_one_or_two_(parser, &it, end, RiToken_Eq, '=', RiToken_EqEq); break;
        case '*': token->kind = ri_lex_one_or_two_(parser, &it, end, RiToken_Star, '=', RiToken_StarEq); break;
        case '%': token->kind = ri_lex_one_or_two_(parser, &it, end, RiToken_Percent, '=', RiToken_PercentEq); break;
        case '^': token->kind = ri_lex_one_or_two_(parser, &it, end, RiToken_Beak, '=', RiToken_BeakEq); break;

        case '!': token->kind = ri_lex_one_or_two_(parser, &it, end, RiToken_Bang, '=', RiToken_BangEq); break;

        // TODO: Simplify following with call:
        // ri_lex_map_(parser, &it, end, "=&", RiToken_Amp, RiToken_AmpEq, RiToken_AmpAmp)

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
            Ri* ri = parser->ri;
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
            } else if (token->id.items == ri->id_import) {
                token->kind = RiToken_Keyword_Import;
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
                if (ri_lex_number_(parser, &it, end, token) == false) {
                    return false;
                }
                RI_CHECK(token->kind == RiToken_Real);
            } else {
                token->kind = RiToken_Dot;
            }
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            if (ri_lex_number_(parser, &it, end, token) == false) {
                return false;
            }
        } break;

        case '"': {
            token->kind = RiToken_String;
            ++it;
            while (it < end && *it != '"') {
                it += 1 + (*it == '\\');
            }            
            if (it < end && *it == '"') {
                ++it;
            } else {
                ri_error_set_(parser->ri, RiError_UnterminatedString, token->pos, "unterminated string");
                return false;
            }
        } break;

        default: {
            ri_error_set_(parser->ri, RiError_UnexpectedCharacter, token->pos, "unexpected character '%c' (%d)", *it, *it);
            return false;
        } break;
    }

    token->end = it;
    stream->it = it;

    // RI_LOG_DEBUG("'%.*s' (%d:%d)",
    //     token->end - token->start,
    //     token->start,
    //     token->pos.row + 1,
    //     token->pos.col + 1
    // );

    return true;
}

static bool
ri_lex_assert_token_(RiParser* parser, RiTokenKind expected_kind)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    if (parser->token.kind != expected_kind) {
        ri_error_set_unexpected_token_(parser, &parser->token);
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
ri_lex_next_if_(RiParser* parser, RiTokenKind expected_kind)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    if (parser->token.kind != expected_kind) {
        return RiLexNextIf_NoMatch;
    }

    if (!ri_lex_next_(parser)) {
        return RiLexNextIf_Error;
    }

    return RiLexNextIf_Match;
}

static bool
ri_lex_expect_token_(RiParser* parser, RiTokenKind expected_kind)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    if (!ri_lex_assert_token_(parser, expected_kind)) {
        return false;
    }

    if (!ri_lex_next_(parser)) {
        return false;
    }

    return true;
}

//
// String parser
//

static void
ri_parse_string_(const RiToken token, CharArray* buffer)
{
    // NOTE: We assume that the lexer did a good job here.
    char* it = token.start + 1;
    char* end = token.end - 1;
    RI_CHECK(it <= end);
    array_reserve(buffer, end - it);
    while (it < end) {
        char c = *it;
        if (c == '\\') {
            ++it;
            c = *it;
            switch (c)
            {
                case 'a': c = '\a'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                case 'v': c = '\v'; break;
                // TODO: \xHH
                // TODO: \uHHHH
                // TODO: \UHHHHHHHH
            }
        }
        array_push(buffer, c);
        ++it;
    }
}

//
// Semicolons
//

bool
ri_parse_semicolons_(RiParser* parser)
{
    while (parser->token.kind == RiToken_Semicolon) {
        if (!ri_lex_next_(parser)) {
            return false;
        }
    }
    return true;
}

//
// Expressions parser.
//

static RiNode* ri_parse_decl_(RiParser* parser);
static RiNode* ri_parse_scope_(RiParser* parser, RiTokenKind end, RiNodeKind scope_kind);
static RiNode* ri_parse_spec_func_type_(RiParser* parser);
static RiNode* ri_parse_spec_func_or_func_type_(RiParser* parser);
static RiNode* ri_parse_spec_struct_type_(RiParser* parser);

static RiNode* ri_parse_expr_(RiParser* parser);
static RiNode* ri_parse_expr_operator_(RiParser* parser, int precedence);

RiNode*
ri_parse_expr_operand_(RiParser* parser)
{
    switch (parser->token.kind)
    {
        case RiToken_Identifier: {
            RiToken token = parser->token;
            if (ri_lex_next_(parser)) {
                return ri_make_expr_id_(parser->arena, parser->scope, token.pos, token.id);
            }
            return NULL;
        } break;

        case RiToken_Integer: {
            RiNode* node = ri_make_node_(parser->arena, parser->scope, parser->token.pos, RiNode_Value_Constant);
            node->value.constant.literal.integer = parser->token.integer;
            node->value.constant.type = parser->ri->nodes[RiNode_Spec_Type_Number_None_Int];
            if (ri_lex_next_(parser)) {
                return node;
            }
            return NULL;
        } break;

        case RiToken_Real: {
            RiNode* node = ri_make_node_(parser->arena, parser->scope, parser->token.pos, RiNode_Value_Constant);
            node->value.constant.literal.real = parser->token.real;
            node->value.constant.type = parser->ri->nodes[RiNode_Spec_Type_Number_None_Real];
            if (ri_lex_next_(parser)) {
                return node;
            }
            return NULL;
        } break;

        // case RiToken_String: {
        //     RiNode* node = ri_make_node_(parser->arena, parser->scope, parser->token.pos, RiNode_Value_Constant);
        //     node->value.constant.literal.string = parser->token.id;
        //     // TODO: We'll need array types here.
        // } break;

        // TODO: case RiToken_String:

        case RiToken_Keyword_False:
        case RiToken_Keyword_True: {
            RiNode* node = ri_make_node_(parser->arena, parser->scope, parser->token.pos, RiNode_Value_Constant);
            node->value.constant.literal.boolean = (parser->token.kind == RiToken_Keyword_True);
            node->value.constant.type = parser->ri->nodes[RiNode_Spec_Type_Number_Bool];
            if (ri_lex_next_(parser)) {
                return node;
            }
            return NULL;
        }

        case RiToken_LP: {
            if (ri_lex_next_(parser)) {
                RiNode* node = ri_parse_expr_(parser);
                if (node && ri_lex_expect_token_(parser, RiToken_RP)) {
                    return node;
                }
            }
            return NULL;
        } break;

        // Ct expressions.

        case RiToken_Keyword_Struct: {
            return ri_parse_spec_struct_type_(parser);
        } break;

        case RiToken_Keyword_Func: {
            return ri_parse_spec_func_type_(parser);
        } break;
    }

    ri_error_set_unexpected_token_(parser, &parser->token);
    return NULL;
}

RiNode*
ri_parse_expr_dot_(RiParser* parser)
{
    RiNode* L = ri_parse_expr_operand_(parser);
    while (L && parser->token.kind == RiToken_Dot) {
        RiToken token = parser->token;
        if (ri_lex_next_(parser)) {
            RiNode* R = ri_parse_expr_operand_(parser);
            if (R) {
                L = ri_make_expr_binary_(parser->arena, parser->scope, token.pos, RiNode_Expr_Select, L, R);
                continue;
            }
        }
        return NULL;
    }
    return L;
}

static bool
ri_parse_expr_call_arguments_(RiParser* parser, RiNode* call)
{
    while (ri_lex_next_if_(parser, RiToken_RP) == RiLexNextIf_NoMatch) {
        RiNode* expr = ri_parse_expr_(parser);
        if (!expr) {
            return false;
        }
        array_push(&call->call.arguments, expr);
        if (ri_lex_next_if_(parser, RiToken_Comma) == RiLexNextIf_Error) {
            return false;
        }
    }
    if (parser->ri->error.kind) {
        return false;
    }
    return true;
}

RiNode*
ri_parse_expr_call_(RiParser* parser)
{
    RiNode* L = ri_parse_expr_dot_(parser);
    while (parser->token.kind == RiToken_LP) {
        if (ri_lex_next_(parser)) {
            L = ri_make_expr_call_(parser->arena, parser->scope, L->pos, L);
            if (ri_parse_expr_call_arguments_(parser, L)) {
                continue;
            }
        }
        return NULL;
    }
    return L;
}

static RiNode*
ri_parse_expr_operator_unary_(RiParser* parser)
{
    switch (parser->token.kind) {
        case RiToken_Bang:
        case RiToken_Tilde:
        case RiToken_Plus:
        case RiToken_Minus:
        case RiToken_Amp: {
            RiToken token = parser->token;
            if (ri_lex_next_(parser)) {
                RiNode* R = ri_parse_expr_operator_unary_(parser);
                if (R) {
                    return ri_make_expr_unary_(parser->arena, parser->scope, token.pos, RI_TOKEN_TO_OP_[token.kind].unary, R);
                }
            }
        } return NULL;
    }
    return ri_parse_expr_call_(parser);
}

static RiNode*
ri_parse_expr_operator_binary_(RiParser* parser, RiNodeKind op, RiNode* L, int super_precedence)
{
    RI_CHECK(ri_is_in(op, RiNode_Expr_Binary));
    RiToken token = parser->token;
    if (ri_lex_next_(parser)) {
        RiNode* R = ri_parse_expr_operator_(parser, super_precedence);
        if (R) {
            return ri_make_expr_binary_(parser->arena, parser->scope, token.pos, op, L, R);
        }
    }
    return NULL;
}

static RiNode*
ri_parse_expr_operator_(RiParser* parser, int precedence)
{
    RI_CHECK(precedence >= 0);
    if (precedence == 0) {
        return ri_parse_expr_operator_unary_(parser);
    }

    RiNode* L = ri_parse_expr_operator_(parser, precedence - 100);
    RiTokenKind token_kind;
    while (L)
    {
        token_kind = parser->token.kind;
        switch (precedence)
        {
            case 0:
                RI_UNREACHABLE;
                return L;

            // * / %
            case 500:
                switch (token_kind) {
                    case RiToken_Star: case RiToken_Slash: case RiToken_Percent:
                        L = ri_parse_expr_operator_binary_(
                            parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_UNREACHABLE;

            // + -
            case 600:
                switch (token_kind) {
                    case RiToken_Minus: case RiToken_Plus:
                        L = ri_parse_expr_operator_binary_(
                            parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_UNREACHABLE;

            // << >>
            case 700:
                switch (token_kind) {
                    case RiToken_LtLt: case RiToken_GtGt:
                        L = ri_parse_expr_operator_binary_(
                            parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_UNREACHABLE;

            // > >= < <=
            case 800:
                switch (token_kind) {
                    case RiToken_Gt: case RiToken_GtEq:
                    case RiToken_Lt: case RiToken_LtEq:
                        L = ri_parse_expr_operator_binary_(
                            parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_UNREACHABLE;

            // == !=
            case 900:
                switch (token_kind) {
                    case RiToken_EqEq: case RiToken_BangEq:
                        L = ri_parse_expr_operator_binary_(
                            parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                        continue;
                    default:
                        return L;
                }
                RI_UNREACHABLE;

            // &
            case 1000: if (token_kind == RiToken_Amp) {
                L = ri_parse_expr_operator_binary_(
                    parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // ^
            case 1100: if (token_kind == RiToken_Beak) {
                L = ri_parse_expr_operator_binary_(
                    parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // |
            case 1200: if (token_kind == RiToken_Pipe) {
                L = ri_parse_expr_operator_binary_(
                    parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // &&
            case 1300: if (token_kind == RiToken_AmpAmp) {
                L = ri_parse_expr_operator_binary_(
                    parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // ||
            case 1400: if (token_kind == RiToken_PipePipe) {
                L = ri_parse_expr_operator_binary_(
                    parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence - 100);
                continue;
            } return L;

            // =
            // case 1600: if (token_kind == RiToken_Eq) {
            //     L = ri_parse_expr_operator_binary_(
            //         parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence);
            // } return L;

            // :
            // case 1800: if (token_kind == RiToken_Colon) {
            //     L = ri_parse_expr_operator_binary_(
            //         parser, RI_TOKEN_TO_OP_[token_kind].binary, L, precedence);
            // } return L;

            default:
                return L;
        }
    }
    return L;
}

static RiNode*
ri_parse_expr_(RiParser* parser)
{
    return ri_parse_expr_operator_(parser, 2000);
}

//
// Declarations parser.
//

static RiNode*
ri_parse_type_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiToken token = parser->token;
    switch (parser->token.kind)
    {
        case RiToken_Keyword_Func:
            return ri_parse_spec_func_type_(parser);
        case RiToken_Keyword_Struct:
            return ri_parse_spec_struct_type_(parser);
        case RiToken_Identifier:
            return ri_parse_expr_(parser);
            // if (!ri_lex_next_(parser)) {
            //     return NULL;
            // }
            // if (parser->token.kind == RiToken_Dot)
            // while (parser->token.kind == RiToken_Dot) {
            //     if (ri_lex_next_(parser) && parser->token.kind == RiToken_Identifier) {
            //         if (ri_lex_next_(parser)) {
            //             continue;
            //         }
            //     }
            //     return NULL;
            // }
            // return ri_make_expr_id_(parser->arena, parser->scope, token.pos, token.id);
        default:
            ri_error_set_unexpected_token_(parser, &token);
            return NULL;
    }
}

//
// Variables
//

static RiNode*
ri_parse_decl_variable_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);
    RI_CHECK(parser->token.kind == RiToken_Keyword_Variable);
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiToken token = parser->token;
    if (!ri_lex_expect_token_(parser, RiToken_Identifier)) {
        return NULL;
    }

    RiNode* type;
    // `=` or `type-spec`
    if (parser->token.kind == RiToken_Eq) {
        // if (!ri_lex_next_(parser)) {
        //     return NULL;
        // }
        type = parser->ri->nodes[RiNode_Spec_Type_Infer];
    } else {
        type = ri_parse_type_(parser);
        if (!type) {
            return NULL;
        }
    }

    RiNode* spec = ri_make_spec_var_(parser->arena, parser->scope, token.pos, type, RiVar_Local);
    RiNode* decl = ri_make_decl_(parser->arena, parser->scope, token.pos, token.id, spec);
    return decl;
}

//
// Functions
//

static RiNode*
ri_parse_decl_type_func_input_arg_(RiParser* parser)
{
    // <id> <type-expr>
    // type-spec: <id> | _ | function(...)(...) | struct {...} ...

    // TODO: Implement short syntax for arguments of same type `function (a, b int32)`.
    // TODO: Implement `...int32` for variable arity.

    RiToken token = parser->token;
    if (!ri_lex_expect_token_(parser, RiToken_Identifier)) {
        return NULL;
    }

    RiNode* type = ri_parse_type_(parser);
    if (!type) {
        return NULL;
    }

    RiNode* spec = ri_make_spec_var_(parser->arena, parser->scope, token.pos, type, RiVar_Input);
    RiNode* decl = ri_make_decl_(parser->arena, parser->scope, token.pos, token.id, spec);
    return decl;
}

static bool
ri_parse_decl_type_func_input_arg_list_(RiParser* parser, RiNodeArray* list)
{
    if (!ri_lex_expect_token_(parser, RiToken_LP)) {
        return false;
    }

    RiNode* decl_arg;
    while (parser->token.kind != RiToken_RP)
    {
        decl_arg = ri_parse_decl_type_func_input_arg_(parser);
        if (!decl_arg) {
            return false;
        }
        array_push(list, decl_arg);
        if (ri_lex_next_if_(parser, RiToken_Comma) == RiLexNextIf_Error) {
            return false;
        }
    }
    if (!ri_lex_next_(parser)) {
        return false;
    }

    return true;
}

static bool
ri_parse_decl_type_func_output_arg_(RiParser* parser, RiNodeArray* outputs)
{
    if (parser->token.kind != RiToken_Identifier) {
        return true;
    }

    RiNode* type = ri_parse_type_(parser);
    if (type) {
        RiNode* spec = ri_make_spec_var_(parser->arena, parser->scope, type->pos, type, RiVar_Output);
        RiNode* decl = ri_make_decl_(parser->arena, parser->scope, type->pos, RI_ID_NULL, spec);
        array_push(outputs, decl);
        return true;
    }

    return false;
}

static RiNode*
ri_parse_spec_partial_func_type_(RiParser* parser, RiPos pos)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiNodeArray inputs = {0};
    RiNodeArray outputs = {0};
    if (!ri_parse_decl_type_func_input_arg_list_(parser, &inputs)) {
        return NULL;
    }

    if (!ri_parse_decl_type_func_output_arg_(parser, &outputs)) {
        return NULL;
    }

    RiNode* type = ri_make_spec_type_func_(parser->arena, parser->scope, pos, inputs, outputs);
    return type;
}

static RiNode*
ri_parse_spec_partial_func_(RiParser* parser, RiPos pos, String id)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);
    RI_CHECK(id.items);
    RI_CHECK(id.count);

    // NOTE: Starting and setting scope here, so all the nodes
    // created within have owner set to the scope.
    RiNode* scope = ri_make_scope_(parser->arena, parser->scope, pos, RiNode_Scope_Function_Root);
    parser->scope = scope;
    // RI_LOG_DEBUG("function '%S' scope %d", id, scope->index);

    RiNode* type = ri_parse_spec_partial_func_type_(parser, pos);
    if (!type) {
        return NULL;
    }

    RiNode* scope_body = NULL;
    switch (ri_lex_next_if_(parser, RiToken_LB))
    {
        case RiLexNextIf_Match: {
            scope_body = ri_parse_scope_(parser, RiToken_RB, RiNode_Scope_Function_Body);
            if (!scope_body) {
                return NULL;
            }

            // TODO: Make parsing arguments actually generate variable decls, so
            // this won't be needed.

            RiNode* it;
            array_each(&type->spec.type.func.inputs, &it) {
                map_put(&scope->scope.map,
                    (ValueScalar){ .ptr = it->decl.id.items },
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

    RI_CHECK(parser->scope == scope);
    parser->scope = scope->owner;

    RiNode* spec = ri_make_spec_func_(parser->arena, parser->scope, pos, type, scope_body ? scope : NULL);
    RiNode* decl = ri_make_decl_(parser->arena, parser->scope, pos, id, spec);
    return decl;
}

static RiNode*
ri_parse_spec_func_type_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);
    RI_CHECK(parser->token.kind == RiToken_Keyword_Func);
    RiToken token = parser->token;
    if (!ri_lex_next_(parser)) {
        return NULL;
    }
    return ri_parse_spec_partial_func_type_(parser, token.pos);
}

static RiNode*
ri_parse_spec_func_or_func_type_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);
    RI_CHECK(parser->token.kind == RiToken_Keyword_Func);
    RiPos pos = parser->token.pos;
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiNode* node = NULL;
    String id = parser->token.id;
    switch (ri_lex_next_if_(parser, RiToken_Identifier))
    {
        case RiLexNextIf_Match:
            node = ri_parse_spec_partial_func_(parser, pos, id);
            break;
        case RiLexNextIf_NoMatch:
            node = ri_parse_spec_partial_func_type_(parser, pos);
            break;
        case RiLexNextIf_Error:
            node = NULL;
            break;
    }

    return node;
}

//
// Struct
//

static RiNode*
ri_parse_spec_struct_type_(RiParser* parser)
{
    // struct { [
    //   <expr> <expr>; |
    //   <expr>;
    // ]+ }

    RI_CHECK(parser->ri->error.kind == RiError_None);
    RI_CHECK(parser->token.kind == RiToken_Keyword_Struct);
    RiPos pos_struct = parser->token.pos;
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiPos pos_scope = parser->token.pos;
    if (!ri_lex_expect_token_(parser, RiToken_LB)) {
        return NULL;
    }

    RiNode* scope = ri_make_scope_(parser->arena, parser->scope, pos_scope, RiNode_Scope_Struct);
    parser->scope = scope;

    for (;;)
    {
        if (parser->token.kind == RiToken_RB) {
            if (!ri_lex_next_(parser)) {
                return NULL;
            }
            break;
        }

        // TODO: type X ...;

        RiToken token_id = parser->token;
        RiNode* expr_id = ri_parse_expr_(parser);
        if (!expr_id) {
            return NULL;
        }

        String id = RI_ID_NULL;
        RiNode* expr_type = NULL;
        if (parser->token.kind != RiToken_Semicolon) {
            // No semicolons expr1 should follow.
            expr_type = ri_parse_expr_(parser);
            if (!expr_type) {
                return NULL;
            }
            // If we have type, we expect the expr_id to be just id.
            // TODO: In future we'll need to be able to have an actual node as decl.id, since now it is only a String.
            if (expr_id->kind != RiNode_Expr_Id) {
                ri_error_set_unexpected_token_(parser, &token_id);
                return NULL;
            }
            RI_CHECK(expr_id->kind == RiNode_Expr_Id);
            id = expr_id->id.name;
        } else {
            expr_type = expr_id;
            expr_id = NULL;
            switch (expr_type->kind) {
                case RiNode_Expr_Id:
                    id = expr_type->id.name;
                    break;
                case RiNode_Expr_Select:
                    RI_CHECK(expr_type->binary.argument1->kind == RiNode_Expr_Id);
                    id = expr_type->binary.argument1->id.name;
                    break;
                case RiNode_Spec_Type_Struct:
                    // Nested unnamed struct.
                    break;
                default:
                    RI_UNREACHABLE;
                    break;
            }
        }

        if (id.items != NULL) {
            RiNode* field = ri_make_spec_var_(parser->arena, parser->scope, expr_type->pos, expr_type, RiVar_Field);
            RiNode* decl = ri_make_decl_(parser->arena, parser->scope, expr_id->pos, id, field);
            if (!ri_scope_set_(parser->ri, scope, decl)) {
                return NULL;
            }
            array_push(&scope->scope.statements, decl);
        } else {
            array_push(&scope->scope.statements, expr_type);
        }

        if (!ri_lex_expect_token_(parser, RiToken_Semicolon)) {
            return NULL;
        }
        if (!ri_parse_semicolons_(parser)) {
            return NULL;
        }
    }

    RI_CHECK(parser->scope == scope);
    parser->scope = scope->owner;

    RiNode* spec = ri_make_spec_type_struct_(parser->arena, parser->scope, pos_struct, scope);
    return spec;
}

//
// Declarations
//

static RiNode*
ri_parse_decl_type_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);
    RI_CHECK(parser->token.kind == RiToken_Keyword_Type);
    RiPos pos = parser->token.pos;
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    String id = parser->token.id;
    if (!ri_lex_expect_token_(parser, RiToken_Identifier)) {
        return NULL;
    }

    RiNode* type = ri_parse_type_(parser);
    if (!type) {
        return NULL;
    }

    RiNode* decl = ri_make_decl_(parser->arena, parser->scope, pos, id, type);
    return decl;
}


static RiNode*
ri_parse_decl_import_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RI_CHECK(parser->token.kind == RiToken_Keyword_Import);
    RiPos pos_decl = parser->token.pos;
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiToken string = parser->token;
    if (!ri_lex_expect_token_(parser, RiToken_String)) {
        return NULL;
    }

    // TODO: Make this more efficient?
    CharArray buffer = {0};
    ri_parse_string_(string, &buffer);
    String id = ri_make_id_(parser->ri, buffer.slice);
    array_purge(&buffer);

    RiNode* spec = ri_make_spec_module_(parser->arena, parser->scope, string.pos);
    RiNode* decl = ri_make_decl_(parser->arena, parser->scope, pos_decl, id, spec);
    return decl;
}

static RiNode*
ri_parse_decl_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiNode* node = NULL;
    switch (parser->token.kind)
    {
        // TODO: Type is not expected here, only function declaration.
        case RiToken_Keyword_Func: node = ri_parse_spec_func_or_func_type_(parser); break;
        case RiToken_Keyword_Variable: node = ri_parse_decl_variable_(parser); break;
        case RiToken_Keyword_Type: node = ri_parse_decl_type_(parser); break;
        case RiToken_Keyword_Import: node = ri_parse_decl_import_(parser); break;
        default:
            ri_error_set_unexpected_token_(parser, &parser->token);
            break;
    }

    if (node) {
        if (!ri_scope_set_(parser->ri, parser->scope, node)) {
            return NULL;
        }
    }
    return node;
}

//
// Statements parser.
//

static RiNode* ri_parse_st_(RiParser* parser, RiNodeKind scope_kind);
static RiNode* ri_parse_st_simple_(RiParser* parser);

static RiNode*
ri_parse_st_expr_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiNode* expr = ri_parse_expr_(parser);
    if (!expr) {
        return NULL;
    }

    RiPos pos = parser->token.pos;
    RiTokenKind token_kind = parser->token.kind;
    if (ri_tokenkind_in(token_kind, RiToken_Assign)) {
        if (!ri_lex_next_(parser)) {
            return NULL;
        }
        RiNode* right = ri_parse_expr_(parser);
        if (!right) {
            return NULL;
        }
        return ri_make_st_assign_(parser->arena, parser->scope, pos, RI_TOKEN_TO_OP_[token_kind].assign, expr, right);
    } else {
        return ri_make_st_expr_(parser->arena, parser->scope, expr->pos, expr);
    }
}

static RiNode*
ri_parse_st_return_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiPos pos = parser->token.pos;
    RI_CHECK(parser->token.kind == RiToken_Keyword_Return);
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiNode* argument = NULL;
    if (parser->token.kind != RiToken_Semicolon) {
        argument = ri_parse_expr_(parser);
        if (!argument) {
            return NULL;
        }
    }

    if (!ri_lex_expect_token_(parser, RiToken_Semicolon)) {
        return NULL;
    }

    RiNode* statement = ri_make_st_return_(parser->arena, parser->scope, pos, argument);
    return statement;
}

static RiNode*
ri_parse_st_if_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiPos pos = parser->token.pos;
    RI_CHECK(parser->token.kind == RiToken_Keyword_If);
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiNode* scope = ri_make_scope_(parser->arena, parser->scope, pos, RiNode_Scope_If_Root);
    parser->scope = scope;

    RiNode* condition = NULL;
    RiNode* pre = ri_parse_st_simple_(parser);
    if (!pre) {
        return NULL;
    }

    switch (ri_lex_next_if_(parser, RiToken_Semicolon))
    {
        case RiLexNextIf_Match: {
            condition = ri_parse_expr_(parser);
            if (!condition) {
                return NULL;
            }
        } break;

        case RiLexNextIf_NoMatch: {
            condition = pre;
            if (condition->kind != RiNode_St_Expr) {
                ri_error_set_(parser->ri, RiError_UnexpectedStatement, condition->pos, "conditional expression expected");
                return NULL;
            }
            condition = condition->st_expr;
            RI_CHECK(ri_is_expr_like(condition->kind));
            pre = NULL;
        } break;

        case RiLexNextIf_Error:
            return NULL;
    }


    if (!ri_lex_expect_token_(parser, RiToken_LB)) {
        return NULL;
    }

    RiNode* scope_else = NULL;
    RiNode* scope_then = ri_parse_scope_(parser, RiToken_RB, RiNode_Scope_If_Body);
    if (!scope_then) {
        return NULL;
    }
    array_push(&scope->scope.statements, scope_then);

    switch (ri_lex_next_if_(parser, RiToken_Keyword_Else))
    {
        case RiLexNextIf_Match:
            if (parser->token.kind == RiToken_Keyword_If) {
                RiNode* else_if = ri_parse_st_if_(parser);
                if (!else_if) {
                    return NULL;
                }
                array_push(&scope->scope.statements, else_if);
            } else {
                if (!ri_lex_expect_token_(parser, RiToken_LB)) {
                    return NULL;
                }
                scope_else = ri_parse_scope_(parser, RiToken_RB, RiNode_Scope_If_Body);
                if (!scope_else) {
                    return NULL;
                }
                array_push(&scope->scope.statements, scope_else);
            }
            break;
        case RiLexNextIf_Error:
            return NULL;
    }

    RI_CHECK(parser->scope == scope);
    parser->scope = scope->owner;

    RiNode* node = ri_make_st_if_(parser->arena, parser->scope, pos, pre, condition, scope);
    return node;
}

static RiNode*
ri_parse_st_for_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);
    RiPos pos = parser->token.pos;
    RI_CHECK(parser->token.kind == RiToken_Keyword_For);
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiNode* scope = ri_make_scope_(parser->arena, parser->scope, pos, RiNode_Scope_For_Root);
    parser->scope = scope;

    RiNode* pre = NULL;
    RiNode* condition = NULL;
    RiNode* post = NULL;
    RiNode* scope_block = NULL;

    if (ri_lex_next_if_(parser, RiToken_LB) == RiLexNextIf_NoMatch) {
        // for pre?; condition?; post? { ... }
        if (ri_lex_next_if_(parser, RiToken_Semicolon) == RiLexNextIf_NoMatch) {
            pre = ri_parse_st_simple_(parser);
            if (!pre) {
                return NULL;
            }
            if (ri_lex_next_if_(parser, RiToken_Semicolon) == RiLexNextIf_NoMatch) {
                // for condition { ... }
                condition = pre;
                if (condition->kind != RiNode_St_Expr) {
                    ri_error_set_(parser->ri, RiError_UnexpectedStatement, condition->pos, "conditional expression expected");
                    return NULL;
                }
                condition = condition->st_expr;
                RI_CHECK(ri_is_expr_like(condition->kind));
                pre = NULL;
                goto skip;
            }
        }

        if (parser->ri->error.kind) {
            return NULL;
        }

        // for ; condition?; post? { ... }
        if (ri_lex_next_if_(parser, RiToken_Semicolon) == RiLexNextIf_NoMatch) {
            // for ; condition; post?
            condition = ri_parse_expr_(parser);
            if (!condition) {
                return NULL;
            }
            if (!ri_lex_expect_token_(parser, RiToken_Semicolon)) {
                return NULL;
            }
        } else {
            // for ; ; post?
        }

        if (parser->ri->error.kind) {
            return NULL;
        }

        if (parser->token.kind != RiToken_LB) {
            // for ; condition?; post { ... }
            post = ri_parse_st_simple_(parser);
            if (!post) {
                return NULL;
            }
        } else {
            // for ; condition?; { ... }
        }

skip:;
        if (!ri_lex_expect_token_(parser, RiToken_LB)) {
            return NULL;
        }
    }


    scope_block = ri_parse_scope_(parser, RiToken_RB, RiNode_Scope_For_Body);
    if (!scope_block) {
        return NULL;
    }
    array_push(&scope->scope.statements, scope_block);

    RI_CHECK(parser->scope == scope);
    parser->scope = scope->owner;

    RiNode* node = ri_make_st_for_(parser->arena, parser->scope, pos, pre, condition, post, scope);
    return node;
}

static RiNode*
ri_parse_st_switch_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiPos pos = parser->token.pos;
    RI_CHECK(parser->token.kind == RiToken_Keyword_Switch);
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiNode* scope = ri_make_scope_(parser->arena, parser->scope, pos, RiNode_Scope_Switch_Root);
    parser->scope = scope;

    RiNode* pre = NULL;
    RiNode* condition = NULL;

    if (ri_lex_next_if_(parser, RiToken_Semicolon) == RiLexNextIf_NoMatch)
    {
        // switch pre; condition
        // switch condition
        pre = ri_parse_st_simple_(parser);
        if (!pre) {
            return NULL;
        }

        if (ri_lex_next_if_(parser, RiToken_Semicolon) == RiLexNextIf_NoMatch)
        {
            // switch condition { ... }
            condition = pre;
            if (condition->kind != RiNode_St_Expr) {
                ri_error_set_(parser->ri, RiError_UnexpectedStatement, condition->pos, "conditional expression expected");
                return NULL;
            }
            condition = condition->st_expr;
            RI_CHECK(ri_is_expr_like(condition->kind));
            pre = NULL;
            goto skip;
        }
    }

    if (parser->ri->error.kind) {
        return NULL;
    }

    condition = ri_parse_expr_(parser);
    if (!condition) {
        return NULL;
    }

skip:;

    if (!ri_lex_expect_token_(parser, RiToken_LB)) {
        return NULL;
    }

    RiNode* scope_block = ri_parse_scope_(parser, RiToken_RB, RiNode_Scope_Switch_Body);
    if (!scope_block) {
        return NULL;
    }

    array_push(&scope->scope.statements, scope_block);

    RI_CHECK(parser->scope == scope);
    parser->scope = scope->owner;

    RiNode* node = ri_make_st_switch_(parser->arena, parser->scope, pos, pre, condition, scope);
    return node;
}

static RiNode*
ri_parse_st_switch_case_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiPos pos = parser->token.pos;
    RI_CHECK(parser->token.kind == RiToken_Keyword_Case);
    if (!ri_lex_next_(parser)) {
        return NULL;
    }

    RiNode* expr = ri_parse_expr_(parser);
    if (!ri_lex_expect_token_(parser, RiToken_Colon)) {
        return NULL;
    }

    RiNode* node = ri_make_st_switch_case_(parser->arena, parser->scope, pos, expr);
    return node;
}

static RiNode*
ri_parse_st_switch_default_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiPos pos = parser->token.pos;
    RI_CHECK(parser->token.kind == RiToken_Keyword_Default);
    if (!ri_lex_next_(parser)) {
        return NULL;
    }
    if (!ri_lex_expect_token_(parser, RiToken_Colon)) {
        return NULL;
    }

    RiNode* node = ri_make_st_switch_default_(parser->arena, parser->scope, pos);
    return node;
}

static RiNode*
ri_parse_st_simple_(RiParser* parser)
{
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiPos pos;
    RiNode* node = NULL;
    switch (parser->token.kind)
    {
        case RiToken_Keyword_Variable:
            node = ri_parse_decl_(parser);
            if (!node) {
                return NULL;
            }

            // Assignment.
            pos = parser->token.pos;
            switch (ri_lex_next_if_(parser, RiToken_Eq))
            {
                case RiLexNextIf_Match: {
                    RiNode* right = ri_parse_expr_(parser);
                    if (right) {
                        node = ri_make_st_assign_(parser->arena, parser->scope, pos, RiNode_St_Assign, node, right);
                    } else {
                        node = NULL;
                    }
                } break;
                // case RiLexNextIf_NoMatch: {
                //     if (node->decl.spec->spec.var.type->kind == RiNode_Spec_Type_Infer) {
                //         ri_error_set_unexpected_token_(parser, &parser->token);
                //         return NULL;
                //     }
                // }
                case RiLexNextIf_Error:
                    node = NULL;
                    break;
            }
            break;

        default:
            node = ri_parse_st_expr_(parser);
            break;
    }

    return node;
}


static RiNode*
ri_parse_st_(RiParser* parser, RiNodeKind scope_kind)
{
    RI_CHECK(ri_is_in(scope_kind, RiNode_Scope));
    RI_CHECK(parser->ri->error.kind == RiError_None);

    RiNode* node = NULL;
    switch (parser->token.kind)
    {
        case RiToken_Keyword_Import:
        case RiToken_Keyword_Type:
            node = ri_parse_decl_(parser);
            if (!node) {
                return NULL;
            }
            if (ri_lex_next_if_(parser, RiToken_Semicolon) == RiLexNextIf_Error) {
                return NULL;
            }
            break;

        case RiToken_Keyword_Func:
            node = ri_parse_decl_(parser);
            if (!node) {
                return NULL;
            }
            RI_CHECK(node->kind == RiNode_Decl);
            if (node->decl.spec->kind == RiNode_Spec_Func) {
                if (ri_lex_next_if_(parser, RiToken_Semicolon) == RiLexNextIf_Error) {
                    return NULL;
                }
            } else if (!ri_lex_expect_token_(parser, RiToken_Semicolon)) {
                return NULL;
            }
            break;

        case RiToken_Keyword_Return:
            node = ri_parse_st_return_(parser);
            break;

        case RiToken_Keyword_If:
            node = ri_parse_st_if_(parser);
            break;

        case RiToken_Keyword_For:
            node = ri_parse_st_for_(parser);
            break;

        case RiToken_Keyword_Switch:
            node = ri_parse_st_switch_(parser);
            break;

        case RiToken_Keyword_Case:
            if (scope_kind != RiNode_Scope_Switch_Body) {
                ri_error_set_unexpected_token_(parser, &parser->token);
                return NULL;
            }
            node = ri_parse_st_switch_case_(parser);
            break;

        case RiToken_Keyword_Default:
            if (scope_kind != RiNode_Scope_Switch_Body) {
                ri_error_set_unexpected_token_(parser, &parser->token);
                return NULL;
            }
            node = ri_parse_st_switch_default_(parser);
            break;

        case RiToken_Keyword_Break:
            if (scope_kind != RiNode_Scope_Switch_Body && scope_kind != RiNode_Scope_For_Body) {
                ri_error_set_unexpected_token_(parser, &parser->token);
                return NULL;
            }
            RiPos pos = parser->token.pos;
            if (ri_lex_next_(parser)) {
                node = ri_make_st_break_(parser->arena, parser->scope, pos);
                if (ri_lex_expect_token_(parser, RiToken_Semicolon)) {
                    break;
                }
            }
            return NULL;

        // TODO: Merge with `break`?
        // TODO: Continue/break is allowed if one of the parents is switch or for.
        // - We can do this with counted allow_break++, allow_continue++.
        // - We will also need to assign break and continue with the correct parent.
        case RiToken_Keyword_Continue: {
            if (scope_kind != RiNode_Scope_For_Body) {
                ri_error_set_unexpected_token_(parser, &parser->token);
                return NULL;
            }
            RiPos pos = parser->token.pos;
            if (ri_lex_next_(parser)) {
                node = ri_make_st_continue_(parser->arena, parser->scope, pos);
                if (ri_lex_expect_token_(parser, RiToken_Semicolon)) {
                    break;
                }
            }
        } return NULL;

        default:
            node = ri_parse_st_simple_(parser);
            if (!node || !ri_lex_expect_token_(parser, RiToken_Semicolon)) {
                return NULL;
            }
            break;
    }

    // RI_CHECK(node);
    return node;
}

static RiNode*
ri_parse_scope_(RiParser* parser, RiTokenKind end, RiNodeKind scope_kind)
{
    RI_CHECK(ri_is_in(scope_kind, RiNode_Scope));
    RiNode* scope = ri_make_scope_(parser->arena, parser->scope, parser->token.pos, scope_kind);
    parser->scope = scope;
    while (parser->token.kind != end) {
        RiNode* statement = ri_parse_st_(parser, scope_kind);
        if (statement == NULL) {
            return NULL;
        }
        array_push(&parser->scope->scope.statements, statement);
    }

    if (end == RiToken_RB && !ri_lex_next_(parser)) {
        return NULL;
    }
    RI_ASSERT(parser->scope == scope);
    parser->scope = scope->owner;

    return scope;
}

//
//
//

RiNode*
ri_parse_(Ri* ri, ByteSlice stream)
{
    RiParser parser = {
        .ri = ri,
        .arena = &ri->module->arena,
        .stream.start = stream.items,
        .stream.end = stream.items + stream.count,
        .stream.it = stream.items,
        .stream.line = stream.items,
        .scope = ri->scope,
    };

    if (!ri_lex_next_(&parser)) {
        return NULL;
    }

    return ri_parse_scope_(&parser, RiToken_End, RiNode_Scope_Module);
}