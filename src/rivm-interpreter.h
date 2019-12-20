#pragma once

#include "rivm.h"

typedef struct RiVmExec RiVmExec;
typedef struct RiVmStack RiVmStack;

struct RiVmStack
{
    RiVmValue* start;
    RiVmValue* it;
    RiVmValue* end;
};

struct RiVmExec
{
    void* host;
    RiVmStack stack;
};

void rivm_exec_init(RiVmExec* context);
void rivm_exec_purge(RiVmExec* context);

RiVmValue rivm_exec(RiVmExec* context, RiVmFunc* func, RiVmValue* args, int args_count);