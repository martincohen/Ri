#pragma once

#include "rivm.h"

void rivm_dump_module(RiVmModule* module, CharArray* out);
void rivm_dump_func(RiVmFunc* func, CharArray* out);