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

static int x_eopnotsupp() {
    return ENOTSUP;
}
#define vn_default_error x_eopnotsupp
#define eopnotsupp x_eopnotsupp

static int x_nullop() {
    return 0;
}
#define nullop x_nullop

static void x_buf_setvnode(buf_t bp, vnode_t vp) {
    bp->b_vp = vp;
}
#define buf_setvnode x_buf_setvnode

#define bcopy(a, b, c) memcpy(b, a, c)
#define memcpy memmove

// support from white_loader
extern struct vnodeop_desc *vfs_op_descs[];

#if VERSION >= 0x040300
#define vnop_default_desc (*vfs_op_descs[0])
#define vnop_strategy_desc (*vfs_op_descs[1])
#define vnop_bwrite_desc (*vfs_op_descs[2])
#define vnop_lookup_desc (*vfs_op_descs[3])
#define vnop_create_desc (*vfs_op_descs[4])
#define vnop_mknod_desc (*vfs_op_descs[5])
#define vnop_whiteout_desc (*vfs_op_descs[6])
#define vnop_open_desc (*vfs_op_descs[7])
#define vnop_compound_open_desc (*vfs_op_descs[8])
#define vnop_close_desc (*vfs_op_descs[9])
#define vnop_access_desc (*vfs_op_descs[10])
#define vnop_getattr_desc (*vfs_op_descs[11])
#define vnop_setattr_desc (*vfs_op_descs[12])
#define vnop_read_desc (*vfs_op_descs[13])
#define vnop_write_desc (*vfs_op_descs[14])
#define vnop_ioctl_desc (*vfs_op_descs[15])
#define vnop_select_desc (*vfs_op_descs[16])
#define vnop_exchange_desc (*vfs_op_descs[17])
#define vnop_kqfilt_add_desc (*vfs_op_descs[18])
#define vnop_kqfilt_remove_desc (*vfs_op_descs[19])
#define vnop_revoke_desc (*vfs_op_descs[21])
#define vnop_mmap_desc (*vfs_op_descs[22])
#define vnop_mnomap_desc (*vfs_op_descs[23])
#define vnop_fsync_desc (*vfs_op_descs[24])
#define vnop_remove_desc (*vfs_op_descs[25])
#define vnop_compound_remove_desc (*vfs_op_descs[26])
#define vnop_link_desc (*vfs_op_descs[27])
#define vnop_rename_desc (*vfs_op_descs[28])
#define vnop_compound_rename_desc (*vfs_op_descs[29])
#define vnop_mkdir_desc (*vfs_op_descs[30])
#define vnop_compound_mkdir_desc (*vfs_op_descs[31])
#define vnop_rmdir_desc (*vfs_op_descs[32])
#define vnop_compound_rmdir_desc (*vfs_op_descs[33])
#define vnop_symlink_desc (*vfs_op_descs[34])
#define vnop_readdir_desc (*vfs_op_descs[35])
#define vnop_readdirattr_desc (*vfs_op_descs[36])
#define vnop_readlink_desc (*vfs_op_descs[37])
#define vnop_inactive_desc (*vfs_op_descs[38])
#define vnop_reclaim_desc (*vfs_op_descs[39])
#define vnop_pathconf_desc (*vfs_op_descs[40])
#define vnop_advlock_desc (*vfs_op_descs[41])
#define vnop_allocate_desc (*vfs_op_descs[42])
#define vnop_pagein_desc (*vfs_op_descs[43])
#define vnop_pageout_desc (*vfs_op_descs[44])
#define vnop_searchfs_desc (*vfs_op_descs[45])
#define vnop_copyfile_desc (*vfs_op_descs[46])
#define vnop_getxattr_desc (*vfs_op_descs[47])
#define vnop_setxattr_desc (*vfs_op_descs[48])
#define vnop_removexattr_desc (*vfs_op_descs[49])
#define vnop_listxattr_desc (*vfs_op_descs[50])
#define vnop_blktooff_desc (*vfs_op_descs[51])
#define vnop_offtoblk_desc (*vfs_op_descs[52])
#define vnop_blockmap_desc (*vfs_op_descs[53])
#define vnop_monitor_desc (*vfs_op_descs[54])
#else
#if VERSION >= 0x040201
#define vnop_default_desc (*vfs_op_descs[0])
#define vnop_strategy_desc (*vfs_op_descs[1])
#define vnop_bwrite_desc (*vfs_op_descs[2])
#define vnop_lookup_desc (*vfs_op_descs[3])
#define vnop_create_desc (*vfs_op_descs[4])
#define vnop_mknod_desc (*vfs_op_descs[5])
#define vnop_whiteout_desc (*vfs_op_descs[6])
#define vnop_open_desc (*vfs_op_descs[7])
#define vnop_close_desc (*vfs_op_descs[8])
#define vnop_access_desc (*vfs_op_descs[9])
#define vnop_getattr_desc (*vfs_op_descs[10])
#define vnop_setattr_desc (*vfs_op_descs[11])
#define vnop_read_desc (*vfs_op_descs[12])
#define vnop_write_desc (*vfs_op_descs[13])
#define vnop_ioctl_desc (*vfs_op_descs[14])
#define vnop_select_desc (*vfs_op_descs[15])
#define vnop_exchange_desc (*vfs_op_descs[16])
#define vnop_kqfilt_add_desc (*vfs_op_descs[17])
#define vnop_kqfilt_remove_desc (*vfs_op_descs[18])
#define vnop_revoke_desc (*vfs_op_descs[20])
#define vnop_mmap_desc (*vfs_op_descs[21])
#define vnop_mnomap_desc (*vfs_op_descs[22])
#define vnop_fsync_desc (*vfs_op_descs[23])
#define vnop_remove_desc (*vfs_op_descs[24])
#define vnop_link_desc (*vfs_op_descs[25])
#define vnop_rename_desc (*vfs_op_descs[26])
#define vnop_mkdir_desc (*vfs_op_descs[27])
#define vnop_rmdir_desc (*vfs_op_descs[28])
#define vnop_symlink_desc (*vfs_op_descs[29])
#define vnop_readdir_desc (*vfs_op_descs[30])
#define vnop_readdirattr_desc (*vfs_op_descs[31])
#define vnop_readlink_desc (*vfs_op_descs[32])
#define vnop_inactive_desc (*vfs_op_descs[33])
#define vnop_reclaim_desc (*vfs_op_descs[34])
#define vnop_pathconf_desc (*vfs_op_descs[35])
#define vnop_advlock_desc (*vfs_op_descs[36])
#define vnop_allocate_desc (*vfs_op_descs[37])
#define vnop_pagein_desc (*vfs_op_descs[38])
#define vnop_pageout_desc (*vfs_op_descs[39])
#define vnop_searchfs_desc (*vfs_op_descs[40])
#define vnop_copyfile_desc (*vfs_op_descs[41])
#define vnop_getxattr_desc (*vfs_op_descs[42])
#define vnop_setxattr_desc (*vfs_op_descs[43])
#define vnop_removexattr_desc (*vfs_op_descs[44])
#define vnop_listxattr_desc (*vfs_op_descs[45])
#define vnop_blktooff_desc (*vfs_op_descs[46])
#define vnop_offtoblk_desc (*vfs_op_descs[47])
#define vnop_blockmap_desc (*vfs_op_descs[48])
#define vnop_monitor_desc (*vfs_op_descs[49])
#else
#error unknown desc layout
#endif
#endif

extern void *union_dircheckp asm("$ldr_$_T_4d_4b_1b_68_73_b1_0d_f5_92_60");

extern void IOLog(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
//#define printf(args...) ((void) (args))

