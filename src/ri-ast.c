#include "ri-ast.h"

RiNode*
ri_make_node_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind)
{
    RI_CHECK(arena);
    RI_CHECK(owner == NULL || ri_is_in(owner->kind, RiNode_Scope));
    RiNode* node = arena_push_t(arena, RiNode);
    memset(node, 0, sizeof(RiNode));
    node->kind = kind;
    node->owner = owner;
    node->pos = pos;
    return node;
}

RiNode*
ri_make_scope_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind)
{
    RI_CHECK(ri_is_in(kind, RiNode_Scope));
    RiNode* node = ri_make_node_(arena, owner, pos, kind);
    return node;
}

//
// Variable
//

RiNode*
ri_make_spec_var_(Arena* arena, RiNode* owner, RiPos pos, RiNode* type, RiVarKind kind)
{
    RiNode* spec = ri_make_node_(arena, owner, pos, RiNode_Spec_Var);
    spec->spec.var.type = type;
    spec->spec.var.kind = kind;
    spec->spec.var.slot = RI_INVALID_SLOT;
    return spec;
}

//
// Constant
//

RiNode*
ri_make_spec_constant_(Arena* arena, RiNode* owner, RiPos pos, RiNode* type, RiLiteral value)
{
    RI_CHECK(type);
    RI_CHECK(ri_is_in(type->kind, RiNode_Spec_Type_Number));
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Spec_Constant);
    node->spec.constant.type = type;
    node->spec.constant.value = value;
    return node;
}

//
// Modules
//

RiNode*
ri_make_spec_module_(Arena* arena, RiNode* owner, RiPos pos)
{
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Spec_Module);
    return node;
}

//
// Functions
//

RiNode*
ri_make_spec_func_(Arena* arena, RiNode* owner, RiPos pos, RiNode* type, RiNode* scope)
{
    RI_CHECK(type);
    RI_CHECK(type->kind == RiNode_Spec_Type_Func);
    RI_CHECK(scope);
    RI_CHECK(scope->kind == RiNode_Scope_Function_Root);

    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Spec_Func);
    node->spec.func.type = type;
    node->spec.func.scope = scope;
    node->spec.func.slot = RI_INVALID_SLOT;
    return node;
}

//
// Types
//

RiNode*
ri_make_spec_type_func_(Arena* arena, RiNode* owner, RiPos pos, RiNodeArray inputs, RiNodeArray outputs) {
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Spec_Type_Func);
    node->spec.type.func.inputs = inputs;
    node->spec.type.func.outputs = outputs;
    return node;
}

RiNode*
ri_make_spec_type_number_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind)
{
    RI_CHECK(ri_is_in(kind, RiNode_Spec_Type_Number));
    RiNode* node = ri_make_node_(arena, owner, pos, kind);
    return node;
}

RiNode*
ri_make_spec_type_struct_(Arena* arena, RiNode* owner, RiPos pos, RiNode* scope)
{
    RI_CHECK(scope);
    RI_CHECK(scope->kind == RiNode_Scope_Struct);
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Spec_Type_Struct);
    node->spec.type.compound.scope = scope;
    return node;
}

//
//
//

RiNode*
ri_make_decl_(Arena* arena, RiNode* owner, RiPos pos, String id, RiNode* spec)
{
    RI_CHECK(spec);
    RI_CHECK(ri_is_in(spec->kind, RiNode_Spec) || ri_is_expr_like(spec->kind));

    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Decl);
    node->decl.spec = spec;
    node->decl.id = id;
    array_push(&node->owner->scope.decl, node);
    return node;
}

//
// Expressions
//

RiNode*
ri_make_expr_id_(Arena* arena, RiNode* owner, RiPos pos, String name)
{
    CHECK(name.items != 0);
    CHECK(name.count > 0);
    // TODO: Checks.
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Expr_Id);
    node->id.name = name;
    return node;
}

RiNode*
ri_make_expr_field_(Arena* arena, RiPos pos, RiNode* parent, RiNode* child)
{
    RiNode* owner = NULL;
    switch (parent->kind) {
        case RiNode_Spec_Type_Struct:
            owner = parent->spec.type.compound.scope;
            break;
        default:
            RI_UNREACHABLE;
            break;
    }

    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Expr_Field);
    node->field.parent = parent;
    node->field.child = child;
    return node;
}

RiNode*
ri_make_expr_call_(Arena* arena, RiNode* owner, RiPos pos, RiNode* func)
{
    RI_CHECK(func);
    // TODO: Check func node type?

    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Expr_Call);
    node->call.func = func;
    return node;
}

RiNode*
ri_make_expr_binary_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind, RiNode* argument0, RiNode* argument1)
{
    RI_CHECK(ri_is_in(kind, RiNode_Expr_Binary) || kind == RiNode_Expr_Select);
    RI_CHECK(argument0);
    RI_CHECK(ri_is_expr_like(argument0->kind));
    RI_CHECK(argument1);
    RI_CHECK(ri_is_expr_like(argument1->kind));

    RiNode* node = ri_make_node_(arena, owner, pos, kind);
    node->binary.argument0 = argument0;
    node->binary.argument1 = argument1;
    return node;
}

RiNode*
ri_make_expr_unary_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind, RiNode* argument)
{
    RI_CHECK(ri_is_in(kind, RiNode_Expr_Unary));
    RI_CHECK(argument);
    RI_CHECK(ri_is_expr_like(argument->kind));

    RiNode* node = ri_make_node_(arena, owner, pos, kind);
    node->unary.argument = argument;
    return node;
}


RiNode*
ri_make_expr_cast_(Arena* arena, RiNode* owner, RiPos pos, RiNode* expr, RiNode* type_to)
{
    RI_CHECK(expr);
    RI_CHECK(type_to);
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_Expr_Cast);
    array_push(&node->call.arguments, type_to);
    array_push(&node->call.arguments, expr);
    return node;
}

//
// Statements
//

RiNode*
ri_make_st_assign_(Arena* arena, RiNode* owner, RiPos pos, RiNodeKind kind, RiNode* argument0, RiNode* argument1)
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

    RiNode* node = ri_make_node_(arena, owner, pos, kind);
    node->binary.argument0 = argument0;
    node->binary.argument1 = argument1;
    return node;
}

RiNode*
ri_make_st_expr_(Arena* arena, RiNode* owner, RiPos pos, RiNode* expr)
{
    RI_CHECK(ri_is_expr_like(expr->kind));

    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_Expr);
    node->st_expr = expr;
    return node;
}

RiNode*
ri_make_st_return_(Arena* arena, RiNode* owner, RiPos pos, RiNode* argument)
{
    RI_CHECK(argument == NULL || ri_is_expr_like(argument->kind));
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_Return);
    node->st_return.argument = argument;
    return node;
}

RiNode*
ri_make_st_if_(Arena* arena, RiNode* owner, RiPos pos, RiNode* pre, RiNode* condition, RiNode* scope)
{
    RI_CHECK(condition);
    RI_CHECK(scope);
    RI_CHECK(scope->scope.statements.count >= 0 && scope->scope.statements.count <= 2);

    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_If);
    node->st_if.pre = pre;
    node->st_if.condition = condition;
    node->st_if.scope = scope;
    return node;
}

RiNode*
ri_make_st_for_(Arena* arena, RiNode* owner, RiPos pos, RiNode* pre, RiNode* condition, RiNode* post, RiNode* scope)
{
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_For);
    node->st_for.pre = pre;
    node->st_for.condition = condition;
    node->st_for.post = post;
    node->st_for.scope = scope;
    return node;
}

RiNode*
ri_make_st_switch_(Arena* arena, RiNode* owner, RiPos pos, RiNode* pre, RiNode* expr, RiNode* scope)
{
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_Switch);
    node->st_switch.pre = pre;
    node->st_switch.expr = expr;
    node->st_switch.scope = scope;
    return node;
}

RiNode*
ri_make_st_switch_case_(Arena* arena, RiNode* owner, RiPos pos, RiNode* expr)
{
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_Switch_Case);
    node->st_switch_case.expr = expr;
    return node;
}

RiNode*
ri_make_st_switch_default_(Arena* arena, RiNode* owner, RiPos pos)
{
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_Switch_Default);
    return node;
}

RiNode*
ri_make_st_break_(Arena* arena, RiNode* owner, RiPos pos)
{
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_Break);
    return node;
}

RiNode*
ri_make_st_continue_(Arena* arena, RiNode* owner, RiPos pos)
{
    RiNode* node = ri_make_node_(arena, owner, pos, RiNode_St_Continue);
    return node;
}

//
//
//

void
ri_type_get_title_(RiNode* type, CharArray* o_buffer)
{
    switch (type->kind)
    {
        case RiNode_Value_Type:
            chararray_push(o_buffer, type->value.decl->decl.id);
            break;

        case RiNode_Spec_Type_Number_None_Int: chararray_push(o_buffer, S("untyped-int")); break;
        case RiNode_Spec_Type_Number_None_Real: chararray_push(o_buffer, S("untyped-real")); break;

        // Use ri->id_* constants.
        case RiNode_Spec_Type_Number_Bool:    chararray_push(o_buffer, S("bool"));    break;
        case RiNode_Spec_Type_Number_Int64:   chararray_push(o_buffer, S("int64"));   break;
        case RiNode_Spec_Type_Number_Int32:   chararray_push(o_buffer, S("int32"));   break;
        case RiNode_Spec_Type_Number_Int16:   chararray_push(o_buffer, S("int16"));   break;
        case RiNode_Spec_Type_Number_Int8:    chararray_push(o_buffer, S("int8"));    break;
        case RiNode_Spec_Type_Number_UInt64:  chararray_push(o_buffer, S("uint64"));  break;
        case RiNode_Spec_Type_Number_UInt32:  chararray_push(o_buffer, S("uint32"));  break;
        case RiNode_Spec_Type_Number_UInt16:  chararray_push(o_buffer, S("uint16"));  break;
        case RiNode_Spec_Type_Number_UInt8:   chararray_push(o_buffer, S("uint8"));   break;
        case RiNode_Spec_Type_Number_Float64: chararray_push(o_buffer, S("float64")); break;
        case RiNode_Spec_Type_Number_Float32: chararray_push(o_buffer, S("float32")); break;

        case RiNode_Spec_Type_Func: {
            chararray_push(o_buffer, S("func ("));
            RiNode* it;
            RiNodeArray* args = &type->spec.type.func.inputs;
            array_eachi(args, i, &it) {
                ri_type_get_title_(it, o_buffer);
                if (i < args->count - 1) {
                    chararray_push(o_buffer, S(", "));
                }
            }
            chararray_push(o_buffer, S(") ("));
            args = &type->spec.type.func.outputs;
            array_eachi(args, i, &it) {
                ri_type_get_title_(it, o_buffer);
                if (i < args->count - 1) {
                    chararray_push(o_buffer, S(", "));
                }
            }
        } break;

        default:
            RI_UNREACHABLE;
    }
}
