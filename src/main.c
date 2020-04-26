#include <co.h>
#ifdef CO_MINIMAL
#include <co-minimal.c>
#endif
#include <co-common.c>
#include <co-cstring.c>
#include <co-log.c>
#include <co-assert.c>
#include <co-hash.c>
#include <co-memory.c>
#include <co-array.c>
#include <co-arena.c>
#include <co-string.c>
#include <co-map.c>
#include <co-win32.c>
#include <co-utf-encoder.c>
#include <co-file-io.c>
#include <co-intern.c>

#include "ri.c"
#include "ri-ast-dump.c"

// #include "rivm.c"
// #include "rivm-compiler.c"
// #include "rivm-interpreter.c"
// #include "rivm-dump.c"

#include "test-ri.c"
// #include "test-rivm-compiler.c"
// #include "test-rivm-interpreter.c"
// #include "test-rivm-dcall.c"

int main(int argc, char** argv)
{
    COLOG("* %d bytes", sizeof(RiNode));
    RiNode node;
    COLOG(".symbol %d bytes", sizeof(node.symbol));
    COLOG(".module %d bytes", sizeof(node.module));
    COLOG(".scope %d bytes", sizeof(node.scope));
    COLOG(".decl %d bytes", sizeof(node.decl));
    COLOG(".spec %d bytes", sizeof(node.spec));
    COLOG(".binary %d bytes", sizeof(node.binary));
    COLOG(".unary %d bytes", sizeof(node.unary));
    COLOG(".call %d bytes", sizeof(node.call));
    COLOG(".constant %d bytes", sizeof(node.constant));
    COLOG(".st_return %d bytes", sizeof(node.st_return));
    COLOG(".st_if %d bytes", sizeof(node.st_if));
    COLOG(".st_for %d bytes", sizeof(node.st_for));
    COLOG(".st_switch %d bytes", sizeof(node.st_switch));
    COLOG(".st_switch_case %d bytes", sizeof(node.st_switch_case));
    COLOG(".st_expr %d bytes", sizeof(node.st_expr));

    testri_main();
    // testrivm_compiler_main();
    // testrivm_dcall_main();
    // testrivm_interpreter_main();

    return 0;
}


void core_main()
{
    // AttachConsole(ATTACH_PARENT_PROCESS);
    ExitProcess(main(0, 0));
}