#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <stdio.h>
//#include <servers/bootstrap.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Locutus.h"
#include <syslog.h>
#include <dispatch/dispatch.h>
#include <notify.h>

extern boolean_t Locutus_server(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

static mach_port_t locutus_port;

kern_return_t test(mach_port_t port) {
    return 0;
}
        
// I was using bootstrap, but it didn't work properly when started more than once (no way to "check out")
// so here's this dumb hack

__attribute__((constructor))
static void init() {
    int token;
    fprintf(stderr, "I'm alive\n");
    notify_register_dispatch("go-away-locutus", &token, dispatch_get_main_queue(), ^(int token_) {
        fprintf(stderr, "s/k\n");
        mach_port_deallocate(mach_task_self(), locutus_port);
        locutus_port = 0;
        notify_cancel(token);
    });

    if(task_get_special_port(mach_task_self(), TASK_GSSD_PORT, &locutus_port)) abort();
    
    fprintf(stderr, "l_p=%d\n", locutus_port);
    while(locutus_port) {
        //fprintf(stderr, "mmso\n");
        fprintf(stderr, "mmso %d\n", mach_msg_server_once(Locutus_server, 8192, locutus_port, 0));
        break;
    }
    thread_terminate(mach_thread_self());
}
