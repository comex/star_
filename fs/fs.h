#define IS_64BIT_PROCESS(x) 0
#include <sys/buf_internal.h>
int x_namei(struct nameidata *ndp)
asm("$bl1__vnode_lookup");
void x_nameidone(struct nameidata *)
asm("$bl2__vnode_lookup");
#define namei x_namei
#define nameidone x_nameidone
#define thread_funnel_set(...) 0

uio_t x_uio_createwithbuffer(int a_iovcount, off_t a_offset, int a_spacetype, int a_iodirection, void *a_buf_p, size_t a_buffer_size) asm("$bl2__uio_create");
#define uio_createwithbuffer x_uio_createwithbuffer

void x_vnode_reclaim_internal(struct vnode * vp, int locked, int reuse, int flags) asm("$bl3__vnode_recycle");
#define vnode_reclaim_internal x_vnode_reclaim_internal

errno_t x_vn_create(vnode_t, vnode_t *, struct nameidata *, struct vnode_attr *, int flags, int mode, int unk, vfs_context_t) asm("$strref_22_76_6e_6f_64_65_5f_63_72_65_61_74_65_3a_20_75_6e_6b_6e_6f_77_6e_20_76_74_79_70_65_20_25_64");
// '"vnode_create: unknown vtype %d'
#define vn_create x_vn_create

#if 0

static void x_lck_mtx_lock(lck_mtx_t *lck) {
    printf("About to lock %p\n", lck);
    lck_mtx_lock(lck);
    printf("Locked %p\n", lck);
}
#define lck_mtx_lock x_lck_mtx_lock
#endif

inline int eopnotsupp() {
    return ENOTSUP;
}
#define vn_default_error eopnotsupp

inline int nullop() {
    return 0;
}

inline void buf_setvnode(buf_t bp, vnode_t vp) {
    bp->b_vp = vp;
}

#define bcopy(a, b, c) memcpy(b, a, c)


extern void IOLog(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
#define printf(args...) ((void) (args))

