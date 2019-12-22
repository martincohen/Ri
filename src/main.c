#include <co-lib.c>

#include "ri.c"
#include "rivm.c"
#include "rivm-compiler.c"
#include "rivm-interpreter.c"
#include "rivm-dump.c"

#include "test-ri.c"
#include "test-rivm-compiler.c"
#include "test-rivm-interpreter.c"
#include "test-rivm-dcall.c"

int main(int argc, char** argv)
{
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