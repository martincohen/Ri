#include "rivm-compiler.h"

void
testrivm_compiler_compile_file_(const char* name, void* host)
{
    COLOG("'%s': testing", name);

    Ri ri;
    ri_init(&ri, host);

    RiNode* ast_module = NULL;
    {
        CoCharArray path_source = {0};
        cochararray_push_f(&path_source, "./src/test/vmc/%s.ri", name);
        coarray_zero_term(&path_source);
            CoByteArray source = {0};
            ASSERT(cofile_read_c(&source, path_source.items, 0));
            ast_module = ri_build(&ri, S((char*)source.items, source.count), path_source.slice);
            ASSERT(ast_module);
            coarray_purge(&source);
        coarray_purge(&path_source);
    }

    CoByteArray expected = {0};
    {
        CoCharArray path_expected = {0};
        cochararray_push_f(&path_expected, "./src/test/vmc/%s.expected.lisp", name);
        coarray_zero_term(&path_expected);
        if (!cofile_read_c(&expected, path_expected.items, 0)) {
            COLOG("'%s': file '%s.expected.lisp' not found", name, name);
        }
        coarray_purge(&path_expected);
    }


    RiVmModule module;
    rivm_module_init(&module);

    RiVmCompiler compiler;
    rivm_init(&compiler, &ri);
    ASSERT(rivm_compile(&compiler, ast_module, &module));

    CoCharArray actual = {0};
    rivm_dump_module(&module, &actual);

    if (expected.items != NULL) {
        if (!string_is_equal(S(expected.items, expected.count), actual.slice)) {
            COLOG("'%s': expected does not match actual", name);
            COLOG("expected:\n%S", expected.slice);
            COLOG("actual:\n%S", actual.slice);
            COLOG("---");
        }
    } else {
        COLOG("actual:\n%S", actual.slice);
    }

    CoCharArray path_recent = {0};
    cochararray_push_f(&path_recent, "./src/test/vmc/%s.recent.lisp", name);
    coarray_zero_term(&path_recent);
    ASSERT(file_write(path_recent.items, actual.items, actual.count, 0));
    coarray_purge(&path_recent);

    coarray_purge(&actual);
    coarray_purge(&expected);

    rivm_purge(&compiler);
    rivm_module_purge(&module);
    ri_purge(&ri);
}

void
testrivm_compiler_compile()
{
    // testrivm_compiler_compile_file_("func", 0);
    // testrivm_compiler_compile_file_("if", 0);
    // testrivm_compiler_compile_file_("if-else", 0);
    // testrivm_compiler_compile_file_("op-binary", 0);
}

void
testrivm_compiler_main() {
    testrivm_compiler_compile();
}