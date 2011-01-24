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
    _assert(sysctl(&name[0], sizeof(name) / sizeof(*name), NULL, &length, NULL, 0) != -1);
    struct kinfo_proc *proc = malloc(length);
    _assert(sysctl(&name[0], sizeof(name) / sizeof(*name), proc, &length, NULL, 0) != -1);
    for(size_t i = 0; i < length; i++) {
        if(!strncmp(proc[i].kp_proc.p_comm, "SpringBoard", sizeof(proc[i].kp_proc.p_comm))) {
            pid_t result = proc[i].kp_proc.p_pid;
            free(proc);
            return result;
        }
    }
    abort();
}

int main() {
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
