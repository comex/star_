#include <unistd.h>
#include <mach/mach.h>

// asynchronous
kern_return_t inject(pid_t pid, const char *path, mach_port_t *gssd);
