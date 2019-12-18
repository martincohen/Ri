#pragma once

#include <co-lib.h>
#include "ri.h"

typedef union RiVmValue RiVmValue;
typedef enum RiVmValueType RiVmValueType;
typedef enum RiVmOp RiVmOp;
typedef struct RiVmInst RiVmInst;
typedef enum RiVmParamKind RiVmParamKind;
typedef enum RiVmParamSlotKind RiVmParamSlotKind;
typedef struct RiVmParam RiVmParam;
typedef struct RiVmFunc RiVmFunc;
typedef struct RiVmModule RiVmModule;

enum RiVmValueType
{
    RiVmValue_None,
    RiVmValue_I32,
    RiVmValue_I64,
    RiVmValue_U32,
    RiVmValue_U64,
    RiVmValue_F32,
    RiVmValue_F64,
    RiVmValue_COUNT__
};

union RiVmValue
{
    int64_t i64;
    uint64_t u64;
    // An alternative to u64.
    void* ptr;

    int32_t i32;
    uint32_t u32;

    float f32;
    double f64;
};

typedef Slice(RiVmValue) RiVmValueSlice;

enum RiVmOp
{
    #define RIVM_GROUP_START(Name) RiVmOp_ ## Name ## _FIRST__,
    #define RIVM_GROUP_END(Name) RiVmOp_ ## Name ## _LAST__,
    #define RIVM_INST(Name, S) RiVmOp_ ## Name,

        #include "rivm-op.h"

    #undef RIVM_INST
    #undef RIVM_GROUP_END
    #undef RIVM_GROUP_START
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

#define rivm_make_param(Kind, ...) \
    (RiVmParam){ .kind = RiVmParam_ ## Kind, __VA_ARGS__ }

//
//
//

struct RiVmInst
{
    RiVmOp op;
    RiVmParam param0;
    RiVmParam param1;
    RiVmParam param2;
};

typedef Slice(RiVmInst) RiVmInstSlice;
typedef ArrayWithSlice(RiVmInstSlice) RiVmInstArray;

//
//
//

struct RiVmFunc
{
    RiVmInstSlice code;
    int debug_inputs_count;
    int debug_outputs_count;
};

typedef Slice(RiVmFunc*) RiVmFuncSlice;
typedef ArrayWithSlice(RiVmFuncSlice) RiVmFuncArray;

//
//
//

struct RiVmModule
{
    Arena arena;
    RiVmFuncArray func;
};

void rivm_module_init(RiVmModule* module);
void rivm_module_purge(RiVmModule* module);

uint32_t rivm_module_emit(RiVmModule* module, const RiVmInst inst);

RiVmFunc* rivm_module_push_func(RiVmModule* module, RiVmInstSlice code);