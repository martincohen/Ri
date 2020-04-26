#include <inttypes.h>
#include "rivm-dump.h"

#ifndef RIVM_DUMP_PARAM_TYPE_
#define RIVM_DUMP_PARAM_TYPE_ ""
#endif

const char* RIVM_DUMP_OP_NAMES_[] = {
    #define RIVM_GROUP_START(Name) [RiVmOp_ ## Name ## _FIRST__] = NULL,
    #define RIVM_GROUP_END(Name) [RiVmOp_ ## Name ## _LAST__] = NULL,
    #define RIVM_INST(Name, S) [RiVmOp_ ## Name] = S,

        #include "rivm-op.h"

    #undef RIVM_INST
    #undef RIVM_GROUP_END
    #undef RIVM_GROUP_START
};

const char* RIVM_DUMP_TYPE_NAMES_[] = {
    [RiVmValue_None] = "none",
    [RiVmValue_I32] = "i32",
    [RiVmValue_I64] = "i64",
    [RiVmValue_U32] = "u32",
    [RiVmValue_U64] = "u64",
    [RiVmValue_F32] = "f32",
    [RiVmValue_F64] = "f64",
};

const char* RIVM_DUMP_TYPE_NAMES_SHORT_[] = {
    [RiVmValue_None] = "?",
    [RiVmValue_I32] = "i",
    [RiVmValue_I64] = "I",
    [RiVmValue_U32] = "u",
    [RiVmValue_U64] = "U",
    [RiVmValue_F32] = "f",
    [RiVmValue_F64] = "F",
};

const char* RIVM_DUMP_SLOT_KIND_[] = {
    [RiSlot_Unknown] = "?",
    [RiSlot_Input] = "i",
    [RiSlot_Output] = "o",
    [RiSlot_Local] = "l",
    [RiSlot_Global] = "g",
    [RiSlot_Temporary] = "t",
};

static void
rivm_dump_param_(RiVmParam* param, CoCharArray* out)
{
    switch (param->kind)
    {
        case RiVmParam_None:
            break;
        case RiVmParam_Imm:
            switch (param->type) {
                case RiVmValue_I32: cochararray_push_f(out, "%"PRIi32 RIVM_DUMP_PARAM_TYPE_ "", param->imm.i32, RIVM_DUMP_TYPE_NAMES_SHORT_[param->type]); break;
                case RiVmValue_I64: cochararray_push_f(out, "%"PRIi64 RIVM_DUMP_PARAM_TYPE_ "", param->imm.i64, RIVM_DUMP_TYPE_NAMES_SHORT_[param->type]); break;
                case RiVmValue_U32: cochararray_push_f(out, "%"PRIu32 RIVM_DUMP_PARAM_TYPE_ "", param->imm.u32, RIVM_DUMP_TYPE_NAMES_SHORT_[param->type]); break;
                case RiVmValue_U64: cochararray_push_f(out, "%"PRIu64 RIVM_DUMP_PARAM_TYPE_ "", param->imm.u64, RIVM_DUMP_TYPE_NAMES_SHORT_[param->type]); break;
                default: RI_UNREACHABLE; break;
            }
            break;
        case RiVmParam_Label:
            cochararray_push_f(out, "_%d", param->label);
            break;
        case RiVmParam_Slot:
            cochararray_push_f(out, "%s%d" RIVM_DUMP_PARAM_TYPE_, RIVM_DUMP_SLOT_KIND_[param->slot.kind], param->slot.index, RIVM_DUMP_TYPE_NAMES_SHORT_[param->type]);
            break;
        case RiVmParam_Func:
            cochararray_push_f(out, "func%d", param->func);
            break;
        default:
            RI_UNREACHABLE;
            break;
    }
}

#undef RIVM_DUMP_PARAM_TYPE_

// void
// rivm_dump_labels(RiVmCompiler* compiler, CoCharArray* out, intptr_t address)
// {
//     for (intptr_t i = 0; i < compiler->labels.count; ++i) {
//         if (compiler->labels.items[i] == address) {
//             cochararray_push_f(out, "_%d:\n", i + 1);
//         }
//     }
// }

void
rivm_dump_func(RiVmFunc* func, CoCharArray* out)
{
    RiVmInst* it = func->code.items;
    CoCharArray s0 = {0};
    CoCharArray s1 = {0};
    CoCharArray s2 = {0};

    for (intptr_t i = 0; i < func->code.count; ++i, ++it)
    {
        // rivm_dump_labels(compiler, out, i);

        coarray_clear(&s0);
        coarray_clear(&s1);
        coarray_clear(&s2);
        rivm_dump_param_(&it->param0, &s0);
        rivm_dump_param_(&it->param1, &s1);
        rivm_dump_param_(&it->param2, &s2);

        const char* sop = RIVM_DUMP_OP_NAMES_[it->op];

        cochararray_push_f(out, "    %4d (", i);
        if (rivm_op_is_in(it->op, Binary)) {
            cochararray_push_f(out, "%S = %S %s %S", s0, s1, sop, s2);
        } else {
            switch (it->op)
            {
                case RiVmOp_Assign:
                    cochararray_push_f(out, "%S = %S", s0, s1);
                    break;

                case RiVmOp_AddrOf:
                    cochararray_push_f(out, "%S = (%s %S)", s0, sop, s1);
                    break;

               case RiVmOp_Call:
                    if (it->param0.kind != RiVmParam_None) {
                        cochararray_push_f(out, "%S = (%s %S %S)", s0, sop, s1, s2);
                    } else {
                        cochararray_push_f(out, "%s %S %S", sop, s1, s2);
                    }
                    break;

                case RiVmOp_If:
                    cochararray_push_f(out, "%s %S != 0 then (goto %S) else (goto %S)", sop, s0, s1, s2);
                    break;

                default:
                    cochararray_push_f(out, "%s", sop);
                    if (it->param0.kind) {
                        cochararray_push_f(out, " %S", s0);
                    }
                    if (it->param1.kind) {
                        cochararray_push_f(out, " %S", s1);
                    }
                    if (it->param2.kind) {
                        cochararray_push_f(out, " %S", s2);
                    }
                    break;
            }
        }
        cochararray_push_f(out, ")\n");
    }

    coarray_purge(&s0);
    coarray_purge(&s1);
    coarray_purge(&s2);
}

void
rivm_dump_module(RiVmModule* module, CoCharArray* out)
{
    RiVmFunc* it;
    slice_eachi(&module->func, i, &it) {
        cochararray_push_f(out, "func%d:\n", it);
        rivm_dump_func(it, out);
    }
}