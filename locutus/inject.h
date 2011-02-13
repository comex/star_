#include <unistd.h>
#include <mach/mach.h>

// asynchronous
// inject the specified dylib
kern_return_t inject(pid_t pid, const char *path);
