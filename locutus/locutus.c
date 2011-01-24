#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <notify.h>
#include <pthread.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

static void _assert(bool x) {
    if(!x) {
        abort();
    }
}

static volatile bool got_lock = false;

static int notify_token;

static void *lock_wait_thread(void *ignored) {
    sleep(2);
    if(!got_lock) {
        fprintf(stderr, "quitting because someone else is busy\n");
        exit(1);
    }
    return NULL;
}

static pid_t find_springboard() {
    static int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    size_t length;
    _assert(!sysctl(&name[0], sizeof(name) / sizeof(*name), NULL, &length, NULL, 0));
    struct kinfo_proc *proc = malloc(length);
    _assert(!sysctl(&name[0], sizeof(name) / sizeof(*name), proc, &length, NULL, 0));
    for(size_t i = 0; i < length; i++) {
        if(!strncmp(proc[i].kp_proc.p_comm, "SpringBoard", sizeof(proc[i].kp_proc.p_comm))) {
            pid_t result = proc[i].kp_proc.p_pid;
            free(proc);
            return result;
        }
    }
    abort();
}
    

__attribute__((naked)) static uint64_t thread_selfid() {
    asm volatile("mov r12, #0x174; svc #0x80; bx lr");
}

int main() {
    printf("thread_self: %llx\n", thread_selfid());

    uint32_t one = 1;
    _assert(!sysctlbyname("security.mac.vnode_enforce", NULL, NULL, &one, sizeof(one));

    notify_post("go-away-locutus");

    notify_register_signal("go-away-locutus", SIGTERM, &notify_token);

    int fd = open("/tmp/locutus.lock", O_WRONLY | O_CREAT, 0666);
    _assert(fd >= 0);
    pthread_t pt;
    pthread_create(&pt, NULL, lock_wait_thread, NULL);
    _assert(!flock(fd, LOCK_EX));
    got_lock = true;

    pid_t pid = find_springboard();
    printf("pid = %d\n", (int) pid);
    return 0;
}
