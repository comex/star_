#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/kauth.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vnode_internal.h>
#include <sys/mount_internal.h>
#include <sys/namei.h>
#include <sys/malloc.h>
#include <sys/buf_internal.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include "union.h"
#include <sys/mount_internal.h>
#include <stdbool.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

#define CMD_ITERATE(hdr, cmd) for(struct load_command *cmd = (struct load_command *)((hdr) + 1), *end = (struct load_command *)((char *)(hdr + 1) + (hdr)->sizeofcmds); cmd < end; cmd = (struct load_command *)((char *)cmd + cmd->cmdsize))

extern struct {
    int maxvfsconf;
    struct vfstable *vfsconf;
} stuff 

#ifdef IPAD2
// this stuff was modified prior to the dump, it's not a real difference
asm("$in___DATA__11_00_00_00_03_00_00_00_C_15_00_00_00");
#else
asm("$in___DATA__11_00_00_00_00_00_00_00_C_11_00_00_00");
#endif

extern int union_lookup(struct vnop_lookup_args *ap);
extern lck_mtx_t *  union_mtxp;
extern LIST_HEAD(unhead, union_node) *unhead;
extern int *unvplock;
extern int (**union_vnodeop_p)(void *);

static void *sym(void *ptr, const char *find) {
    union {
        uintptr_t addr;
        struct mach_header *mh;
    } u;
    u.addr = ((uintptr_t) ptr) & ~0xfff;
    // search for the header
    while(!(u.mh->magic == MH_MAGIC && u.mh->cputype == CPU_TYPE_ARM && u.mh->cpusubtype == CPU_SUBTYPE_ARM_V7)) {
        u.addr -= 0x1000;
    }
    uint32_t le_begin = 0, le_end = 0;
    void *le_ptr = NULL;
    struct nlist *syms;
    const char *strs;
    uint32_t nsyms;
    uint32_t slide = 0;

    CMD_ITERATE(u.mh, cmd) {
        if(cmd->cmd == LC_SEGMENT) {
            struct segment_command *sc = (void *) cmd;
            if(!slide) slide = sc->vmaddr;
            if(!strncmp(sc->segname, "__LINKEDIT", 16)) {
                le_begin = sc->fileoff;
                le_end = le_begin + sc->filesize;
                le_ptr = (void *) sc->vmaddr;
            }
        } else if(cmd->cmd == LC_SYMTAB) {
            struct symtab_command *sc = (void *) cmd;
            if(le_ptr && sc->symoff >= le_begin && sc->symoff < le_end && sc->stroff >= le_begin && sc->stroff < le_end) {
                syms = le_ptr + (sc->symoff - le_begin);
                strs = le_ptr + (sc->stroff - le_begin);
                nsyms = sc->nsyms;
                goto ok;
            }
        }
    }
    IOLog("couldn't find syms\n");
    return 0;
    ok:;
    size_t findlen = strlen(find);
    for(uint32_t i = 0; i < nsyms; i++) {
        const char *name = strs + syms[i].n_un.n_strx;
        if(!strncmp(name, find, findlen) && (name[findlen] == 0 || name[findlen] == '.')) {
            return (void *) syms[i].n_value + slide;
        }
    }
    return NULL;
}

bool splice() {
    int real_maxvfsconf = 0;
    struct vfstable *union_tbl = NULL;
    for(struct vfstable *tbl = stuff.vfsconf; tbl; tbl = tbl->vfc_next) {
        if(tbl->vfc_typenum >= real_maxvfsconf) real_maxvfsconf = tbl->vfc_typenum;
        if(!strncmp(tbl->vfc_name, "unionfs", MFSNAMELEN)) union_tbl = tbl;
    }
    stuff.maxvfsconf = real_maxvfsconf + 1;

    if(!union_tbl) return false;

    IOLog("Replacing union_lookup...\n");

    struct nameidata nd;
    NDINIT(&nd, LOOKUP, FOLLOW, UIO_SYSSPACE, (user_addr_t) "/Applications", vfs_context_current());
    int error;
    if(error = namei(&nd)) {
        IOLog("couldn't look up /Applications\n");
        return true;
    }
    nameidone(&nd);
    if(nd.ni_vp->v_mount->mnt_vtable != union_tbl) {
        IOLog("/Applications is not unionfs\n");
        goto end;
    }
    void **lookup = (void **) &nd.ni_vp->v_op[vnop_lookup_desc.vdesc_offset];

    void *old = *lookup;
    union_mtxp = *(void **)sym(*lookup, "_union_mtxp");
    unhead = sym(*lookup, "_unhead_storage") ?: sym(*lookup, "_unhead");
    unvplock = sym(*lookup, "_unvplock_storage") ?: sym(*lookup, "_unvplock");
    union_vnodeop_p = *(void **)sym(*lookup, "_union_vnodeop_p");

    IOLog("union_mtxp = %p head=%p vplock=%p ul=%p\n", union_mtxp, unhead, unvplock, union_lookup);
    *lookup = &union_lookup;
    end:
    vnode_put(nd.ni_vp);

    return true;
}
