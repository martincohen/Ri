#pragma once

#include "rivm-compiler.h"

void rivm_dump_module(RiVmCompiler* rix, RiVmModule* module, CharArray* out);
void rivm_dump_func(RiVmCompiler* rix, RiVmFunc* func, CharArray* out);