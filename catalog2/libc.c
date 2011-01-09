#include <sys/sysctl.h>
#include <string.h>
#include <mach/mach.h>
extern mach_port_t mach_reply_port();

NDR_record_t NDR_record = {0, 0, 0, 0, 1, 0, 0, 0};

mach_port_t mig_get_reply_port() {
    return mach_reply_port();
}

void mig_put_reply_port(mach_port_t reply_port) {
}

void mig_dealloc_reply_port(mach_port_t migport) {
}

extern mach_msg_return_t mach_msg_trap(mach_msg_header_t *msg, mach_msg_option_t option, mach_msg_size_t send_size, mach_msg_size_t rcv_size, mach_port_t rcv_name, mach_msg_timeout_t timeout, mach_port_t notify);


#define LIBMACH_OPTIONS (MACH_SEND_INTERRUPT|MACH_RCV_INTERRUPT)
mach_msg_return_t
mach_msg(msg, option, send_size, rcv_size, rcv_name, timeout, notify)
        mach_msg_header_t *msg;
        mach_msg_option_t option;
        mach_msg_size_t send_size;
        mach_msg_size_t rcv_size;
        mach_port_t rcv_name;
        mach_msg_timeout_t timeout;
        mach_port_t notify;
{
        mach_msg_return_t mr;

        /*
         * Consider the following cases:
         *      1) Errors in pseudo-receive (eg, MACH_SEND_INTERRUPTED
         *      plus special bits).
         *      2) Use of MACH_SEND_INTERRUPT/MACH_RCV_INTERRUPT options.
         *      3) RPC calls with interruptions in one/both halves.
         *
         * We refrain from passing the option bits that we implement
         * to the kernel.  This prevents their presence from inhibiting
         * the kernel's fast paths (when it checks the option value).
         */

        mr = mach_msg_trap(msg, option &~ LIBMACH_OPTIONS,
                           send_size, rcv_size, rcv_name,
                           timeout, notify);
        if (mr == MACH_MSG_SUCCESS)
                return MACH_MSG_SUCCESS;

        if ((option & MACH_SEND_INTERRUPT) == 0)
                while (mr == MACH_SEND_INTERRUPTED)
                        mr = mach_msg_trap(msg,
                                option &~ LIBMACH_OPTIONS,
                                send_size, rcv_size, rcv_name,
                                timeout, notify);

        if ((option & MACH_RCV_INTERRUPT) == 0)
                while (mr == MACH_RCV_INTERRUPTED)
                        mr = mach_msg_trap(msg,
                                option &~ (LIBMACH_OPTIONS|MACH_SEND_MSG),
                                0, rcv_size, rcv_name,
                                timeout, notify);

        return mr;
}

size_t strlen(const char *s) {
    size_t result = 0;
    while(*s++) result++;
    return result;
}

int
sysctlbyname(const char *name, void *oldp, size_t *oldlenp, void *newp,
         size_t newlen)
{
    int name2oid_oid[2];
    int real_oid[CTL_MAXNAME+2];
    int error;
    size_t oidlen;

    name2oid_oid[0] = 0;    /* This is magic & undocumented! */
    name2oid_oid[1] = 3;

    oidlen = sizeof(real_oid);
    error = sysctl(name2oid_oid, 2, real_oid, &oidlen, (void *)name,
               strlen(name));
    if (error < 0)
        return error;
    oidlen /= sizeof (int);
    error = sysctl(real_oid, oidlen, oldp, oldlenp, newp, newlen);
    return (error);
}

extern int __sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp, size_t newlen);
int sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp, size_t newlen) {
    return __sysctl(name, namelen, oldp, oldlenp, newp, newlen);
}

#undef memcpy
__attribute__((externally_visible))
void *memcpy(void *restrict s1, const void *restrict s2, size_t n) {
    char *d = s1;
    const char *s = s2;
    while(n--) *d++ = *s++;
    return s1;
}

