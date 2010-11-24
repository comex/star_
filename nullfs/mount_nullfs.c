#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
struct null_args {
    char        *target;    /* Target of loopback  */
};


int main() {
    struct null_args args;
    args.target = "/y";
    printf("%d ", mount("loopback", "/x", 0, &args));
    printf("%s\n", strerror(errno));
}
