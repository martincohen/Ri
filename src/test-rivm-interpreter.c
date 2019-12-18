#include "rivm-compiler.h"
#include "rivm-interpreter.h"

RiVmValue
testrivm_interpreter_exec_file_(const char* name)
{
    RiVmModule module;
    rivm_module_init(&module);

    CharArray path_source = {0};
    chararray_push_f(&path_source, "./src/test/vmi/%s.ri", name);
    array_zero_term(&path_source);

    rivm_compile_file(path_source.slice, &module);
    
    RiVmExec context;
    rivm_exec_init(&context);
    RiVmValue args[] = { 0 };
    RiVmValue value = rivm_exec(
        &context,
        array_at(&module.func, 0),
        args,
        COUNTOF(args)
    );
    rivm_exec_purge(&context);
    
    array_purge(&path_source);
    rivm_module_purge(&module);

    return value;
}

void
testrivm_interpreter_exec() {
    ASSERT(testrivm_interpreter_exec_file_("test1").i64 == 1);
}

void
testrivm_interpreter_main() {
    testrivm_interpreter_exec();
}