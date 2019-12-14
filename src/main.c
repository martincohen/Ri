#include <co-lib.c>

#include "ri.c"
#include "rivm.c"
#include "rivm-compiler.c"
#include "rivm-interpreter.c"
#include "rivm-dump.c"

#include "test-ri.c"
#include "test-rivm-compiler.c"
#include "test-rivm-interpreter.c"

int main(int argc, char** argv)
{
    // testri_main();
    // testrivm_compiler_main();
    testrivm_interpreter_main();

    return 0;
}


void core_main()
{
    ExitProcess(main(0, 0));
}