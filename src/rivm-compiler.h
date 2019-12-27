#pragma once

#include "ri.h"
#include "rivm.h"

typedef struct RiVmCompiler RiVmCompiler;

//
//
//

struct RiVmCompiler
{
    Ri* ri;
    RiVmModule* module;
    
    RiVmInstArray code;

    uint32_t slot_next;
    Array(uint32_t) slot_pool;
    Array(RiVmParam) slot;

    Array(uint32_t) labels;
    RiNodeArray pending;

    // RiNode* ast_func;
};

void rivm_init(RiVmCompiler* rix, Ri* ri);
void rivm_purge(RiVmCompiler* rix);
bool rivm_compile(RiVmCompiler* rix, RiNode* ast_module, RiVmModule* module);
bool rivm_compile_file(String path, RiVmModule* module, void* host);