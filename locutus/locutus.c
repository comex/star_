#include <stdio.h>
#include <unistd.h>

int main() {
    printf("borg and all that euid=%d uid=%d\n", geteuid(), getuid());
}
