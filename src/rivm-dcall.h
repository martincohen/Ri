#pragma once

#include "rivm-interpreter.h"

#define RIVM_DCALL_F(Name, Ret) Ret Name(uint64_t stack_size, void* stack, void* registers, void* f)
typedef RIVM_DCALL_F(RiVmDCallF64, double);
typedef RIVM_DCALL_F(RiVmDCallF32, float);
typedef RIVM_DCALL_F(RiVmDCallI64, int64_t);
typedef RIVM_DCALL_F(RiVmDCallU64, uint64_t);

extern RiVmValue rivm_dcall(uint64_t stack_size, void* stack, void* registers, void* f);
