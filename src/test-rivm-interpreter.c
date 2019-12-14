#include "rivm-compiler.h"
#include "rivm-interpreter.h"

RiVmValue
testrivm_interpreter_exec_file_(const char* name)
{
    CharArray path_source = {0};
    chararray_push_f(&path_source, "./src/test/vmi/%s.ri", name);
    array_zero_term(&path_source);

    RiVmModule module;
    rivm_module_init(&module);
    rivm_compile_file(path_source.slice, &module);
    
    RiVmValue value = rivm_exec(array_at(&module.func, 0), NULL, 0);
    
    rivm_module_purge(&module);
    array_purge(&path_source);

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