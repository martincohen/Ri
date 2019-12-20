#include "rivm-compiler.h"

void
testrivm_compiler_compile_file_(const char* name)
{
    LOG("'%s': testing", name);

    Ri ri;
    ri_init(&ri);

    RiNode* ast_module = NULL;
    {
        CharArray path_source = {0};
        chararray_push_f(&path_source, "./src/test/vmc/%s.ri", name);
        array_zero_term(&path_source);
            ByteArray source = {0};
            ASSERT(file_read(&source, path_source.items, 0));
            ast_module = ri_build(&ri, S((char*)source.items, source.count), path_source.slice);
            ASSERT(ast_module);
            array_purge(&source);
        array_purge(&path_source);
    }

    ByteArray expected = {0};
    {
        CharArray path_expected = {0};
        chararray_push_f(&path_expected, "./src/test/vmc/%s.expected.lisp", name);
        array_zero_term(&path_expected);
        if (!file_read(&expected, path_expected.items, 0)) {
            LOG("'%s': file '%s.expected.lisp' not found", name, name);
        }
        array_purge(&path_expected);
    }


    RiVmModule module;
    RiVmCompiler compiler;
    rivm_init(&compiler, &ri);
    ASSERT(rivm_compile(&compiler, ast_module, &module));

    CharArray actual = {0};
    rivm_dump_module(&module, &actual);

    if (expected.items != NULL) {
        if (!string_is_equal(S(expected.items, expected.count), actual.slice)) {
            LOG("'%s': expected does not match actual", name);
            LOG("expected:\n%S", expected.slice);
            LOG("actual:\n%S", actual.slice);
            LOG("---");
        }
    } else {
        LOG("actual:\n%S", actual.slice);
    }

    CharArray path_recent = {0};
    chararray_push_f(&path_recent, "./src/test/vmc/%s.recent.lisp", name);
    array_zero_term(&path_recent);
    ASSERT(file_write(path_recent.items, actual.items, actual.count, 0));
    array_purge(&path_recent);

    array_purge(&actual);
    array_purge(&expected);

    rivm_purge(&compiler);
    ri_purge(&ri);
}

void
testrivm_compiler_compile()
{
    testrivm_compiler_compile_file_("func");
    testrivm_compiler_compile_file_("if");
    testrivm_compiler_compile_file_("if-else");
    // TODO: Fix the bug in TODO.md first.
    testrivm_compiler_compile_file_("op-binary");
}

void
testrivm_compiler_main() {
    testrivm_compiler_compile();
}