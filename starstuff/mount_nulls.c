// this can't be a shell script because bash depends on readline, but readline is behind the mount (not insurmountable but this is easier)
#include <stdlib.h>
#include <spawn.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define UNMNT_ABOVE 0x0001      /* Target appears above mount point */
struct union_args {
    char        *target;    /* Target of loopback  */
    int     mntflags;   /* Options on the mount */
};

dev_t root_dev;

static void do_mount(char *to, char *from) {
    struct stat st;
    if(lstat(to, &st)) {
        fprintf(stderr, "couldn't lstat %s\n", to);
        return;
    }
    if((st.st_mode & S_IFMT) == S_IFLNK) {
        fprintf(stderr, "not mounting %s because it is a symlink\n", from);
        return;
    }
    if(st.st_dev != root_dev) {
        fprintf(stderr, "not mounting %s because it is not on the same mount as root (already mounted I guess)\n", from);
        return;
    }
    struct union_args args;
    args.mntflags = UNMNT_ABOVE;
    args.target = from;
    fprintf(stderr, "mounting %s\n", from);
    if(mount("unionfs", to, 0, &args)) {     
        fprintf(stderr, "couldn't mount %s: %s\n", from, strerror(errno));
    }

}

int main() {
    if(!USE_NULL) return 0;
    char *argv[] = {"/boot/white_loader", "-l", "/boot/union_prelink.dylib", NULL};
    int stat;
    pid_t pid;
    if(posix_spawn(&pid, argv[0], NULL, NULL, argv, NULL) || pid != waitpid(pid, &stat, 0) || !WIFEXITED(stat) || WEXITSTATUS(stat)) fprintf(stderr, "couldn't run white_loader\n");
    struct stat st;
    if(lstat("/", &st)) {
        fprintf(stderr, "couldn't lstat /\n");
        return 1;
    }
    root_dev = st.st_dev;
    
#define A(x) do_mount("/" x, "/private/var/null/" x)
    A("Applications");
    A("Library");
    A("System");
    //A("bin");
    //A("sbin");
    A("usr");
    A("private/etc");
    
    return 0;
}
