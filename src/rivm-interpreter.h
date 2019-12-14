#pragma once

#include "rivm.h"

RiVmValue rivm_exec(RiVmFunc* func, RiVmValue* args, int args_count);
