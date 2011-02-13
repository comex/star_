#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <notify.h>
#include <pthread.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "inject.h"
#include <servers/bootstrap.h>
#include "bzlib.h"
#include "Locutus.h"
#include <common/common.h>
#include <dispatch.h>

extern char packed_server_bz2[];
extern unsigned int packed_server_bz2_len;

static volatile bool got_lock = false;

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

static void init_state() {
    int fd = open("/tmp/locutus.state", O_RDWR | O_CREAT, 0666);
    _assert(fd >= 0);
    _assert_zero(flock(fd, LOCK_EX));
    if(0 == lseek(fd, 0, SEEK_END)) {
        // we created it
    } else {
        lseek(fd, 0, SEEK_SET);
        FILE *fp = fdopen(fd, "r");
        _assert(fp);
        char mode[16]; int pid; double progress;
        _assert(2 == fscanf(fp, "%.16s %d %f\n", &mode[0], &pid, &progress));
        if(!strcmp(mode, "installing")) {
            // we don't want to stop it now
            exit(0);
        } else {
            _assert(!strcmp(mode, "downloading"), mode);
            kill((pid_t) pid, SIGTERM);
        }
    }
    flock(fd, LOCK_UN);
    close(fd);
}

static void set_state(const char *mode, double progress) {
    int fd = open("/tmp/locutus.state", O_RDWR | O_CREAT, 0666);
    _assert(fd >= 0);
    _assert_zero(flock(fd, LOCK_EX));
    _assert_zero(ftruncate(fd, 0));
    FILE *fp = fdopen(fd, "w");
    _assert(fp);

    fprintf(fp, "%s %d %f\n", mode, (int) getpid(), progress);

    flock(fd, LOCK_UN);
    close(fd);
}

int main() {
    uint32_t one = 1;
    _assert_zero(sysctlbyname("security.mac.vnode_enforce", NULL, NULL, &one, sizeof(one)));

    init_state();




    got_lock = true;
    _assert_zero(notify_register_check("locutus.pid", &pid_token);
    _assert_zero(notify_set_state(pid_token, (uint64_t) getpid()));
    _assert_zero(notify_post("locutus.go-away"));
    

    find_oneness();

    // dump our load
    const char *name = tempnam("/tmp/", "locutus.dylib-");
    int dylib_fd = open(name, O_WRONLY | O_CREAT, 0644);
    _assert(dylib_fd >= 0);
    unsigned int stuff_len = 0x10000;
    char *stuff = malloc(stuff_len);
    _assert(!BZ2_bzBuffToBuffDecompress(stuff, &stuff_len, packed_server_bz2, packed_server_bz2_len, 0, 0));

    for(unsigned int i = 0; i < stuff_len - 32; i++) {
        if(!memcmp(stuff + i, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 32)) {
            memcpy(stuff + i, name, 32);
        }
    }

    _assert(write(dylib_fd, stuff, stuff_len) == (ssize_t) stuff_len);
    close(dylib_fd);
    free(stuff);

    pid_t pid = find_springboard();
    printf("pid = %d\n", (int) pid);

    inject(pid, name);

    unlink(name);

    printf("OK\n");

    printf("test=%x\n", test(port));

    return 0;
}
