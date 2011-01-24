#include <unistd.h>
#include <mach/mach.h>

// asynchronous
kern_return_t inject(pid_t pid, const char *path, kern_return_t (^waiter)());
