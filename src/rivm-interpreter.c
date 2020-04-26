#include "rivm-interpreter.h"
#include "rivm-dcall.h"


// Implementation of calling convetion inside of VM:
// - Caller pushes input arguments.
// - Caller calls callee with a stack pointer pointing to first input argument.
// - Callee reserves a value on C side for return value that is set when `ret <expr>` is called.
// - Callee pushes space needed for it's local and temporary variables. (`enter N`)
// - Callee uses slot indices in instruction params to operate over inputs, outputs, locals and temporaries.
// - Callee pops space needed for it's local and temporary variables. (`leave N`)
// - Caller pops space needed for input and output arguments.

//
//
//

void
rivm_exec_init(RiVmExec* context)
{
    memset(context, 0, sizeof(RiVmExec));
    intptr_t capacity = COMEGABYTES(1);
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
rivm_stack_push(RiVmStack* stack, intptr_t count)
{
    RI_ASSERT((stack->it + count) <= stack->end);
    RiVmValue* r = stack->it;
    stack->it += count;
    return r;
}

static inline RiVmValue*
rivm_stack_pop(RiVmStack* stack, intptr_t count)
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

#define binary_op_tt_(Member, TT, Op) \
    switch (TT) { \
        case RiVmParam_SlotSlot: get_local(inst->param0). ## Member = get_local(inst->param1). ## Member Op get_local(inst->param2). ## Member; break; \
        case RiVmParam_SlotImm:  get_local(inst->param0). ## Member = get_local(inst->param1). ## Member Op inst->param2.imm. ## Member; break; \
        case RiVmParam_ImmSlot:  get_local(inst->param0). ## Member = inst->param1.imm. ## Member        Op get_local(inst->param2). ## Member; break; \
        case RiVmParam_ImmImm:   get_local(inst->param0). ## Member = inst->param1.imm. ## Member        Op inst->param2.imm. ## Member; break; \
        default: RI_UNREACHABLE; break; \
    }

#define binary_op_i_(TT, Op) \
    case RiVmValue_I32: binary_op_tt_(i32, TT, Op); break; \
    case RiVmValue_I64: binary_op_tt_(i64, TT, Op); break; \
    case RiVmValue_U32: binary_op_tt_(u32, TT, Op); break; \
    case RiVmValue_U64: binary_op_tt_(u64, TT, Op); break;

#define binary_op_f_(TT, Op) \
    case RiVmValue_F32: binary_op_tt_(f32, TT, Op); break; \
    case RiVmValue_F64: binary_op_tt_(f64, TT, Op); break;

#define binary_op(Op) { \
        int tt = RIVMPARAMKIND_PAIR(inst->param1.kind, inst->param2.kind); \
        switch (inst->param0.type) { \
            binary_op_i_(tt, Op) \
            binary_op_f_(tt, Op) \
            default: RI_UNREACHABLE; break; \
        } \
    }

#define binary_op_i(Op) { \
        int tt = RIVMPARAMKIND_PAIR(inst->param1.kind, inst->param2.kind); \
        switch (inst->param0.type) { \
            binary_op_i_(tt, Op) \
            default: RI_UNREACHABLE; break; \
        } \
    }

RiVmValue
rivm_exec_(RiVmExec* context, RiVmValue* stack, RiVmFunc* func)
{
    RiVmInst* inst;
    int64_t i = 0;

    RiVmValue result;
    uint64_t locals_count = 0;

    for (;;)
    {
        inst = &coarray_at(&func->code, i);
        ++i;

        switch (inst->op)
        {
            case RiVmOp_Enter:
                locals_count = inst->param0.imm.u64;
                rivm_stack_push(&context->stack, locals_count);
                break;

            case RiVmOp_Ret:
                switch (inst->param0.kind)
                {
                    case RiVmParam_None:
                        break;
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

            case RiVmOp_Cast:
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

            case RiVmOp_CallArg: {
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
                if (inst->param1.kind == RiVmParam_Func) {
                    RiVmFunc* callee_func = inst->param1.func;
                    RiVmValue callee_result = rivm_exec_(context, context->stack.it - inst->param2.imm.u64, callee_func);
                    get_local(inst->param0).u64 = callee_result.u64;
                    rivm_stack_pop(&context->stack, inst->param2.imm.u64);
                } else {
                    void* callee_func = 0;
                    switch (inst->param1.kind) {
                        case RiVmParam_Imm:
                            callee_func = inst->param1.func;
                            break;
                        case RiVmParam_Slot:
                            callee_func = get_local(inst->param1).ptr;
                            break;
                        default:
                            RI_UNREACHABLE;
                            break;
                    };
                    // NOTE: Return type is given by type of the result slot, so compiler has to make sure that the type is correct and do casting/masking needed afterwards.
                    uint64_t args_count = inst->param2.imm.u64;
                    // NOTE: Do this before we align the stack for x64 calling convention.
                    RiVmValue* args = context->stack.it - args_count;
#if defined(SYSTEM_WINDOWS)
                    // NOTE: On windows dcall is written so it's reading 4 arguments past the args pointer, so we fill in space on stack.
                    rivm_stack_push(&context->stack, MAXIMUM(0, 4 - args_count));
#else
    #error Unsupported platform.
#endif
                    uint64_t args_stack_count = MAXIMUM(0, (int64_t)args_count - 4);
                    RiVmValue* args_stack = args + args_count;
                    RiVmValue* args_registers = args;
                    if (inst->param0.kind == RiVmParam_None) {
                        ((RiVmDCallI64*)rivm_dcall)(args_stack_count, args_stack, args_registers, callee_func);
                    }
                    else switch (inst->param0.type)
                    {
                        // Return is always delivered in `RAX`.
                        case RiVmValue_I64: case RiVmValue_I32: get_local(inst->param0).i64 = ((RiVmDCallI64*)rivm_dcall)(args_stack_count, args_stack, args_registers, callee_func); break;
                        case RiVmValue_U64: case RiVmValue_U32: get_local(inst->param0).u64 = ((RiVmDCallU64*)rivm_dcall)(args_stack_count, args_stack, args_registers, callee_func); break;
                        case RiVmValue_F64: get_local(inst->param0).f64 = ((RiVmDCallF64*)rivm_dcall)(args_stack_count, args_stack, args_registers, callee_func); break;
                        case RiVmValue_F32: get_local(inst->param0).f32 = ((RiVmDCallF32*)rivm_dcall)(args_stack_count, args_stack, args_registers, callee_func); break;
                        default: RI_UNREACHABLE; break;
                    }
                    rivm_stack_pop(&context->stack, args_count + MAXIMUM(0, 4 - args_count));
                }
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
            case RiVmOp_Binary_Mul: binary_op(*); break;
            case RiVmOp_Binary_Div: binary_op(/); break;
            case RiVmOp_Binary_Mod: binary_op_i(%); break;
            case RiVmOp_Binary_BXor: binary_op_i(^); break;
            case RiVmOp_Binary_BAnd: binary_op_i(&); break;
            case RiVmOp_Binary_BOr: binary_op_i(|); break;
            case RiVmOp_Binary_BShL: binary_op_i(<<); break;
            case RiVmOp_Binary_BShR: binary_op_i(>>); break;
            case RiVmOp_Binary_And: binary_op(&&); break;
            case RiVmOp_Binary_Or: binary_op(||); break;
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