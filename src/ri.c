#include "ri.h"

#include "ri-common.c"
#include "ri-ast.c"
#include "ri-parse.c"

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

void
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

    chararray_push_f(&ri->error.path, "%S/%S.ri", ri->module->path, ri->module->id);
}

void
ri_error_format_(Ri* ri, CharArray* buffer)
{
    chararray_push_f(
        buffer,
        "error %S:%d:%d: %S",
        ri->error.path.slice,
        ri->error.pos.row + 1,
        ri->error.pos.col + 1,
        ri->error.message.slice
    );
}

void
ri_error_log(Ri* ri)
{
    if (ri->error.kind != RiError_None)
    {
        CharArray message = {0};
        ri_error_format_(ri, &message);
        RI_LOG("%S", message.slice);
        array_purge(&message);
    }
}

void
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
        // ri->nodes[type0->kind]->spec.id,
        type0->spec.id,
        op,
        // ri->nodes[type1->kind]->spec.id
        type1->spec.id
    );
}

//
//
//

String
ri_make_id_r_(Ri* ri, char* start, char* end)
{
    RI_CHECK(start);
    RI_CHECK(end);
    RI_CHECK(start <= end);
    return (String){
        .items = (char*)intern_put_r(&ri->intern, start, end),
        .count = end - start
    };
}

String
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

// TODO: Only take 'decl` and register it to the owner?
bool
ri_scope_set_(Ri* ri, RiNode* scope, RiNode* decl)
{
    RI_CHECK(scope);
    RI_CHECK(ri_is_in(scope->kind, RiNode_Scope));
    RI_CHECK(decl);
    RI_CHECK(decl->decl.spec);
    RI_CHECK(ri_is_in(decl->decl.spec->kind, RiNode_Spec));
    RI_CHECK(scope == decl->owner);
    ValueScalar id_ = { .ptr = decl->decl.spec->spec.id.items };
    RiNode* decl_found = map_get(&scope->scope.map, id_).ptr;
    if (decl_found != NULL) {
        ri_error_set_(ri, RiError_Declared, decl->pos, "'%S' is already declared", decl->decl.spec->spec.id);
        return false;
    }
    map_put(&scope->scope.map, id_, (ValueScalar){ .ptr = decl });
    return decl;
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

static bool ri_type_equal_(Ri* ri, RiNode* t0, RiNode* t1);

static bool
ri_type_equal_function_arguments_(Ri* ri, RiNodeArray* a0, RiNodeArray* a1)
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
        if (ri_type_equal_(ri, ri_get_spec_(ri, it0->decl.spec->spec.var.type), ri_get_spec_(ri, it1->decl.spec->spec.var.type)) == false) {
            return false;
        }
    }

    return true;
}

static bool
ri_type_equal_(Ri* ri, RiNode* t0, RiNode* t1)
{
    RI_CHECK(t0 && ri_is_in(t0->kind, RiNode_Spec_Type));
    RI_CHECK(t1 && ri_is_in(t1->kind, RiNode_Spec_Type));

    if (t0->kind != t1->kind) {
        return false;
    }

    if (ri_is_in(t0->kind, RiNode_Spec_Type_Number)) {
        return true;
    }

    switch (t0->kind)
    {
        case RiNode_Spec_Type_Func:
            return (
                ri_type_equal_function_arguments_(ri, &t0->spec.type.func.inputs, &t1->spec.type.func.inputs) &&
                ri_type_equal_function_arguments_(ri, &t0->spec.type.func.outputs, &t1->spec.type.func.outputs)
            );

        case RiNode_Spec_Type_Pointer:
            return ri_type_equal_(ri, t0->spec.type.pointer.base, t1->spec.type.pointer.base);

        case RiNode_Spec_Type_None:
            return true;

        case RiNode_Spec_Type_Infer:
        default:
            RI_UNREACHABLE;
            return false;
    }
}

//
// Type identity
//

// static void
// ri_get_type_identity_(Ri* ri, RiNode* node)
// {
//     RI_CHECK(ri_is_in(node->kind, RiNode_Spec_Type));
//     RI_CHECK(node->spec.type.identity != NULL);
//     return node->spec.type.identity;
// }

static void
ri_add_type_identity_(Ri* ri, RiNode* node)
{
    RI_CHECK(ri_is_in(node->kind, RiNode_Spec_Type));
    RI_CHECK(node->spec.type.identity == NULL);
    array_push(&ri->types, node);
    node->spec.type.identity = node;
}

static void
ri_set_type_identity_(Ri* ri, RiNode* node)
{
    RI_CHECK(ri_is_in(node->kind, RiNode_Spec_Type));
    if (node->spec.type.identity != 0) {
        return;
    }

    RiNode* it;
    array_eachi(&ri->types, i, &it) {
        if (ri_type_equal_(ri, it, node)) {
            node->spec.type.identity = it;
            return;
        }
    }

    ri_add_type_identity_(ri, node);
}

//
// Completing types
//

static RiNode*
ri_complete_type_(Ri* ri, RiPos pos, RiNode* type)
{
    RI_CHECK(ri->error.kind == RiError_None);
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
    RI_CHECK(ri->error.kind == RiError_None);

    if (ri_is_in(node->kind, RiNode_Expr_Binary_Comparison)) {
        // Always bool
        return ri->nodes[RiNode_Spec_Type_Number_Bool];
    } else if (ri_is_in(node->kind, RiNode_Expr_Binary_Numeric)) {
        // Same return value as zero argument.
        return ri_retof_(ri, node->binary.argument0);
    } else {
        // RI_ASSERT(ri_is_rt(node));
        switch (node->kind)
        {
            // Ct

            case RiNode_Spec_Var:
                return ri_get_spec_(ri, node->spec.var.type);
            case RiNode_Spec_Func:
                return ri_get_spec_(ri, node->spec.func.type);
            case RiNode_Spec_Constant:
                return ri_get_spec_(ri, node->spec.constant.type);


            // Rt

            case RiNode_Value_Var:
                // if (node->value.spec->kind == RiNode_Spec_Var) {
                //     return node->value.type;
                // } else {
                //     ri_error_set_(ri, RiError_UnexpectedValue, node->pos, "variable expected");
                //     return NULL;
                // }
                RI_CHECK(node->value.spec->kind == RiNode_Spec_Var);
                return ri_get_spec_(ri, node->value.spec->spec.var.type);
            case RiNode_Value_Constant:
                RI_CHECK(node->value.constant.type);
                return ri_get_spec_(ri, node->value.constant.type);
            case RiNode_Value_Func:
                RI_CHECK(node->value.spec->kind == RiNode_Spec_Func);
                return ri_get_spec_(ri, node->value.spec->spec.func.type);

            case RiNode_Expr_Cast:
                return array_at(&node->call.arguments, 0);
            case RiNode_Expr_Call: {
                RI_CHECK(node->call.func);
                RiNode* func = node->call.func;
                RiNode* func_type = NULL;
                switch (node->call.func->kind)
                {
                    case RiNode_Value_Var:
                        func_type = ri_get_spec_(ri, func->value.spec->spec.var.type);
                        break;
                    case RiNode_Value_Func:
                        func_type = ri_get_spec_(ri, func->value.spec->spec.func.type);
                        break;
                    default:
                        RI_UNREACHABLE;
                        break;
                }
                RI_CHECK(func_type->kind == RiNode_Spec_Type_Func);
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
                        return ri->nodes[RiNode_Spec_Type_None];

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
// AST Walker
//

typedef enum RiWalkResult
{
    RiWalk_Skip = -1,
    RiWalk_Error = 0,
    RiWalk_Continue = 1,
}
RiWalkResult;

#define RIWALK_F(Name) RiWalkResult Name(Ri* ri, RiNode** node, void* context)
typedef RIWALK_F(RiWalkF);

// TODO: Check every node (perhaps introduce a `phase` arg to check whether there are Ids or not)

static RiWalkResult
ri_walk_(Ri* ri, RiNode** node, RiWalkF* f, void* context)
{
    switch (f(ri, node, context)) {
        case RiWalk_Error: return RiWalk_Error;
        case RiWalk_Skip: return RiWalk_Continue;
    }

    RiNode* n = *node;

    RiNode* it;
    if (ri_is_in(n->kind, RiNode_Spec_Type_Number)) {
        return RiWalk_Continue;
    } if (ri_is_in(n->kind, RiNode_St_Assign)) {
        if (ri_walk_(ri, &n->binary.argument0, f, context) == RiWalk_Error) {
            return RiWalk_Error;
        }
        if (ri_walk_(ri, &n->binary.argument1, f, context) == RiWalk_Error) {
            return RiWalk_Error;
        }
        return RiWalk_Continue;
    } if (ri_is_in(n->kind, RiNode_Expr_Binary)) {
        if (ri_walk_(ri, &n->binary.argument0, f, context) == RiWalk_Error) {
            return RiWalk_Error;
        }
        if (ri_walk_(ri, &n->binary.argument1, f, context) == RiWalk_Error) {
            return RiWalk_Error;
        }
        return RiWalk_Continue;
    } if (ri_is_in(n->kind, RiNode_Expr_Unary)) {
        return ri_walk_(ri, &n->unary.argument, f, context);
    } if (ri_is_in(n->kind, RiNode_Scope)) {
        array_each(&n->scope.statements, &it) {
            if (!ri_walk_(ri, &it, f, context)) {
                return RiWalk_Error;
            }
        }
        return RiWalk_Continue;
    }
    else switch (n->kind)
    {
        case RiNode_Module:
            return ri_walk_(ri, &n->module.scope, f, context);
        case RiNode_Decl:
            return ri_walk_(ri, &n->decl.spec, f, context);
        case RiNode_Id:
            return RiWalk_Continue;
        case RiNode_Spec_Var:
            return ri_walk_(ri, &n->spec.var.type, f, context);
        case RiNode_Spec_Constant:
            return ri_walk_(ri, &n->spec.constant.type, f, context);
        case RiNode_Spec_Func:
            if (ri_walk_(ri, &n->spec.func.type, f, context) == RiWalk_Error) {
                return RiWalk_Error;
            }
            if (n->spec.func.scope && ri_walk_(ri, &n->spec.func.scope, f, context) == RiWalk_Error) {
                return RiWalk_Error;
            }
            return RiWalk_Continue;
        case RiNode_Spec_Type_Func:
            array_each(&n->spec.type.func.inputs, &it) {
                if (ri_walk_(ri, &it, f, context) == RiWalk_Error) {
                    return RiWalk_Error;
                }
            }
            array_each(&n->spec.type.func.outputs, &it) {
                if (ri_walk_(ri, &it, f, context) == RiWalk_Error) {
                    return RiWalk_Error;
                }
            }
            return RiWalk_Continue;
        case RiNode_Spec_Module:
            return RiWalk_Continue;
        case RiNode_Spec_Type_Infer:
            return RiWalk_Continue;
        case RiNode_Value_Var:
            return ri_walk_(ri, &n->value.spec, f, context);
        case RiNode_Value_Func:
            return ri_walk_(ri, &n->value.spec, f, context);
        case RiNode_Value_Type:
            return ri_walk_(ri, &n->value.spec, f, context);
        case RiNode_Value_Constant:
            return ri_walk_(ri, &n->value.constant.type, f, context);
        case RiNode_Value_Module:
            return ri_walk_(ri, &n->value.spec, f, context);

        case RiNode_Expr_Cast:
        case RiNode_Expr_Call:
            if (ri_walk_(ri, &n->call.func, f, context) != RiWalk_Error) {
                array_each(&n->call.arguments, &it) {
                    if (ri_walk_(ri, &it, f, context) == RiWalk_Error) {
                        return RiWalk_Error;
                    }
                }
                return RiWalk_Continue;
            }
            return RiWalk_Error;

        case RiNode_Expr_Binary_Select:
            if (ri_walk_(ri, &n->binary.argument0, f, context) != RiWalk_Error && ri_walk_(ri, &n->binary.argument1, f, context)) {
                return RiWalk_Continue;
            }
            return RiWalk_Error;

        case RiNode_St_Expr:
            return ri_walk_(ri, &n->st_expr, f, context);

        case RiNode_St_Return:
            if (n->st_return.argument) {
                return ri_walk_(ri, &n->st_return.argument, f, context);
            }
            return RiWalk_Continue;

        case RiNode_St_If:
            if (n->st_if.pre && ri_walk_(ri, &n->st_if.pre, f, context) == RiWalk_Error) {
                return RiWalk_Error;
            }
            if (n->st_if.condition && ri_walk_(ri, &n->st_if.condition, f, context) == RiWalk_Error) {
                return RiWalk_Error;
            }
            RI_CHECK(n->st_if.scope && (n->st_if.scope->scope.statements.count == 1 || n->st_if.scope->scope.statements.count == 2));
            // TODO: Check if there's an 'else' block, that it might either be a scope or another st_if.
            if (!ri_walk_(ri, &n->st_if.scope, f, context)) {
                return RiWalk_Error;
            }
            return RiWalk_Continue;

        case RiNode_St_For:
            if (n->st_for.pre && !ri_walk_(ri, &n->st_for.pre, f, context)) {
                return RiWalk_Error;
            }
            if (n->st_for.condition && !ri_walk_(ri, &n->st_for.condition, f, context)) {
                return RiWalk_Error;
            }
            if (n->st_for.post && !ri_walk_(ri, &n->st_for.post, f, context)) {
                return RiWalk_Error;
            }
            if (!ri_walk_(ri, &n->st_for.scope, f, context)) {
                return RiWalk_Error;
            }
            return RiWalk_Continue;

        case RiNode_St_Switch:
            if (n->st_if.pre && !ri_walk_(ri, &n->st_if.pre, f, context)) {
                return RiWalk_Error;
            }
            if (n->st_if.condition && !ri_walk_(ri, &n->st_if.condition, f, context)) {
                return RiWalk_Error;
            }
            if (!ri_walk_(ri, &n->st_for.scope, f, context)) {
                return RiWalk_Error;
            }
            return RiWalk_Continue;

        case RiNode_St_Switch_Case:
            return ri_walk_(ri, &n->st_switch_case.expr, f, context);
    }

    RI_UNREACHABLE;
    return RiWalk_Error;
}

//
//
//

#if 0
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
                ri->nodes[arg_type->kind]->spec.id
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
                ri->nodes[arg_type->kind]->spec.id
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
                ri->nodes[arg_type->kind]->spec.id
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

// TODO: Validate in type_patch_phase.
// static RI_RESOLVE_F_(ri_resolve_st_if_)
// {
//     RiNode* n = *node;

//     if (n->st_if.pre && !ri_resolve_node_(ri, &n->st_if.pre)) {
//         return false;
//     }

//     // No expected type, as we expect it'll be bool right away.
//     // So `if 1 { ... }` won't pass and result in error.
//     if (!ri_resolve_node_(ri, &n->st_if.condition)) {
//         return false;
//     }
//     RiNode* condition_type = ri_retof_(ri, n->st_if.condition);
//     if (condition_type == NULL) {
//         return false;
//     }
//     if (condition_type->kind != RiNode_Spec_Type_Number_Bool) {
//         ri_error_set_(ri, RiError_Type, n->st_if.condition->pos, "'bool' expected");
//         return false;
//     }

//     if (!ri_resolve_node_(ri, &n->st_if.scope)) {
//         return false;
//     }

//     return true;
// }

// TODO: Validate in type_patch phase.
// static RI_RESOLVE_F_(ri_resolve_st_for_)
// {
//     RiNode* n = *node;

//     if (n->st_for.pre && !ri_resolve_node_(ri, &n->st_for.pre)) {
//         return false;
//     }
//     if (n->st_for.condition) {
//         if (!ri_resolve_node_(ri, &n->st_for.condition)) {
//             return false;
//         }
//         RiNode* condition_type = ri_retof_(ri, n->st_for.condition);
//         if (condition_type == NULL) {
//             return false;
//         }
//         if (condition_type->kind != RiNode_Spec_Type_Number_Bool) {
//             ri_error_set_(ri, RiError_Type, n->st_for.condition->pos, "'bool' expected");
//             return false;
//         }
//     }
//     if (n->st_for.post && !ri_resolve_node_(ri, &n->st_for.post)) {
//         return false;
//     }
//     if (!ri_resolve_node_(ri, &n->st_for.scope)) {
//         return false;
//     }

//     return true;
// }

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

    if (ri_is_in(n->kind, RiNode_St_Assign)) {
        return ri_resolve_assign_(ri, node);
    } else if (ri_is_in(n->kind, RiNode_Expr_Binary)) {
        return ri_resolve_binary_(ri, node);
    } else if (ri_is_in(n->kind, RiNode_Expr_Unary)) {
        return ri_resolve_unary_(ri, node);
    } else if (ri_is_in(n->kind, RiNode_Spec_Type_Number)) {
        ri_set_type_identity_(ri, n);
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

            case RiNode_Decl: {
                RI_CHECK(n->decl.state == RiDecl_Unresolved);
                n->decl.state = RiDecl_Resolving;
                if (!ri_resolve_node_(ri, &n->decl.spec)) {
                    return false;
                }
                n->decl.state = RiDecl_Resolved;
                array_push(&n->owner->scope.decl, n);
            } return true;

            case RiNode_Spec_Type_Func:
                if (
                    ri_resolve_func_args_(ri, n, &n->spec.type.func.inputs.slice) &&
                    ri_resolve_func_args_(ri, n, &n->spec.type.func.outputs.slice)
                ) {
                    ri_set_type_identity_(ri, n);
                    return true;
                }
                return false;

            case RiNode_Spec_Func:
                if (!ri_resolve_node_(ri, &n->spec.func.type)) {
                    return false;
                }
                array_push(&ri->pending, n->spec.func.scope);
                return true;

            case RiNode_Spec_Var:
                return ri_resolve_node_(ri, &n->spec.var.type);

            case RiNode_Value_Constant:
                return ri_resolve_node_(ri, &n->value.spec->spec.constant.type);

            case RiNode_Value_Type:
            case RiNode_Value_Func:
            case RiNode_Value_Var:
                // NOTE: Nothing to do, these come resolved from ri_resolve_identifier_().
                return true;

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
#endif

static RIWALK_F(ri_resolve_node_);
static RIWALK_F(ri_resolve_identifier_);

static RIWALK_F(ri_resolve_decl_)
{
    RiNode* decl = *node;

    RI_CHECK(decl->kind == RiNode_Decl);

    RiNode* spec = decl->decl.spec;
    if (spec->kind == RiNode_Id) {
        if (ri_resolve_identifier_(ri, &spec, context) == RiWalk_Error) {
            return RiWalk_Error;
        }
        RI_CHECK(ri_is_in(spec->kind, RiNode_Spec));
    }
    decl->decl.spec = spec;

    if (spec->spec.state == RiSpec_Resolving) {
        ri_error_set_(ri, RiError_CyclicDeclaration, decl->pos, "cyclic declaration");
        return RiWalk_Error;
    } else if (spec->spec.state == RiSpec_Resolved) {
        return RiWalk_Continue;
    }

    spec->spec.state = RiSpec_Resolving;
    if (spec->kind == RiNode_Spec_Module) {
        RiModule* module = ri_import(ri, spec->spec.id);
        if (module == NULL) {
            ri->error.pos = decl->pos;
            return RiWalk_Error;
        }
        spec->spec.module = module->node;
    } else if (ri_walk_(ri, &spec, &ri_resolve_node_, context) == RiWalk_Error) {
        return RiWalk_Error;
    }
    spec->spec.state = RiSpec_Resolved;
    RI_CHECK(decl->decl.spec == spec);

    return RiWalk_Continue;
}

// 1. Takes identifier node
// 2. Looks up corresponding declaration in scope and parents.
// 3. If the declaration is not resolved, walks over the declaration node to resolve it's identifiers.
// 4. Mutates identifier node to a spec taken from the resolved declaration.
//
static RIWALK_F(ri_resolve_identifier_)
{
    RiNode* id = *node;

    LOG("id %S", id->id.name);

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
        return RiWalk_Error;
    }

    RI_CHECK(decl->owner == scope);
    RI_CHECK(decl->kind == RiNode_Decl);

    if (ri_resolve_decl_(ri, &decl, context) == RiWalk_Error) {
        return RiWalk_Error;
    }

    switch (decl->decl.spec->kind)
    {
        case RiNode_Spec_Var:
            id->kind = RiNode_Value_Var;
            id->value.spec = decl->decl.spec;
            // id->value.type = ri_get_spec_(ri, decl->decl.spec->spec.var.type);
            break;

        case RiNode_Spec_Constant:
            id->kind = RiNode_Value_Constant;
            id->value.spec = decl->decl.spec;
            id->value.constant.type = ri_get_spec_(ri, decl->decl.spec->spec.constant.type);
            id->value.constant.literal = decl->decl.spec->spec.constant.value;
            break;

        case RiNode_Spec_Func:
            id->kind = RiNode_Value_Func;
            id->value.spec = decl->decl.spec;
            // id->value.type = ri_get_spec_(ri, decl->decl.spec->spec.func.type);
            break;

        case RiNode_Spec_Module:
            id->kind = RiNode_Value_Module;
            id->value.spec = decl->decl.spec;
            break;

        default:
            RI_CHECK(ri_is_in(decl->decl.spec->kind, RiNode_Spec_Type));
            id->kind = RiNode_Value_Type;
            id->value.spec = decl->decl.spec;
    }
    RI_CHECK(ri_is_in(id->value.spec->kind, RiNode_Spec));

    *node = id;

    return RiWalk_Continue;
}

bool ri_resolve_select_(Ri* ri, RiNode* node, void* context)
{
    RI_CHECK(node->kind == RiNode_Expr_Binary_Select);

    // lib.variable.x.y
    // (expr-select
    //     (expr-select
    //         (expr-select
    //             (id 'lib')
    //             (id 'variable')
    //         )
    //         (id 'x')
    //     )
    //     (id 'y')
    // )

    __debugbreak();
    if (ri_walk_(ri, &node->binary.argument0, &ri_resolve_node_, context) == RiWalk_Error) {
        return false;
    }

    RiNode* left = node->binary.argument0;
    switch (left->kind)
    {
        case RiNode_Spec_Module:

            break;
        default:
            RI_TODO;
            break;
    }

    return true;
}

static RIWALK_F(ri_resolve_node_)
{
    RiNode* n = *node;
    if (ri_is_in(n->kind, RiNode_Spec)) {
        if (n->spec.state == RiSpec_Resolved) {
            return RiWalk_Skip;
        }
    }
    else switch (n->kind)
    {
        case RiNode_Id:
            return ri_resolve_identifier_(ri, node, context);

        case RiNode_Expr_Binary_Select: {
            if (ri_resolve_select_(ri, n, context) == false) {
                return RiWalk_Error;
            }
        } return RiWalk_Skip;

        case RiNode_Expr_Call: {
            if (ri_walk_(ri, &n->call.func, &ri_resolve_node_, context) == RiWalk_Error) {
                return RiWalk_Error;
            }

            RiNode* callable = ri_get_spec_(ri, n->call.func);
            if (ri_is_in(callable->kind, RiNode_Spec_Type))
            {
                if (n->call.arguments.count != 1) {
                    ri_error_set_(ri, RiError_Argument, n->pos, "1 argument expected");
                    return false;
                }
                n->kind = RiNode_Expr_Cast;
                array_insert(&n->call.arguments, 0, callable);
            }
            else switch (callable->kind)
            {
                case RiNode_Spec_Func:
                case RiNode_Spec_Var:
                    break;
                default:
                    RI_UNREACHABLE;
                    break;
            }
        } break;
    }

    return RiWalk_Continue;
}

static bool
ri_resolve_(Ri* ri, RiNode* node)
{
    if (ri_walk_(ri, &node, &ri_resolve_node_, 0) == RiWalk_Error) {
        return false;
    }
    return true;
}

//
//
//

static RiNode* ri_type_patch_node_(Ri* ri, RiNode* node);

static bool
ri_type_patch_cast_const_(Ri* ri, RiNode* expr, RiNode* type)
{
    RI_CHECK(ri_is_in(type->kind, RiNode_Spec_Type));
    RI_CHECK(type->spec.type.identity == type);

    if (ri_is_in(expr->kind, RiNode_Expr_Binary)) {
        return (
            ri_type_patch_cast_const_(ri, expr->binary.argument0, type) &&
            ri_type_patch_cast_const_(ri, expr->binary.argument1, type)
        );
    } else if (ri_is_in(expr->kind, RiNode_Expr_Unary)) {
        return ri_type_patch_cast_const_(ri, expr->unary.argument, type);
    } else if (expr->kind == RiNode_Value_Constant) {
        RI_ASSERT(ri_is_in(expr->value.constant.type->kind, RiNode_Spec_Type_Number_None));
        expr->value.constant.type = type;
        // TODO: We need to be sure that the const type can actually be implicitly cast to `type`.
        return true;
    }
    return true;
}

static RiNode*
ri_type_patch_get_untyped_default_type_(Ri* ri, RiNodeKind untyped)
{
    switch (untyped)
    {
        case RiNode_Spec_Type_Number_None_Int:
            return ri->nodes[RiNode_Spec_Type_Number_Int64];
            break;
        case RiNode_Spec_Type_Number_None_Real:
            return ri->nodes[RiNode_Spec_Type_Number_Float32];
            break;
    }
    RI_UNREACHABLE;
    return NULL;
}

static RiNode*
ri_type_patch_expr_cast_(Ri* ri, RiNode* node)
{
    RI_CHECK(node->call.arguments.count == 2);

    RiNode* type_to = ri_get_spec_(ri, node->call.arguments.items[0]);
    RiNode* type_from = ri_type_patch_node_(ri, node->call.arguments.items[1]);

    // TODO: Warn if the cast is something unexpected (float to pointer, func to func of incompatible type)

    // RiNodeKind type_from_kind = type_from->kind;
    // RiNodeKind type_to_kind = type_to->kind;

    // RI_CHECK(ri_is_in(type_from_kind, RiNode_Spec_Type));
    // RI_CHECK(ri_is_in(type_to_kind, RiNode_Spec_Type));

    // bool from_bool = type_from_kind == RiNode_Spec_Type_Number_Bool;
    // bool from_int = ri_is_in(type_from_kind, RiNode_Spec_Type_Number_Int);
    // bool from_float = ri_is_in(type_from_kind, RiNode_Spec_Type_Number_Float);

    // bool to_bool = type_to_kind == RiNode_Spec_Type_Number_Bool;
    // bool to_int = ri_is_in(type_to_kind, RiNode_Spec_Type_Number_Int);
    // bool to_float = ri_is_in(type_to_kind, RiNode_Spec_Type_Number_Float);

    // if (type_from_kind == type_to_kind) {
    //     // TODO: Warn of unnecessary cast.
    // } else if (from_int && to_int) {
    //     // Allow from any int to any int.
    // } else if (from_int && to_float) {
    //     // Allow from any int to any float.
    // } else if (from_float && to_float) {
    //     // Allow from any float to any float.
    // } else if (from_float && to_int) {
    //     // Allow cast from any float to any int.
    // } else if (from_bool && to_int) {
    //     // Allow bool to any int.
    // } else if (from_bool && to_float) {
    //     // Allow bool to any float.
    // } else {
    //     // TODO: Would fail on Func type.
    //     ri_error_set_(ri, RiError_Type, node->pos, "cannot cast from %S to %S",
    //         ri->nodes[type_from_kind]->spec.id,
    //         ri->nodes[type_to_kind]->spec.id
    //     );
    //     return NULL;
    // }

    return type_to;
}

static RiNode*
ri_type_patch_node_(Ri* ri, RiNode* node)
{
    RiNode* type_none = ri->nodes[RiNode_Spec_Type_None];

    if (ri_is_in(node->kind, RiNode_Expr_Binary_Comparison))
    {
        RiNode* t0 = ri_type_patch_node_(ri, node->binary.argument0);
        RiNode* t1 = ri_type_patch_node_(ri, node->binary.argument1);
        bool d0 = !ri_is_in(t0->kind, RiNode_Spec_Type_Number_None);
        bool d1 = !ri_is_in(t1->kind, RiNode_Spec_Type_Number_None);
        if (t0 == NULL || t1 == NULL) {
            // Error.
            return NULL;
        } else if (ri_type_equal_(ri, t0, t1)) {
            // Types are the same, so return one.
            if (d0 == false) {
                RI_CHECK(d1 == false);
                // Both are not-dominant, so we'll set their respective defaults.
                t0 = t1 = ri_type_patch_get_untyped_default_type_(ri, t0->kind);
                if (!ri_type_patch_cast_const_(ri, node, t0)) {
                    return NULL;
                }
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
                if (!ri_type_patch_cast_const_(ri, node->binary.argument1, t0)) {
                    return NULL;
                }
            } else {
                RI_CHECK(d0 == false && d1 == true);
                // Right is dominant.
                t0 = t1;
                if (!ri_type_patch_cast_const_(ri, node->binary.argument0, t1)) {
                    return NULL;
                }
            }
        }

        // We always return bool.
        return ri->nodes[RiNode_Spec_Type_Number_Bool];
    }
    else if (ri_is_in(node->kind, RiNode_Expr_Binary))
    {
        RiNode* t0 = ri_type_patch_node_(ri, node->binary.argument0);
        RiNode* t1 = ri_type_patch_node_(ri, node->binary.argument1);
        if (t0 == NULL || t1 == NULL) {
            // Error.
            return NULL;
        } else if (ri_type_equal_(ri, t0, t1)) {
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
                if (!ri_type_patch_cast_const_(ri, node->binary.argument1, t0)) {
                    return NULL;
                }
            } else {
                RI_CHECK(d0 == false && d1 == true);
                // Right is dominant.
                // Set left to t1.
                t0 = t1;
                if (!ri_type_patch_cast_const_(ri, node->binary.argument0, t1)) {
                    return NULL;
                }
            }
        }

        return t0;
    } else if (ri_is_in(node->kind, RiNode_Expr_Unary)) {
        return ri_type_patch_node_(ri, node->unary.argument);
    } else if (ri_is_in(node->kind, RiNode_Scope)) {
        RiNode* it;
        array_each(&node->scope.statements, &it) {
            if (!ri_type_patch_node_(ri, it)) {
                return NULL;
            }
        }
        return type_none;
    } else {
        switch (node->kind)
        {
            case RiNode_Module:
                return ri_type_patch_node_(ri, node->module.scope);

            case RiNode_Decl: {
                switch (node->decl.spec->kind) {
                    case RiNode_Spec_Func: {
                        RiNode* f = ri->module->func;
                        ri->module->func = node->decl.spec;
                        if (!ri_type_patch_node_(ri, node->decl.spec->spec.func.scope)) {
                            return NULL;
                        }
                        ri->module->func = f;
                    } break;
                    case RiNode_Spec_Var:
                        break;
                }
                return type_none;
            } break;

            case RiNode_Expr_Cast:
                return ri_type_patch_expr_cast_(ri, node);

            case RiNode_Expr_Call: {
                RiNode* func = ri_get_spec_(ri, node->call.func);
                RiNode* func_type = 0;
                switch (func->kind)
                {
                    case RiNode_Spec_Func:
                        func_type = ri_get_spec_(ri, func->spec.func.type);
                        break;
                    case RiNode_Spec_Var:
                        func_type = ri_get_spec_(ri, func->spec.var.type);
                        break;
                    default:
                        RI_UNREACHABLE;
                        break;
                }

                RiNodeArray* inputs = &func_type->spec.type.func.inputs;
                RiNode* it;
                array_eachi(&node->call.arguments, i, &it)
                {
                    RiNode* t = ri_type_patch_node_(ri, it);
                    if (!t) {
                        return NULL;
                    }
                    if (ri_is_in(t->kind, RiNode_Spec_Type_Number_None)) {
                        // TODO: Cast constant to type required by input argument at `i`.
                        RiNode* arg = array_at(inputs, i);
                        RI_CHECK(arg->kind == RiNode_Decl);
                        RI_CHECK(arg->decl.spec->kind == RiNode_Spec_Var);
                        if (!ri_type_patch_cast_const_(ri, it, ri_get_spec_(ri, arg->decl.spec->spec.var.type))) {
                            return NULL;
                        }
                    }
                }
                return ri_retof_(ri, node);
            }

            case RiNode_St_Expr: {
                if (!ri_type_patch_node_(ri, node->st_expr)) {
                    return NULL;
                }
                return type_none;
            } break;

            case RiNode_St_Return: {
                RiNode* func_type = ri->module->func->spec.func.type;
                RI_CHECK(func_type->kind == RiNode_Spec_Type_Func);
                RiNode* func_output_type = 0;
                if (func_type->spec.type.func.outputs.count > 0) {
                    func_output_type = ri_retof_(ri, func_type->spec.type.func.outputs.items[0]->decl.spec);
                    RI_CHECK(ri_is_in(func_output_type->kind, RiNode_Spec_Type));
                }
                if (node->st_return.argument) {
                    if (func_output_type == NULL) {
                        ri_error_set_(ri, RiError_Argument, node->st_return.argument->pos, "returning value from function with no return type");
                        return NULL;
                    }
                    RiNode* t0 = ri_type_patch_node_(ri, node->st_return.argument);
                    if (!t0) {
                        return NULL;
                    }
                    if (ri_is_in(t0->kind, RiNode_Spec_Type_Number_None)) {
                        if (!ri_type_patch_cast_const_(ri, node->st_return.argument, func_output_type)) {
                            return NULL;
                        }
                    } else if (!ri_type_equal_(ri, t0, func_output_type)) {
                        // TODO: Customize error.
                        ri_error_set_mismatched_types_(ri, node->st_return.argument->pos, t0, func_output_type, "return");
                        return NULL;
                    }
                } else if (func_output_type != NULL) {
                    ri_error_set_(ri, RiError_Argument, node->pos, "function must return a value");
                    return NULL;
                }
                return type_none;
            } break;

            case RiNode_St_Assign: {
                // TODO: Check for l-value.
                RiNode* t0 = NULL;
                if (node->binary.argument0->kind == RiNode_Decl) {
                    t0 = ri_retof_(ri, node->binary.argument0->decl.spec);
                } else {
                    t0 = ri_retof_(ri, node->binary.argument0);
                }
                RiNode* t1 = ri_type_patch_node_(ri, node->binary.argument1);
                if (!t0 || !t1) {
                    return NULL;
                }
                if (t0->kind == RiNode_Spec_Type_Infer) {
                    RI_CHECK(t1->kind != RiNode_Spec_Type_Infer);
                    if (ri_is_in(t1->kind, RiNode_Spec_Type_Number_None)) {
                        // If right is untyped, set it to default.
                        t1 = ri_type_patch_get_untyped_default_type_(ri, t1->kind);
                        if (!ri_type_patch_cast_const_(ri, node->binary.argument1, t1)) {
                            return NULL;
                        }
                    }

                    // Set left type to same type as we now have for right.
                    RiNode* var = node->binary.argument0;
                    RI_CHECK(var->kind == RiNode_Decl);
                    RI_CHECK(var->decl.spec->kind == RiNode_Spec_Var);
                    var->decl.spec->spec.var.type = t1;
                }
                else if (!ri_type_equal_(ri, t0, t1))
                {
                    if (
                        (
                            t0->kind == RiNode_Spec_Type_Number_Float64 ||
                            t0->kind == RiNode_Spec_Type_Number_Float32
                        )
                        &&
                        t1->kind == RiNode_Spec_Type_Number_None_Real
                    ) {
                        if (!ri_type_patch_cast_const_(ri, node->binary.argument1, t0)) {
                            return NULL;
                        }
                    } else if (ri_is_in(t0->kind, RiNode_Spec_Type_Number_Int) &&
                        t1->kind == RiNode_Spec_Type_Number_None_Int
                    ) {
                        if (!ri_type_patch_cast_const_(ri, node->binary.argument1, t0)) {
                            return NULL;
                        }
                    } else {
                        ri_error_set_mismatched_types_(ri, node->pos, t0, t1, "=");
                        return NULL;
                    }
                }
                return type_none;
            } break;

            case RiNode_St_If: {
                if (node->st_if.pre && !ri_type_patch_node_(ri, node->st_if.pre)) {
                    return NULL;
                }
                if (!ri_type_patch_node_(ri, node->st_if.condition)) {
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
ri_type_patch(Ri* ri, RiNode* node)
{
    return ri_type_patch_node_(ri, node);
}

//
//
//

void
rimodule_purge_(RiModule* module)
{
    arena_purge(&module->arena);
    array_purge(&module->pending);
}

void
rimodule_init_(RiModule* module, String path, String id)
{
    memset(module, 0, sizeof(RiModule));
    arena_init(&module->arena, MEGABYTES(1));

    module->id = id;

    String path_dir = ri_path_get_dir_(path);
    module->path.count = path_dir.count;
    module->path.items = arena_push_nt(&module->arena, char, path_dir.count);
    memcpy(module->path.items, path_dir.items, path_dir.count);
}

//
//
//

bool
ri_load(Ri* ri, String id, CharArray* path, ByteArray* stream)
{
    array_clear(stream);

    int i = 0;
    for (; i < ri->paths.count; ++i) {
        array_clear(path);
        chararray_push_f(path, "%S/%S.ri", ri->paths.items[i], id);
        RI_LOG("scanning %S", path->slice);
        array_zero_term(path);
        if (file_read(stream, path->items, NULL)) {
            return true;
        }
    }

    // NOTE: The position is meant to be patched from caller.
    ri_error_set_(ri, RiError_ModuleNotFound, RI_POS_OUTSIDE, "unable to load '%S' on paths:", id);
    for (i = 0; i < ri->paths.count; ++i) {
        chararray_push_f(&ri->error.message, "\n- '%S'", ri->paths.items[i]);
    };
    return false;
}

RiModule*
ri_module(Ri* ri, String path, String id)
{
    RiModule* module = arena_push_t(&ri->arena_, RiModule);
    rimodule_init_(module, path, id);
    return module;
}

bool
ri_parse(Ri* ri, RiModule* module, ByteSlice stream)
{
    RiModule* previous = ri->module;
    ri->module = module;

    RiNode* node = ri_make_node_(&ri->module->arena, ri->scope, (RiPos){0}, RiNode_Module);
    module->node = node;

    RiNode* scope = ri_parse_(ri, stream);
    if (scope == NULL) {
        return false;
    }
    node->module.scope = scope;
    ri->module = previous;

    return true;
}

bool
ri_resolve(Ri* ri, RiModule* module)
{
    RiModule* previous = ri->module;
    ri->module = module;

    if (!ri_resolve_(ri, module->node)) {
        return false;
    }

    if (!ri_type_patch(ri, module->node)) {
        return false;
    }

    ri->module = previous;

    return true;
}

RiModule*
ri_import(Ri* ri, String id)
{
    CharArray path = {0};
    ByteArray stream = {0};

    if (!ri_load(ri, id, &path, &stream)) {
        return NULL;
    }

    RiModule* module = ri_module(ri, path.slice, id);
    array_purge(&path);

    if (!ri_parse(ri, module, stream.slice)) {
        module = NULL;
    }

    array_purge(&stream);

    if (!ri_resolve(ri, module)) {
        module = NULL;
    }

    return module;
}

//
//
//

void
ri_init(Ri* ri, void* host)
{
    // RI_LOG_DEBUG("node %d bytes", sizeof(RiNode));

    memset(ri, 0, sizeof(Ri));
    arena_init(&ri->arena_, MEGABYTES(1));
    intern_init(&ri->intern);

    ri->id_func        = ri_make_id_(ri, S("func")).items;
    ri->id_var         = ri_make_id_(ri, S("var")).items;
    ri->id_const       = ri_make_id_(ri, S("const")).items;
    ri->id_type        = ri_make_id_(ri, S("type")).items;
    ri->id_struct      = ri_make_id_(ri, S("struct")).items;
    ri->id_union       = ri_make_id_(ri, S("union")).items;
    ri->id_enum        = ri_make_id_(ri, S("enum")).items;

    ri->id_import      = ri_make_id_(ri, S("import")).items;

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

    ri->scope = ri_make_scope_(&ri->arena_, 0, RI_POS_OUTSIDE, RiNode_Scope_Root);

    ri->nodes[RiNode_Spec_Type_None] =
        ri_make_node_(&ri->arena_, ri->scope, RI_POS_OUTSIDE, RiNode_Spec_Type_None);
    ri->nodes[RiNode_Spec_Type_None]->spec.id = ri_make_id_(ri, S("none"));
    ri_add_type_identity_(ri, ri->nodes[RiNode_Spec_Type_None]);

    ri->nodes[RiNode_Spec_Type_Number_None_Int] =
        ri_make_node_(&ri->arena_, ri->scope, RI_POS_OUTSIDE, RiNode_Spec_Type_Number_None_Int);
    ri->nodes[RiNode_Spec_Type_Number_None_Int]->spec.id = ri_make_id_(ri, S("untyped-int"));

    ri->nodes[RiNode_Spec_Type_Number_None_Real] =
        ri_make_node_(&ri->arena_, ri->scope, RI_POS_OUTSIDE, RiNode_Spec_Type_Number_None_Real);
    ri->nodes[RiNode_Spec_Type_Number_None_Real]->spec.id = ri_make_id_(ri, S("untyped-real"));

    ri->nodes[RiNode_Spec_Type_Infer] =
        ri_make_node_(&ri->arena_, ri->scope, RI_POS_OUTSIDE, RiNode_Spec_Type_Infer);
    ri->nodes[RiNode_Spec_Type_Infer]->spec.id = ri_make_id_(ri, S("infer"));

    #define DECL_TYPE(Name, Type) { \
        String name = ri_make_id_(ri, S(Name)); \
        RiNode* spec = ri_make_spec_type_number_(&ri->arena_, ri->scope, \
            RI_POS_OUTSIDE, \
            name, \
            RiNode_Spec_Type_ ## Type \
        ); \
        ri->nodes[RiNode_Spec_Type_ ## Type] = spec; \
        ri_add_type_identity_(ri, spec); \
        ri_scope_set_(ri, ri->scope, ri_make_decl_(&ri->arena_, ri->scope, spec->pos, spec)); \
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

    ri_scope_set_(ri, ri->scope,
        ri_make_decl_(&ri->arena_, ri->scope, RI_POS_OUTSIDE,
            ri_make_spec_constant_(&ri->arena_, ri->scope,
                RI_POS_OUTSIDE,
                ri_make_id_(ri, S("host")),
                ri->nodes[RiNode_Spec_Type_Number_UInt64],
                (RiLiteral){ .pointer = host }
            )
        )
    );
}

void
ri_purge(Ri* ri)
{
    RI_LOG_DEBUG("types %d", ri->types.count);
    RiModule* it;
    array_each(&ri->modules, &it) {
        rimodule_purge_(it);
    }
    intern_purge(&ri->intern);
    array_purge(&ri->types);
    arena_purge(&ri->arena_);
}