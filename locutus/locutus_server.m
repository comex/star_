#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <stdio.h>

static kern_return_t go_away() {
    fprintf(stderr, "Yeah.  I'm away.  Totally.\n");
}

__attribute__((constructor))
static void init() {
    thread_terminate(mach_thread_self());
}
