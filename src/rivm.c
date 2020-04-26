#include "ri.h"
#include "rivm.h"

//
//
//

static void*
rivm_module_push__(RiVmModule* module, int64_t size)
{
    void* ptr = arena_push(&module->arena, size, 8);
    memset(ptr, 0, size);
    return ptr;
}

#define rivm_module_push_(RiVmModule, Type) \
    rivm_module_push__(RiVmModule, sizeof(Type))

//
//
//

void
rivm_module_init(RiVmModule* module)
{
    memset(module, 0, sizeof(RiVmModule));
    coarena_init(&module->arena, KILOBYTES(64));
}

void
rivm_module_purge(RiVmModule* module)
{
    RiVmFunc* it;
    coarray_each(&module->func, &it) {
        heap_free(it->code.items);
    }
    coarray_purge(&module->func);
    coarena_purge(&module->arena);
}

RiVmFunc*
rivm_module_push_func(RiVmModule* module, RiVmInstSlice code)
{
    RiVmFunc* func = rivm_module_push_(module, RiVmFunc);
    func->code = code;
    coarray_push(&module->func, func);
    return func;
}