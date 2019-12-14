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
    arena_init(&module->arena, KILOBYTES(64));
}

void
rivm_module_purge(RiVmModule* module)
{
    arena_purge(&module->arena);
    RiVmFunc* it;
    array_each(&module->func, &it) {
        heap_free(&it->code.items);
    }
    array_purge(&module->func);
}

RiVmFunc*
rivm_module_push_func(RiVmModule* module, RiVmInstSlice code)
{
    RiVmFunc* func = rivm_module_push_(module, RiVmFunc);
    func->code = code;
    return func;
}