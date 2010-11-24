#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc_internal.h>
#include <sys/errno.h>
#include <sys/kauth.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vnode_internal.h>
#include <sys/mount_internal.h>
#include <sys/namei.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/vm.h>

#define THREAD_SAFE_FS(VP)  \
    ((VP)->v_unsafefs ? 0 : 1)

struct mount *dead_mountp = NULL; // this is wrong

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
		//funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_vget)(mp, ino, vpp, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		//(void) thread_funnel_set(kernel_flock, funnel_state);
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
		//funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_fhtovp)(mp, fhlen, fhp, vpp, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		//(void) thread_funnel_set(kernel_flock, funnel_state);
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
		//funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*vp->v_mount->mnt_op->vfs_vptofh)(vp, fhlenp, fhp, ctx);

#ifndef __LP64__
	if (!thread_safe) {
		//(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return(error);
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
		//funnel_state = thread_funnel_set(kernel_flock, TRUE);
	}
#endif /* __LP64__ */

	error = (*mp->mnt_op->vfs_getattr)(mp, vfa, ctx);
	
#ifndef __LP64__
	if (!thread_safe) {
		//(void) thread_funnel_set(kernel_flock, funnel_state);
	}
#endif /* __LP64__ */

	return(error);
}


int
vfs_getattr(mount_t mp, struct vfs_attr *vfa, vfs_context_t ctx)
{
	int		error;

	if ((error = VFS_GETATTR(mp, vfa, ctx)) != 0)
		return(error);

	/*
 	 * If we have a filesystem create time, use it to default some others.
 	 */
 	if (VFSATTR_IS_SUPPORTED(vfa, f_create_time)) {
 		if (VFSATTR_IS_ACTIVE(vfa, f_modify_time) && !VFSATTR_IS_SUPPORTED(vfa, f_modify_time))
 			VFSATTR_RETURN(vfa, f_modify_time, vfa->f_create_time);
 	}

	return(0);
}
