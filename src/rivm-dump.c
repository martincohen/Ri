#include <inttypes.h>
#include "rivm-dump.h"

#ifndef RIVM_DUMP_PARAM_TYPE_
#define RIVM_DUMP_PARAM_TYPE_ ""
#endif

const char* RIVM_DEBUG_OP_NAMES_[] = {
    #define RIVM_GROUP_START(Name) [RiVmOp_ ## Name ## _FIRST__] = NULL,
    #define RIVM_GROUP_END(Name) [RiVmOp_ ## Name ## _LAST__] = NULL,
    #define RIVM_INST(Name, S) [RiVmOp_ ## Name] = S,

        #include "rivm-op.h"

    #undef RIVM_INST
    #undef RIVM_GROUP_END
    #undef RIVM_GROUP_START
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
rivm_dump_labels(RiVmCompiler* compiler, CharArray* out, iptr address)
{
    for (iptr i = 0; i < compiler->labels.count; ++i) {
        if (compiler->labels.items[i] == address) {
            chararray_push_f(out, "_%d:\n", i + 1);
        }
    }
}

void
rivm_dump_func(RiVmCompiler* compiler, RiVmFunc* func, CharArray* out)
{
    RiVmInst* it = func->code.items;
    CharArray s0 = {0};
    CharArray s1 = {0};
    CharArray s2 = {0};

    for (iptr i = 0; i < func->code.count; ++i, ++it)
    {
        rivm_dump_labels(compiler, out, i);

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

                case RiVmOp_AddrOf:
                    chararray_push_f(out, "%S = (%s %S)", s0, sop, s1);
                    break;

               case RiVmOp_Call:
                    chararray_push_f(out, "%S = (%s %S)", s0, sop, s1);
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
rivm_dump_module(RiVmCompiler* compiler, RiVmModule* module, CharArray* out)
{
    RiVmFunc* it;
    slice_eachi(&module->func, i, &it) {
        chararray_push_f(out, "func%d:\n", i);
        rivm_dump_func(compiler, it, out);
    }
}