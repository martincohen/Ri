#include "rivm-compiler.h"
#include "rivm-dump.h"

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

//
//
//

void
rivm_init(RiVmCompiler* compiler, Ri* ri)
{
    memset(compiler, 0, sizeof(RiVmCompiler));
    compiler->ri = ri;
}

void
rivm_purge(RiVmCompiler* compiler)
{
    array_purge(&compiler->slot_pool);
    array_purge(&compiler->labels);
    memset(compiler, 0, sizeof(RiVmCompiler));
}

//
//
//

uint32_t
rivm_code_emit_(RiVmInstArray* code, const RiVmInst inst)
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
                // No arguments, we're storing the result to output slot.
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

            case RiVmOp_AddrOf:
                RI_CHECK(inst.param0.kind == RiVmParam_Slot);
                RI_CHECK(inst.param1.kind == RiVmParam_Slot);
                break;

            case RiVmOp_If:
                RI_CHECK(inst.param0.type);
                RI_CHECK(inst.param1.kind == RiVmParam_Label);
                RI_CHECK(inst.param2.kind == RiVmParam_Label);
                break;

            case RiVmOp_GoTo:
                RI_CHECK(inst.param0.kind == RiVmParam_Label);
                break;

            case RiVmOp_Enter:
                // Eventually gets changed to Imm.
                RI_CHECK(inst.param0.kind == RiVmParam_None);
                break;

            case RiVmOp_Call:
                RI_CHECK(inst.param0.kind == RiVmParam_Slot);
                RI_CHECK(inst.param1.kind == RiVmParam_Func);
                RI_CHECK(inst.param1.func != 0);
                break;

            case RiVmOp_ArgPush:
                RI_CHECK(inst.param0.kind != RiVmParam_None);
                break;

            case RiVmOp_ArgPopN:
                RI_CHECK(inst.param0.kind == RiVmParam_Imm);
                break;

            default:
                RI_UNREACHABLE;
                break;
        }
    }

    array_push(code, inst);
    return (uint32_t)(code->count - 1);
}

#define rivm_code_emit(RiVmInstArray, Op, ...) \
    rivm_code_emit_(RiVmInstArray, (RiVmInst){ \
        .op = RiVmOp_ ## Op, \
        __VA_ARGS__ \
    })

//
//
//

static RiVmParam
rivm_acquire_slot_(RiVmCompiler* compiler, RiVmParamSlotKind kind, RiVmValueType type)
{
    uint32_t index;
    if (compiler->slot_pool.count) {
        index = array_at(&compiler->slot_pool, 0);
        array_remove(&compiler->slot_pool, 0, 1);
    } else {
        index = compiler->slot_next;
        compiler->slot_next++;
    }

    RiVmParam param = (RiVmParam) {
        .kind = RiVmParam_Slot,
        .type = type,
        .slot.kind = kind,
        .slot.index = index,
    };

    if (compiler->slot.count <= index) {
        array_resize(&compiler->slot, index + 1);
    }
    array_at(&compiler->slot, index) = param;

    // RI_LOG_DEBUG("allocating %s slot %d of type %s",
    //     RIVM_DEBUG_SOURCE_NAMES_[kind],
    //     index,
    //     RIVM_DEBUG_TYPE_NAMES_[type]
    // );

    return param;
}

static void
rivm_release_slot_(RiVmCompiler* compiler, RiVmParam param)
{
    if (param.kind == RiVmParam_Slot) {
        if (param.slot.kind == RiSlot_Temporary) {
            RI_ASSERT(param.slot.index >= 0);
            param.kind = RiVmParam_None;
            array_push(&compiler->slot_pool, param.slot.index);
        }
    } else {
        RI_ASSERT(param.kind == RiVmParam_Imm);
    }
}

static RiVmParam
rivm_get_slot_param_(RiVmCompiler* compiler, uint32_t index)
{
    return array_at(&compiler->slot, index);
}

//
//
//

static RiVmParam
rivm_create_label_(RiVmCompiler* compiler)
{
    array_push(&compiler->labels, -1);
    return (RiVmParam) {
        .kind = RiVmParam_Label,
        .label = compiler->labels.count
    };
}

static void
rivm_mark_label_(RiVmCompiler* compiler, RiVmParam param)
{
    RI_ASSERT(param.kind == RiVmParam_Label);
    RI_ASSERT(param.label > 0);
    array_at(&compiler->labels, param.label - 1) = compiler->code.count;
}

//
//
//

static RiVmValueType
rivm_get_type_(RiVmCompiler* compiler, RiNode* ast_spec_type)
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
rivm_get_type_from_expr_(RiVmCompiler* compiler, RiNode* ast_expr)
{
    RiNode* ast_type = ri_retof_(compiler->ri, ast_expr);
    return rivm_get_type_(compiler, ast_type);
}

static RiVmParam
rivm_get_param_(RiVmCompiler* compiler, RiNode* ast_var)
{
    RI_ASSERT(ast_var->kind == RiNode_Value_Var);
    RiNode* ast_spec = ast_var->value.spec;
    RiVmValueType type = rivm_get_type_from_expr_(compiler, ast_var);
    if (ast_spec->spec.var.slot == RI_INVALID_SLOT) {
        RiVmParam param = rivm_acquire_slot_(
            compiler, RiSlot_Local, type);
        ast_spec->spec.var.slot = param.slot.index;
        return param;
    }

    return rivm_get_slot_param_(compiler, ast_spec->spec.var.slot);
}

static RiVmParam
rivm_get_param_for_output_(RiVmCompiler* compiler, int output_index)
{
    RiNode* ast_func_type = compiler->ast_func->spec.func.type;
    RiNode* output = array_at(&ast_func_type->spec.type.func.outputs, output_index);
    return rivm_get_slot_param_(compiler, output->decl.spec->spec.var.slot);
}

//
//
//

// static void
// rivm_compile_st_(RiVmCompiler* compiler, RiNode* ast_st)
// {
//     switch (ast_st->kind)
//     {
//     }
// }

static RiVmParam
rivm_compile_expr_(RiVmCompiler* compiler, RiNode* ast_expr)
{
    RI_ASSERT(
        ri_is_in(ast_expr->kind, RiNode_Expr) ||
        ast_expr->kind == RiNode_Value_Var ||
        ast_expr->kind == RiNode_Value_Const
    );

    if (ri_is_in(ast_expr->kind, RiNode_Expr_Binary)) {
        RiNode* a0 = ast_expr->binary.argument0;
        RiNode* a1 = ast_expr->binary.argument1;
        RiVmParam result = rivm_acquire_slot_(compiler,
            RiSlot_Temporary,
            ri_is_in(ast_expr->kind, RiNode_Expr_Binary_Comparison)
                ? RiVmValue_I32
                : rivm_get_type_from_expr_(compiler, a0)
        );
        RiVmParam p0 = rivm_compile_expr_(compiler, a0);
        RiVmParam p1 = rivm_compile_expr_(compiler, a1);

        RiVmOp op = RIVM_TO_OP_[ast_expr->kind];
        RI_ASSERT(op);
        rivm_code_emit_(&compiler->code, (RiVmInst) {
            .op = op, result, p0, p1
        });
        return result;
    } else {
        switch (ast_expr->kind)
        {
            case RiNode_Expr_Call: {
                RiNodeArray* arguments = &ast_expr->call.arguments;
                for (iptr i = 0; i < arguments->count; ++i)
                {
                    RiVmParam arg = rivm_compile_expr_(compiler, arguments->items[i]);
                    rivm_code_emit(&compiler->code, ArgPush, arg);
                    rivm_release_slot_(compiler, arg);
                }

                RiNode* spec = ast_expr->call.func->value.spec;
                RiNode* type = spec->spec.func.type;
                RI_ASSERT(type->spec.type.func.outputs.count < 2);

                // RiVmParam result = {0};
                // for (iptr i = 0; i < type->spec.type.func.outputs.count; ++i) {
                //     result = rivm_acquire_slot_(compiler, RiSlot_Temporary, RiVmValue_I32);
                //     RiVmParam result_addr = rivm_acquire_slot_(compiler, RiSlot_Temporary, RiVmValue_U64);
                //     rivm_code_emit(&compiler->code, AddrOf, result_addr, result);
                //     rivm_code_emit(&compiler->code, ArgPush, result_addr);
                // }

                RiVmParam result = result = rivm_acquire_slot_(compiler, RiSlot_Temporary, RiVmValue_I32);

                rivm_code_emit(&compiler->code,
                    Call,
                    result,
                    rivm_make_param(Func,
                        .func = spec
                    )
                );

                rivm_code_emit(&compiler->code,
                    ArgPopN,
                    rivm_make_param(Imm,
                        .type = RiVmValue_U64,
                        .imm.i64 = arguments->count
                    )
                );

                return result;
            }

            case RiNode_Value_Var:
                return rivm_get_param_(compiler, ast_expr);

            case RiNode_Value_Const: {
                RiVmValueType type = rivm_get_type_from_expr_(compiler, ast_expr);
                switch (type)
                {
                    case RiVmValue_I32:
                        return rivm_make_param(Imm, .type = type, .imm.i32 = (int32_t)ast_expr->value.constant.integer);
                    case RiVmValue_I64:
                        return rivm_make_param(Imm, .type = type, .imm.i64 = (int64_t)ast_expr->value.constant.integer);
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
rivm_compile_st_(RiVmCompiler* compiler, RiNode* ast_st)
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
                rivm_compile_st_(compiler, it);
            }
        } break;

        case RiNode_St_Return: {
            if (ast_st->st_return.argument) {
                // TODO: Optimization: Allow for rivm_compile_expr_ to take target for expression result? This can also be done as an additional optimization step.
                RiVmParam result = rivm_compile_expr_(compiler, ast_st->st_return.argument);
                rivm_code_emit(&compiler->code, Ret, result);
                // RiVmParam target = rivm_get_param_for_output_(compiler, 0);
                // rivm_code_emit(&compiler->code, Assign, target, result);
            } else {
                rivm_code_emit(&compiler->code, Ret);
            }
        } break;

        case RiNode_St_Assign: {
            RiVmParam result = rivm_compile_expr_(compiler, ast_st->binary.argument1);
            RiVmParam target = rivm_get_param_(compiler, ast_st->binary.argument0);
            rivm_code_emit(&compiler->code, Assign, target, result);
            rivm_release_slot_(compiler, result);
        } break;

        case RiNode_St_If: {
            if (ast_st->st_if.pre) {
                rivm_compile_st_(compiler, ast_st->st_if.pre);
            }
            RiVmParam condition = rivm_compile_expr_(compiler, ast_st->st_if.condition);
            RiVmParam label_then = rivm_create_label_(compiler);
            RiVmParam label_else = rivm_create_label_(compiler);
            rivm_code_emit(&compiler->code, If, condition, label_then, label_else);
            rivm_release_slot_(compiler, condition);

            RiNodeArray* statements = &ast_st->st_if.scope->scope.statements;
            rivm_mark_label_(compiler, label_then);
            rivm_compile_st_(compiler, array_at(statements, 0));
            if (statements->count > 1) {
                RiVmParam label_end  = rivm_create_label_(compiler);
                rivm_code_emit(&compiler->code, GoTo, label_end);
                rivm_mark_label_(compiler, label_else);
                rivm_compile_st_(compiler, array_at(statements, 1));
                rivm_mark_label_(compiler, label_end);
            } else {
                rivm_mark_label_(compiler, label_else);
            }
        } break;

        default:
            RI_UNREACHABLE;
            break;
    }
}

static void
rivm_acquire_func_args_(RiVmCompiler* compiler, RiVmParamSlotKind source, RiNodeArray* args)
{
    // TODO: Only named args.
    for (iptr i = 0; i < args->count; ++i) {
        RI_ASSERT(args->items[i]->kind == RiNode_Decl);
        RI_ASSERT(args->items[i]->decl.spec->kind == RiNode_Spec_Var);
        // TODO: Extracting a type of var decl is a bit too awkward to do. Is there a better way?
        RiVmParam param = rivm_acquire_slot_(compiler,
            source,
            rivm_get_type_(compiler,
                ri_get_spec_(compiler->ri,
                    args->items[i]->decl.spec->spec.var.type
                )
            )
        );
        args->items[i]->decl.spec->spec.var.slot = param.slot.index;
    }
}

static RiVmFunc*
rivm_compile_func_(RiVmCompiler* compiler, RiNode* ast_func)
{
    compiler->ast_func = ast_func;

    RiNode* ast_func_type = ast_func->spec.func.type;
    // Allocate slots for input and output parameters.
    rivm_acquire_func_args_(compiler, RiSlot_Input, &ast_func_type->spec.type.func.inputs);
    // TODO: Multiple return values.
    RI_ASSERT(ast_func_type->spec.type.func.outputs.count < 2);
    // rivm_acquire_func_args_(compiler, RiSlot_Output, &ast_func_type->spec.type.func.outputs);

    uint32_t slot_locals = compiler->slot_next;
    uint32_t enter_index = rivm_code_emit(&compiler->code, Enter);
    {
        rivm_compile_st_(compiler, ast_func->spec.func.scope);
    }
    RiVmInst* enter = &array_at(&compiler->code, enter_index);
    enter->param0 = rivm_make_param(Imm,
        .type = RiVmValue_U64,
        .imm.u64 = compiler->slot_next - slot_locals
    );
    ast_func->spec.func.slot = compiler->module->func.count;

    RiVmFunc* func = rivm_module_push_func(compiler->module, compiler->code.slice);
    func->debug_inputs_count = ast_func_type->spec.type.func.inputs.count;
    func->debug_outputs_count = ast_func_type->spec.type.func.outputs.count;
    compiler->code = (RiVmInstArray){0};

    array_clear(&compiler->slot_pool);
    compiler->slot_next = 0;

    return func;
}

static void
rivm_patch_label_(RiVmCompiler* compiler, RiVmParam* param)
{
    param->kind = RiVmParam_Imm;
    param->type = RiVmValue_U64;
    param->imm.u64 = array_at(&compiler->labels, param->label - 1);
}

static void
rivm_patch_(RiVmCompiler* compiler, RiVmModule* module)
{
    RiVmFunc* func;
    RiVmInst* inst;
    slice_each(&module->func, &func)
    {
        for (int64_t i = 0; i < func->code.count; ++i) {
            inst = &func->code.items[i];
            switch (inst->op)
            {
                case RiVmOp_Call: {
                    RI_CHECK(inst->param1.kind == RiVmParam_Func);
                    RiNode* ast_func = inst->param1.func;
                    RI_CHECK(ast_func);
                    RI_CHECK(ast_func->spec.func.slot != RI_INVALID_SLOT);
                    RiVmFunc* func = array_at(&module->func, ast_func->spec.func.slot);
                    inst->param1.func = func;
                } break;

                case RiVmOp_If: {
                    rivm_patch_label_(compiler, &inst->param1);
                    rivm_patch_label_(compiler, &inst->param2);
                } break;

                case RiVmOp_GoTo: {
                    rivm_patch_label_(compiler, &inst->param0);
                } break;
            }
        }
    }
}

bool
rivm_compile(RiVmCompiler* compiler, RiNode* ast_module, RiVmModule* module)
{
    compiler->module = module;

    RI_ASSERT(ast_module->kind == RiNode_Module);
    RiNode* ast_scope = ast_module->module.scope;

    RiNode** ast_decl = ast_scope->scope.decl.items;
    for (iptr i = 0; i < ast_scope->scope.decl.count; ++i, ++ast_decl)
    {
        RI_ASSERT((*ast_decl)->kind == RiNode_Decl);
        RiNode* ast_spec = (*ast_decl)->decl.spec;
        if (ast_spec->kind == RiNode_Spec_Func) {
            rivm_compile_func_(compiler, ast_spec);
        }
    }

    rivm_patch_(compiler, module);

    return true;
}

//
//
//

bool
rivm_compile_file(String path, RiVmModule* module)
{
    Ri ri;
    ri_init(&ri);
    CharArray out = {0};

    RiNode* ast_module = NULL;

    {
        CharArray path_source = {0};
        chararray_push(&path_source, path);
        array_zero_term(&path_source);
            ByteArray source = {0};
            ASSERT(file_read(&source, path_source.items, 0));
            ast_module = ri_build(&ri, S((char*)source.items, source.count), path_source.slice);
            ASSERT(ast_module);
            array_purge(&source);
        array_purge(&path_source);
    }

    ri_dump(&ri, ast_module, &out);
    LOG("%S", out);

    RiVmCompiler compiler;
    rivm_init(&compiler, &ri);
    if (!rivm_compile(&compiler, ast_module, module)) {
        // TODO: Return error code.
        return false;
    }

    array_clear(&out);
    rivm_dump_module(module, &out);
    LOG("%S", out);

    array_purge(&out);
    rivm_purge(&compiler);
    ri_purge(&ri);

    // __debugbreak();

    return module;
}