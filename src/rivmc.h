#pragma once

#include "ri.h"

typedef struct RiVmCompiler RiVmCompiler;
typedef struct RiVmValue RiVmValue;
typedef enum RiVmValueType RiVmValueType;
typedef enum RiVmOp RiVmOp;
typedef struct RiVmInst RiVmInst;
typedef enum RiVmParamKind RiVmParamKind;
typedef enum RiVmParamSlotKind RiVmParamSlotKind;
typedef struct RiVmParam RiVmParam;
typedef struct RiVmFunc RiVmFunc;
typedef struct RiVmModule RiVmModule;

enum RiVmValueType {
    RiVmValue_None,
    RiVmValue_I32,
    RiVmValue_I64,
    RiVmValue_U32,
    RiVmValue_U64,
    RiVmValue_F32,
    RiVmValue_F64,
    RiVmValue_COUNT__
};

union RiVmValue {
    int64_t i64;
    uint64_t u64;
    int32_t i32;
    uint32_t u32;
    float f32;
    double f64;
};

enum RiVmOp
{
    #define GROUP_START(Name) RiVmOp_ ## Name ## _FIRST__,
    #define GROUP_END(Name) RiVmOp_ ## Name ## _LAST__,
    #define INST(Name, S) RiVmOp_ ## Name,
        #include "rivmc-op.h"
    #undef INST
    #undef GROUP_END
    #undef GROUP_START
};

#define rivm_op_is_in(Op, Group) \
    (((Op) > RiVmOp_ ## Group ## _FIRST__) && ((Op) < RiVmOp_ ## Group ## _LAST__))

enum RiVmParamKind
{
    RiVmParam_None = 0,
    RiVmParam_Slot,
    RiVmParam_Imm,
    RiVmParam_Label,
    RiVmParam_Func,
};

enum RiVmParamSlotKind
{
    RiSlot_Unknown = 0,
    RiSlot_Input,
    RiSlot_Output,
    RiSlot_Local,
    RiSlot_Global,
    RiSlot_Temporary,
};

struct RiVmParam
{
    RiVmParamKind kind;
    RiVmValueType type;
    union {
        struct {
            // TODO: Only for debug.
            RiVmParamSlotKind kind;
            uint32_t index;
        } slot;
        RiVmValue imm;
        uint32_t label;
        uint32_t func;
    };
};

struct RiVmInst
{
    RiVmOp op;
    RiVmParam param0;
    RiVmParam param1;
    RiVmParam param2;
};

typedef Slice(RiVmInst) RiVmInstSlice;
typedef ArrayWithSlice(RiVmInstSlice) RiVmInstArray;

struct RiVmFunc
{
    RiVmInstSlice code;
};

typedef Slice(RiVmFunc*) RiVmFuncSlice;
typedef ArrayWithSlice(RiVmFuncSlice) RiVmFuncArray;

//
//
//

struct RiVmModule
{
    RiVmFuncSlice func;
};

//
//
//

struct RiVmCompiler
{
    Ri* ri;
    Arena arena;
    RiVmFuncArray func;

    RiVmInstArray code;
    uint32_t slot_next;
    Array(uint32_t) slot_pool;
    Array(uint32_t) labels;

    RiNode* ast_func;
};

void rivm_init(RiVmCompiler* rix, Ri* ri);
void rivm_purge(RiVmCompiler* rix);
RiVmModule* rivm_compile(RiVmCompiler* rix, RiNode* ast_module);
void rivm_dump_module(RiVmCompiler* rix, RiVmModule* module, CharArray* out);
void rivm_dump_func(RiVmCompiler* rix, RiVmFunc* func, CharArray* out);