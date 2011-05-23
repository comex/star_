// this can't be a shell script because bash depends on readline, but readline is behind the mount
#include <stdlib.h>
#include <spawn.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>

#define UNMNT_ABOVE 0x0001      /* Target appears above mount point */
struct union_args {
    char        *target;    /* Target of loopback  */
    int     mntflags;   /* Options on the mount */
};

#define A(x) args.target = "/private/var/null/" x; if(mount("unionfs", "/" x, 0, &args)) fprintf(stderr, "couldn't mount %s\n", "/" x);

int main() {
    return 0; // XXX

    char *argv[] = {"/boot/white_loader", "-l", "/boot/union_prelink.dylib", NULL};
    int stat;
    pid_t pid;
    if(posix_spawn(&pid, argv[0], NULL, NULL, argv, NULL) || pid != waitpid(pid, &stat, 0) || !WIFEXITED(stat) || WEXITSTATUS(stat)) fprintf(stderr, "couldn't run white_loader\n");
    struct union_args args;
    args.mntflags = UNMNT_ABOVE;
    
    A("Applications")
    A("Library")
    A("System")
    A("bin")
    A("sbin")
    A("usr")
    A("private/etc")
    
    return 0;
}
