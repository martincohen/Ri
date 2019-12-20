#include "rivm-dcall.h"

int32_t testrivm_dcall_f0_(int32_t a) {
    return a + 1;
}

void testrivm_dcall_f0() {
    // NOTE: On Windows, we must ensure that we pass space for 4 arguments, even we're using less.
    RiVmValue args[4] = {
        7
    };
    uint64_t stack_size = MAXIMUM(0, (int)COUNTOF(args) - 4) * sizeof(RiVmValue);
    int64_t r = ((RiVmDCallI64*)rivm_dcall)(stack_size, args + 4, args + 0, &testrivm_dcall_f0_);
    ASSERT(r == 8);
}

void testrivm_dcall_main() {
    testrivm_dcall_f0();
}