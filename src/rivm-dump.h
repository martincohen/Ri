#pragma once

#include "rivm.h"

void rivm_dump_module(RiVmModule* module, CoCharArray* out);
void rivm_dump_func(RiVmFunc* func, CoCharArray* out);