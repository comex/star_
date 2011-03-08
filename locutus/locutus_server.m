#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <stdio.h>
//#include <servers/bootstrap.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syslog.h>
#include <dispatch/dispatch.h>
#include <notify.h>

__attribute__((constructor))
static void init() {
    int token;
    fprintf(stderr, "I'm alive\n");
    notify_register_dispatch("locutus.go-away", &token, dispatch_get_main_queue(), ^(int token_) {
        fprintf(stderr, "s/k\n");
        // do something ...
        notify_cancel(token_);
    });

    // ...

    thread_terminate(mach_thread_self());
}
