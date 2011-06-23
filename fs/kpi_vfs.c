/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)kpi_vfs.c
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

/*
 * External virtual filesystem routines
 */


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc_internal.h>
#include <sys/kauth.h>
#include <sys/mount.h>
#include <sys/mount_internal.h>
#include <sys/time.h>
#include <sys/vnode_internal.h>
#include <sys/stat.h>
#include <sys/namei.h>
#include <sys/ucred.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/domain.h>
#include <sys/mbuf.h>
#include <sys/syslog.h>
#include <sys/ubc.h>
#include <sys/vm.h>
#include <sys/sysctl.h>
#include <sys/filedesc.h>
#include <sys/event.h>
#include <sys/fsevents.h>
#include <sys/user.h>
#include <sys/lockf.h>
#include <sys/xattr.h>

#include <kern/assert.h>
#include <kern/kalloc.h>
#include <kern/task.h>

#include <libkern/OSByteOrder.h>

#include <miscfs/specfs/specdev.h>

#include <mach/mach_types.h>
#include <mach/memory_object_types.h>

#if CONFIG_MACF
#include <security/mac_framework.h>
#endif

#define ESUCCESS 0
#undef mount_t
#undef vnode_t

#define COMPAT_ONLY

struct mount *dead_mountp = NULL; // this is wrong

#ifndef __LP64__
#define THREAD_SAFE_FS(VP)  \
	((VP)->v_unsafefs ? 0 : 1)
#endif /* __LP64__ */

#define NATIVE_XATTR(VP)  \
	((VP)->v_mount ? (VP)->v_mount->mnt_kern_flag & MNTK_EXTENDED_ATTRS : 0)

static void xattrfile_remove(vnode_t dvp, const char *basename,
				vfs_context_t ctx, int force);
static void xattrfile_setattr(vnode_t dvp, const char * basename,
				struct vnode_attr * vap, vfs_context_t ctx);

#include "fs.h"

/* ====================================================================== */
/* ************  EXTERNAL KERNEL APIS  ********************************** */
/* ====================================================================== */

/*
 * implementations of exported VFS operations
 */
int 
VFS_MOUNT(mount_t mp, vnode_t devvp, user_addr_t data, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_mount == 0))
		return(ENOTSUP);

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */
	
	if (vfs_context_is64bit(ctx)) {
		if (vfs_64bitready(mp)) {
			error = (*mp->mnt_op->vfs_mount)(mp, devvp, data, ctx);
		}
		else {
			error = ENOTSUP;
		}
	}
	else {
		error = (*mp->mnt_op->vfs_mount)(mp, devvp, data, ctx);
	}
	
#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (error);
}

int 
VFS_START(mount_t mp, int flags, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_start == 0))
		return(ENOTSUP);

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);

	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_start)(mp, flags, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (error);
}

int 
VFS_UNMOUNT(mount_t mp, int flags, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_unmount == 0))
		return(ENOTSUP);

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);

	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_unmount)(mp, flags, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (error);
}

/*
 * Returns:	0			Success
 *		ENOTSUP			Not supported
 *		<vfs_root>:ENOENT
 *		<vfs_root>:???
 *
 * Note:	The return codes from the underlying VFS's root routine can't
 *		be fully enumerated here, since third party VFS authors may not
 *		limit their error returns to the ones documented here, even
 *		though this may result in some programs functioning incorrectly.
 *
 *		The return codes documented above are those which may currently
 *		be returned by HFS from hfs_vfs_root, which is a simple wrapper
 *		for a call to hfs_vget on the volume mount poit, not including
 *		additional error codes which may be propagated from underlying
 *		routines called by hfs_vget.
 */
int 
VFS_ROOT(mount_t mp, struct vnode  ** vpp, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_root == 0))
		return(ENOTSUP);

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_root)(mp, vpp, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (error);
}

int 
VFS_QUOTACTL(mount_t mp, int cmd, uid_t uid, caddr_t datap, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_quotactl == 0))
		return(ENOTSUP);

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_quotactl)(mp, cmd, uid, datap, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (error);
}

int 
VFS_GETATTR(mount_t mp, struct vfs_attr *vfa, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_getattr == 0))
		return(ENOTSUP);

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_getattr)(mp, vfa, ctx);
	
#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return(error);
}

int 
VFS_SETATTR(mount_t mp, struct vfs_attr *vfa, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_setattr == 0))
		return(ENOTSUP);

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_setattr)(mp, vfa, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return(error);
}

int 
VFS_SYNC(mount_t mp, int flags, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_sync == 0))
		return(ENOTSUP);

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_sync)(mp, flags, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return(error);
}

int 
VFS_VGET(mount_t mp, ino64_t ino, struct vnode **vpp, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_vget == 0))
		return(ENOTSUP);

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_vget)(mp, ino, vpp, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return(error);
}

int 
VFS_FHTOVP(mount_t mp, int fhlen, unsigned char * fhp, vnode_t * vpp, vfs_context_t ctx) 
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((mp == dead_mountp) || (mp->mnt_op->vfs_fhtovp == 0))
		return(ENOTSUP);

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}

#ifndef __LP64__
	thread_safe = (mp->mnt_vtable->vfc_vfsflags & VFC_VFSTHREADSAFE);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_fhtovp)(mp, fhlen, fhp, vpp, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return(error);
}

int 
VFS_VPTOFH(struct vnode * vp, int *fhlenp, unsigned char * fhp, vfs_context_t ctx)
{
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if ((vp->v_mount == dead_mountp) || (vp->v_mount->mnt_op->vfs_vptofh == 0))
		return(ENOTSUP);

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*vp->v_mount->mnt_op->vfs_vptofh)(vp, fhlenp, fhp, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return(error);
}



void
vfs_markdependency(mount_t mp)
{
	proc_t p = current_proc();
	mount_lock(mp);
	mp->mnt_dependent_process = p;
	mp->mnt_dependent_pid = proc_pid(p);
	mount_unlock(mp);
}


int
vfs_extendedsecurity(mount_t mp)
{
	return(mp->mnt_kern_flag & MNTK_EXTENDED_SECURITY);
}

int
vfs_setattr(mount_t mp, struct vfs_attr *vfa, vfs_context_t ctx)
{
	int error;
	
	if (vfs_isrdonly(mp))
		return EROFS;

	error = VFS_SETATTR(mp, vfa, ctx);
	
	/*
	 * If we had alternate ways of setting vfs attributes, we'd
	 * fall back here.
	 */

	return error;
}

/*
 * vfs_context_thread
 *
 * Description:	Return the Mach thread associated with a vfs_context_t
 *
 * Parameters:	vfs_context_t			The context to use
 *
 * Returns:	thread_t			The thread for this context, or
 *						NULL, if there is not one.
 *
 * Notes:	NULL thread_t's are legal, but discouraged.  They occur only
 *		as a result of a static vfs_context_t declaration in a function
 *		and will result in this function returning NULL.
 *
 *		This is intentional; this function should NOT return the
 *		current_thread() in this case.
 */
thread_t
vfs_context_thread(vfs_context_t ctx)
{
	return(ctx->vc_thread);
}


/*
 * vfs_context_cwd
 *
 * Description:	Returns a reference on the vnode for the current working
 *		directory for the supplied context
 *
 * Parameters:	vfs_context_t			The context to use
 *
 * Returns:	vnode_t				The current working directory
 *						for this context
 *
 * Notes:	The function first attempts to obtain the current directory
 *		from the thread, and if it is not present there, falls back
 *		to obtaining it from the process instead.  If it can't be
 *		obtained from either place, we return NULLVP.
 */
vnode_t
vfs_context_cwd(vfs_context_t ctx)
{
	vnode_t cwd = NULLVP;

	if(ctx != NULL && ctx->vc_thread != NULL) {
		uthread_t uth = get_bsdthread_info(ctx->vc_thread);
		proc_t proc;

		/*
		 * Get the cwd from the thread; if there isn't one, get it
		 * from the process, instead.
		 */
		if ((cwd = uth->uu_cdir) == NULLVP &&
		    (proc = (proc_t)get_bsdthreadtask_info(ctx->vc_thread)) != NULL &&
		    proc->p_fd != NULL)
			cwd = proc->p_fd->fd_cdir;
	}

	return(cwd);
}



/*
 * Return true if the context is owned by the superuser.
 */
int
vfs_context_issuser(vfs_context_t ctx)
{
	return(kauth_cred_issuser(vfs_context_ucred(ctx)));
}

/* XXXXXXXXXXXXXX VNODE KAPIS XXXXXXXXXXXXXXXXXXXXXXXXX */

 
/*
 * Convert between vnode types and inode formats (since POSIX.1
 * defines mode word of stat structure in terms of inode formats).
 */
enum vtype 
vnode_iftovt(int mode)
{
	return(iftovt_tab[((mode) & S_IFMT) >> 12]);
}

int 
vnode_vttoif(enum vtype indx)
{
	return(vttoif_tab[(int)(indx)]);
}

int 
vnode_makeimode(int indx, int mode)
{
	return (int)(VTTOIF(indx) | (mode));
}


/* is vnode_t a tty */
int
vnode_istty(vnode_t vp)
{
	return ((vp->v_flag & VISTTY) ? 1 : 0);
}

int
vnode_is_openevt(vnode_t vp)
{
	return ((vp->v_flag & VOPENEVT)? 1 : 0);
}

/* is vnode_t a standard one? */
int 
vnode_isstandard(vnode_t vp)
{
	return ((vp->v_flag & VSTANDARD)? 1 : 0);
}

/* don't vflush() if SKIPSYSTEM */
int 
vnode_isnoflush(vnode_t vp)
{
	return ((vp->v_flag & VNOFLUSH)? 1 : 0);
}


int
vnode_isspec(vnode_t vp)
{
	return (((vp->v_type == VCHR) || (vp->v_type == VBLK)) ? 1 : 0);
}
/* is vnode_t a socket? */
int 
vnode_issock(vnode_t vp)
{
	return ((vp->v_type == VSOCK)? 1 : 0);
}

/* is vnode_t a device with multiple active vnodes referring to it? */
int
vnode_isaliased(vnode_t vp)
{	
	enum vtype vt = vp->v_type;
	if (!((vt == VCHR) || (vt == VBLK))) {
		return 0;
	} else {
		return (vp->v_specflags & SI_ALIASED);
	}
}

int 	
vnode_isshadow(
#if NAMEDSTREAMS
		vnode_t vp
#else
		__unused vnode_t vp
#endif
		)
{
#if NAMEDSTREAMS
	return ((vp->v_flag & VISSHADOW) ? 1 : 0);
#else
	return (0);
#endif
}

/* does vnode have associated named stream vnodes ? */
int 
vnode_hasnamedstreams(
#if NAMEDSTREAMS
		vnode_t vp
#else
		__unused vnode_t vp
#endif
		)
{
#if NAMEDSTREAMS
	return ((vp->v_lflag & VL_HASSTREAMS) ? 1 : 0);
#else
	return (0);
#endif
}
void
vnode_set_openevt(vnode_t vp)
{
	vnode_lock_spin(vp);
	vp->v_flag |= VOPENEVT;
	vnode_unlock(vp);
}

void
vnode_clear_openevt(vnode_t vp)
{
	vnode_lock_spin(vp);
	vp->v_flag &= ~VOPENEVT;
	vnode_unlock(vp);
}



/* mark vnode_t to skip vflush() is SKIPSYSTEM */
void 
vnode_setnoflush(vnode_t vp)
{
	vnode_lock_spin(vp);
	vp->v_flag |= VNOFLUSH;
	vnode_unlock(vp);
}

void 
vnode_clearnoflush(vnode_t vp)
{
	vnode_lock_spin(vp);
	vp->v_flag &= ~VNOFLUSH;
	vnode_unlock(vp);
}


vnode_t 
vnode_parent(vnode_t vp)
{

	return(vp->v_parent);
}

void
vnode_setparent(vnode_t vp, vnode_t dvp)
{
	vp->v_parent = dvp;
}

const char *
vnode_name(vnode_t vp)
{
	/* we try to keep v_name a reasonable name for the node */    
	return(vp->v_name);
}

void
vnode_setname(vnode_t vp, char * name)
{
	vp->v_name = name;
}


/* return the visible flags on associated mount point of vnode_t */
uint32_t 
vnode_vfsvisflags(vnode_t vp)
{
	return(vp->v_mount->mnt_flag & MNT_VISFLAGMASK);
}

/* return the command modifier flags on associated mount point of vnode_t */
uint32_t 
vnode_vfscmdflags(vnode_t vp)
{
	return(vp->v_mount->mnt_flag & MNT_CMDFLAGS);
}

/* return a pointer to the RO vfs_statfs associated with vnode_t's mount point */
struct vfsstatfs *
vnode_vfsstatfs(vnode_t vp)
{
        return(&vp->v_mount->mnt_vfsstat);
}

/* return a handle to the FSs specific private handle associated with vnode_t's mount point */
void *
vnode_vfsfsprivate(vnode_t vp)
{
	return(vp->v_mount->mnt_data);
}


/*
 * Returns vnode ref to current working directory; if a per-thread current
 * working directory is in effect, return that instead of the per process one.
 *
 * XXX Published, but not used.
 */
vnode_t 
current_workingdir(void)
{
	return vfs_context_cwd(vfs_context_current());
}

/* returns vnode ref to current root(chroot) directory */
vnode_t 
current_rootdir(void)
{
	proc_t proc = current_proc();
	struct vnode * vp ;

	if ( (vp = proc->p_fd->fd_rdir) ) {
	        if ( (vnode_getwithref(vp)) )
		        return (NULL);
	}
	return vp;
}

/*
 * Get a filesec and optional acl contents from an extended attribute.
 * Function will attempt to retrive ACL, UUID, and GUID information using a
 * read of a named extended attribute (KAUTH_FILESEC_XATTR).
 *
 * Parameters:	vp			The vnode on which to operate.
 *		fsecp			The filesec (and ACL, if any) being
 *					retrieved.
 *		ctx			The vnode context in which the
 *					operation is to be attempted.
 *
 * Returns:	0			Success
 *		!0			errno value
 *
 * Notes:	The kauth_filesec_t in '*fsecp', if retrieved, will be in
 *		host byte order, as will be the ACL contents, if any.
 *		Internally, we will cannonize these values from network (PPC)
 *		byte order after we retrieve them so that the on-disk contents
 *		of the extended attribute are identical for both PPC and Intel
 *		(if we were not being required to provide this service via
 *		fallback, this would be the job of the filesystem
 *		'VNOP_GETATTR' call).
 *
 *		We use ntohl() because it has a transitive property on Intel
 *		machines and no effect on PPC mancines.  This guarantees us
 *
 * XXX:		Deleting rather than ignoreing a corrupt security structure is
 *		probably the only way to reset it without assistance from an
 *		file system integrity checking tool.  Right now we ignore it.
 *
 * XXX:		We should enummerate the possible errno values here, and where
 *		in the code they originated.
 */
static int
vnode_get_filesec(vnode_t vp, kauth_filesec_t *fsecp, vfs_context_t ctx)
{
	kauth_filesec_t fsec;
	uio_t	fsec_uio;
	size_t	fsec_size;
	size_t	xsize, rsize;
	int	error;
	uint32_t	host_fsec_magic;
	uint32_t	host_acl_entrycount;

	fsec = NULL;
	fsec_uio = NULL;
	error = 0;
	
	/* find out how big the EA is */
	if (vn_getxattr(vp, KAUTH_FILESEC_XATTR, NULL, &xsize, XATTR_NOSECURITY, ctx) != 0) {
		/* no EA, no filesec */
		if ((error == ENOATTR) || (error == ENOENT) || (error == EJUSTRETURN))
			error = 0;
		/* either way, we are done */
		goto out;
	}

	/*
	 * To be valid, a kauth_filesec_t must be large enough to hold a zero
	 * ACE entrly ACL, and if it's larger than that, it must have the right
	 * number of bytes such that it contains an atomic number of ACEs,
	 * rather than partial entries.  Otherwise, we ignore it.
	 */
	if (!KAUTH_FILESEC_VALID(xsize)) {
		KAUTH_DEBUG("    ERROR - Bogus kauth_fiilesec_t: %ld bytes", xsize);	
		error = 0;
		goto out;
	}
				
	/* how many entries would fit? */
	fsec_size = KAUTH_FILESEC_COUNT(xsize);

	/* get buffer and uio */
	if (((fsec = kauth_filesec_alloc(fsec_size)) == NULL) ||
	    ((fsec_uio = uio_create(1, 0, UIO_SYSSPACE, UIO_READ)) == NULL) ||
	    uio_addiov(fsec_uio, CAST_USER_ADDR_T(fsec), xsize)) {
		KAUTH_DEBUG("    ERROR - could not allocate iov to read ACL");	
		error = ENOMEM;
		goto out;
	}

	/* read security attribute */
	rsize = xsize;
	if ((error = vn_getxattr(vp,
		 KAUTH_FILESEC_XATTR,
		 fsec_uio,
		 &rsize,
		 XATTR_NOSECURITY,
		 ctx)) != 0) {

		/* no attribute - no security data */
		if ((error == ENOATTR) || (error == ENOENT) || (error == EJUSTRETURN))
			error = 0;
		/* either way, we are done */
		goto out;
	}

	/*
	 * Validate security structure; the validation must take place in host
	 * byte order.  If it's corrupt, we will just ignore it.
	 */

	/* Validate the size before trying to convert it */
	if (rsize < KAUTH_FILESEC_SIZE(0)) {
		KAUTH_DEBUG("ACL - DATA TOO SMALL (%d)", rsize);
		goto out;
	}

	/* Validate the magic number before trying to convert it */
	host_fsec_magic = ntohl(KAUTH_FILESEC_MAGIC);
	if (fsec->fsec_magic != host_fsec_magic) {
		KAUTH_DEBUG("ACL - BAD MAGIC %x", host_fsec_magic);
		goto out;
	}

	/* Validate the entry count before trying to convert it. */
	host_acl_entrycount = ntohl(fsec->fsec_acl.acl_entrycount);
	if (host_acl_entrycount != KAUTH_FILESEC_NOACL) {
		if (host_acl_entrycount > KAUTH_ACL_MAX_ENTRIES) {
			KAUTH_DEBUG("ACL - BAD ENTRYCOUNT %x", host_acl_entrycount);
			goto out;
		}
	    	if (KAUTH_FILESEC_SIZE(host_acl_entrycount) > rsize) {
			KAUTH_DEBUG("ACL - BUFFER OVERFLOW (%d entries too big for %d)", host_acl_entrycount, rsize);
			goto out;
		}
	}

	kauth_filesec_acl_setendian(KAUTH_ENDIAN_HOST, fsec, NULL);

	*fsecp = fsec;
	fsec = NULL;
	error = 0;
out:
	if (fsec != NULL)
		kauth_filesec_free(fsec);
	if (fsec_uio != NULL)
		uio_free(fsec_uio);
	if (error)
		*fsecp = NULL;
	return(error);
}

/*
 * Set a filesec and optional acl contents into an extended attribute.
 * function will attempt to store ACL, UUID, and GUID information using a
 * write to a named extended attribute (KAUTH_FILESEC_XATTR).  The 'acl'
 * may or may not point to the `fsec->fsec_acl`, depending on whether the
 * original caller supplied an acl.
 *
 * Parameters:	vp			The vnode on which to operate.
 *		fsec			The filesec being set.
 *		acl			The acl to be associated with 'fsec'.
 *		ctx			The vnode context in which the
 *					operation is to be attempted.
 *
 * Returns:	0			Success
 *		!0			errno value
 *
 * Notes:	Both the fsec and the acl are always valid.
 *
 *		The kauth_filesec_t in 'fsec', if any, is in host byte order,
 *		as are the acl contents, if they are used.  Internally, we will
 *		cannonize these values into network (PPC) byte order before we
 *		attempt to write them so that the on-disk contents of the
 *		extended attribute are identical for both PPC and Intel (if we
 *		were not being required to provide this service via fallback,
 *		this would be the job of the filesystem 'VNOP_SETATTR' call).
 *		We reverse this process on the way out, so we leave with the
 *		same byte order we started with.
 *
 * XXX:		We should enummerate the possible errno values here, and where
 *		in the code they originated.
 */
static int
vnode_set_filesec(vnode_t vp, kauth_filesec_t fsec, kauth_acl_t acl, vfs_context_t ctx)
{
	uio_t		fsec_uio;
	int		error;
	uint32_t	saved_acl_copysize;

	fsec_uio = NULL;
	
	if ((fsec_uio = uio_create(2, 0, UIO_SYSSPACE, UIO_WRITE)) == NULL) {
		KAUTH_DEBUG("    ERROR - could not allocate iov to write ACL");	
		error = ENOMEM;
		goto out;
	}
	/*
	 * Save the pre-converted ACL copysize, because it gets swapped too
	 * if we are running with the wrong endianness.
	 */
	saved_acl_copysize = KAUTH_ACL_COPYSIZE(acl);

	kauth_filesec_acl_setendian(KAUTH_ENDIAN_DISK, fsec, acl);

	uio_addiov(fsec_uio, CAST_USER_ADDR_T(fsec), KAUTH_FILESEC_SIZE(0) - KAUTH_ACL_SIZE(KAUTH_FILESEC_NOACL));
	uio_addiov(fsec_uio, CAST_USER_ADDR_T(acl), saved_acl_copysize);
	error = vn_setxattr(vp,
	    KAUTH_FILESEC_XATTR,
	    fsec_uio,
	    XATTR_NOSECURITY, 		/* we have auth'ed already */
	    ctx);
	VFS_DEBUG(ctx, vp, "SETATTR - set ACL returning %d", error);

	kauth_filesec_acl_setendian(KAUTH_ENDIAN_HOST, fsec, acl);

out:
	if (fsec_uio != NULL)
		uio_free(fsec_uio);
	return(error);
}



/*
 * Conceived as a function available only in BSD kernel so that if kevent_register
 * changes what a knote of type EVFILT_VNODE is watching, it can push
 * that updated information down to a networked filesystem that may
 * need to update server-side monitoring.
 *
 * Blunted to do nothing--because we want to get both kqueue and fsevents support
 * from the VNOP_MONITOR design, we always want all the events a filesystem can provide us.
 */
void
vnode_knoteupdate(__unused struct knote *kn) 
{
#if 0
	vnode_t vp = (vnode_t)kn->kn_hook;
	if (vnode_getwithvid(vp, kn->kn_hookid) == 0) {
		VNOP_MONITOR(vp, kn->kn_sfflags, VNODE_MONITOR_UPDATE, (void*)kn, NULL);
		vnode_put(vp);
	}
#endif
}


/*
 *  Definition of vnode operations.
 */

#if 0
/*
 *# 
 *#% lookup       dvp     L ? ?
 *#% lookup       vpp     - L -
 */
struct vnop_lookup_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	vfs_context_t a_context;
};
#endif /* 0*/

/*
 * Returns:	0			Success
 *	lock_fsnode:ENOENT		No such file or directory [only for VFS
 *					 that is not thread safe & vnode is
 *					 currently being/has been terminated]
 *	<vfs_lookup>:ENAMETOOLONG
 *	<vfs_lookup>:ENOENT
 *	<vfs_lookup>:EJUSTRETURN
 *	<vfs_lookup>:EPERM
 *	<vfs_lookup>:EISDIR
 *	<vfs_lookup>:ENOTDIR
 *	<vfs_lookup>:???
 *
 * Note:	The return codes from the underlying VFS's lookup routine can't
 *		be fully enumerated here, since third party VFS authors may not
 *		limit their error returns to the ones documented here, even
 *		though this may result in some programs functioning incorrectly.
 *
 *		The return codes documented above are those which may currently
 *		be returned by HFS from hfs_lookup, not including additional
 *		error code which may be propagated from underlying routines.
 */
errno_t 
VNOP_LOOKUP(vnode_t dvp, vnode_t *vpp, struct componentname *cnp, vfs_context_t ctx)
{
	int _err;
	struct vnop_lookup_args a;
	vnode_t vp;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_lookup_desc;
	a.a_dvp = dvp;
	a.a_vpp = vpp;
	a.a_cnp = cnp;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(dvp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(dvp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*dvp->v_op[vnop_lookup_desc.vdesc_offset])(&a);

	vp = *vpp;

#ifndef __LP64__
	if (!thread_safe) {
	        if ( (cnp->cn_flags & ISLASTCN) ) {
		        if ( (cnp->cn_flags & LOCKPARENT) ) {
			        if ( !(cnp->cn_flags & FSNODELOCKHELD) ) {
				        /*
					 * leave the fsnode lock held on
					 * the directory, but restore the funnel...
					 * also indicate that we need to drop the
					 * fsnode_lock when we're done with the
					 * system call processing for this path
					 */
				        cnp->cn_flags |= FSNODELOCKHELD;
					
					(void) thread_funnel_set(kernel_flock, funnel_state);
					return (_err);
				}
			}
		}
		unlock_fsnode(dvp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% create       dvp     L L L
 *#% create       vpp     - L -
 *#
 */
 
struct vnop_create_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	struct vnode_attr *a_vap;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_CREATE(vnode_t dvp, vnode_t * vpp, struct componentname * cnp, struct vnode_attr * vap, vfs_context_t ctx)
{
	int _err;
	struct vnop_create_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_create_desc;
	a.a_dvp = dvp;
	a.a_vpp = vpp;
	a.a_cnp = cnp;
	a.a_vap = vap;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(dvp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(dvp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*dvp->v_op[vnop_create_desc.vdesc_offset])(&a);
	if (_err == 0 && !NATIVE_XATTR(dvp)) {
		/* 
		 * Remove stale Apple Double file (if any).
		 */
		xattrfile_remove(dvp, cnp->cn_nameptr, ctx, 0);
	}

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(dvp, &funnel_state);
	}
#endif /* __LP64__ */

	post_event_if_success(dvp, _err, NOTE_WRITE);

	return (_err);
}

#if 0
/*
 *#
 *#% whiteout     dvp     L L L
 *#% whiteout     cnp     - - -
 *#% whiteout     flag    - - -
 *#
 */
struct vnop_whiteout_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	struct componentname *a_cnp;
	int a_flags;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_WHITEOUT(vnode_t dvp, struct componentname * cnp, int flags, vfs_context_t ctx)
{
	int _err;
	struct vnop_whiteout_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_whiteout_desc;
	a.a_dvp = dvp;
	a.a_cnp = cnp;
	a.a_flags = flags;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(dvp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(dvp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*dvp->v_op[vnop_whiteout_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(dvp, &funnel_state);
	}
#endif /* __LP64__ */

	post_event_if_success(dvp, _err, NOTE_WRITE);

	return (_err);
}

 #if 0
/*
 *#
 *#% mknod        dvp     L U U
 *#% mknod        vpp     - X -
 *#
 */
struct vnop_mknod_args {
       struct vnodeop_desc *a_desc;
       vnode_t a_dvp;
       vnode_t *a_vpp;
       struct componentname *a_cnp;
       struct vnode_attr *a_vap;
       vfs_context_t a_context;
};
#endif /* 0*/
errno_t
VNOP_MKNOD(vnode_t dvp, vnode_t * vpp, struct componentname * cnp, struct vnode_attr * vap, vfs_context_t ctx)
{

       int _err;
       struct vnop_mknod_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

       a.a_desc = &vnop_mknod_desc;
       a.a_dvp = dvp;
       a.a_vpp = vpp;
       a.a_cnp = cnp;
       a.a_vap = vap;
       a.a_context = ctx;

#ifndef __LP64__
       thread_safe = THREAD_SAFE_FS(dvp);
       if (!thread_safe) {
               if ( (_err = lock_fsnode(dvp, &funnel_state)) ) {
                       return (_err);
               }
       }
#endif /* __LP64__ */

       _err = (*dvp->v_op[vnop_mknod_desc.vdesc_offset])(&a);

#ifndef __LP64__
       if (!thread_safe) {
               unlock_fsnode(dvp, &funnel_state);
       }
#endif /* __LP64__ */

       post_event_if_success(dvp, _err, NOTE_WRITE);

       return (_err);
}

#if 0
/*
 *#
 *#% open         vp      L L L
 *#
 */
struct vnop_open_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_mode;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_OPEN(vnode_t vp, int mode, vfs_context_t ctx)
{
	int _err;
	struct vnop_open_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}
	a.a_desc = &vnop_open_desc;
	a.a_vp = vp;
	a.a_mode = mode;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
		if (vp->v_type != VCHR && vp->v_type != VFIFO && vp->v_type != VSOCK) {
		        if ( (_err = lock_fsnode(vp, NULL)) ) {
			        (void) thread_funnel_set(kernel_flock, funnel_state);
			        return (_err);
			}
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_open_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		if (vp->v_type != VCHR && vp->v_type != VFIFO && vp->v_type != VSOCK) {
			unlock_fsnode(vp, NULL);
		}
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% close        vp      U U U
 *#
 */
struct vnop_close_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_fflag;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_CLOSE(vnode_t vp, int fflag, vfs_context_t ctx)
{
	int _err;
	struct vnop_close_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}
	a.a_desc = &vnop_close_desc;
	a.a_vp = vp;
	a.a_fflag = fflag;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
		if (vp->v_type != VCHR && vp->v_type != VFIFO && vp->v_type != VSOCK) {
		        if ( (_err = lock_fsnode(vp, NULL)) ) {
			        (void) thread_funnel_set(kernel_flock, funnel_state);
			        return (_err);
			}
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_close_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		if (vp->v_type != VCHR && vp->v_type != VFIFO && vp->v_type != VSOCK) {
			unlock_fsnode(vp, NULL);
		}
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% access       vp      L L L
 *#
 */
struct vnop_access_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_action;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_ACCESS(vnode_t vp, int action, vfs_context_t ctx)
{
	int _err;
	struct vnop_access_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}
	a.a_desc = &vnop_access_desc;
	a.a_vp = vp;
	a.a_action = action;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_access_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% getattr      vp      = = =
 *#
 */
struct vnop_getattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct vnode_attr *a_vap;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_GETATTR(vnode_t vp, struct vnode_attr * vap, vfs_context_t ctx)
{
	int _err;
	struct vnop_getattr_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_getattr_desc;
	a.a_vp = vp;
	a.a_vap = vap;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_getattr_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% setattr      vp      L L L
 *#
 */
struct vnop_setattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct vnode_attr *a_vap;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_SETATTR(vnode_t vp, struct vnode_attr * vap, vfs_context_t ctx)
{
	int _err;
	struct vnop_setattr_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_setattr_desc;
	a.a_vp = vp;
	a.a_vap = vap;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_setattr_desc.vdesc_offset])(&a);

	/* 
	 * Shadow uid/gid/mod change to extended attribute file.
	 */
	if (_err == 0 && !NATIVE_XATTR(vp)) {
		struct vnode_attr va;
		int change = 0;

		VATTR_INIT(&va);
		if (VATTR_IS_ACTIVE(vap, va_uid)) {
			VATTR_SET(&va, va_uid, vap->va_uid);
			change = 1;
		}
		if (VATTR_IS_ACTIVE(vap, va_gid)) {
			VATTR_SET(&va, va_gid, vap->va_gid);
			change = 1;
		}
		if (VATTR_IS_ACTIVE(vap, va_mode)) {
			VATTR_SET(&va, va_mode, vap->va_mode);
			change = 1;
		}
		if (change) {
		        vnode_t dvp;
			const char   *vname;

			dvp = vnode_getparent(vp);
			vname = vnode_getname(vp);

			xattrfile_setattr(dvp, vname, &va, ctx);
			if (dvp != NULLVP)
			        vnode_put(dvp);
			if (vname != NULL)
			        vnode_putname(vname);
		}
	}

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	/*
	 * If we have changed any of the things about the file that are likely
	 * to result in changes to authorization results, blow the vnode auth
	 * cache
	 */
	if (_err == 0 && (
			  VATTR_IS_SUPPORTED(vap, va_mode) ||
			  VATTR_IS_SUPPORTED(vap, va_uid) ||
			  VATTR_IS_SUPPORTED(vap, va_gid) ||
			  VATTR_IS_SUPPORTED(vap, va_flags) ||
			  VATTR_IS_SUPPORTED(vap, va_acl) ||
			  VATTR_IS_SUPPORTED(vap, va_uuuid) ||
			  VATTR_IS_SUPPORTED(vap, va_guuid))) {
	        vnode_uncache_authorized_action(vp, KAUTH_INVALIDATE_CACHED_RIGHTS);

#if NAMEDSTREAMS
		if (vfs_authopaque(vp->v_mount) && vnode_hasnamedstreams(vp)) {
			vnode_t svp;
			if (vnode_getnamedstream(vp, &svp, XATTR_RESOURCEFORK_NAME, NS_OPEN, 0, ctx) == 0) {
				vnode_uncache_authorized_action(svp, KAUTH_INVALIDATE_CACHED_RIGHTS);
				vnode_put(svp);
		 	} 
		} 
#endif /* NAMEDSTREAMS */
	}


	post_event_if_success(vp, _err, NOTE_ATTRIB);

	return (_err);
}


#if 0
/*
 *#
 *#% select       vp      U U U
 *#
 */
struct vnop_select_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_which;
	int a_fflags;
	void *a_wql;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_SELECT(vnode_t vp, int which , int fflags, void * wql, vfs_context_t ctx)
{
	int _err;
	struct vnop_select_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}
	a.a_desc = &vnop_select_desc;
	a.a_vp = vp;
	a.a_which = which;
	a.a_fflags = fflags;
	a.a_context = ctx;
	a.a_wql = wql;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
		if (vp->v_type != VCHR && vp->v_type != VFIFO && vp->v_type != VSOCK) {
		        if ( (_err = lock_fsnode(vp, NULL)) ) {
			        (void) thread_funnel_set(kernel_flock, funnel_state);
				return (_err);
			}
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_select_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		if (vp->v_type != VCHR && vp->v_type != VFIFO && vp->v_type != VSOCK) {
			unlock_fsnode(vp, NULL);
		}
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}


#if 0
/*
 *#
 *#% exchange fvp         L L L
 *#% exchange tvp         L L L
 *#
 */
struct vnop_exchange_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_fvp;
        vnode_t a_tvp;
        int a_options;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_EXCHANGE(vnode_t fvp, vnode_t tvp, int options, vfs_context_t ctx)
{
	int _err;
	struct vnop_exchange_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
	vnode_t	lock_first = NULL, lock_second = NULL;
#endif /* __LP64__ */

	a.a_desc = &vnop_exchange_desc;
	a.a_fvp = fvp;
	a.a_tvp = tvp;
	a.a_options = options;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(fvp);
	if (!thread_safe) {
		/*
		 * Lock in vnode address order to avoid deadlocks
		 */
		if (fvp < tvp) {
		        lock_first  = fvp;
			lock_second = tvp;
		} else {
		        lock_first  = tvp;
			lock_second = fvp;
		}
		if ( (_err = lock_fsnode(lock_first, &funnel_state)) ) {
		        return (_err);
		}
		if ( (_err = lock_fsnode(lock_second, NULL)) ) {
		        unlock_fsnode(lock_first, &funnel_state);
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*fvp->v_op[vnop_exchange_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(lock_second, NULL);
		unlock_fsnode(lock_first, &funnel_state);
	}
#endif /* __LP64__ */

	/* Don't post NOTE_WRITE because file descriptors follow the data ... */
	post_event_if_success(fvp, _err, NOTE_ATTRIB);
	post_event_if_success(tvp, _err, NOTE_ATTRIB);

	return (_err);
}


#if 0
/*
 *#
 *#% revoke       vp      U U U
 *#
 */
struct vnop_revoke_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_flags;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_REVOKE(vnode_t vp, int flags, vfs_context_t ctx)
{
	struct vnop_revoke_args a;
	int _err;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_revoke_desc;
	a.a_vp = vp;
	a.a_flags = flags;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_revoke_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}


#if 0
/*
 *#
 *# mmap - vp U U U
 *#
 */
struct vnop_mmap_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_fflags;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_MMAP(vnode_t vp, int fflags, vfs_context_t ctx)
{
	int _err;
	struct vnop_mmap_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_mmap_desc;
	a.a_vp = vp;
	a.a_fflags = fflags;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_mmap_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}


#if 0
/*
 *#
 *# mnomap - vp U U U
 *#
 */
struct vnop_mnomap_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_MNOMAP(vnode_t vp, vfs_context_t ctx)
{
	int _err;
	struct vnop_mnomap_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_mnomap_desc;
	a.a_vp = vp;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_mnomap_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% remove       dvp     L U U
 *#% remove       vp      L U U
 *#
 */
struct vnop_remove_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t a_vp;
	struct componentname *a_cnp;
	int a_flags;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_REMOVE(vnode_t dvp, vnode_t vp, struct componentname * cnp, int flags, vfs_context_t ctx)
{
	int _err;
	struct vnop_remove_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_remove_desc;
	a.a_dvp = dvp;
	a.a_vp = vp;
	a.a_cnp = cnp;
	a.a_flags = flags;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(dvp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*dvp->v_op[vnop_remove_desc.vdesc_offset])(&a);

	if (_err == 0) {
	        vnode_setneedinactive(vp);

		if ( !(NATIVE_XATTR(dvp)) ) {
		        /* 
			 * Remove any associated extended attribute file (._ AppleDouble file).
			 */
		        xattrfile_remove(dvp, cnp->cn_nameptr, ctx, 1);
		}
	}

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	post_event_if_success(vp, _err, NOTE_DELETE | NOTE_LINK);
	post_event_if_success(dvp, _err, NOTE_WRITE);
	
	return (_err);
}


#if 0
/*
 *#
 *#% link         vp      U U U
 *#% link         tdvp    L U U
 *#
 */
struct vnop_link_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vnode_t a_tdvp;
	struct componentname *a_cnp;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_LINK(vnode_t vp, vnode_t tdvp, struct componentname * cnp, vfs_context_t ctx)
{
	int _err;
	struct vnop_link_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	/*
	 * For file systems with non-native extended attributes,
	 * disallow linking to an existing "._" Apple Double file.
	 */
	if ( !NATIVE_XATTR(tdvp) && (vp->v_type == VREG)) {
		const char   *vname;

		vname = vnode_getname(vp);
		if (vname != NULL) {
			_err = 0;
			if (vname[0] == '.' && vname[1] == '_' && vname[2] != '\0') {
				_err = EPERM;
			}
			vnode_putname(vname);
			if (_err)
				return (_err);
		}
	}
	a.a_desc = &vnop_link_desc;
	a.a_vp = vp;
	a.a_tdvp = tdvp;
	a.a_cnp = cnp;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*tdvp->v_op[vnop_link_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	post_event_if_success(vp, _err, NOTE_LINK);
	post_event_if_success(tdvp, _err, NOTE_WRITE);

	return (_err);
}


#if 0
/*
 *#
 *#% rename       fdvp    U U U
 *#% rename       fvp     U U U
 *#% rename       tdvp    L U U
 *#% rename       tvp     X U U
 *#
 */
struct vnop_rename_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_fdvp;
	vnode_t a_fvp;
	struct componentname *a_fcnp;
	vnode_t a_tdvp;
	vnode_t a_tvp;
	struct componentname *a_tcnp;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t
VNOP_RENAME(struct vnode *fdvp, struct vnode *fvp, struct componentname *fcnp,
            struct vnode *tdvp, struct vnode *tvp, struct componentname *tcnp,
            vfs_context_t ctx)
{
	int _err = 0;
	int events;
	struct vnop_rename_args a;
	char smallname1[48];
	char smallname2[48];
	char *xfromname = NULL;
	char *xtoname = NULL;
#ifndef __LP64__
	int funnel_state = 0;
	vnode_t	lock_first = NULL, lock_second = NULL;
	vnode_t fdvp_unsafe = NULLVP;
	vnode_t tdvp_unsafe = NULLVP;
#endif /* __LP64__ */
	vnode_t src_attr_vp = NULLVP;
	vnode_t dst_attr_vp = NULLVP;
	struct nameidata fromnd;
	struct nameidata tond;

	a.a_desc = &vnop_rename_desc;
	a.a_fdvp = fdvp;
	a.a_fvp = fvp;
	a.a_fcnp = fcnp;
	a.a_tdvp = tdvp;
	a.a_tvp = tvp;
	a.a_tcnp = tcnp;
	a.a_context = ctx;

#ifndef __LP64__
	if (!THREAD_SAFE_FS(fdvp))
	        fdvp_unsafe = fdvp;
	if (!THREAD_SAFE_FS(tdvp))
	        tdvp_unsafe = tdvp;

	if (fdvp_unsafe != NULLVP) {
		/*
		 * Lock parents in vnode address order to avoid deadlocks
		 * note that it's possible for the fdvp to be unsafe,
		 * but the tdvp to be safe because tvp could be a directory
		 * in the root of a filesystem... in that case, tdvp is the
		 * in the filesystem that this root is mounted on
		 */
		if (tdvp_unsafe == NULL || fdvp_unsafe == tdvp_unsafe) {
			lock_first  = fdvp_unsafe;
			lock_second = NULL;
		} else if (fdvp_unsafe < tdvp_unsafe) {
			lock_first  = fdvp_unsafe;
			lock_second = tdvp_unsafe;
		} else {
			lock_first  = tdvp_unsafe;
			lock_second = fdvp_unsafe;
		}
		if ( (_err = lock_fsnode(lock_first, &funnel_state)) )
			return (_err);

		if (lock_second != NULL && (_err = lock_fsnode(lock_second, NULL))) {
			unlock_fsnode(lock_first, &funnel_state);
			return (_err);
		}

		/*
		 * Lock both children in vnode address order to avoid deadlocks
		 */
		if (tvp == NULL || tvp == fvp) {
			lock_first  = fvp;
			lock_second = NULL;
		} else if (fvp < tvp) {
			lock_first  = fvp;
			lock_second = tvp;
		} else {
			lock_first  = tvp;
			lock_second = fvp;
		}
		if ( (_err = lock_fsnode(lock_first, NULL)) )
			goto out1;

		if (lock_second != NULL && (_err = lock_fsnode(lock_second, NULL))) {
		        unlock_fsnode(lock_first, NULL);
			goto out1;
		}
	}
#endif /* __LP64__ */
	
	/* 
	 * We need to preflight any potential AppleDouble file for the source file
	 * before doing the rename operation, since we could potentially be doing
	 * this operation on a network filesystem, and would end up duplicating
	 * the work.  Also, save the source and destination names.  Skip it if the
	 * source has a "._" prefix.
	 */
	
	if (!NATIVE_XATTR(fdvp) &&
	    !(fcnp->cn_nameptr[0] == '.' && fcnp->cn_nameptr[1] == '_')) {
		size_t len;
		int error;

		/* Get source attribute file name. */
		len = fcnp->cn_namelen + 3;
		if (len > sizeof(smallname1)) {
			MALLOC(xfromname, char *, len, M_TEMP, M_WAITOK);
		} else {
			xfromname = &smallname1[0];
		}
		strlcpy(xfromname, "._", min(sizeof smallname1, len));
		strncat(xfromname, fcnp->cn_nameptr, fcnp->cn_namelen);
		xfromname[len-1] = '\0';

		/* Get destination attribute file name. */
		len = tcnp->cn_namelen + 3;
		if (len > sizeof(smallname2)) {
			MALLOC(xtoname, char *, len, M_TEMP, M_WAITOK);
		} else {
			xtoname = &smallname2[0];
		}
		strlcpy(xtoname, "._", min(sizeof smallname2, len));
		strncat(xtoname, tcnp->cn_nameptr, tcnp->cn_namelen);
		xtoname[len-1] = '\0';
	
		/* 
		 * Look up source attribute file, keep reference on it if exists.
		 * Note that we do the namei with the nameiop of RENAME, which is different than
		 * in the rename syscall. It's OK if the source file does not exist, since this
		 * is only for AppleDouble files.
		 */
		if (xfromname != NULL) {
			NDINIT(&fromnd, RENAME, NOFOLLOW | USEDVP | CN_NBMOUNTLOOK, UIO_SYSSPACE,
					CAST_USER_ADDR_T(xfromname), ctx);
			fromnd.ni_dvp = fdvp;
			error = namei(&fromnd);
		
			/* 
			 * If there was an error looking up source attribute file, 
			 * we'll behave as if it didn't exist. 
			 */

			if (error == 0) {
				if (fromnd.ni_vp) {
					/* src_attr_vp indicates need to call vnode_put / nameidone later */
					src_attr_vp = fromnd.ni_vp;
					
					if (fromnd.ni_vp->v_type != VREG) {
						src_attr_vp = NULLVP;
						vnode_put(fromnd.ni_vp);
					}
				} 
				/*
				 * Either we got an invalid vnode type (not a regular file) or the namei lookup 
				 * suppressed ENOENT as a valid error since we're renaming. Either way, we don't 
				 * have a vnode here, so we drop our namei buffer for the source attribute file
				 */
				if (src_attr_vp == NULLVP) {
					nameidone(&fromnd);
				}
			}
		}
	}


	/* do the rename of the main file. */
	_err = (*fdvp->v_op[vnop_rename_desc.vdesc_offset])(&a);

#ifndef  __LP64__
	if (fdvp_unsafe != NULLVP) {
	        if (lock_second != NULL)
		        unlock_fsnode(lock_second, NULL);
		unlock_fsnode(lock_first, NULL);
	}
#endif /* __LP64__ */

	if (_err == 0) {
		if (tvp && tvp != fvp)
		        vnode_setneedinactive(tvp);
	}

	/* 
	 * Rename any associated extended attribute file (._ AppleDouble file).
	 */
	if (_err == 0 && !NATIVE_XATTR(fdvp) && xfromname != NULL) {
		int error = 0;
	
		/*
		 * Get destination attribute file vnode.
		 * Note that tdvp already has an iocount reference. Make sure to check that we
		 * get a valid vnode from namei.
		 */
		NDINIT(&tond, RENAME,
		       NOCACHE | NOFOLLOW | USEDVP | CN_NBMOUNTLOOK, UIO_SYSSPACE,
		       CAST_USER_ADDR_T(xtoname), ctx);
		tond.ni_dvp = tdvp;
		error = namei(&tond);

		if (error) 
			goto out;
		
		if (tond.ni_vp) {
			dst_attr_vp = tond.ni_vp;
		}
		
		if (src_attr_vp) {
			/* attempt to rename src -> dst */

			a.a_desc = &vnop_rename_desc;
			a.a_fdvp = fdvp;
			a.a_fvp = src_attr_vp;
			a.a_fcnp = &fromnd.ni_cnd;
			a.a_tdvp = tdvp;
			a.a_tvp = dst_attr_vp;
			a.a_tcnp = &tond.ni_cnd;
			a.a_context = ctx;

#ifndef __LP64__
			if (fdvp_unsafe != NULLVP) {
				/*
				 * Lock in vnode address order to avoid deadlocks
				 */
				if (dst_attr_vp == NULL || dst_attr_vp == src_attr_vp) {
					lock_first  = src_attr_vp;
					lock_second = NULL;
				} else if (src_attr_vp < dst_attr_vp) {
					lock_first  = src_attr_vp;
					lock_second = dst_attr_vp;
				} else {
					lock_first  = dst_attr_vp;
					lock_second = src_attr_vp;
				}
				if ( (error = lock_fsnode(lock_first, NULL)) == 0) {
					if (lock_second != NULL && (error = lock_fsnode(lock_second, NULL)) )
						unlock_fsnode(lock_first, NULL);
				}
			}
#endif /* __LP64__ */
			if (error == 0) {
				const char *oname;
				vnode_t oparent;

				/* Save these off so we can later verify them (fix up below) */
				oname   = src_attr_vp->v_name;
				oparent = src_attr_vp->v_parent;

				error = (*fdvp->v_op[vnop_rename_desc.vdesc_offset])(&a);

#ifndef __LP64__
				if (fdvp_unsafe != NULLVP) {
					if (lock_second != NULL)
						unlock_fsnode(lock_second, NULL);
					unlock_fsnode(lock_first, NULL);
				}
#endif /* __LP64__ */

				if (error == 0) {
					vnode_setneedinactive(src_attr_vp);

					if (dst_attr_vp && dst_attr_vp != src_attr_vp)
						vnode_setneedinactive(dst_attr_vp);
					/*
					 * Fix up name & parent pointers on ._ file
					 */
					if (oname == src_attr_vp->v_name &&
							oparent == src_attr_vp->v_parent) {
						int update_flags;

						update_flags = VNODE_UPDATE_NAME;

						if (fdvp != tdvp)
							update_flags |= VNODE_UPDATE_PARENT;

						vnode_update_identity(src_attr_vp, tdvp,
								tond.ni_cnd.cn_nameptr,
								tond.ni_cnd.cn_namelen,
								tond.ni_cnd.cn_hash,
								update_flags);
					}
				}
			}
			/* kevent notifications for moving resource files 
			 * _err is zero if we're here, so no need to notify directories, code
			 * below will do that.  only need to post the rename on the source and
			 * possibly a delete on the dest
			 */
			post_event_if_success(src_attr_vp, error, NOTE_RENAME);
			if (dst_attr_vp) {
				post_event_if_success(dst_attr_vp, error, NOTE_DELETE);	
			}

		} else if (dst_attr_vp) {
			/*
			 * Just delete destination attribute file vnode if it exists, since
			 * we didn't have a source attribute file.
			 * Note that tdvp already has an iocount reference.
			 */

			struct vnop_remove_args args;
			
			args.a_desc    = &vnop_remove_desc;
			args.a_dvp     = tdvp;
			args.a_vp      = dst_attr_vp;
			args.a_cnp     = &tond.ni_cnd;
			args.a_context = ctx;

#ifndef __LP64__
			if (fdvp_unsafe != NULLVP)
				error = lock_fsnode(dst_attr_vp, NULL);
#endif /* __LP64__ */
			if (error == 0) {
				error = (*tdvp->v_op[vnop_remove_desc.vdesc_offset])(&args);

#ifndef __LP64__
				if (fdvp_unsafe != NULLVP)
					unlock_fsnode(dst_attr_vp, NULL);
#endif /* __LP64__ */

				if (error == 0)
					vnode_setneedinactive(dst_attr_vp);
			}
			
			/* kevent notification for deleting the destination's attribute file
			 * if it existed.  Only need to post the delete on the destination, since
			 * the code below will handle the directories. 
			 */
			post_event_if_success(dst_attr_vp, error, NOTE_DELETE);	
		}
	}
out:
	if (src_attr_vp) {
		vnode_put(src_attr_vp);
		nameidone(&fromnd);
	}
	if (dst_attr_vp) {
		vnode_put(dst_attr_vp);
		nameidone(&tond);
	}

	if (xfromname && xfromname != &smallname1[0]) {
		FREE(xfromname, M_TEMP);
	}
	if (xtoname && xtoname != &smallname2[0]) {
		FREE(xtoname, M_TEMP);
	}

#ifndef __LP64__
out1:
	if (fdvp_unsafe != NULLVP) {
	        if (tdvp_unsafe != NULLVP)
		        unlock_fsnode(tdvp_unsafe, NULL);
		unlock_fsnode(fdvp_unsafe, &funnel_state);
	}
#endif /* __LP64__ */

	/* Wrote at least one directory.  If transplanted a dir, also changed link counts */
	if (0 == _err) {
		events = NOTE_WRITE;
		if (vnode_isdir(fvp)) {
			/* Link count on dir changed only if we are moving a dir and...
			 * 	--Moved to new dir, not overwriting there
			 * 	--Kept in same dir and DID overwrite
			 */
			if (((fdvp != tdvp) && (!tvp)) || ((fdvp == tdvp) && (tvp))) {
				events |= NOTE_LINK;
			}
		}

		lock_vnode_and_post(fdvp, events);
		if (fdvp != tdvp) {
			lock_vnode_and_post(tdvp,  events);
		}

		/* If you're replacing the target, post a deletion for it */
		if (tvp)
		{
			lock_vnode_and_post(tvp, NOTE_DELETE);
		}

		lock_vnode_and_post(fvp, NOTE_RENAME);
	}

	return (_err);
}

 #if 0
/*
 *#
 *#% mkdir        dvp     L U U
 *#% mkdir        vpp     - L -
 *#
 */
struct vnop_mkdir_args {
       struct vnodeop_desc *a_desc;
       vnode_t a_dvp;
       vnode_t *a_vpp;
       struct componentname *a_cnp;
       struct vnode_attr *a_vap;
       vfs_context_t a_context;
};
#endif /* 0*/
errno_t
VNOP_MKDIR(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp,
           struct vnode_attr *vap, vfs_context_t ctx)
{
       int _err;
       struct vnop_mkdir_args a;
#ifndef __LP64__
       int thread_safe;
       int funnel_state = 0;
#endif /* __LP64__ */

       a.a_desc = &vnop_mkdir_desc;
       a.a_dvp = dvp;
       a.a_vpp = vpp;
       a.a_cnp = cnp;
       a.a_vap = vap;
       a.a_context = ctx;

#ifndef __LP64__
       thread_safe = THREAD_SAFE_FS(dvp);
       if (!thread_safe) {
               if ( (_err = lock_fsnode(dvp, &funnel_state)) ) {
                       return (_err);
               }
       }
#endif /* __LP64__ */

       _err = (*dvp->v_op[vnop_mkdir_desc.vdesc_offset])(&a);
	if (_err == 0 && !NATIVE_XATTR(dvp)) {
		/* 
		 * Remove stale Apple Double file (if any).
		 */
		xattrfile_remove(dvp, cnp->cn_nameptr, ctx, 0);
	}

#ifndef __LP64__
       if (!thread_safe) {
               unlock_fsnode(dvp, &funnel_state);
       }
#endif /* __LP64__ */

       post_event_if_success(dvp, _err, NOTE_LINK | NOTE_WRITE);

       return (_err);
}


#if 0
/*
 *#
 *#% rmdir        dvp     L U U
 *#% rmdir        vp      L U U
 *#
 */
struct vnop_rmdir_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t a_vp;
	struct componentname *a_cnp;
	vfs_context_t a_context;
};

#endif /* 0*/
errno_t
VNOP_RMDIR(struct vnode *dvp, struct vnode *vp, struct componentname *cnp, vfs_context_t ctx)
{
	int _err;
	struct vnop_rmdir_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_rmdir_desc;
	a.a_dvp = dvp;
	a.a_vp = vp;
	a.a_cnp = cnp;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(dvp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_rmdir_desc.vdesc_offset])(&a);

	if (_err == 0) {
	        vnode_setneedinactive(vp);

		if ( !(NATIVE_XATTR(dvp)) ) {
		        /* 
			 * Remove any associated extended attribute file (._ AppleDouble file).
			 */
		        xattrfile_remove(dvp, cnp->cn_nameptr, ctx, 1);
		}
	}

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	/* If you delete a dir, it loses its "." reference --> NOTE_LINK */
	post_event_if_success(vp, _err, NOTE_DELETE | NOTE_LINK);
	post_event_if_success(dvp, _err, NOTE_LINK | NOTE_WRITE);

	return (_err);
}

/*
 * Remove a ._ AppleDouble file
 */
#define AD_STALE_SECS  (180)
static void
xattrfile_remove(vnode_t dvp, const char * basename, vfs_context_t ctx, int force) 
{
	vnode_t xvp;
	struct nameidata nd;
	char smallname[64];
	char *filename = NULL;
	size_t len;

	if ((basename == NULL) || (basename[0] == '\0') ||
	    (basename[0] == '.' && basename[1] == '_')) {
		return;
	}
	filename = &smallname[0];
	len = snprintf(filename, sizeof(smallname), "._%s", basename);
	if (len >= sizeof(smallname)) {
		len++;  /* snprintf result doesn't include '\0' */
		MALLOC(filename, char *, len, M_TEMP, M_WAITOK);
		len = snprintf(filename, len, "._%s", basename);
	}
	NDINIT(&nd, DELETE, WANTPARENT | LOCKLEAF | NOFOLLOW | USEDVP, UIO_SYSSPACE,
	       CAST_USER_ADDR_T(filename), ctx);
	nd.ni_dvp = dvp;
	if (namei(&nd) != 0)
		goto out2;

	xvp = nd.ni_vp;
	nameidone(&nd);
	if (xvp->v_type != VREG)
		goto out1;

	/*
	 * When creating a new object and a "._" file already
	 * exists, check to see if its a stale "._" file.
	 *
	 */
	if (!force) {
		struct vnode_attr va;

		VATTR_INIT(&va);
		VATTR_WANTED(&va, va_data_size);
		VATTR_WANTED(&va, va_modify_time);
		if (VNOP_GETATTR(xvp, &va, ctx) == 0  &&
		    VATTR_IS_SUPPORTED(&va, va_data_size)  &&
		    VATTR_IS_SUPPORTED(&va, va_modify_time)  &&
		    va.va_data_size != 0) {
			struct timeval tv;

			microtime(&tv);
			if ((tv.tv_sec > va.va_modify_time.tv_sec) &&
			    (tv.tv_sec - va.va_modify_time.tv_sec) > AD_STALE_SECS) {
				force = 1;  /* must be stale */
			}
		}
	}
	if (force) {
		struct vnop_remove_args a;
		int  error;
#ifndef __LP64__
		int thread_safe = THREAD_SAFE_FS(dvp);
#endif /* __LP64__ */
	
		a.a_desc    = &vnop_remove_desc;
		a.a_dvp     = nd.ni_dvp;
		a.a_vp      = xvp;
		a.a_cnp     = &nd.ni_cnd;
		a.a_context = ctx;

#ifndef __LP64__
		if (!thread_safe) {
			if ( (lock_fsnode(xvp, NULL)) )
				goto out1;
		}
#endif /* __LP64__ */

		error = (*dvp->v_op[vnop_remove_desc.vdesc_offset])(&a);

#ifndef __LP64__
		if (!thread_safe)
			unlock_fsnode(xvp, NULL);
#endif /* __LP64__ */

		if (error == 0)
			vnode_setneedinactive(xvp);

		post_event_if_success(xvp, error, NOTE_DELETE);
		post_event_if_success(dvp, error, NOTE_WRITE);
	}

out1:		
	vnode_put(dvp);
	vnode_put(xvp);
out2:
	if (filename && filename != &smallname[0]) {
		FREE(filename, M_TEMP);
	}
}

/*
 * Shadow uid/gid/mod to a ._ AppleDouble file
 */
static void
xattrfile_setattr(vnode_t dvp, const char * basename, struct vnode_attr * vap,
                  vfs_context_t ctx) 
{
	vnode_t xvp;
	struct nameidata nd;
	char smallname[64];
	char *filename = NULL;
	size_t len;

	if ((dvp == NULLVP) ||
	    (basename == NULL) || (basename[0] == '\0') ||
	    (basename[0] == '.' && basename[1] == '_')) {
		return;
	}
	filename = &smallname[0];
	len = snprintf(filename, sizeof(smallname), "._%s", basename);
	if (len >= sizeof(smallname)) {
		len++;  /* snprintf result doesn't include '\0' */
		MALLOC(filename, char *, len, M_TEMP, M_WAITOK);
		len = snprintf(filename, len, "._%s", basename);
	}
	NDINIT(&nd, LOOKUP, NOFOLLOW | USEDVP, UIO_SYSSPACE,
	       CAST_USER_ADDR_T(filename), ctx);
	nd.ni_dvp = dvp;
	if (namei(&nd) != 0)
		goto out2;

	xvp = nd.ni_vp;
	nameidone(&nd);

	if (xvp->v_type == VREG) {
#ifndef __LP64__
		int thread_safe = THREAD_SAFE_FS(dvp);
#endif /* __LP64__ */
		struct vnop_setattr_args a;

		a.a_desc = &vnop_setattr_desc;
		a.a_vp = xvp;
		a.a_vap = vap;
		a.a_context = ctx;

#ifndef __LP64__
		if (!thread_safe) {
			if ( (lock_fsnode(xvp, NULL)) )
				goto out1;
		}
#endif /* __LP64__ */

		(void) (*xvp->v_op[vnop_setattr_desc.vdesc_offset])(&a);

#ifndef __LP64__
		if (!thread_safe) {
			unlock_fsnode(xvp, NULL);
		}
#endif /* __LP64__ */
	}


#ifndef __LP64__
out1:		
#endif /* __LP64__ */
	vnode_put(xvp);

out2:
	if (filename && filename != &smallname[0]) {
		FREE(filename, M_TEMP);
	}
}

 #if 0
/*
 *#
 *#% symlink      dvp     L U U
 *#% symlink      vpp     - U -
 *#
 */
struct vnop_symlink_args {
       struct vnodeop_desc *a_desc;
       vnode_t a_dvp;
       vnode_t *a_vpp;
       struct componentname *a_cnp;
       struct vnode_attr *a_vap;
       char *a_target;
       vfs_context_t a_context;
};

#endif /* 0*/
errno_t
VNOP_SYMLINK(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp,
             struct vnode_attr *vap, char *target, vfs_context_t ctx)
{
       int _err;
       struct vnop_symlink_args a;
#ifndef __LP64__
       int thread_safe;
       int funnel_state = 0;
#endif /* __LP64__ */

       a.a_desc = &vnop_symlink_desc;
       a.a_dvp = dvp;
       a.a_vpp = vpp;
       a.a_cnp = cnp;
       a.a_vap = vap;
       a.a_target = target;
       a.a_context = ctx;

#ifndef __LP64__
       thread_safe = THREAD_SAFE_FS(dvp);
       if (!thread_safe) {
               if ( (_err = lock_fsnode(dvp, &funnel_state)) ) {
                       return (_err);
               }
       }
#endif /* __LP64__ */

       _err = (*dvp->v_op[vnop_symlink_desc.vdesc_offset])(&a);   
	if (_err == 0 && !NATIVE_XATTR(dvp)) {
		/* 
		 * Remove stale Apple Double file (if any).  Posts its own knotes
		 */
		xattrfile_remove(dvp, cnp->cn_nameptr, ctx, 0);
	}


#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(dvp, &funnel_state);
	}
#endif /* __LP64__ */
	
	post_event_if_success(dvp, _err, NOTE_WRITE);

	return (_err);
}

#if 0
/*
 *#
 *#% readdir      vp      L L L
 *#
 */
struct vnop_readdir_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct uio *a_uio;
	int a_flags;
	int *a_eofflag;
	int *a_numdirent;
	vfs_context_t a_context;
};

#endif /* 0*/
errno_t 
VNOP_READDIR(struct vnode *vp, struct uio *uio, int flags, int *eofflag,
             int *numdirent, vfs_context_t ctx)
{
	int _err;
	struct vnop_readdir_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_readdir_desc;
	a.a_vp = vp;
	a.a_uio = uio;
	a.a_flags = flags;
	a.a_eofflag = eofflag;
	a.a_numdirent = numdirent;
	a.a_context = ctx;
#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);

	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_readdir_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */
	return (_err);
}

#if 0
/*
 *#
 *#% readdirattr  vp      L L L
 *#
 */
struct vnop_readdirattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct attrlist *a_alist;
	struct uio *a_uio;
	uint32_t a_maxcount;
	uint32_t a_options;
	uint32_t *a_newstate;
	int *a_eofflag;
	uint32_t *a_actualcount;
	vfs_context_t a_context;
};

#endif /* 0*/
errno_t 
VNOP_READDIRATTR(struct vnode *vp, struct attrlist *alist, struct uio *uio, uint32_t maxcount,
                 uint32_t options, uint32_t *newstate, int *eofflag, uint32_t *actualcount, vfs_context_t ctx)
{
	int _err;
	struct vnop_readdirattr_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_readdirattr_desc;
	a.a_vp = vp;
	a.a_alist = alist;
	a.a_uio = uio;
	a.a_maxcount = maxcount;
	a.a_options = options;
	a.a_newstate = newstate;
	a.a_eofflag = eofflag;
	a.a_actualcount = actualcount;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_readdirattr_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% readlink     vp      L L L
 *#
 */
struct vnop_readlink_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct uio *a_uio;
	vfs_context_t a_context;
};
#endif /* 0 */

/*
 * Returns:	0			Success
 *		lock_fsnode:ENOENT	No such file or directory [only for VFS
 *					 that is not thread safe & vnode is
 *					 currently being/has been terminated]
 *		<vfs_readlink>:EINVAL
 *		<vfs_readlink>:???
 *
 * Note:	The return codes from the underlying VFS's readlink routine
 *		can't be fully enumerated here, since third party VFS authors
 *		may not limit their error returns to the ones documented here,
 *		even though this may result in some programs functioning
 *		incorrectly.
 *
 *		The return codes documented above are those which may currently
 *		be returned by HFS from hfs_vnop_readlink, not including
 *		additional error code which may be propagated from underlying
 *		routines.
 */
errno_t 
VNOP_READLINK(struct vnode *vp, struct uio *uio, vfs_context_t ctx)
{
	int _err;
	struct vnop_readlink_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_readlink_desc;
	a.a_vp = vp;
	a.a_uio = uio;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_readlink_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% inactive     vp      L U U
 *#
 */
struct vnop_inactive_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_INACTIVE(struct vnode *vp, vfs_context_t ctx)
{
	int _err;
	struct vnop_inactive_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_inactive_desc;
	a.a_vp = vp;
	a.a_context = ctx;
	
#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_inactive_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

#if NAMEDSTREAMS
	/* For file systems that do not support namedstream natively, mark 
	 * the shadow stream file vnode to be recycled as soon as the last 
	 * reference goes away.  To avoid re-entering reclaim code, do not 
	 * call recycle on terminating namedstream vnodes.
	 */
	if (vnode_isnamedstream(vp) &&
	    (vp->v_parent != NULLVP) &&
	    vnode_isshadow(vp) &&
	    ((vp->v_lflag & VL_TERMINATE) == 0)) {
		vnode_recycle(vp);
	}
#endif

	return (_err);
}


#if 0
/*
 *#
 *#% reclaim      vp      U U U
 *#
 */
struct vnop_reclaim_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t
VNOP_RECLAIM(struct vnode *vp, vfs_context_t ctx)
{
	int _err;
	struct vnop_reclaim_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_reclaim_desc;
	a.a_vp = vp;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_reclaim_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}


/*
 * Returns:	0			Success
 *	lock_fsnode:ENOENT		No such file or directory [only for VFS
 *					 that is not thread safe & vnode is
 *					 currently being/has been terminated]
 *	<vnop_pathconf_desc>:???	[per FS implementation specific]
 */
#if 0
/*
 *#
 *#% pathconf     vp      L L L
 *#
 */
struct vnop_pathconf_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_name;
	int32_t *a_retval;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_PATHCONF(struct vnode *vp, int name, int32_t *retval, vfs_context_t ctx)
{
	int _err;
	struct vnop_pathconf_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_pathconf_desc;
	a.a_vp = vp;
	a.a_name = name;
	a.a_retval = retval;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_pathconf_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

/*
 * Returns:	0			Success
 *	err_advlock:ENOTSUP
 *	lf_advlock:???
 *	<vnop_advlock_desc>:???
 *
 * Notes:	VFS implementations of advisory locking using calls through
 *		<vnop_advlock_desc> because lock enforcement does not occur
 *		locally should try to limit themselves to the return codes
 *		documented above for lf_advlock and err_advlock.
 */
#if 0
/*
 *#
 *#% advlock      vp      U U U
 *#
 */
struct vnop_advlock_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	caddr_t a_id;
	int a_op;
	struct flock *a_fl;
	int a_flags;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_ADVLOCK(struct vnode *vp, caddr_t id, int op, struct flock *fl, int flags, vfs_context_t ctx)
{
	int _err;
	struct vnop_advlock_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_advlock_desc;
	a.a_vp = vp;
	a.a_id = id;
	a.a_op = op;
	a.a_fl = fl;
	a.a_flags = flags;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	/* Disallow advisory locking on non-seekable vnodes */
	if (vnode_isfifo(vp)) {
		_err = err_advlock(&a);
	} else {
		if ((vp->v_flag & VLOCKLOCAL)) {
			/* Advisory locking done at this layer */
			_err = lf_advlock(&a);
		} else {
			/* Advisory locking done by underlying filesystem */
			_err = (*vp->v_op[vnop_advlock_desc.vdesc_offset])(&a);
		}
	}

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}



#if 0
/*
 *#
 *#% allocate     vp      L L L
 *#
 */
struct vnop_allocate_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	off_t a_length;
	u_int32_t a_flags;
	off_t *a_bytesallocated;
	off_t a_offset;
	vfs_context_t a_context;
};

#endif /* 0*/
errno_t 
VNOP_ALLOCATE(struct vnode *vp, off_t length, u_int32_t flags, off_t *bytesallocated, off_t offset, vfs_context_t ctx)
{
	int _err;
	struct vnop_allocate_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_allocate_desc;
	a.a_vp = vp;
	a.a_length = length;
	a.a_flags = flags;
	a.a_bytesallocated = bytesallocated;
	a.a_offset = offset;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_allocate_desc.vdesc_offset])(&a);
#if CONFIG_FSE
	if (_err == 0) {
		add_fsevent(FSE_STAT_CHANGED, ctx, FSE_ARG_VNODE, vp, FSE_ARG_DONE);
	}
#endif

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% pagein       vp      = = =
 *#
 */
struct vnop_pagein_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	upl_t a_pl;
	upl_offset_t a_pl_offset;
	off_t a_f_offset;
	size_t a_size;
	int a_flags;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_PAGEIN(struct vnode *vp, upl_t pl, upl_offset_t pl_offset, off_t f_offset, size_t size, int flags, vfs_context_t ctx)
{
	int _err;
	struct vnop_pagein_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_pagein_desc;
	a.a_vp = vp;
	a.a_pl = pl;
	a.a_pl_offset = pl_offset;
	a.a_f_offset = f_offset;
	a.a_size = size;
	a.a_flags = flags;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_pagein_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */
	
	return (_err);
}

#if 0
/*
 *#
 *#% pageout      vp      = = =
 *#
 */
struct vnop_pageout_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	upl_t a_pl;
	upl_offset_t a_pl_offset;
	off_t a_f_offset;
	size_t a_size;
	int a_flags;
	vfs_context_t a_context;
};

#endif /* 0*/
errno_t 
VNOP_PAGEOUT(struct vnode *vp, upl_t pl, upl_offset_t pl_offset, off_t f_offset, size_t size, int flags, vfs_context_t ctx)
{
	int _err;
	struct vnop_pageout_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_pageout_desc;
	a.a_vp = vp;
	a.a_pl = pl;
	a.a_pl_offset = pl_offset;
	a.a_f_offset = f_offset;
	a.a_size = size;
	a.a_flags = flags;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_pageout_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	post_event_if_success(vp, _err, NOTE_WRITE);

	return (_err);
}


#if 0
/*
 *#
 *#% searchfs     vp      L L L
 *#
 */
struct vnop_searchfs_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	void *a_searchparams1;
	void *a_searchparams2;
	struct attrlist *a_searchattrs;
	uint32_t a_maxmatches;
	struct timeval *a_timelimit;
	struct attrlist *a_returnattrs;
	uint32_t *a_nummatches;
	uint32_t a_scriptcode;
	uint32_t a_options;
	struct uio *a_uio;
	struct searchstate *a_searchstate;
	vfs_context_t a_context;
};

#endif /* 0*/
errno_t 
VNOP_SEARCHFS(struct vnode *vp, void *searchparams1, void *searchparams2, struct attrlist *searchattrs, uint32_t maxmatches, struct timeval *timelimit, struct attrlist *returnattrs, uint32_t *nummatches, uint32_t scriptcode, uint32_t options, struct uio *uio, struct searchstate *searchstate, vfs_context_t ctx)
{
	int _err;
	struct vnop_searchfs_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_searchfs_desc;
	a.a_vp = vp;
	a.a_searchparams1 = searchparams1;
	a.a_searchparams2 = searchparams2;
	a.a_searchattrs = searchattrs;
	a.a_maxmatches = maxmatches;
	a.a_timelimit = timelimit;
	a.a_returnattrs = returnattrs;
	a.a_nummatches = nummatches;
	a.a_scriptcode = scriptcode;
	a.a_options = options;
	a.a_uio = uio;
	a.a_searchstate = searchstate;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_searchfs_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% copyfile fvp U U U
 *#% copyfile tdvp L U U
 *#% copyfile tvp X U U
 *#
 */
struct vnop_copyfile_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_fvp;
	vnode_t a_tdvp;
	vnode_t a_tvp;
	struct componentname *a_tcnp;
	int a_mode;
	int a_flags;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_COPYFILE(struct vnode *fvp, struct vnode *tdvp, struct vnode *tvp, struct componentname *tcnp,
              int mode, int flags, vfs_context_t ctx)
{
	int _err;
	struct vnop_copyfile_args a;
	a.a_desc = &vnop_copyfile_desc;
	a.a_fvp = fvp;
	a.a_tdvp = tdvp;
	a.a_tvp = tvp;
	a.a_tcnp = tcnp;
	a.a_mode = mode;
	a.a_flags = flags;
	a.a_context = ctx;
	_err = (*fvp->v_op[vnop_copyfile_desc.vdesc_offset])(&a);
	return (_err);
}


errno_t
VNOP_REMOVEXATTR(vnode_t vp, const char *name, int options, vfs_context_t ctx)
{
	struct vnop_removexattr_args a;
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_removexattr_desc;
	a.a_vp = vp;
	a.a_name = name;
	a.a_options = options;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (error = lock_fsnode(vp, &funnel_state)) ) {
			return (error);
		}
	}
#endif /* __LP64__ */

	error = (*vp->v_op[vnop_removexattr_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	post_event_if_success(vp, error, NOTE_ATTRIB);
	
	return (error);
}

errno_t
VNOP_LISTXATTR(vnode_t vp, uio_t uio, size_t *size, int options, vfs_context_t ctx)
{
	struct vnop_listxattr_args a;
	int error;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_listxattr_desc;
	a.a_vp = vp;
	a.a_uio = uio;
	a.a_size = size;
	a.a_options = options;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (error = lock_fsnode(vp, &funnel_state)) ) {
			return (error);
		}
	}
#endif /* __LP64__ */

	error = (*vp->v_op[vnop_listxattr_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return (error);
}


#if 0
/*
 *#
 *#% blktooff vp = = =
 *#
 */
struct vnop_blktooff_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	daddr64_t a_lblkno;
	off_t *a_offset;
};
#endif /* 0*/
errno_t 
VNOP_BLKTOOFF(struct vnode *vp, daddr64_t lblkno, off_t *offset)
{
	int _err;
	struct vnop_blktooff_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_blktooff_desc;
	a.a_vp = vp;
	a.a_lblkno = lblkno;
	a.a_offset = offset;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_blktooff_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% offtoblk vp = = =
 *#
 */
struct vnop_offtoblk_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	off_t a_offset;
	daddr64_t *a_lblkno;
};
#endif /* 0*/
errno_t 
VNOP_OFFTOBLK(struct vnode *vp, off_t offset, daddr64_t *lblkno)
{
	int _err;
	struct vnop_offtoblk_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = &vnop_offtoblk_desc;
	a.a_vp = vp;
	a.a_offset = offset;
	a.a_lblkno = lblkno;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_offtoblk_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
/*
 *#
 *#% blockmap vp L L L
 *#
 */
struct vnop_blockmap_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	off_t a_foffset;
	size_t a_size;
	daddr64_t *a_bpn;
	size_t *a_run;
	void *a_poff;
	int a_flags;
	vfs_context_t a_context;
};
#endif /* 0*/
errno_t 
VNOP_BLOCKMAP(struct vnode *vp, off_t foffset, size_t size, daddr64_t *bpn, size_t *run, void *poff, int flags, vfs_context_t ctx)
{
	int _err;
	struct vnop_blockmap_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	if (ctx == NULL) {
		ctx = vfs_context_current();
	}
	a.a_desc = &vnop_blockmap_desc;
	a.a_vp = vp;
	a.a_foffset = foffset;
	a.a_size = size;
	a.a_bpn = bpn;
	a.a_run = run;
	a.a_poff = poff;
	a.a_flags = flags;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_blockmap_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return (_err);
}

#if 0
struct vnop_kqfilt_add_args {
	struct vnodeop_desc *a_desc;
	struct vnode *a_vp;
	struct knote *a_kn;
	vfs_context_t a_context;
};
#endif
errno_t
VNOP_KQFILT_ADD(struct vnode *vp, struct knote *kn, vfs_context_t ctx)
{
	int _err;
	struct vnop_kqfilt_add_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = VDESC(vnop_kqfilt_add);
	a.a_vp = vp;
	a.a_kn = kn;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_kqfilt_add_desc.vdesc_offset])(&a);
	
#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return(_err);
}

#if 0
struct vnop_kqfilt_remove_args {
	struct vnodeop_desc *a_desc;
	struct vnode *a_vp;
	uintptr_t a_ident;
	vfs_context_t a_context;
};
#endif
errno_t
VNOP_KQFILT_REMOVE(struct vnode *vp, uintptr_t ident, vfs_context_t ctx)
{
	int _err;
	struct vnop_kqfilt_remove_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = VDESC(vnop_kqfilt_remove);
	a.a_vp = vp;
	a.a_ident = ident;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_kqfilt_remove_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return(_err);
}

errno_t
VNOP_MONITOR(vnode_t vp, uint32_t events, uint32_t flags, void *handle, vfs_context_t ctx)
{
	int _err;
	struct vnop_monitor_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = VDESC(vnop_monitor);
	a.a_vp = vp;
	a.a_events = events;
	a.a_flags = flags;
	a.a_handle = handle;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */
	
	_err = (*vp->v_op[vnop_monitor_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return(_err);
}

#if 0
struct vnop_setlabel_args {
	struct vnodeop_desc *a_desc;
	struct vnode *a_vp;
	struct label *a_vl;
	vfs_context_t a_context;
};
#endif
errno_t
VNOP_SETLABEL(struct vnode *vp, struct label *label, vfs_context_t ctx)
{
	int _err;
	struct vnop_setlabel_args a;
#ifndef __LP64__
	int thread_safe;
	int funnel_state = 0;
#endif /* __LP64__ */

	a.a_desc = VDESC(vnop_setlabel);
	a.a_vp = vp;
	a.a_vl = label;
	a.a_context = ctx;

#ifndef __LP64__
	thread_safe = THREAD_SAFE_FS(vp);
	if (!thread_safe) {
		if ( (_err = lock_fsnode(vp, &funnel_state)) ) {
			return (_err);
		}
	}
#endif /* __LP64__ */

	_err = (*vp->v_op[vnop_setlabel_desc.vdesc_offset])(&a);

#ifndef __LP64__
	if (!thread_safe) {
		unlock_fsnode(vp, &funnel_state);
	}
#endif /* __LP64__ */

	return(_err);
}


#if NAMEDSTREAMS
/*
 * Get a named streamed
 */
errno_t 
VNOP_GETNAMEDSTREAM(vnode_t vp, vnode_t *svpp, const char *name, enum nsoperation operation, int flags, vfs_context_t ctx)
{
	struct vnop_getnamedstream_args a;

#ifndef __LP64__
	if (!THREAD_SAFE_FS(vp))
		return (ENOTSUP);
#endif /* __LP64__ */

	a.a_desc = &vnop_getnamedstream_desc;
	a.a_vp = vp;
	a.a_svpp = svpp;
	a.a_name = name;
	a.a_operation = operation;
	a.a_flags = flags;
	a.a_context = ctx;

	return (*vp->v_op[vnop_getnamedstream_desc.vdesc_offset])(&a);
}

/*
 * Create a named streamed
 */
errno_t 
VNOP_MAKENAMEDSTREAM(vnode_t vp, vnode_t *svpp, const char *name, int flags, vfs_context_t ctx)
{
	struct vnop_makenamedstream_args a;

#ifndef __LP64__
	if (!THREAD_SAFE_FS(vp))
		return (ENOTSUP);
#endif /* __LP64__ */

	a.a_desc = &vnop_makenamedstream_desc;
	a.a_vp = vp;
	a.a_svpp = svpp;
	a.a_name = name;
	a.a_flags = flags;
	a.a_context = ctx;

	return (*vp->v_op[vnop_makenamedstream_desc.vdesc_offset])(&a);
}


/*
 * Remove a named streamed
 */
errno_t 
VNOP_REMOVENAMEDSTREAM(vnode_t vp, vnode_t svp, const char *name, int flags, vfs_context_t ctx)
{
	struct vnop_removenamedstream_args a;

#ifndef __LP64__
	if (!THREAD_SAFE_FS(vp))
		return (ENOTSUP);
#endif /* __LP64__ */

	a.a_desc = &vnop_removenamedstream_desc;
	a.a_vp = vp;
	a.a_svp = svp;
	a.a_name = name;
	a.a_flags = flags;
	a.a_context = ctx;

	return (*vp->v_op[vnop_removenamedstream_desc.vdesc_offset])(&a);
}
#endif

#ifndef __LP64__
int
lock_fsnode(vnode_t vp, int *funnel_state)
{
        if (funnel_state)
        *funnel_state = thread_funnel_set(kernel_flock, TRUE);

        if (vp->v_unsafefs) {
        if (vp->v_unsafefs->fsnodeowner == current_thread()) {
                vp->v_unsafefs->fsnode_count++;
        } else {
                lck_mtx_lock(&vp->v_unsafefs->fsnodelock);

            if (vp->v_lflag & (VL_TERMWANT | VL_TERMINATE | VL_DEAD)) {
                    lck_mtx_unlock(&vp->v_unsafefs->fsnodelock);

                if (funnel_state)
                        (void) thread_funnel_set(kernel_flock, *funnel_state);
                return (ENOENT);
            }
            vp->v_unsafefs->fsnodeowner = current_thread();
            vp->v_unsafefs->fsnode_count = 1;
        }
    }
    return (0);
}


void
unlock_fsnode(vnode_t vp, int *funnel_state)
{
        if (vp->v_unsafefs) {
        if (--vp->v_unsafefs->fsnode_count == 0) {
                vp->v_unsafefs->fsnodeowner = NULL;
            lck_mtx_unlock(&vp->v_unsafefs->fsnodelock);
        }
    }
    if (funnel_state)
            (void) thread_funnel_set(kernel_flock, *funnel_state);
}
#endif /* __LP64__ */
void
lock_vnode_and_post(vnode_t vp, int kevent_num)
{       
    /* Only take the lock if there's something there! */
    if (vp->v_knotes.slh_first != NULL) {
        vnode_lock(vp);
        KNOTE(&vp->v_knotes, kevent_num);
        vnode_unlock(vp);
    }   
} 
int
relookup(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp)
{
	struct vnode *dp = NULL;		/* the directory we are searching */
	int wantparent;			/* 1 => wantparent or lockparent flag */
	int rdonly;			/* lookup read-only flag bit */
	int error = 0;
#ifdef NAMEI_DIAGNOSTIC
	int i, newhash;			/* DEBUG: check name hash */
	char *cp;			/* DEBUG: check name ptr/len */
#endif
	vfs_context_t ctx = cnp->cn_context;;

	/*
	 * Setup: break out flag bits into variables.
	 */
	wantparent = cnp->cn_flags & (LOCKPARENT|WANTPARENT);
	rdonly = cnp->cn_flags & RDONLY;
	cnp->cn_flags &= ~ISSYMLINK;

	if (cnp->cn_flags & NOCACHE)
	        cnp->cn_flags &= ~MAKEENTRY;
	else
	        cnp->cn_flags |= MAKEENTRY;

	dp = dvp;

	/*
	 * Check for degenerate name (e.g. / or "")
	 * which is a way of talking about a directory,
	 * e.g. like "/." or ".".
	 */
	if (cnp->cn_nameptr[0] == '\0') {
		if (cnp->cn_nameiop != LOOKUP || wantparent) {
			error = EISDIR;
			goto bad;
		}
		if (dp->v_type != VDIR) {
			error = ENOTDIR;
			goto bad;
		}
		if ( (vnode_get(dp)) ) {
		        error = ENOENT;
			goto bad;
		}
		*vpp = dp;

		if (cnp->cn_flags & SAVESTART)
			panic("lookup: SAVESTART");
		return (0);
	}
	/*
	 * We now have a segment name to search for, and a directory to search.
	 */
	if ( (error = VNOP_LOOKUP(dp, vpp, cnp, ctx)) ) {
		if (error != EJUSTRETURN)
			goto bad;
#if DIAGNOSTIC
		if (*vpp != NULL)
			panic("leaf should be empty");
#endif
		/*
		 * If creating and at end of pathname, then can consider
		 * allowing file to be created.
		 */
		if (rdonly) {
			error = EROFS;
			goto bad;
		}
		/*
		 * We return with ni_vp NULL to indicate that the entry
		 * doesn't currently exist, leaving a pointer to the
		 * (possibly locked) directory inode in ndp->ni_dvp.
		 */
		return (0);
	}
	dp = *vpp;

#if DIAGNOSTIC
	/*
	 * Check for symbolic link
	 */
	if (dp->v_type == VLNK && (cnp->cn_flags & FOLLOW))
		panic ("relookup: symlink found.\n");
#endif

	/*
	 * Disallow directory write attempts on read-only file systems.
	 */
	if (rdonly &&
	    (cnp->cn_nameiop == DELETE || cnp->cn_nameiop == RENAME)) {
		error = EROFS;
		goto bad2;
	}
	/* ASSERT(dvp == ndp->ni_startdir) */
	
	return (0);

bad2:
	vnode_put(dp);
bad:	
	*vpp = NULL;

	return (error);
}

void
vnode_reclaim(struct vnode * vp)
{
	vnode_reclaim_internal(vp, 0, 0, 0);
}
void
vnode_lock(vnode_t vp)
{
	lck_mtx_lock(&vp->v_lock);
}

void
vnode_lock_spin(vnode_t vp)
{
	lck_mtx_lock_spin(&vp->v_lock);
}

void
vnode_unlock(vnode_t vp)
{
	lck_mtx_unlock(&vp->v_lock);
}

void
vnode_setneedinactive(vnode_t vp)
{
        cache_purge(vp);

        vnode_lock_spin(vp);
	vp->v_lflag |= VL_NEEDINACTIVE;
	vnode_unlock(vp);
}

void
vn_checkunionwait(vnode_t vp)
{
    vnode_lock_spin(vp);
    while ((vp->v_flag & VISUNION) == VISUNION)
        msleep((caddr_t)&vp->v_flag, &vp->v_lock, 0, 0, 0);
    vnode_unlock(vp);
}

void
vfs_unbusy(mount_t mp)
{
    lck_rw_done(&mp->mnt_rwlock);
}
int         
vnode_isdir(vnode_t vp)
{           
    return ((vp->v_type == VDIR)? 1 : 0);
}   

