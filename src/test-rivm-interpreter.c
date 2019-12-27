#include "rivm-compiler.h"
#include "rivm-interpreter.h"

RiVmValue
testrivm_interpreter_exec_file_(const char* name, void* host)
{
    RiVmModule module;
    rivm_module_init(&module);

    CharArray path_source = {0};
    chararray_push_f(&path_source, "./src/test/vmi/%s.ri", name);
    array_zero_term(&path_source);

    rivm_compile_file(path_source.slice, &module, host);

    RiVmExec context;
    rivm_exec_init(&context);
    // RiVmValue args[] = { 0 };
    double t = perf_get();
    RiVmValue value = rivm_exec(
        &context,
        array_at(&module.func, 0),
        0, // args,
        0 // COUNTOF(args)
    );
    t = perf_get() - t;
    rivm_exec_purge(&context);

    array_purge(&path_source);
    rivm_module_purge(&module);

    LOG("%s: %d (%.3fms)", name, value.i64, t * 1e3);

    return value;
}

void host_print(int32_t i) {
    LOG("%d", i);
}

void
testrivm_interpreter_exec() {
    // ASSERT(testrivm_interpreter_exec_file_("test1").i32 == 2);
    // ASSERT(testrivm_interpreter_exec_file_("fib34", 0).i32 == 5702887);
    testrivm_interpreter_exec_file_("host-call", &host_print);
}

void
testrivm_interpreter_main() {
    testrivm_interpreter_exec();
}