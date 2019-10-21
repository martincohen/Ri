#include "rivmc.h"

const char* RIVM_DEBUG_OP_NAMES_[] = {
    #define GROUP_START(Name) [RiVmOp_ ## Name ## _FIRST__] = NULL,
    #define GROUP_END(Name) [RiVmOp_ ## Name ## _LAST__] = NULL,
    #define INST(Name, S) [RiVmOp_ ## Name] = S,
        #include "rivmc-op.h"
    #undef INST
    #undef GROUP_END
    #undef GROUP_START
};

const char* RIVM_DEBUG_TYPE_NAMES_[] = {
    [RiVmValue_None] = "none",
    [RiVmValue_I32] = "i32",
    [RiVmValue_I64] = "i64",
    [RiVmValue_U32] = "u32",
    [RiVmValue_U64] = "u64",
    [RiVmValue_F32] = "f32",
    [RiVmValue_F64] = "f64",
};

const char* RIVM_DEBUG_TYPE_NAMES_SHORT_[] = {
    [RiVmValue_None] = "?",
    [RiVmValue_I32] = "i",
    [RiVmValue_I64] = "I",
    [RiVmValue_U32] = "u",
    [RiVmValue_U64] = "U",
    [RiVmValue_F32] = "f",
    [RiVmValue_F64] = "F",
};

const char* RIVM_DEBUG_SOURCE_NAMES_[] = {
    [RiSlot_Unknown] = "unknown",
    [RiSlot_Input] = "input",
    [RiSlot_Output] = "output",
    [RiSlot_Local] = "local",
    [RiSlot_Global] = "global",
    [RiSlot_Temporary] = "temporary",
};

RiVmOp RIVM_TO_OP_[RiNode_COUNT__] = {
    [RiNode_Expr_Binary_Numeric_Arithmetic_Add] = RiVmOp_Binary_Add,
    [RiNode_Expr_Binary_Numeric_Arithmetic_Sub] = RiVmOp_Binary_Sub,
    [RiNode_Expr_Binary_Numeric_Arithmetic_Mul] = RiVmOp_Binary_Mul,
    [RiNode_Expr_Binary_Numeric_Arithmetic_Div] = RiVmOp_Binary_Div,
    [RiNode_Expr_Binary_Numeric_Arithmetic_Mod] = RiVmOp_Binary_Mod,
    [RiNode_Expr_Binary_Numeric_Bitwise_BXor] = RiVmOp_Binary_BXor,
    [RiNode_Expr_Binary_Numeric_Bitwise_BAnd] = RiVmOp_Binary_BAnd,
    [RiNode_Expr_Binary_Numeric_Bitwise_BOr] = RiVmOp_Binary_BOr,
    [RiNode_Expr_Binary_Numeric_Bitwise_BShL] = RiVmOp_Binary_BShL,
    [RiNode_Expr_Binary_Numeric_Bitwise_BShR] = RiVmOp_Binary_BShR,
    [RiNode_Expr_Binary_Numeric_Boolean_And] = RiVmOp_Binary_And,
    [RiNode_Expr_Binary_Numeric_Boolean_Or] = RiVmOp_Binary_Or,
    [RiNode_Expr_Binary_Comparison_Lt] = RiVmOp_Binary_Comparison_Lt,
    [RiNode_Expr_Binary_Comparison_Gt] = RiVmOp_Binary_Comparison_Gt,
    [RiNode_Expr_Binary_Comparison_LtEq] = RiVmOp_Binary_Comparison_LtEq,
    [RiNode_Expr_Binary_Comparison_GtEq] = RiVmOp_Binary_Comparison_GtEq,
    [RiNode_Expr_Binary_Comparison_Eq] = RiVmOp_Binary_Comparison_Eq,
    [RiNode_Expr_Binary_Comparison_NotEq] = RiVmOp_Binary_Comparison_NotEq,
};

#define rivm_make_param_(Kind, ...) \
    (RiVmParam){ .kind = RiVmParam_ ## Kind, __VA_ARGS__ }

//
//
//

static void*
rivm_push__(RiVmCompiler* rix, iptr size)
{
    void* ptr = arena_push(&rix->arena, size, 8);
    memset(ptr, 0, size);
    return ptr;
}

#define rivm_push_(RiVmC, Type) \
    rivm_push__(RiVmC, sizeof(Type))
//
//
//

void
rivm_init(RiVmCompiler* rix, Ri* ri)
{
    memset(rix, 0, sizeof(RiVmCompiler));
    arena_init(&rix->arena, KILOBYTES(64));
    rix->ri = ri;
}

void
rivm_purge(RiVmCompiler* rix)
{
    arena_purge(&rix->arena);
    array_purge(&rix->code);
    array_purge(&rix->slot_pool);
    array_purge(&rix->labels);
    memset(rix, 0, sizeof(RiVmCompiler));
}

//
//
//

uint32_t
rivm_emit__(RiVmCompiler* rix, const RiVmInst inst)
{
    RI_CHECK(inst.param0.type >= RiVmValue_None);
    RI_CHECK(inst.param0.type < RiVmValue_COUNT__);
    RI_CHECK(inst.param1.type >= RiVmValue_None);
    RI_CHECK(inst.param1.type < RiVmValue_COUNT__);

    if (rivm_op_is_in(inst.op, Binary))
    {
        RI_CHECK(inst.param1.type);
        RI_CHECK(inst.param2.type);
    }
    else
    {
        switch (inst.op)
        {
            case RiVmOp_Nop:
                RI_CHECK(inst.param0.type == RiVmValue_None);
                RI_CHECK(inst.param1.type == RiVmValue_None);
                RI_CHECK(inst.param2.type == RiVmValue_None);
                break;

            case RiVmOp_Ret:
                // Zero or one argument is allowed.
                // RI_CHECK(inst.param0.type == RiVmValue_None);
                RI_CHECK(inst.param1.type == RiVmValue_None);
                RI_CHECK(inst.param2.type == RiVmValue_None);
                break;

            case RiVmOp_Assign:
            case RiVmOp_Store:
            case RiVmOp_Load:
                // TODO: Load/Store target to be 64-bit Unsigned.
                RI_CHECK(inst.param0.type);
                RI_CHECK(inst.param1.type);
                break;

            case RiVmOp_If:
                RI_CHECK(inst.param0.type);
                RI_CHECK(inst.param1.kind == RiVmParam_Label);
                RI_CHECK(inst.param2.kind == RiVmParam_Label);
                break;

            case RiVmOp_GoTo:
                RI_CHECK(inst.param0.kind == RiVmParam_Label);
                break;

            default:
                RI_UNREACHABLE;
                break;
        }
    }

    array_push(&rix->code, inst);
    return (uint32_t)(rix->code.count - 1);
}

#define rivm_emit_(RiVm, Op, ...) \
    rivm_emit__(RiVm, (RiVmInst){ \
        .op = RiVmOp_ ## Op, \
        __VA_ARGS__ \
    })

//
//
//

static RiVmParam
rivm_acquire_slot_(RiVmCompiler* rix, RiVmParamSlotKind kind, RiVmValueType type)
{
    uint32_t index;
    if (rix->slot_pool.count) {
        index = array_at(&rix->slot_pool, 0);
        array_remove(&rix->slot_pool, 0, 1);
    } else {
        rix->slot_next++;
        index = rix->slot_next;
    }

    // RI_LOG_DEBUG("allocating %s slot %d of type %s",
    //     RIVM_DEBUG_SOURCE_NAMES_[kind],
    //     index,
    //     RIVM_DEBUG_TYPE_NAMES_[type]
    // );

    return (RiVmParam) {
        .kind = RiVmParam_Slot,
        .type = type,
        .slot.kind = kind,
        .slot.index = index,
    };
}

static void
rivm_release_slot_(RiVmCompiler* rix, RiVmParam param)
{
    if (param.kind == RiVmParam_Slot) {
        RI_ASSERT(param.slot.index > 0);
        RI_ASSERT(param.slot.kind == RiSlot_Temporary);
        array_push(&rix->slot_pool, param.slot.index);
    } else {
        RI_ASSERT(param.kind == RiVmParam_Imm);
    }
}

static RiVmParam
rivm_create_label_(RiVmCompiler* rix)
{
    array_push(&rix->labels, -1);
    return (RiVmParam) {
        .kind = RiVmParam_Label,
        .label = rix->labels.count
    };
}

static void
rivm_mark_label_(RiVmCompiler* rix, RiVmParam param)
{
    RI_ASSERT(param.kind == RiVmParam_Label);
    RI_ASSERT(param.label > 0);
    array_at(&rix->labels, param.label - 1) = rix->code.count;
}

//
//
//

static RiVmValueType
rivm_get_type_(RiVmCompiler* rix, RiNode* ast_spec_type)
{
    RI_ASSERT(ast_spec_type);
    RI_ASSERT(ri_is_in(ast_spec_type->kind, RiNode_Spec_Type));
    switch (ast_spec_type->kind)
    {
        case RiNode_Spec_Type_Number_Bool:
            return RiVmValue_U32;
        case RiNode_Spec_Type_Number_Int32:
            return RiVmValue_I32;
        case RiNode_Spec_Type_Number_Int64:
            return RiVmValue_I64;
    }
    RI_UNREACHABLE;
    return RiVmValue_None;
}


static RiVmValueType
rivm_get_type_from_expr_(RiVmCompiler* rix, RiNode* ast_expr)
{
    RiNode* ast_type = ri_retof_(rix->ri, ast_expr);
    return rivm_get_type_(rix, ast_type);
}

static RiVmParam
rivm_get_param_(RiVmCompiler* rix, RiNode* ast_var)
{
    RI_ASSERT(ast_var->kind == RiNode_Value_Var);
    RiNode* ast_spec = ast_var->value.spec;
    RiVmValueType type = rivm_get_type_from_expr_(rix, ast_var);
    if (ast_spec->spec.var.slot == 0) {
        RiVmParam param = rivm_acquire_slot_(
            rix, RiSlot_Local, type);
        ast_spec->spec.var.slot = param.slot.index;
        return param;
    }
    return (RiVmParam){
        .kind = RiVmParam_Slot,
        .type = type,
        .slot.kind = RiSlot_Local,
        .slot.index = ast_spec->spec.var.slot
    };
}

//
//
//

static RiVmParam
rivm_compile_expr_(RiVmCompiler* rix, RiNode* ast_expr)
{
    RI_ASSERT(
        ri_is_in(ast_expr->kind, RiNode_Expr) ||
        ast_expr->kind == RiNode_Value_Var ||
        ast_expr->kind == RiNode_Value_Const
    );

    if (ri_is_in(ast_expr->kind, RiNode_Expr_Binary)) {
        RiNode* a0 = ast_expr->binary.argument0;
        RiNode* a1 = ast_expr->binary.argument1;
        RiVmParam result = rivm_acquire_slot_(rix,
            RiSlot_Temporary,
            ri_is_in(ast_expr->kind, RiNode_Expr_Binary_Comparison)
                ? RiVmValue_I32
                : rivm_get_type_from_expr_(rix, a0)
        );
        RiVmParam p0 = rivm_compile_expr_(rix, a0);
        RiVmParam p1 = rivm_compile_expr_(rix, a1);

        RiVmOp op = RIVM_TO_OP_[ast_expr->kind];
        RI_ASSERT(op);
        rivm_emit__(rix, (RiVmInst) {
            .op = op, result, p0, p1
        });
        return result;
    } else {
        switch (ast_expr->kind)
        {
            case RiNode_Value_Var:
                return rivm_get_param_(rix, ast_expr);

            case RiNode_Value_Const: {
                RiVmValueType type = rivm_get_type_from_expr_(rix, ast_expr);
                switch (type)
                {
                    case RiVmValue_I32:
                        return rivm_make_param_(Imm, .type = type, .imm.i32 = (int32_t)ast_expr->value.constant.integer);
                    case RiVmValue_I64:
                        return rivm_make_param_(Imm, .type = type, .imm.i64 = (int64_t)ast_expr->value.constant.integer);
                    default:
                        RI_UNREACHABLE;
                        break;
                }
            } break;
        }
    }
    RI_ABORT("invalid expr");
    return (RiVmParam){0};
}

static void
rivm_compile_st_(RiVmCompiler* rix, RiNode* ast_st)
{
    RI_ASSERT(ast_st);

    switch (ast_st->kind)
    {
        case RiNode_Decl: {
            // Skip.
        } break;

        case RiNode_Scope: {
            RiNode* it;
            array_each(&ast_st->scope.statements, &it) {
                rivm_compile_st_(rix, it);
            }
        } break;

        case RiNode_St_Return: {
            RiVmParam result = rivm_compile_expr_(rix, ast_st->st_return.argument);
            rivm_emit_(rix, Ret, result);
        } break;

        case RiNode_St_Assign: {
            RiVmParam result = rivm_compile_expr_(rix, ast_st->binary.argument1);
            RiVmParam target = rivm_get_param_(rix, ast_st->binary.argument0);
            rivm_emit_(rix, Assign, target, result);
            rivm_release_slot_(rix, result);
        } break;

        case RiNode_St_If: {
            if (ast_st->st_if.pre) {
                rivm_compile_st_(rix, ast_st->st_if.pre);
            }
            RiVmParam condition = rivm_compile_expr_(rix, ast_st->st_if.condition);
            RiVmParam label_then = rivm_create_label_(rix);
            RiVmParam label_else = rivm_create_label_(rix);
            rivm_emit_(rix, If, condition, label_then, label_else);
            rivm_release_slot_(rix, condition);

            RiNodeArray* statements = &ast_st->st_if.scope->scope.statements;
            rivm_mark_label_(rix, label_then);
            rivm_compile_st_(rix, array_at(statements, 0));
            if (statements->count > 1) {
                RiVmParam label_end  = rivm_create_label_(rix);
                rivm_emit_(rix, GoTo, label_end);
                rivm_mark_label_(rix, label_else);
                rivm_compile_st_(rix, array_at(statements, 1));
                rivm_mark_label_(rix, label_end);
            } else {
                rivm_mark_label_(rix, label_else);
            }
        } break;

        default:
            RI_UNREACHABLE;
            break;
    }
}

static void
rivm_acquire_func_args_(RiVmCompiler* rix, RiVmParamSlotKind source, RiNodeArray* args)
{
    // TODO: Only named args.
    for (iptr i = 0; i < args->count; ++i) {
        RI_ASSERT(args->items[i]->kind == RiNode_Decl);
        RI_ASSERT(args->items[i]->decl.spec->kind == RiNode_Spec_Var);
        // TODO: Extracting a type of var decl is a bit too awkward to do. Is there a better way?
        RiVmParam param = rivm_acquire_slot_(rix,
            source,
            rivm_get_type_(rix,
                ri_get_spec_(rix->ri,
                    args->items[i]->decl.spec->spec.var.type
                )
            )
        );
        args->items[i]->decl.spec->spec.var.slot = param.slot.index;
    }
}

static RiVmFunc*
rivm_compile_func_(RiVmCompiler* rix, RiNode* ast_func)
{
    rix->ast_func = ast_func;

    RiNode* ast_func_type = ast_func->spec.func.type;
    // Allocate slots for input and output parameters.
    rivm_acquire_func_args_(rix, RiSlot_Input, &ast_func_type->spec.type.func.inputs);
    rivm_acquire_func_args_(rix, RiSlot_Output, &ast_func_type->spec.type.func.outputs);
    rivm_compile_st_(rix, ast_func->spec.func.scope);

    RiVmFunc* func = rivm_push_(rix, RiVmFunc);
    func->code = rix->code.slice;

    rix->code = (RiVmInstArray){0};
    array_clear(&rix->slot_pool);
    rix->slot_next = 0;

    array_push(&rix->func, func);
    return func;
}

RiVmModule*
rivm_compile(RiVmCompiler* rix, RiNode* ast_module)
{
    RI_ASSERT(ast_module->kind == RiNode_Module);
    RiNode* ast_scope = ast_module->module.scope;

    RiNode** ast_decl = ast_scope->scope.decl.items;
    for (iptr i = 0; i < ast_scope->scope.decl.count; ++i, ++ast_decl)
    {
        RI_ASSERT((*ast_decl)->kind == RiNode_Decl);
        RiNode* ast_spec = (*ast_decl)->decl.spec;
        if (ast_spec->kind == RiNode_Spec_Func) {
            rivm_compile_func_(rix, ast_spec);
        }
    }

    RiVmModule* module = rivm_push_(rix, RiVmModule);
    module->func = rix->func.slice;
    rix->func = (RiVmFuncArray){0};

    return module;
}

//
// Dump
//

// #define RIVM_DUMP_PARAM_TYPE_ ".%s"

#ifndef RIVM_DUMP_PARAM_TYPE_
#define RIVM_DUMP_PARAM_TYPE_ ""
#endif

static void
rivm_dump_param_(RiVmParam* param, CharArray* out)
{
    switch (param->kind)
    {
        case RiVmParam_None:
            break;
        case RiVmParam_Imm:
            switch (param->type) {
                case RiVmValue_I32: chararray_push_f(out, "%"PRIi32 RIVM_DUMP_PARAM_TYPE_ "", param->imm.i32, RIVM_DEBUG_TYPE_NAMES_SHORT_[param->type]); break;
                case RiVmValue_I64: chararray_push_f(out, "%"PRIi64 RIVM_DUMP_PARAM_TYPE_ "", param->imm.i64, RIVM_DEBUG_TYPE_NAMES_SHORT_[param->type]); break;
                case RiVmValue_U32: chararray_push_f(out, "%"PRIu32 RIVM_DUMP_PARAM_TYPE_ "", param->imm.u32, RIVM_DEBUG_TYPE_NAMES_SHORT_[param->type]); break;
                case RiVmValue_U64: chararray_push_f(out, "%"PRIu64 RIVM_DUMP_PARAM_TYPE_ "", param->imm.u64, RIVM_DEBUG_TYPE_NAMES_SHORT_[param->type]); break;
                default: RI_UNREACHABLE; break;
            }
            break;
        case RiVmParam_Label:
            chararray_push_f(out, "_%d", param->label);
            break;
        case RiVmParam_Slot:
            chararray_push_f(out, "t%d" RIVM_DUMP_PARAM_TYPE_, param->slot.index, RIVM_DEBUG_TYPE_NAMES_SHORT_[param->type]);
            break;
        case RiVmParam_Func:
            chararray_push_f(out, "func%d", param->func);
            break;
        default:
            RI_UNREACHABLE;
            break;
    }
}

#undef RIVM_DUMP_PARAM_TYPE_

void
rivm_dump_labels(RiVmCompiler* rix, CharArray* out, iptr address)
{
    for (iptr i = 0; i < rix->labels.count; ++i) {
        if (rix->labels.items[i] == address) {
            chararray_push_f(out, "_%d:\n", i + 1);
        }
    }
}

void
rivm_dump_func(RiVmCompiler* rix, RiVmFunc* func, CharArray* out)
{
    RiVmInst* it = func->code.items;
    CharArray s0 = {0};
    CharArray s1 = {0};
    CharArray s2 = {0};

    for (iptr i = 0; i < func->code.count; ++i, ++it)
    {
        rivm_dump_labels(rix, out, i);

        array_clear(&s0);
        array_clear(&s1);
        array_clear(&s2);
        rivm_dump_param_(&it->param0, &s0);
        rivm_dump_param_(&it->param1, &s1);
        rivm_dump_param_(&it->param2, &s2);

        const char* sop = RIVM_DEBUG_OP_NAMES_[it->op];

        chararray_push_f(out, "    (");
        if (rivm_op_is_in(it->op, Binary)) {
            chararray_push_f(out, "%S = %S %s %S", s0, s1, sop, s2);
        } else {
            switch (it->op)
            {
                case RiVmOp_Assign:
                    chararray_push_f(out, "%S = %S", s0, s1);
                    break;

                case RiVmOp_If:
                    chararray_push_f(out, "%s %S != 0 then (goto %S) else (goto %S)", sop, s0, s1, s2);
                    break;

                default:
                    chararray_push_f(out, "%s", sop);
                    if (it->param0.kind) {
                        chararray_push_f(out, " %S", s0);
                    }
                    if (it->param1.kind) {
                        chararray_push_f(out, " %S", s1);
                    }
                    if (it->param2.kind) {
                        chararray_push_f(out, " %S", s2);
                    }
                    break;
            }
        }
        chararray_push_f(out, ")\n");
    }

    array_purge(&s0);
    array_purge(&s1);
    array_purge(&s2);
}

void
rivm_dump_module(RiVmCompiler* rix, RiVmModule* module, CharArray* out)
{
    RiVmFunc* it;
    slice_eachi(&module->func, i, &it) {
        chararray_push_f(out, "func%d:\n", i);
        rivm_dump_func(rix, it, out);
    }
}