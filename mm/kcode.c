#include "kinc.h"

__attribute__((externally_visible))
int mysyscall(void *p, void *uap, int32_t *retval)
{
    IOLog("Hi\n");
    //IOLog("kernel_pmap = %p nx_enabled = %d\n", kernel_pmap, kernel_pmap[0x420/4]);
    // Turn off nx_enabled so we can make pages executable in kernel land.
    //kernel_pmap[0x420/4] = 0;
    return 0;
}
