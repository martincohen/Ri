#include <co-lib.c>

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
    LOG("* %d bytes", sizeof(RiNode));
    RiNode node;
    LOG(".symbol %d bytes", sizeof(node.symbol));
    LOG(".module %d bytes", sizeof(node.module));
    LOG(".scope %d bytes", sizeof(node.scope));
    LOG(".decl %d bytes", sizeof(node.decl));
    LOG(".spec %d bytes", sizeof(node.spec));
    LOG(".binary %d bytes", sizeof(node.binary));
    LOG(".unary %d bytes", sizeof(node.unary));
    LOG(".call %d bytes", sizeof(node.call));
    LOG(".constant %d bytes", sizeof(node.constant));
    LOG(".st_return %d bytes", sizeof(node.st_return));
    LOG(".st_if %d bytes", sizeof(node.st_if));
    LOG(".st_for %d bytes", sizeof(node.st_for));
    LOG(".st_switch %d bytes", sizeof(node.st_switch));
    LOG(".st_switch_case %d bytes", sizeof(node.st_switch_case));
    LOG(".st_expr %d bytes", sizeof(node.st_expr));

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