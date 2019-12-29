#include "ri-ast-dump.h"

#include "ri-print.c"

typedef struct RiDump_ {
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

    RiNode* it;

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
    } else if (ri_is_in(node->kind, RiNode_Scope)) {
        riprinter_print(&D->printer, "(scope\n\t");
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
    } else {
        switch (node->kind)
        {
            case RiNode_Module: {
                riprinter_print(&D->printer, "(module\n\t");
                ri_dump_(D, node->module.scope);
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
                switch (node->value.constant.type->kind) {
                    case RiNode_Spec_Type_Number_Int8:
                    case RiNode_Spec_Type_Number_Int16:
                    case RiNode_Spec_Type_Number_Int32:
                    case RiNode_Spec_Type_Number_UInt8:
                    case RiNode_Spec_Type_Number_UInt16:
                    case RiNode_Spec_Type_Number_UInt32:
                        riprinter_print(&D->printer, "%d\n\t", node->value.constant.literal.integer);
                        break;

                    case RiNode_Spec_Type_Number_UInt64:
                    case RiNode_Spec_Type_Number_None_Int:
                        riprinter_print(&D->printer, "%"PRIu64"\n\t", node->value.constant.literal.integer);
                        break;
                    case RiNode_Spec_Type_Number_Float32:
                    case RiNode_Spec_Type_Number_Float64:
                    case RiNode_Spec_Type_Number_None_Real:
                        riprinter_print(&D->printer, "%f\n\t", node->value.constant.literal.real);
                        break;
                    case RiNode_Spec_Type_Number_Bool:
                        riprinter_print(&D->printer, "%s\n\t",
                            node->value.constant.literal.boolean ? "true" : "false");
                        break;

                    case RiNode_Spec_Type_Number_Int64:
                        riprinter_print(&D->printer, "%"PRIi64"\n\t", node->value.constant.literal.integer);
                        break;

                    default:
                        riprinter_print(&D->printer, "UNKNOWN\n\t");
                        break;
                }
                ri_dump_(D, node->value.constant.type);
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

            case RiNode_Spec_Type_Infer: {
                riprinter_print(&D->printer, "(spec-type-infer)\n");
            } break;

            case RiNode_Spec_Module: {
                riprinter_print(&D->printer, "(spec-module '%S')\n", node->spec.id);
            } break;

            default: {
                riprinter_print(&D->printer, "(UNKNOWN)\n");
            } break;
        }
    }
}

void
ri_dump(RiNode* node, CharArray* buffer)
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
ri_log(RiNode* node)
{
    CharArray buffer = {0};
    ri_dump(node, &buffer);
    RI_LOG("%S", buffer.slice);
    array_purge(&buffer);
}
