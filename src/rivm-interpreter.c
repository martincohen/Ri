#include "rivm-interpreter.h"

// Implementation of calling convetion inside of VM:
// - Caller pushes input arguments.
// - Caller calls callee with a stack pointer pointing to first input argument.
// - Callee reserves a value on C side for return value that is set when `ret <expr>` is called.
// - Callee pushes space needed for it's local and temporary variables. (`enter N`)
// - Callee uses slot indices in instruction params to operate over inputs, outputs, locals and temporaries.
// - Callee pops space needed for it's local and temporary variables. (`leave N`)
// - Caller pops space needed for input and output arguments.

void
rivm_exec_init(RiVmExec* context)
{
    memset(context, 0, sizeof(RiVmExec));
    iptr capacity = MEGABYTES(1);
    context->stack.start = virtual_alloc(0, capacity);
    context->stack.it = context->stack.start;
    context->stack.end = context->stack.it + (capacity / sizeof(RiVmValue));
}

void
rivm_exec_purge(RiVmExec* context)
{
    virtual_free(context->stack.it, context->stack.end - context->stack.end);
}

//
// Stack
//

static inline RiVmValue*
rivm_stack_push(RiVmStack* stack, iptr count)
{
    RI_ASSERT((stack->it + count) <= stack->end);
    RiVmValue* r = stack->it;
    stack->it += count;
    return r;
}

static inline RiVmValue*
rivm_stack_pop(RiVmStack* stack, iptr count)
{
    RI_ASSERT((stack->it - count) >= stack->start);
    RiVmValue* r = stack->it;
    stack->it -= count;
    return r;
}

//
//
//

#define get_local(Param) \
    stack[ \
        RI_CHECK((Param).kind == RiVmParam_Slot), \
        (Param).slot.index \
    ]

#define binary_op_tt(Member, TT, Op) \
    switch (TT) { \
        case RiVmParam_SlotSlot: get_local(inst->param0). ## Member = get_local(inst->param1). ## Member Op get_local(inst->param2). ## Member; break; \
        case RiVmParam_SlotImm:  get_local(inst->param0). ## Member = get_local(inst->param1). ## Member Op inst->param2.imm. ## Member; break; \
        case RiVmParam_ImmSlot:  get_local(inst->param0). ## Member = inst->param1.imm. ## Member        Op get_local(inst->param2). ## Member; break; \
        case RiVmParam_ImmImm:   get_local(inst->param0). ## Member = inst->param1.imm. ## Member        Op inst->param2.imm. ## Member; break; \
        default: RI_UNREACHABLE; break; \
    }

#define binary_op(Op) { \
        int tt = RIVMPARAMKIND_PAIR(inst->param1.kind, inst->param2.kind); \
        switch (inst->param0.type) { \
            case RiVmValue_I32: binary_op_tt(i32, tt, Op); break; \
            case RiVmValue_I64: binary_op_tt(i64, tt, Op); break; \
            case RiVmValue_U32: binary_op_tt(u32, tt, Op); break; \
            case RiVmValue_U64: binary_op_tt(u64, tt, Op); break; \
            case RiVmValue_F32: binary_op_tt(f32, tt, Op); break; \
            case RiVmValue_F64: binary_op_tt(f64, tt, Op); break; \
            default: RI_UNREACHABLE; break; \
        } \
    }

RiVmValue
rivm_exec_(RiVmExec* context, RiVmValue* stack, RiVmFunc* func)
{
    RiVmInst* inst;
    int64_t i = 0;

    RiVmValue result;
    RiVmValue* callee_stack = NULL;
    uint64_t locals_count = 0;

    for (;;)
    {
        inst = &array_at(&func->code, i);
        ++i;

        switch (inst->op)
        {
            case RiVmOp_Enter:
                locals_count = inst->param0.imm.u64;
                rivm_stack_push(&context->stack, locals_count);
                callee_stack = context->stack.it;
                break;

            case RiVmOp_Ret:
                switch (inst->param0.kind)
                {
                    case RiVmParam_Imm:
                        result.u64 = inst->param0.imm.u64;
                        break;
                    case RiVmParam_Slot:
                        result.u64 = get_local(inst->param0).u64;
                        break;
                    default: RI_UNREACHABLE; break;
                }
                rivm_stack_pop(&context->stack, locals_count);
                goto end;

            case RiVmOp_Assign:
                switch (inst->param1.kind)
                {
                    case RiVmParam_Imm:
                        get_local(inst->param0).u64 = inst->param1.imm.u64;
                        break;
                    case RiVmParam_Slot:
                        get_local(inst->param0).u64 = get_local(inst->param1).u64;
                        break;
                    default: RI_UNREACHABLE; break;
                }
                break;

            case RiVmOp_ArgPush: {
                switch (inst->param0.kind)
                {
                    case RiVmParam_Imm:
                        rivm_stack_push(&context->stack, 1)->u64 = inst->param0.imm.u64;
                        break;
                    case RiVmParam_Slot:
                        rivm_stack_push(&context->stack, 1)->u64 = get_local(inst->param0).u64;
                        break;
                    default: RI_UNREACHABLE; break;
                }
            } break;

            case RiVmOp_Call: {
                RiVmFunc* callee_func = inst->param1.func;
                RiVmValue callee_result = rivm_exec_(context, callee_stack, callee_func);
                get_local(inst->param0).u64 = callee_result.u64;
                rivm_stack_pop(&context->stack, inst->param2.imm.u64);
            } break;

            case RiVmOp_If: {
                switch (inst->param0.kind)
                {
                    case RiVmParam_Imm:
                        if (inst->param0.imm.u64) {
                            i = inst->param1.imm.i64;
                        } else {
                            i = inst->param2.imm.i64;
                        }
                        break;
                    case RiVmParam_Slot:
                        if (get_local(inst->param0).u64) {
                            i = inst->param1.imm.i64;
                        } else {
                            i = inst->param2.imm.i64;
                        }
                        break;
                    default: RI_UNREACHABLE; break;
                }
            } break;

            case RiVmOp_Binary_Add: binary_op(+); break;
            case RiVmOp_Binary_Sub: binary_op(-); break;
            case RiVmOp_Binary_Comparison_Lt: binary_op(<); break;
            case RiVmOp_Binary_Comparison_Gt: binary_op(<); break;
            case RiVmOp_Binary_Comparison_LtEq: binary_op(<=); break;
            case RiVmOp_Binary_Comparison_GtEq: binary_op(>=); break;
            case RiVmOp_Binary_Comparison_Eq: binary_op(==); break;
            case RiVmOp_Binary_Comparison_NotEq: binary_op(!=); break;

            default:
                RI_UNREACHABLE;
                break;
        }
    }

end:;
    return result;
}

#undef get_local

RiVmValue
rivm_exec(RiVmExec* context, RiVmFunc* func, RiVmValue* args, int args_count)
{
    RI_ASSERT(args_count == func->debug_inputs_count);

    RiVmValue* stack = rivm_stack_push(&context->stack, args_count);
    memcpy(stack, args, args_count * sizeof(RiVmValue));
    RiVmValue r = rivm_exec_(context, stack, func);
    rivm_stack_pop(&context->stack, args_count);
    return r;
}