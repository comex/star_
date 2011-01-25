#include <unistd.h>
#include <mach/mach.h>

// asynchronous
// inject the specified dylib
// also create a new port which we have a send right to and it has the receive right to, and store it in its TASK_GSSD_PORT slot
kern_return_t inject(pid_t pid, const char *path, mach_port_t *gssd);
