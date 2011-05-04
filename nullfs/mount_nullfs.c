#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
struct null_args {
    char        *target;    /* Target of loopback  */
};


int main(int argc, char **argv) {
    struct null_args args;
    args.target = argv[1];
    printf("%d ", mount("loopback", argv[2], MNT_UNION, &args));
    printf("%s\n", strerror(errno));
}
