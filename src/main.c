#include <co-lib.c>

#include "ri.c"
#include "test-ri.c"

int main(int argc, char** argv)
{
    testri_main();

    return 0;
}


void core_main()
{
    ExitProcess(main(0, 0));
}