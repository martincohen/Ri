#include "ri.h"
#include "ri-print.c"

#define RI_TODO FAIL("todo")
#define RI_POS_OUTSIDE (RiPos){ -1, -1 }

static const struct {
    RiNodeKind unary;
    RiNodeKind binary;
    RiNodeKind assign;
} RI_TOKEN_TO_OP_[RiToken_COUNT__] = {
    [RiToken_Dot]           = { .binary = RiNode_Expr_Binary_Select },
    [RiToken_Plus]          = { .unary = RiNode_Expr_Unary_Positive,
                                .binary = RiNode_Expr_Binary_Add },
    [RiToken_Minus]         = { .unary = RiNode_Expr_Unary_Negative,
                                .binary = RiNode_Expr_Binary_Sub },
    [RiToken_Star]          = { .binary = RiNode_Expr_Binary_Mul },
    [RiToken_Slash]         = { .binary = RiNode_Expr_Binary_Div },
    [RiToken_Percent]       = { .binary = RiNode_Expr_Binary_Mod },

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
                                .binary = RiNode_Expr_Binary_BAnd },
    [RiToken_Pipe]          = { .binary = RiNode_Expr_Binary_BOr },
    [RiToken_Beak]          = { .binary = RiNode_Expr_Binary_BXor },
    [RiToken_Tilde]         = { .unary = RiNode_Expr_Unary_BNeg },
    [RiToken_LtLt]          = { .binary = RiNode_Expr_Binary_BShL },
    [RiToken_GtGt]          = { .binary = RiNode_Expr_Binary_BShR },

    [RiToken_Bang]          = { .unary = RiNode_Expr_Unary_Not },
    [RiToken_AmpAmp]        = { .binary = RiNode_Expr_Binary_And },
    [RiToken_PipePipe]      = { .binary = RiNode_Expr_Binary_Or },
};

//
//
//

inline String ri_make_id_0(Ri* ri, char* start, char* end);
inline String ri_make_id_(Ri* ri, String string);

static inline void*
ri_push_t__(Ri* ri, iptr size, iptr align) {
    void* p = arena_push(&ri->arena, size, align);
    memset(p, 0, size);
    return p;
}

#define ri_push_t_(Ri, Type) \
    ((Type*)ri_push_t__(Ri, SIZEOF(Type), ALIGNOF(Type)));

static inline void*
ri_push_slice__(Ri* ri, void* ptr, iptr count, iptr item_size) {
    void* p = arena_push(&ri->arena, count * item_size, 8);
    memcpy(p, ptr, count * item_size);
    return p;
}

#define ri_push_slice_(Ri, SliceType, Slice) \
    ((SliceType){ \
        .items = ri_push_slice__(Ri, (Slice).items, (Slice).count, sizeof((Slice).items[0])), \
        .count = (Slice).count \
    }) \

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
    RI_LOG("error source.ri:%d:%d: %S", pos.row + 1, pos.col + 1, ri->error.message.slice);
    __debugbreak();
}

static inline void
ri_error_set_unexpected_token_(Ri* ri, RiToken* token)
{
    ri_error_set_(ri, RiError_UnexpectedToken, token->pos, "unexpected token %d", token->kind);
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

static inline int
ri_rune_is_iden_(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static inline int
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
        case '.': ++it; token->kind = RiToken_Dot; break;
        case ',': ++it; token->kind = RiToken_Comma; break;
        case ';': ++it; token->kind = RiToken_Semicolon; break;

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
            token->kind = RiToken_Plus;
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
            while (it < end && ri_rune_is_iden_(*it)) {
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
            }
        } break;

        default: {
            ri_error_set_(ri, RiError_UnexpectedCharacter, ri->token.pos, "unexpected character '%c' (%d)", *it, *it);
            return false;
        } break;
    }

    token->end = it;
    stream->it = it;

    // RI_LOG_DEBUG("'%.*s'", token->end - token->start, token->start);

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

static bool
ri_lex_next_if_(Ri* ri, RiTokenKind expected_kind)
{
    ri_error_check_(ri);

    if (ri->token.kind != expected_kind) {
        return false;
    }

    if (!ri_lex_next_(ri)) {
        return false;
    }

    return true;
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

    return ri_lex_next_(ri);
}

//
//
//

static RiNode*
ri_complete_type_(Ri* ri, RiPos pos, RiNode* type)
{
    ri_error_check_(ri);
    RI_CHECK(type);
    RI_CHECK(ri_is_in(type->kind, RiNode_Type));

    if (type->type.completeness == RiType_Completing) {
        ri_error_set_(ri, RiError_CyclicType, pos, "cyclic type completion");
        return 0;
    } else if (type->type.completeness == RiType_Completed) {
        return type;
    }
    RI_CHECK(type->type.completeness == RiType_Incomplete);
    type->type.completeness = RiType_Completing;
    switch (type->kind)
    {
        case RiNode_Type_Struct: {
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
            ri_error_set_(ri, RiError_CompletingType, pos, "could not complete type");
            return 0;
    }
    type->type.completeness = RiType_Completed;

    return type;
}


static RiNode*
ri_retof_(Ri* ri, RiNode* node)
{
    ri_error_check_(ri);
    RI_CHECK(ri_is_in(node->kind, RiNode_Expr));

    // RI_ASSERT(ri_is_rt(node));
    switch (node->kind)
    {
        case RiNode_Expr_Literal_Integer:
            // TODO: i32?
            return ri->node_meta[RiNode_Type_Number_UInt64].node;
        case RiNode_Expr_Literal_Real:
            // TODO: f32?
            return ri->node_meta[RiNode_Type_Number_Float64].node;
        case RiNode_Expr_Binary_Cast:
            return node->binary.argument0;
        case RiNode_Expr_Variable:
            return node->expr_variable.type;
        case RiNode_Expr_Call:
            // if (ri_is_op_function(node->call.callable)) {
            //     return node->call.callable->function.op.ret;
            // } else if (ri_is_rt_function(node->call.callable)) {
            //     return node->call.callable->function.type->type.function.ret;
            // }
            RI_CHECK(node->expr_call.func);
            RI_CHECK(node->expr_call.func->kind == RiNode_Func);
            RI_CHECK(node->expr_call.func->func.type);
            RI_CHECK(node->expr_call.func->func.type->kind == RiNode_Type_Func);
            RI_CHECK(node->expr_call.func->func.type->type.func.outputs.count == 1);
            return array_at(&node->expr_call.func->func.type->type.func.outputs, 0);

        default:
            if (ri_is_in(node->kind, RiNode_Expr_Unary)) {
                return ri_retof_(ri, node->unary.argument);
            // } else if (ri_is_in(node->kind, RiNode_Expr_Binary)) {
            //     RI_ABORT("todo");
            }

        // TODO:
        // case RiNode_Field:
        //     RI_CHECK(expr->field.child);
        //     return ri_retof_(ri, expr->field.child);
    }
    RI_ABORT("unknown expr type");
    return 0;
}

static uint64_t
ri_sizeof_(Ri* ri, RiPos pos, RiNode* type)
{
    switch (type->kind)
    {
        case RiNode_Type_Pointer:
        case RiNode_Type_Number_Int64:
        case RiNode_Type_Number_UInt64:
        case RiNode_Type_Number_Float64:
            return 8;
        case RiNode_Type_Number_Enum:
        case RiNode_Type_Number_Int32:
        case RiNode_Type_Number_UInt32:
        case RiNode_Type_Number_Float32:
            return 4;
        case RiNode_Type_Number_Int16:
        case RiNode_Type_Number_UInt16:
            return 2;
        case RiNode_Type_Number_Int8:
        case RiNode_Type_Number_UInt8:
            return 1;
        case RiNode_Type_Struct:
        case RiNode_Type_Union:
            type = ri_complete_type_(ri, pos, type);
            return type->type.compound.size;
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
        case RiNode_Type_Struct:
        case RiNode_Type_Union:
        case RiNode_Type_Pointer:
        case RiNode_Type_Number_Int64:
        case RiNode_Type_Number_UInt64:
        case RiNode_Type_Number_Float64:
            return 8;
        case RiNode_Type_Number_Enum:
        case RiNode_Type_Number_Int32:
        case RiNode_Type_Number_UInt32:
        case RiNode_Type_Number_Float32:
            return 4;
        case RiNode_Type_Number_Int16:
        case RiNode_Type_Number_UInt16:
            return 2;
        case RiNode_Type_Number_Int8:
        case RiNode_Type_Number_UInt8:
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
    RiNode* node = ri_push_t_(ri, RiNode);
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
ri_make_decl_ref_(Ri* ri, RiPos pos, RiNode* decl)
{
    // TODO: Checks.
    RiNode* node = ri_make_node_(ri, pos, RiNode_Ref);
    node->ref = decl;
    return node;
}

static RiNode*
ri_make_decl_variable_(Ri* ri, RiPos pos, String id, RiNode* type) {
    RI_CHECK(id.items);
    RI_CHECK(id.count);
    RI_CHECK(type);
    RI_CHECK(type->kind == RiNode_Expr_Identifier);
    // RI_CHECK(ri_is_in(type->kind, RiNode_Type));

    RiNode* node = ri_make_node_(ri, pos, RiNode_Decl_Variable);
    node->decl.id = id;
    node->decl.variable.type = type;
    return node;
}

static RiNode*
ri_make_decl_type_func_arg_(Ri* ri, RiPos pos, String id, RiNode* type)
{
    RI_CHECK(id.items);
    RI_CHECK(id.count);
    RI_CHECK(type);
    RI_CHECK(type->kind == RiNode_Expr_Identifier);
    // RI_CHECK(ri_is_in(type->kind, RiNode_Identifier));

    RiNode* node = ri_make_node_(ri, pos, RiNode_Decl_Type_Func_Arg);
    node->decl.id = id;
    node->decl.type_func_arg.type = type;
    return node;
}

static RiNode*
ri_make_decl_type_func_(Ri* ri, RiPos pos, RiNodeArray inputs, RiNodeArray outputs) {
    RiNode* node = ri_make_node_(ri, pos, RiNode_Type_Func);
    node->decl.type_func.inputs = inputs;
    node->decl.type_func.outputs = outputs;
    return node;
}

static RiNode*
ri_make_decl_type_number_(Ri* ri, RiPos pos, String id, RiNodeKind kind)
{
    RI_CHECK(ri_is_in(kind, RiNode_Type_Number));
    RiNode* node = ri_make_node_(ri, pos, kind);
    node->decl.id = id;
    node->decl.state = RiDecl_Resolved;
    return node;
}

static RiNode*
ri_make_decl_func_(Ri* ri, RiPos pos, String id, RiNode* type, RiNode* scope)
{
    RI_CHECK(type);
    RI_CHECK(type->kind == RiNode_Type_Func);
    RI_CHECK(scope);
    RI_CHECK(scope->kind == RiNode_Scope);

    RiNode* node = ri_make_node_(ri, pos, RiNode_Decl_Func);
    node->decl.id = id;
    node->decl.func.type = type;
    node->decl.func.scope = scope;
    return node;
}

static RiNode*
ri_make_func_(Ri* ri, RiPos pos, RiNode* decl)
{
    RI_CHECK(decl);
    RI_CHECK(decl->kind == RiNode_Decl_Func);

    RiNode* node = ri_make_node_(ri, pos, RiNode_Func);
    node->func.decl = decl;
    node->func.id = decl->decl.id;
    node->func.type = decl->decl.func.type;
    node->func.scope = decl->decl.func.scope;
    return node;
}

static RiNode*
ri_make_expr_identifier_(Ri* ri, RiPos pos, String id)
{
    RiNode* node = ri_make_node_(ri, pos, RiNode_Expr_Identifier);
    node->expr_id = id;
    return node;
}

static RiNode*
ri_make_expr_variable_(Ri* ri, RiPos pos, RiNode* decl)
{
    RI_CHECK(decl);
    RI_CHECK(decl->kind == RiNode_Decl_Variable);

    RiNode* node = ri_make_node_(ri, pos, RiNode_Expr_Variable);
    node->expr_variable.decl = decl;
    node->expr_variable.id = decl->decl.id;
    node->expr_variable.type = decl->decl.variable.type;
    return node;
}

static RiNode*
ri_make_expr_variable_from_arg_decl_(Ri* ri, RiPos pos, RiNode* decl)
{
    RI_CHECK(decl);
    RI_CHECK(decl->kind == RiNode_Decl_Type_Func_Arg);

    RiNode* node = ri_make_node_(ri, pos, RiNode_Expr_Variable);
    node->expr_variable.decl = decl;
    node->expr_variable.id = decl->decl.id;
    node->expr_variable.type = decl->decl.type_func_arg.type;
    return node;
}

static RiNode*
ri_make_expr_call_(Ri* ri, RiPos pos, RiNode* func)
{
    RI_CHECK(func);
    // TODO: Check func node type?

    RiNode* node = ri_make_node_(ri, pos, RiNode_Expr_Call);
    node->expr_call.func = func;
    return node;
}

static RiNode*
ri_make_expr_binary_(Ri* ri, RiPos pos, RiNodeKind kind, RiNode* argument0, RiNode* argument1)
{
    RI_CHECK(ri_is_in(kind, RiNode_Expr_Binary));
    RI_CHECK(argument0);
    RI_CHECK(ri_is_in(argument0->kind, RiNode_Expr));
    RI_CHECK(argument1);
    RI_CHECK(ri_is_in(argument1->kind, RiNode_Expr));

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
    RI_CHECK(ri_is_in(argument->kind, RiNode_Expr));

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
        RiNode* node = ri_make_node_(ri, pos, RiNode_Expr_Binary_Cast);
        node->binary.argument0 = type_to;
        node->binary.argument1 = expr;
        return node;
    }
    return expr;
}

static RiNode*
ri_make_st_assign_(Ri *ri, RiPos pos, RiNodeKind kind, RiNode* argument0, RiNode* argument1)
{
    RI_CHECK(ri_is_in(kind, RiNode_St_Assign));
    RI_CHECK(argument0);
    RI_CHECK(
        ri_is_in(argument0->kind, RiNode_Expr) ||
        (argument0->kind == RiNode_Ref && argument0->ref->kind == RiNode_Decl_Variable)
    );
    RI_CHECK(argument1);
    RI_CHECK(ri_is_in(argument1->kind, RiNode_Expr));

    RiNode* node = ri_make_node_(ri, pos, kind);
    node->binary.argument0 = argument0;
    node->binary.argument1 = argument1;
    return node;
}

static RiNode*
ri_make_st_expr_(Ri* ri, RiPos pos, RiNode* expr)
{
    RI_CHECK(ri_is_in(expr->kind, RiNode_Expr));

    RiNode* node = ri_make_node_(ri, pos, RiNode_St_Expr);
    node->st_expr = expr;
    return node;
}

static RiNode*
ri_make_st_return_(Ri* ri, RiPos pos, RiNode* argument)
{
    RI_CHECK(argument == NULL || ri_is_in(argument->kind, RiNode_Expr));
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

//
//
//

static bool
ri_scope_set_(Ri* ri, RiNode* decl)
{
    ValueScalar id_ = { .ptr = decl->decl.id.items };
    RiNode* decl_found = map_get(&ri->scope->scope.map, id_).ptr;
    if (decl_found != NULL) {
        ri_error_set_(ri, RiError_Declared, decl->pos, "'%S' is already declared", decl->decl.id);
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
                return ri_make_expr_identifier_(ri, token.pos, token.id);
            }
            return NULL;
        } break;

        // case RiToken_Integer: {
        //     RiNode* node = ri_make_node(ri, ri->token.pos, RiNode_Literal_Integer);
        //     node->literal.integer = ri->token.integer;
        //     if (!ri_lex_next_(ri)) {
        //         return NULL;
        //     }
        //     return node;
        // } break;

        // TODO: case RiToken_Real:
        // TODO: case RiToken_String:

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

bool
ri_parse_expr_call_arguments_(Ri* ri, RiNode* call)
{
    while (!ri_lex_next_if_(ri, RiToken_RP)) {
        RiNode* expr = ri_parse_expr_(ri);
        if (!expr) {
            return false;
        }
        array_push(&call->expr_call.arguments, expr);
        ri_lex_next_if_(ri, RiToken_Comma);
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
static RiNode* ri_parse_scope_(Ri* ri, RiTokenKind end);

static RiNode*
ri_parse_type_(Ri* ri)
{
    ri_error_check_(ri);

    RiToken token = ri->token;
    if (!ri_lex_expect_token_(ri, RiToken_Identifier)) {
        return NULL;
    }

    RiNode* decl = ri_make_expr_identifier_(ri, token.pos, token.id);
    return decl;
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

    RiNode* type = ri_parse_type_(ri);
    if (!type) {
        return NULL;
    }
    RiNode* decl = ri_make_decl_variable_(ri, token.pos, token.id, type);
    return decl;
}

static RiNode*
ri_parse_decl_type_func_arg_(Ri* ri)
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

    RiNode* node = ri_make_decl_type_func_arg_(ri, token.pos, token.id, type);
    return node;
}

static bool
ri_parse_decl_type_func_arg_list_(Ri* ri, RiNodeArray* list)
{
    if (!ri_lex_expect_token_(ri, RiToken_LP)) {
        return false;
    }

    if (ri->token.kind != RiToken_RP)
    {
        RiNode* decl_arg;
        while (ri->token.kind != RiToken_RP)
        {
            decl_arg = ri_parse_decl_type_func_arg_(ri);
            if (!decl_arg) {
                return false;
            }
            array_push(list, decl_arg);
            ri_lex_next_if_(ri, RiToken_Comma);
        }
        ri_lex_next_(ri);
    }
    return true;
}

static RiNode*
ri_parse_decl_partial_func_type_(Ri* ri, RiPos pos)
{
    ri_error_check_(ri);

    RiNodeArray inputs = {0};
    RiNodeArray outputs = {0};
    if (!ri_parse_decl_type_func_arg_list_(ri, &inputs)) {
        return NULL;
    }
    // TODO: (...)
    // TODO: <name> <type-spec>
    if (!ri_parse_decl_type_func_arg_list_(ri, &outputs)) {
        return NULL;
    }

    RiNode* type = ri_make_decl_type_func_(ri, pos, inputs, outputs);
    return type;
}

static RiNode*
ri_parse_decl_partial_func_(Ri* ri, RiPos pos, String id)
{
    ri_error_check_(ri);
    RI_CHECK(id.items);
    RI_CHECK(id.count);

    // NOTE: Starting and setting scope here, so all the nodes
    // created within have owner set to the scope.
    RiNode* scope = ri_make_scope_(ri, pos);
    ri->scope = scope;
    RI_LOG_DEBUG("function '%S' scope %d", id, scope->index);

    RiNode* type = ri_parse_decl_partial_func_type_(ri, pos);
    if (!type) {
        return NULL;
    }

    RiNode* scope_body = NULL;
    if (ri_lex_next_if_(ri, RiToken_LB))
    {
        scope_body = ri_parse_scope_(ri, RiToken_RB);
        if (!scope_body) {
            return NULL;
        }

        // TODO: Make parsing arguments actually generate variable decls, so
        // this won't be needed.

        RiNode* it;
        array_each(&type->decl.type_func.inputs, &it) {
            map_put(&scope->scope.map,
                (ValueScalar){ .ptr = it->decl.id.items },
                (ValueScalar){ .ptr = it }
            );
        }
        array_each(&type->decl.type_func.outputs, &it) {
            map_put(&scope->scope.map,
                (ValueScalar){ .ptr = it->decl.id.items },
                (ValueScalar){ .ptr = it }
            );
        }
        array_push(&scope->scope.statements, scope_body);
    }

    RI_CHECK(ri->scope == scope);
    ri->scope = scope->owner;

    RiNode* func = ri_make_decl_func_(
        ri, pos, id, type, scope_body ? scope : NULL);
    return func;
}

static RiNode*
ri_parse_decl_func_or_func_type_(Ri* ri)
{
    ri_error_check_(ri);
    RI_CHECK(ri->token.kind == RiToken_Keyword_Func);
    RiPos pos = ri->token.pos;
    if (!ri_lex_next_(ri)) {
        return NULL;
    }

    RiNode* node = NULL;
    String id = ri->token.id;
    if (ri_lex_next_if_(ri, RiToken_Identifier)) {
        node = ri_parse_decl_partial_func_(ri, pos, id);
    } else {
        node = ri_parse_decl_partial_func_type_(ri, pos);
    }

    return node;
}

static RiNode*
ri_parse_decl_(Ri* ri)
{
    ri_error_check_(ri);

    RiNode* node = NULL;
    switch (ri->token.kind)
    {
        case RiToken_Keyword_Func: node = ri_parse_decl_func_or_func_type_(ri); break;
        case RiToken_Keyword_Variable: node = ri_parse_decl_variable_(ri); break;
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

static RiNode* ri_parse_st_(Ri* ri);
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

    if (ri_lex_next_if_(ri, RiToken_Semicolon)) {
        condition = ri_parse_expr_(ri);
        if (!condition) {
            return NULL;
        }
    } else {
        condition = pre;
        pre = NULL;
    }

    if (!ri_lex_expect_token_(ri, RiToken_LB)) {
        return NULL;
    }

    RiNode* scope_else = NULL;
    RiNode* scope_then = ri_parse_scope_(ri, RiToken_RB);
    if (!scope_then) {
        return NULL;
    }
    array_push(&scope->scope.statements, scope_then);

    if (ri_lex_next_if_(ri, RiToken_Keyword_Else)) {
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
            scope_else = ri_parse_scope_(ri, RiToken_RB);
            if (!scope_else) {
                return NULL;
            }
            array_push(&scope->scope.statements, scope_else);
        }
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

    if (!ri_lex_next_if_(ri, RiToken_LB)) {
        // for pre?; condition?; post? { ... }
        if (!ri_lex_next_if_(ri, RiToken_Semicolon)) {
            pre = ri_parse_st_simple_(ri);
            if (!pre) {
                return NULL;
            }
            if (!ri_lex_next_if_(ri, RiToken_Semicolon)) {
                // for condition { ... }
                // TODO: Check if `pre` is expr, and not an assignment, etc.
                condition = pre;
                pre = NULL;
                goto skip;
            }
        }

        // for ; condition?; post? { ... }
        if (!ri_lex_next_if_(ri, RiToken_Semicolon)) {
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


    scope_block = ri_parse_scope_(ri, RiToken_RB);
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

            node = ri_make_decl_ref_(ri, pos, node);

            // Assignment.
            // TODO: Make required if the var decl lacks type.
            if (ri_lex_next_if_(ri, RiToken_Eq)) {
                RiNode* right = ri_parse_expr_(ri);
                if (!right) {
                    return NULL;
                }
                node = ri_make_st_assign_(ri, pos, RiNode_St_Assign, node, right);
            }
            break;

        default:
            node = ri_parse_st_expr_(ri);
            break;
    }

    return node;
}

static RiNode*
ri_parse_st_(Ri* ri)
{
    ri_error_check_(ri);

    RiNode* node = NULL;
    switch (ri->token.kind)
    {
        case RiToken_Keyword_Func:
            node = ri_parse_decl_(ri);
            if (!node) {
                return NULL;
            }
            if (!ri_lex_expect_token_(ri, RiToken_Semicolon)) {
                return NULL;
            }
            node = ri_make_decl_ref_(ri, node->pos, node);
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

        default:
            node = ri_parse_st_simple_(ri);
            if (node && !ri_lex_expect_token_(ri, RiToken_Semicolon)) {
                return NULL;
            }
            break;
    }

    return node;
}

static RiNode*
ri_parse_scope_(Ri* ri, RiTokenKind end)
{
    RiNode* scope = ri_make_scope_(ri, ri->token.pos);
    ri->scope = scope;
    while (ri->token.kind != end) {
        RiNode* statement = ri_parse_st_(ri);
        if (statement == NULL) {
            return NULL;
        }
        RI_CHECK(!ri_is_in(statement->kind, RiNode_Decl));
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
ri_parse(Ri* ri, String stream)
{
    ri_error_check_(ri);

    if (!ri_stream_set_(ri, stream)) {
        return NULL;
    }

    RiNode* block = ri_parse_scope_(ri, RiToken_End);

    return block;
}

//
//
//

#define RI_RESOLVE_F_(Name) bool Name(Ri* ri, RiNode** node)
typedef RI_RESOLVE_F_(RiResolveF_);

static RI_RESOLVE_F_(ri_resolve_decl_from_identifier_);
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

static RI_RESOLVE_F_(ri_resolve_decl_)
{
    RiNode* n = *node;
    switch (n->kind)
    {
        case RiNode_Expr_Identifier: {
            if (!ri_resolve_decl_from_identifier_(ri, &n)) {
                return false;
            }
        } break;

        case RiNode_Type_Func: {
            if (!ri_resolve_slice_with_(ri, n->decl.type_func.inputs.slice, &ri_resolve_decl_)) {
                return false;
            }
            if (!ri_resolve_slice_with_(ri, n->decl.type_func.outputs.slice, &ri_resolve_decl_)) {
                return false;
            }
        } break;

        case RiNode_Decl_Type_Func_Arg: {
            if (!ri_resolve_decl_(ri, &n->decl.type_func_arg.type)) {
                return false;
            }
            n = ri_make_expr_variable_from_arg_decl_(ri, n->pos, n);
        } break;

        case RiNode_Decl_Func: {
            if (!ri_resolve_decl_(ri, &n->decl.func.type)) {
                return false;
            }
            array_push(&ri->pending, n->decl.func.scope);
            n = ri_make_func_(ri, n->pos, n);
        } break;

        case RiNode_Decl_Variable: {
            if (!ri_resolve_decl_(ri, &n->decl.variable.type)) {
                return false;
            }
            n = ri_make_expr_variable_(ri, n->pos, n);
        } break;

        case RiNode_Type_Number_Float32:
        case RiNode_Type_Number_Int32: {
            // Skip.
        } break;

        default: {
            RI_ABORT("unexpected decl node");
            return false;
        } break;
    }

    *node = n;

    return true;
}

static RI_RESOLVE_F_(ri_resolve_decl_from_identifier_)
{
    RiNode* id = *node;

    RI_CHECK(id->kind == RiNode_Expr_Identifier);

    RI_LOG_DEBUG("%S", id->expr_id);

    RiNode* decl = NULL;
    RiNode* scope = id->owner;
    while (scope) {
        decl = map_get(&scope->scope.map, (ValueScalar){ .ptr = id->expr_id.items }).ptr;
        if (decl) {
            break;
        }
        scope = scope->owner;
    }

    if (!decl) {
        ri_error_set_(ri, RiError_NotDeclared, id->pos, "'%S' is not declared", id->expr_id);
        return false;
    }

    // TODO: Setup built-ins as Decl_Type.
    if (ri_is_in(decl->kind, RiNode_Type)) {
        *node = decl;
        return true;
    }

    // Resolved variable.
    if (decl->kind == RiNode_Expr_Variable) {
        *node = decl;
        return true;
    }

    // Resolved func.
    if (decl->kind == RiNode_Func) {
        *node = decl;
        return true;
    }

    RI_CHECK(ri_is_in(decl->kind, RiNode_Decl));

    if (decl->decl.state == RiDecl_Resolved) {
        RI_ABORT("unexpected");
        *node = decl;
        return true;
    } else if (decl->decl.state == RiDecl_Resolving) {
        ri_error_set_(ri, RiError_CyclicDeclaration, decl->pos, "cyclic declaration");
        return false;
    }

    RI_CHECK(decl->decl.state == RiDecl_Unresolved);
    decl->decl.state = RiDecl_Resolving;

    RiNode* decl_original = decl;
    if (!ri_resolve_decl_(ri, &decl)) {
        return false;
    }
    // NOTE: Mark the original declaration as resolved.
    decl_original->decl.state = RiDecl_Resolved;

    // NOTE: Node is no longer a declaration, so we'll update the map entry.
    RI_ASSERT(decl_original != decl);
    RI_CHECK(!ri_is_in(decl->kind, RiNode_Decl));
    array_push(&scope->scope.decl, decl);
    map_put(&scope->scope.map,
        (ValueScalar){ .ptr = id->expr_id.items },
        (ValueScalar){ .ptr = decl }
    );

    *node = decl;

    return true;
}

static RI_RESOLVE_F_(ri_resolve_unary_)
{
    RiNode* n = *node;

    if (!ri_resolve_node_(ri, &n->unary.argument)) {
        return 0;
    }

    RiNode* ret_type = 0;
    RiNode* arg_type = ri_retof_(ri, n->unary.argument);

    // TODO: Warn if we're using `-` with unsigned types, the type doesn't change.
    // warning C4146: unary minus operator applied to unsigned type, result still unsigned

    switch (n->kind)
    {
    case RiNode_Expr_Unary_Positive:
    case RiNode_Expr_Unary_Negative:
        ret_type = arg_type;
        break;
    default:
        RI_TODO;
    }

    n->unary.argument = ri_make_expr_cast_(ri, n->unary.argument->pos, n->unary.argument, arg_type);

    RI_ASSERT(ret_type);

    return true;
}

static RI_RESOLVE_F_(ri_resolve_binary_)
{
    RiNode* n = *node;
    if (!ri_resolve_node_(ri, &n->binary.argument0)) {
        return false;
    }
    if (!ri_resolve_node_(ri, &n->binary.argument1)) {
        return false;
    }

    RiNode* a0  = n->binary.argument0;
    RiNode* a1 = n->binary.argument1;
    RiNode* a0_type  = ri_retof_(ri, a0);
    RI_CHECK(a0_type);
    RiNode* a1_type = ri_retof_(ri, a1);
    RI_CHECK(a1_type);

    if (a0_type != a1_type) {
        // TODO: Print left and right type.
        ri_error_set_(ri, RiError_Type, n->pos, "mismatched types");
        return false;
    }

    return true;
}

static RI_RESOLVE_F_(ri_resolve_assign_)
{
    RiNode* n = *node;

    if (!ri_resolve_node_(ri, &n->binary.argument0)) {
        return false;
    }
    if (!ri_resolve_node_(ri, &n->binary.argument1)) {
        return false;
    }

    RiNode* a0  = n->binary.argument0;
    RiNode* a1 = n->binary.argument1;
    RiNode* a0_type  = ri_retof_(ri, a0);
    RI_CHECK(a0_type);
    RiNode* a1_type = ri_retof_(ri, a1);
    RI_CHECK(a1_type);

    if (a0_type != a1_type) {
        // TODO: Print left and right type.
        ri_error_set_(ri, RiError_Type, n->pos, "mismatched types");
        return false;
    }

    return true;
}

static RI_RESOLVE_F_(ri_resolve_call_func_)
{
    RiNode* n = *node;
    RiNode* func = n->expr_call.func;
    // TODO: Check number of arguments.
    // TOOD: Check if the argument types match.
    if (!ri_resolve_slice_with_(ri, n->expr_call.arguments.slice, &ri_resolve_node_)) {
        return false;
    }
    return true;
}

static RI_RESOLVE_F_(ri_resolve_call_type_)
{
    RiNode* n = *node;
    if (n->expr_call.arguments.count != 1) {
        ri_error_set_(ri, RiError_Argument, n->pos, "1 argument expected");
        return false;
    }
    if (!ri_resolve_slice_with_(ri, n->expr_call.arguments.slice, &ri_resolve_node_)) {
        return false;
    }
    return true;
}

static RI_RESOLVE_F_(ri_resolve_node_)
{
    RiNode* n = *node;

    if (ri_is_in(n->kind, RiNode_St_Assign)) {
        if (!ri_resolve_assign_(ri, &n)) {
            return false;
        }
    } else if (ri_is_in(n->kind, RiNode_Expr_Binary)) {
        if (!ri_resolve_binary_(ri, &n)) {
            return false;
        }
    } else if (ri_is_in(n->kind, RiNode_Expr_Unary)) {
        if (!ri_resolve_unary_(ri, &n)) {
            return false;
        }
    } else {
        switch (n->kind)
        {
            case RiNode_Ref: {
                // Skip
            } break;

            case RiNode_Scope: {
                if (!ri_resolve_slice_with_(ri, n->scope.statements.slice, &ri_resolve_node_)) {
                    return false;
                }
            } break;

            case RiNode_Expr_Identifier: {
                if (!ri_resolve_decl_from_identifier_(ri, &n)) {
                    return false;
                }
            } break;

            case RiNode_Expr_Call: {
                if (!ri_resolve_node_(ri, &n->expr_call.func)) {
                    return false;
                }
                RiNode* callable = n->expr_call.func;
                if (callable->kind == RiNode_Func) {
                    if (!ri_resolve_call_func_(ri, &n)) {
                        return false;
                    }
                } else if (ri_is_in(callable->kind, RiNode_Type)) {
                    if (!ri_resolve_call_type_(ri, &n)) {
                        return false;
                    }
                }
            } break;

            case RiNode_St_Expr: {
                if (!ri_resolve_node_(ri, &n->st_expr)) {
                    return false;
                }
            } break;

            case RiNode_St_Return: {
                if (n->st_return.argument && !ri_resolve_node_(ri, &n->st_return.argument)) {
                    return false;
                }
            } break;

            case RiNode_St_If: {
                if (n->st_if.pre && !ri_resolve_node_(ri, &n->st_if.pre)) {
                    return false;
                }
                if (!ri_resolve_node_(ri, &n->st_if.condition)) {
                    return false;
                }
                if (!ri_resolve_node_(ri, &n->st_if.scope)) {
                    return false;
                }
            } break;

            case RiNode_St_For: {
                if (n->st_for.pre && !ri_resolve_node_(ri, &n->st_for.pre)) {
                    return false;
                }
                if (n->st_for.condition && !ri_resolve_node_(ri, &n->st_for.condition)) {
                    return false;
                }
                if (n->st_for.post && !ri_resolve_node_(ri, &n->st_for.post)) {
                    return false;
                }
                if (!ri_resolve_node_(ri, &n->st_for.scope)) {
                    return false;
                }
            } break;

            default: {
                RI_ABORT("unexpected node");
                return false;
            }
        }
    }

    *node = n;

    return true;
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
    [RiNode_Expr_Unary_Positive] = "expr-positive",
    [RiNode_Expr_Unary_Negative] = "expr-negative",
    [RiNode_Expr_Unary_IncPre] = "expr-inc-prefix",
    [RiNode_Expr_Unary_DecPre] = "expr-dec-prefix",
    [RiNode_Expr_Unary_IncPost] = "expr-inc-postfix",
    [RiNode_Expr_Unary_DecPost] = "expr-dec-postfix",
    [RiNode_Expr_Unary_BNeg] = "expr-bneg",
    [RiNode_Expr_Unary_Not] = "expr-not",
    [RiNode_Expr_Binary_Add] = "expr-add",
    [RiNode_Expr_Binary_Sub] = "expr-sub",
    [RiNode_Expr_Binary_Mul] = "expr-mul",
    [RiNode_Expr_Binary_Div] = "expr-div",
    [RiNode_Expr_Binary_Mod] = "expr-mod",
    [RiNode_Expr_Binary_BXor] = "expr-bxor",
    [RiNode_Expr_Binary_BAnd] = "expr-band",
    [RiNode_Expr_Binary_BOr] = "expr-bor",
    [RiNode_Expr_Binary_BShL] = "expr-bshl",
    [RiNode_Expr_Binary_BShR] = "expr-bshr",
    [RiNode_Expr_Binary_And] = "expr-and",
    [RiNode_Expr_Binary_Or] = "expr-or",
    [RiNode_Expr_Binary_Select] = "expr-select",
    [RiNode_Expr_Binary_Cast] = "expr-cast",
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
    } else {
        RiNode* it;
        switch (node->kind)
        {
            case RiNode_Scope: {
                riprinter_print(&D->printer, "(scope %d\n\t", node->index);

                riprinter_print(&D->printer, "(decl");
                if (node->scope.decl.count) {
                    riprinter_print(&D->printer, "\n\t");
                    array_each(&node->scope.decl, &it) {
                        switch (it->kind) {
                            case RiNode_Expr_Variable:
                                ri_dump_(D, it->expr_variable.decl);
                                break;
                            case RiNode_Func:
                                ri_dump_(D, it->func.decl);
                                break;
                            default:
                                ri_dump_(D, it);
                                break;
                        }
                    }
                    riprinter_print(&D->printer, "\b");
                }
                riprinter_print(&D->printer, ")\n(code");
                if (is_logged) {
                    riprinter_print(&D->printer, " (recursive)");
                } else {
                    ri_dump_slice_(D, &node->scope.statements.slice, NULL);
                }
                riprinter_print(&D->printer, ")\n");

                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Ref: {
                riprinter_print(&D->printer, "(ref '%S')\n", node->ref->decl.id);
            } break;

            case RiNode_Expr_Identifier: {
                riprinter_print(&D->printer, "(expr-id '%S')\n", node->expr_id);
            } break;

            case RiNode_Expr_Call: {
                riprinter_print(&D->printer, "(expr-call\n\t");
                ri_dump_(D, node->expr_call.func);
                ri_dump_slice_(D, &node->expr_call.arguments.slice, "arguments");
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

            case RiNode_Decl_Variable: {
                riprinter_print(&D->printer, "(decl-var '%S'\n\t", node->decl.id);
                ri_dump_(D, node->decl.variable.type);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Expr_Variable: {
                riprinter_print(&D->printer, "(var '%S')\n", node->expr_variable.id);
            } break;

            case RiNode_Decl_Func: {
                riprinter_print(&D->printer, "(decl-func '%S'\n\t", node->decl.id);
                ri_dump_(D, node->decl.func.type);
                ri_dump_(D, node->decl.func.scope);
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Func: {
                riprinter_print(&D->printer, "(func '%S')\n", node->func.id);
            } break;

            case RiNode_Decl_Type_Func_Arg: {
                riprinter_print(&D->printer, "(decl-arg '%S'\n\t", node->decl.id);
                ri_dump_(D, node->decl.type_func_arg.type);
                riprinter_print(&D->printer, "\b)\n", node->decl.id);
            } break;

            case RiNode_Type_Func: {
                riprinter_print(&D->printer, "(type-func '%S'\n\t", node->decl.id);
                ri_dump_slice_(D, &node->decl.type_func.inputs.slice, "in");
                ri_dump_slice_(D, &node->decl.type_func.outputs.slice, "out");
                riprinter_print(&D->printer, "\b)\n");
            } break;

            case RiNode_Type_Number_UInt64:
            case RiNode_Type_Number_Int64:
            case RiNode_Type_Number_UInt32:
            case RiNode_Type_Number_Int32:
            case RiNode_Type_Number_UInt16:
            case RiNode_Type_Number_Int16:
            case RiNode_Type_Number_UInt8:
            case RiNode_Type_Number_Int8:
            case RiNode_Type_Number_Float32:
            case RiNode_Type_Number_Float64:
                riprinter_print(&D->printer, "(type %S)\n", node->decl.id);
                break;

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
ri_init(Ri* ri)
{
    RI_LOG_DEBUG("node %d bytes", sizeof(RiNode));

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

    ri->scope = ri_make_scope_(ri, RI_POS_OUTSIDE);
    #define DECL_TYPE(Name, Type) { \
        RiNode* node = ri_make_decl_type_number_(ri, \
            RI_POS_OUTSIDE, \
            ri_make_id_(ri, S(Name)), \
            RiNode_Type_ ## Type \
        ); \
        ri->node_meta[RiNode_Type_ ## Type] = (RiNodeMeta){ .node = node }; \
        ri_scope_set_(ri, node); \
    }

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
}

void
ri_purge(Ri* ri)
{
    RI_LOG_DEBUG("memory %d bytes", ri->arena.head);
    RI_LOG_DEBUG("nodes %d", ri->index);
    arena_purge(&ri->arena);
    intern_purge(&ri->intern);
}