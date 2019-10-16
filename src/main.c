#include <co-lib.c>

#include "ri.c"
#include "rivmc.c"

#include "test-ri.c"
#include "test-rivmc.c"

int main(int argc, char** argv)
{
    testri_main();
    // testrivmc_main();

    return 0;
}


void core_main()
{
    ExitProcess(main(0, 0));
}