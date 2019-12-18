#include "rivm-interpreter.h"

// - Caller pushes input arguments.
// - Caller pushes output arguments.
// - Caller calls callee with a stack pointer pointing to first input arguments.
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

#define rivm_exec_get_local(Param) \
    stack[ \
        RI_CHECK((Param).kind == RiVmParam_Slot), \
        (Param).slot.index \
    ]

RiVmValue
rivm_exec_(RiVmExec* context, RiVmValue* stack, RiVmFunc* func)
{
    RiVmInst* inst;
    int64_t i = 0;
    
    RiVmValue r;
    RiVmValue* callee_stack = NULL;

    for (;;)
    {
        inst = &array_at(&func->code, i);
        ++i;

        switch (inst->op)
        {
            case RiVmOp_Enter:
                rivm_stack_push(&context->stack, inst->param0.imm.u64);
                callee_stack = context->stack.it;
                break;

            case RiVmOp_Leave:
                rivm_stack_pop(&context->stack, inst->param0.imm.u64);
                goto end;

            case RiVmOp_Ret:
                break;

            case RiVmOp_Assign:
                switch (inst->param1.kind)
                {
                    case RiVmParam_Imm:
                        rivm_exec_get_local(inst->param0).u64 = inst->param1.imm.u64;
                        break;
                    case RiVmParam_Slot:
                        rivm_exec_get_local(inst->param0).u64 = rivm_exec_get_local(inst->param1).u64;
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
                        rivm_stack_push(&context->stack, 1)->u64 = rivm_exec_get_local(inst->param0).u64;
                        break;
                    default: RI_UNREACHABLE; break;
                }
            } break;

            case RiVmOp_ArgPopN: {
                rivm_stack_pop(&context->stack, inst->param0.imm.u64);
            } break;

            case RiVmOp_Call: {
                RiVmFunc* callee_func = inst->param1.imm.ptr;
                RiVmValue callee_result = rivm_exec_(context, callee_stack, callee_func);
                rivm_exec_get_local(inst->param0).u64 = callee_result.u64;
            } break;

            case RiVmOp_Binary_Add:
                // TODO
                break;

            default:
                RI_UNREACHABLE;
                break;
        }
    }

end:;
    return r;
}

RiVmValue
rivm_exec(RiVmExec* context, RiVmFunc* func, RiVmValue* args, int args_count)
{
    RI_ASSERT(args_count == (func->debug_inputs_count + func->debug_outputs_count));

    RiVmValue* stack = rivm_stack_push(&context->stack, args_count);
    memcpy(stack, args, args_count * sizeof(RiVmValue));
    RiVmValue r = rivm_exec_(context, stack, func);
    rivm_stack_pop(&context->stack, args_count);
    return r;
}