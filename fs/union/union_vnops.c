// vim: noexpandtab
/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1992, 1993, 1994, 1995 Jan-Simon Pendry.
 * Copyright (c) 1992, 1993, 1994, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry.
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
 *	@(#)union_vnops.c	8.32 (Berkeley) 6/23/95
 */

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
#include <vfs/vfs_support.h>
#include <sys/ubc.h>
#include <sys/kdebug.h>
#include <sys/uio_internal.h>

static int stub(const char *name, vnode_t vp) {
	char buf[512] = "[???]";
	int len = 511;
	vn_getpath(vp, buf, &len);
	buf[len] = 0;
	printf("Stub for %s: %s\n", name, buf);
	return EISDIR;
}
#define STUB(func, typ, field) static int func(typ ap) { return stub(#func, ap->field); }

__attribute__((const))
static char is_union(vnode_t vp) {
	return vp != NULLVP && vp->v_op == union_vnodeop_p;
}

// well, these are almost the same as UPPERVP, LOWERVP, OTHERVP :p

__attribute__((const))
static inline vnode_t upper(vnode_t vp) {
	if(!is_union(vp)) return vp;
	return VTOUNION(vp)->un_uppervp;
}

__attribute__((const))
static inline vnode_t lower(vnode_t vp) {
	if(!is_union(vp)) return vp;
	return VTOUNION(vp)->un_lowervp;
}

__attribute__((const))
static inline vnode_t upper_or_lower(vnode_t vp) {
	if(!is_union(vp)) return vp;
	struct union_node *un = VTOUNION(vp);
	return un->un_uppervp == NULLVP ? un->un_lowervp : un->un_uppervp;
}

/* called with no union lock held */
static int
union_lookup1(struct vnode *udvp, struct vnode **dvpp, struct vnode **vpp,
	struct componentname *cnp)
{
	int error;
	vfs_context_t ctx = cnp->cn_context;
	struct vnode *tdvp;
	struct vnode *dvp;
	struct mount *mp;

	dvp = *dvpp;

	/*
	 * If stepping up the directory tree, check for going
	 * back across the mount point, in which case do what
	 * lookup would do by stepping back down the mount
	 * hierarchy.
	 */
	if (cnp->cn_flags & ISDOTDOT) {
		while ((dvp != udvp) && (dvp->v_flag & VROOT)) {
			/*
			 * Don't do the NOCROSSMOUNT check
			 * at this level.  By definition,
			 * union fs deals with namespaces, not
			 * filesystems.
			 */
			tdvp = dvp;
			*dvpp = dvp = dvp->v_mount->mnt_vnodecovered;
			vnode_put(tdvp);
			vnode_get(dvp);
		}
	}

	error = VNOP_LOOKUP(dvp, &tdvp, cnp, ctx);
	if (error)
		return (error);

	dvp = tdvp;
	/*
	 * Lastly check if the current node is a mount point in
	 * which case walk up the mount hierarchy making sure not to
	 * bump into the root of the mount tree (ie. dvp != udvp).
	 */
	while (dvp != udvp && (dvp->v_type == VDIR) &&
	       (mp = dvp->v_mountedhere)) {
		if (vfs_busy(mp, LK_NOWAIT)) {
			vnode_put(dvp);
			return(ENOENT);
		}
		error = VFS_ROOT(mp, &tdvp, ctx);
		vfs_unbusy(mp);
		if (error) {
			vnode_put(dvp);
			return (error);
		}

		vnode_put(dvp);
		dvp = tdvp;
	}

	*vpp = dvp;
	return (0);
}

#define CHUD(thr) ((uint32_t *) ((char *) (thr) + (VERSION >= 0x040300 ? 0x478 : 0x444)))

int
union_lookup(struct vnop_lookup_args *ap)
/*
	struct vnop_lookup_args {
		struct vnodeop_desc *a_desc;
		struct vnode *a_dvp;
		struct vnode **a_vpp;
		struct componentname *a_cnp;
		vfs_context_t a_context;
	} *ap)
*/
{
	int error = 0;
	int uerror = ENOENT;
	int lerror = ENOENT;
	struct vnode *uppervp, *lowervp;
	struct vnode *upperdvp, *lowerdvp;
	struct vnode *dvp = ap->a_dvp;
	struct union_node *dun;
	struct componentname *cnp = ap->a_cnp;
	vfs_context_t ctx = cnp->cn_context;
	struct union_mount *um;
	kauth_cred_t saved_cred;
	struct vnode_attr va;
	int retry_count = 0;
	int old_flags = cnp->cn_flags;
	char mode = 2;
	uint32_t *chud = CHUD(current_thread());	

	/*uint32_t crap; chud = &crap;
	  if(cnp->cn_nameiop == CREATE || cnp->cn_nameiop == RENAME) return EROFS;*/

	*ap->a_vpp = NULLVP;
	
	cnp->cn_flags &= ~(LOCKPARENT | WANTPARENT);
	cnp->cn_flags |= WANTPARENT;

	union_lock();
	um = MOUNTTOUNIONMOUNT(dvp->v_mount);
	dun = VTOUNION(dvp);
	upperdvp = dun->un_uppervp;
	lowerdvp = dun->un_lowervp;
	uppervp = NULLVP;
	lowervp = NULLVP;
	//printf("in: %p RC=%d / %d\n", dun->un_uppervp, dun->un_uppervp ? dun->un_uppervp->v_iocount : -1, uppervp ? uppervp->v_iocount : -1);

	char slash = cnp->cn_nameptr[cnp->cn_namelen] != 0;

	if(DEBUG_PRINTF) {
		const char *nameiop = "?";
		switch(cnp->cn_nameiop) {
		#define A(x) case x: nameiop = #x; break;
		A(LOOKUP) A(CREATE) A(DELETE) A(RENAME)
		#undef A
		}
		printf("looking up %s(%d) lp=%d wp=%d nameiop=%s upperdvp=%p lowerdvp=%p slash=%d\n", cnp->cn_nameptr, (int) cnp->cn_namelen, old_flags & LOCKPARENT, old_flags & WANTPARENT, nameiop, upperdvp, lowerdvp, (int) slash);
	}


	union_unlock();
	if(upperdvp != NULLVP)
		vnode_get(upperdvp);
	if(lowerdvp != NULLVP)
		vnode_get(lowerdvp);

	/*
	 * do the lookup in the upper level.
	 * if that level comsumes additional pathnames,
	 * then assume that something special is going
	 * on and just return that vnode.
	 */
	if(um->um_uppervp != NULLVP && upperdvp != NULLVP) {
		uerror = union_lookup1(um->um_uppervp, &upperdvp,
					&uppervp, cnp);

		if(cnp->cn_consume != 0 || (!uerror && !vnode_isdir(uppervp))) {
			vnode_get(*ap->a_vpp = uppervp);
			mode = 3;
			error = uerror;
			goto out;
		}

		if(uerror && uerror != ENOENT && uerror != EJUSTRETURN) {
			error = uerror;
			goto out;
		}
	}

	// x: so the upper either does not exist or is a directory

	/*
	 * in a similar way to the upper layer, do the lookup
	 * in the lower layer.   this time, if there is some
	 * component magic going on, then vnode_put whatever we got
	 * back from the upper layer and return the lower vnode
	 * instead.
	 */

	if(um->um_lowervp != NULLVP && lowerdvp != NULLVP) {
		lerror = union_lookup1(um->um_lowervp, &lowerdvp,
				&lowervp, cnp);

		if(cnp->cn_consume != 0 || (!lerror && !vnode_isdir(lowervp))) {
			vnode_get(*ap->a_vpp = lowervp);
			mode = 4;
			error = lerror;
			goto out;
		}
		
		if(lerror && lerror != ENOENT && lerror != EJUSTRETURN) {
			error = lerror;
			goto out;
		}
	}

	/*
	 * at this point, we have uerror and lerror indicating
	 * possible errors with the lookups in the upper and lower
	 * layers.  additionally, uppervp and lowervp are (locked)
	 * references to existing vnodes in the upper and lower layers.
	 *
	 * there are now three cases to consider.
	 * 1. if both layers returned an error, then return whatever
	 *    error the upper layer generated.
	 *
	 * 2. if the top layer failed and the bottom layer succeeded
	 *    then two subcases occur.
	 *    a.  the bottom vnode is not a directory, in which
	 *	  case just return a new union vnode referencing
	 *	  an empty top layer and the existing bottom layer.
	 *    b.  the bottom vnode is a directory, in which case
	 *	  create a new directory in the top-level and
	 *	  continue as in case 3.
	 *
	 * 3. if the top layer succeeded then return a new union
	 *    vnode referencing whatever the new top layer and
	 *    whatever the bottom layer returned.
	 */
	
	printf("lookup (from %p <- %p <- %p): uerror=%d lerror=%d lowervp=%p upperdvp=%p uppervp=%p isdir=%d\n", __builtin_return_address(1), __builtin_return_address(2), __builtin_return_address(3), uerror, lerror, lowervp, upperdvp, uppervp, lowervp != NULLVP ? vnode_isdir(lowervp) : -1);

	/* case 1. */
	if ((uerror != 0) && (lerror != 0)) {
		if(lerror == EJUSTRETURN || uerror == EJUSTRETURN) {
			error = EJUSTRETURN;
		} else {
			error = uerror == ENOENT ? lerror : uerror;
		}
		goto out;
	}

	if(lowervp != NULLVP && upperdvp != NULLVP && uppervp == NULLVP && vnode_isdir(lowervp)) {
		
		struct vnode_attr va;
		VATTR_INIT(&va);
		VATTR_WANTED(&va, va_uid);
		VATTR_WANTED(&va, va_gid);
		//VATTR_WANTED(&va, va_create_time);
		VATTR_WANTED(&va, va_access_time);
		VATTR_WANTED(&va, va_modify_time);
		VATTR_WANTED(&va, va_change_time);
		//VATTR_WANTED(&va, va_backup_time);
		int serror = vnode_getattr(lowervp, &va, ctx);
		printf("serror from vnode_getattr: %d\n", serror);
		if(!serror) {
			serror = union_mkshadow(um, upperdvp, cnp, &va, &uppervp);
			printf("serror from union_mkshadow: %d\n", serror);
			if(serror && uppervp) {
				vnode_put(uppervp);
				uppervp = NULLVP;
			}
		}
	}

	printf("lookup: okay we are allocvp'ing upper=%p lower=%p dvp=%p %d\n", uppervp, lowervp, dvp, uppervp ? uppervp->v_iocount : -1);

	union_lock();
	error = union_allocvp(ap->a_vpp, dvp->v_mount, dvp, upperdvp ?: lowerdvp, cnp,
			      uppervp, lowervp, 0 /*1*/);
	union_unlock();

	printf("lookup: got past allocvp\n");

	goto already_put;

	out:
	if (uppervp != NULLVP)
		vnode_put(uppervp);
	if (lowervp != NULLVP)
		vnode_put(lowervp);

	already_put:
	if (upperdvp != NULLVP)
		vnode_put(upperdvp);
	if (lowerdvp != NULLVP)
		vnode_put(lowerdvp);
	// XXX lockparent?
			
	cnp->cn_flags = old_flags;
	
	// sigh
	if(!slash &&
	   ((error == EJUSTRETURN && cnp->cn_nameiop == RENAME) ?
		*chud :
		!((uppervp != NULLVP && vnode_isdir(uppervp)) || 
	  	  (lowervp != NULLVP && vnode_isdir(lowervp)))) &&
		(cnp->cn_nameiop == CREATE ||
		 cnp->cn_nameiop == RENAME) &&
		(error == 0 ||
		 error == EJUSTRETURN)) {
		printf("mega hack %p %p %p chud=%x\n", ap->a_vpp[1], upper(ap->a_vpp[1]), lower(ap->a_vpp[1]), *chud);
		vnode_t ovp = ap->a_vpp[1];
		if(ovp != NULLVP) {
			vnode_t newvp;
			if((cnp->cn_flags & ~(FSNODELOCKHELD | HASBUF | CN_VOLFSPATH | MAKEENTRY | ISSYMLINK | ISLASTCN)) == (LOCKPARENT | AUDITVNPATH2 | CN_NBMOUNTLOOK)) { // link()
				switch(*chud) {
					case 3: newvp = upper(ovp); break;
					case 4: newvp = lower(ovp); break;
					default: newvp = upper_or_lower(ovp); break;
				}
				if(newvp == NULLVP) goto nvm;
			} else {
				newvp = upper_or_lower(ovp);
			}
			if(newvp != ovp) {
				// did we lock it?
				char locked = (void *) ovp->v_lock.opaque[0] == current_thread();
				if(locked) vnode_unlock(ovp);
				vnode_get(ap->a_vpp[1] = newvp);
				vnode_put(ovp);
				if(locked) vnode_lock(newvp);
				if(cnp->cn_nameiop == RENAME && *ap->a_vpp != NULLVP && newvp->v_mount != (*ap->a_vpp)->v_mount) {
					vnode_put(*ap->a_vpp);
					*ap->a_vpp = NULLVP;
					if(error == 0) error = EJUSTRETURN;
				}
			}
		}
	}
		
	if(cnp->cn_nameiop == DELETE) {
		*chud = (*ap->a_vpp != NULLVP && !vnode_isdir(*ap->a_vpp));
	} else if(cnp->cn_nameiop == LOOKUP) {
		*chud = mode;
	}
	nvm:

	//printf("out: %p RC=%d / %d error=%d isdir=%d\n", dun->un_uppervp, dun->un_uppervp ? dun->un_uppervp->v_iocount : -1, uppervp ? uppervp->v_iocount : -1, error, *ap->a_vpp ? vnode_isdir(*ap->a_vpp) : -1);
	return (error);
}

int
union_create(struct vnop_create_args *ap)
/*
	struct vnop_create_args {
		struct vnode *a_dvp;
		struct vnode **a_vpp;
		struct componentname *a_cnp;
		struct vnode_attr *a_vap;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct union_node *un = VTOUNION(ap->a_dvp);
	struct vnode *dvp = upper_or_lower(ap->a_dvp);
	struct componentname *cnp = ap->a_cnp;

	printf("union_create %p %s\n", dvp, cnp->cn_nameptr);

	if (dvp != NULLVP) {
		int error;
		struct vnode *vp;
		struct mount *mp;


		mp = ap->a_dvp->v_mount;

		/* note that this is a direct passthrough to the filesystem */
		error = VNOP_CREATE(dvp, &vp, cnp, ap->a_vap, ap->a_context);
		if (error)
			return (error);

		/* if this is faulting filesystem and is a reg file, skip allocation of union node */
		if(vp != NULLVP && !vnode_isdir(vp)) {
			*ap->a_vpp = vp;
			return(0);
		}


		union_lock();
		error = union_allocvp(ap->a_vpp, mp, NULLVP, NULLVP, cnp, vp,
				NULLVP, 1);
		union_unlock();
		if (error)
			vnode_put(vp);
		return (error);
	}

	return (EROFS);
}

STUB(union_whiteout, struct vnop_whiteout_args *, a_dvp)

/* mknod can do  fifos, chr, blk or whiteout entries */
int
union_mknod(struct vnop_mknod_args *ap)
/*
	struct vnop_mknod_args {
		struct vnode *a_dvp;
		struct vnode **a_vpp;
		struct componentname *a_cnp;
		struct vnode_attr *a_vap;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct union_node *un = VTOUNION(ap->a_dvp);
	struct vnode *dvp = upper_or_lower(ap->a_dvp);
	struct componentname *cnp = ap->a_cnp;

	if (dvp != NULLVP) {
		int error;
		struct vnode *vp;
		struct mount *mp;


		mp = ap->a_dvp->v_mount;

		/* note that this is a direct passthrough to the filesystem */
		error = VNOP_MKNOD(dvp, &vp, cnp, ap->a_vap, ap->a_context);
		if (error)
			return (error);

		if (vp != NULLVP) {
			union_lock();
			error = union_allocvp(ap->a_vpp, mp, NULLVP, NULLVP,
					cnp, vp, NULLVP, 1);
			union_unlock();
			if (error)
				vnode_put(vp);
		}
		return (error);
	}
	return (EROFS);
}

int
union_open(struct vnop_open_args *ap)
{
	stub("union_open", ap->a_vp);
    struct union_node *un = VTOUNION(ap->a_vp);
	printf("upper=%p lower=%p\n", un->un_uppervp, un->un_lowervp);
	if(un->un_uppervp != NULLVP) return VNOP_OPEN(un->un_uppervp, ap->a_mode, ap->a_context);
	un->un_openl++;
	if(un->un_lowervp != NULLVP) return VNOP_OPEN(un->un_lowervp, ap->a_mode, ap->a_context);
	return EROFS;
}

int
union_close(struct vnop_close_args *ap)
/*
	struct vnop_close_args {
		struct vnode *a_vp;
		int  a_fflag;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct union_node *un = VTOUNION(ap->a_vp);
	struct vnode *vp;
	int error = 0;

	if ((vp = un->un_uppervp) == NULLVP) {
#ifdef UNION_DIAGNOSTIC
		if (un->un_openl <= 0)
			panic("union: un_openl cnt");
#endif
		--un->un_openl;
		vp = un->un_lowervp;
	}

	ap->a_vp = vp;
	error =  (VCALL(vp, VOFFSET(vnop_close), ap));
	return(error);
}

/*
 * Check access permission on the union vnode.
 * The access check being enforced is to check
 * against both the underlying vnode, and any
 * copied vnode.  This ensures that no additional
 * file permissions are given away simply because
 * the user caused an implicit file copy.
 */
int
union_access(struct vnop_access_args *ap)
/*
	struct vnop_access_args {
		struct vnodeop_desc *a_desc;
		struct vnode *a_vp;
		int a_action;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct union_node *un = VTOUNION(ap->a_vp);
	int error = EACCES;
	struct vnode *vp;

	if ((vp = un->un_uppervp) != NULLVP) {
		ap->a_vp = vp;
		return (VCALL(vp, VOFFSET(vnop_access), ap));
	}

	if ((vp = un->un_lowervp) != NULLVP) {
		ap->a_vp = vp;
		error = VCALL(vp, VOFFSET(vnop_access), ap);
		if (error == 0) {
			struct union_mount *um = MOUNTTOUNIONMOUNT(vp->v_mount);

			if (um->um_op == UNMNT_BELOW) {
				error = VCALL(vp, VOFFSET(vnop_access), ap);
			}
		}
		if (error)
			return (error);
	}

	return (error);
}

/*
 * We handle getattr only to change the fsid and
 * track object sizes
 */
int
union_getattr(struct vnop_getattr_args *ap)
/*
	struct vnop_getattr_args {
		struct vnode *a_vp;
		struct vnode_attr *a_vap;
		vfs_context_t a_context;
	} *ap;
*/
{
	int error=0;
	struct union_node *un = VTOUNION(ap->a_vp);
	struct vnode *vp = un->un_uppervp;
	struct vnode_attr *vap;
	struct vnode_attr va;


	/*
	 * Some programs walk the filesystem hierarchy by counting
	 * links to directories to avoid stat'ing all the time.
	 * This means the link count on directories needs to be "correct".
	 * The only way to do that is to call getattr on both layers
	 * and fix up the link count.  The link count will not necessarily
	 * be accurate but will be large enough to defeat the tree walkers.
	 */

	vap = ap->a_vap;

	vp = un->un_uppervp;
	if (vp != NULLVP) {
		/*
		 * It's not clear whether vnop_getattr is to be
		 * called with the vnode locked or not.  stat() calls
		 * it with (vp) locked, and fstat calls it with
		 * (vp) unlocked.
		 * In the mean time, compensate here by checking
		 * the union_node's lock flag.
		 */

		error = vnode_getattr(vp, vap, ap->a_context);
		if (error) {
			return (error);
		}
		union_lock();
		union_newsize(ap->a_vp, vap->va_data_size, VNOVAL);
		union_unlock();
	}

	if (vp == NULLVP) {
		vp = un->un_lowervp;
	} else if (vp->v_type == VDIR) {
		vp = un->un_lowervp;
		VATTR_INIT(&va);
		/* all we want from the lower node is the link count */
		VATTR_WANTED(&va, va_nlink);
		vap = &va;
	} else {
		vp = NULLVP;
	}

	if (vp != NULLVP) {
		error = vnode_getattr(vp, vap, ap->a_context);
		if (error) {
			return (error);
		}
		union_lock();
		union_newsize(ap->a_vp, VNOVAL, vap->va_data_size);
		union_unlock();
	}

	if ((vap != ap->a_vap) && (vap->va_type == VDIR))
		ap->a_vap->va_nlink += vap->va_nlink;

	VATTR_RETURN(ap->a_vap, va_fsid, ap->a_vp->v_mount->mnt_vfsstat.f_fsid.val[0]);
	return (0);
}

int
union_setattr(struct vnop_setattr_args *ap)
/*
	struct vnop_setattr_args {
		struct vnode *a_vp;
		struct vnode_attr *a_vap;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct union_node *un = VTOUNION(ap->a_vp);
	int error;

	/*
	 * Handle case of truncating lower object to zero size,
	 * by creating a zero length upper object.  This is to
	 * handle the case of open with O_TRUNC and O_CREAT.
	 */
	if (VATTR_IS_ACTIVE(ap->a_vap, va_data_size) &&
	    (un->un_uppervp == NULLVP) &&
	    /* assert(un->un_lowervp != NULLVP) */
	    (un->un_lowervp->v_type == VREG)) {
		union_lock();
		error = union_copyup(un, (ap->a_vap->va_data_size != 0), ap->a_context);
		union_unlock();
		if (error) {
			return (error);
		}
	}

	/*
	 * Try to set attributes in upper layer,
	 * otherwise return read-only filesystem error.
	 */
	if (un->un_uppervp != NULLVP) {
		error = vnode_setattr(un->un_uppervp, ap->a_vap, ap->a_context);
		if ((error == 0) && VATTR_IS_ACTIVE(ap->a_vap, va_data_size)) {
			union_lock();
			union_newsize(ap->a_vp, ap->a_vap->va_data_size, VNOVAL);
			union_unlock();
		}
	} else {
		error = EROFS;
	}

	return (error);
}

STUB(union_read, struct vnop_read_args *, a_vp)
STUB(union_write, struct vnop_read_args *, a_vp)

int
union_ioctl(struct vnop_ioctl_args *ap)
/*
	struct vnop_ioctl_args {
		struct vnode *a_vp;
		int  a_command;
		caddr_t  a_data;
		int  a_fflag;
		vfs_context_t a_context;
	} *ap;
*/
{
	register struct vnode *ovp = OTHERVP(ap->a_vp);

	ap->a_vp = ovp;
	return (VCALL(ovp, VOFFSET(vnop_ioctl), ap));
}

int
union_select(struct vnop_select_args *ap)
/*
	struct vnop_select_args {
		struct vnode *a_vp;
		int  a_which;
		int  a_fflags;
		void * a_wql;
		vfs_context_t a_context;
	} *ap;
*/
{
	register struct vnode *ovp = OTHERVP(ap->a_vp);

	ap->a_vp = ovp;
	return (VCALL(ovp, VOFFSET(vnop_select), ap));
}

int
union_revoke(struct vnop_revoke_args *ap)
/*
	struct vnop_revoke_args {
		struct vnode *a_vp;
		int a_flags;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct vnode *vp = ap->a_vp;

	if (UPPERVP(vp))
		VNOP_REVOKE(UPPERVP(vp), ap->a_flags, ap->a_context);
	if (LOWERVP(vp))
		VNOP_REVOKE(LOWERVP(vp), ap->a_flags, ap->a_context);
	vnode_reclaim(vp);

	return (0);
}

STUB(union_mmap, struct vnop_mmap_args *, a_vp)
STUB(union_mnomap, struct vnop_mnomap_args *, a_vp)

int
union_fsync(struct vnop_fsync_args *ap)
/*
	struct vnop_fsync_args {
		struct vnode *a_vp;
		int  a_waitfor;
		vfs_context_t a_context;
	} *ap;
*/
{
	// When this is called from vclean, it's not safe to call fsync because of hfs lock crap; but it's also not necessary, because upper will be synced when it itself is cleaned (and in the case of the mega hack, that won't happen because it is referenced)
	// This is stupid and I should just fix the original issue but...
	uint32_t crap = *((uint32_t *) ((uintptr_t)__builtin_return_address(1) & ~1));
	if(crap == 0x46282200) {
		//printf("not fsyncing sub\n");
		return 0;
	}

	int error = 0;
	struct vnode *targetvp = OTHERVP(ap->a_vp);

	if (targetvp != NULLVP) {

		error = VNOP_FSYNC(targetvp, ap->a_waitfor, ap->a_context);
	}

	return (error);
}

int
union_remove(struct vnop_remove_args *ap)
/*
	struct vnop_remove_args {
		struct vnode *a_dvp;
		struct vnode *a_vp;
		struct componentname *a_cnp;
		vfs_context_t a_context;
	} *ap;
*/
{
	int error, flags;
	vnode_t dvp = ap->a_dvp;
	vnode_t vp = ap->a_vp;

	printf("vps: %p, %p\n", dvp, vp);
	printf("upper:%p, %p\n", upper(dvp), upper(vp));
	printf("lower:%p, %p\n", lower(dvp), lower(vp));

	if(is_union(vp)) return EPWROFF;

	if(upper(dvp)->v_mount == vp->v_mount) {
		return VNOP_REMOVE(upper(dvp), vp, ap->a_cnp, ap->a_flags, ap->a_context);
	} else if(lower(dvp)->v_mount == vp->v_mount) {
		return VNOP_REMOVE(lower(dvp), vp, ap->a_cnp, ap->a_flags, ap->a_context);
	} else {
		return EROFS;
	}
}

int
union_link(struct vnop_link_args *ap)
/*
	struct vnop_link_args {
		struct vnode *a_vp;
		struct vnode *a_tdvp;
		struct componentname *a_cnp;
		vfs_context_t a_context;
	} *ap;
*/
{
	int error = 0;
	struct componentname *cnp = ap->a_cnp;
	struct union_node *un;
	struct vnode *vp;
	struct vnode *tdvp;


	if(upper(ap->a_tdvp) != NULLVP && (error = VNOP_LINK(upper(ap->a_vp), upper(ap->a_tdvp), ap->a_cnp, ap->a_context)) && error != EXDEV) return error;
	if(lower(ap->a_tdvp) != NULLVP && (error = VNOP_LINK(lower(ap->a_vp), lower(ap->a_tdvp), ap->a_cnp, ap->a_context)) && error != EXDEV) return error;
	return EROFS;
}

int
union_rename(struct vnop_rename_args *ap)
/*
	struct vnop_rename_args {
		struct vnode *a_fdvp;
		struct vnode *a_fvp;
		struct componentname *a_fcnp;
		struct vnode *a_tdvp;
		struct vnode *a_tvp;
		struct componentname *a_tcnp;
		vfs_context_t a_context;
	} *ap;
*/
{
	printf("union_rename in\n");
	// This is probably wrong in most cases
	int error;

	struct vnode *fdvp = ap->a_fdvp;
	struct vnode *fvp = ap->a_fvp;
	struct vnode *tdvp = ap->a_tdvp;
	struct vnode *tvp = ap->a_tvp;

	struct union_node *dun = VTOUNION(fdvp);

	int uerror = 0, lerror = 0;
	char ok = 0;
	char is_u = is_union(fvp);
	
	// this is generally wrong
	
	if(upper(fdvp) != NULLVP && upper(fvp) != NULLVP && upper(tdvp) != NULLVP && upper(fdvp)->v_mount == upper(tdvp)->v_mount) {
		ok = 1;
		uerror = VNOP_RENAME(upper(fdvp), upper(fvp), ap->a_fcnp, upper(tdvp), upper(tvp), ap->a_tcnp, ap->a_context);
		printf("uerror = %d\n", uerror);
		if(uerror && uerror != ENOENT) return uerror;
	}
	if((is_u || !ok || uerror) && lower(fdvp) != NULLVP && lower(fvp) != NULLVP && lower(tdvp) != NULLVP && lower(fdvp)->v_mount == lower(tdvp)->v_mount) {
		ok = 1;
		lerror = VNOP_RENAME(lower(fdvp), lower(fvp), ap->a_fcnp, lower(tdvp), lower(tvp), ap->a_tcnp, ap->a_context);
		printf("lerror = %d\n", lerror);
	}
	if(!ok) {
		return EROFS;
	}
	error = (uerror == ENOENT ? lerror : uerror);
	printf("union_rename out %d\n", error);
	return error;
}

int
union_mkdir(struct vnop_mkdir_args *ap)
/*
	struct vnop_mkdir_args {
		struct vnode *a_dvp;
		struct vnode **a_vpp;
		struct componentname *a_cnp;
		struct vnode_attr *a_vap;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct union_node *un = VTOUNION(ap->a_dvp);
	struct vnode *dvp = upper_or_lower(ap->a_dvp);
	struct componentname *cnp = ap->a_cnp;

	if (dvp != NULLVP) {
		int error;
		struct vnode *vp;


		/* note that this is a direct fallthrough to the filesystem */
		error = VNOP_MKDIR(dvp, &vp, cnp, ap->a_vap, ap->a_context);
		if (error)
			return (error);

		union_lock();
		error = union_allocvp(ap->a_vpp, ap->a_dvp->v_mount, ap->a_dvp,
				NULLVP, cnp, vp, NULLVP, 1);
		union_unlock();
		if (error)
			vnode_put(vp);
		return (error);
	}



	return (EROFS);
}

int
union_rmdir(struct vnop_rmdir_args *ap)
/*
	struct vnop_rmdir_args {
		struct vnode *a_dvp;
		struct vnode *a_vp;
		struct componentname *a_cnp;
		vfs_context_t a_context;
	} *ap;
*/
{
	// note that VNOP_RMDIR calls vp's implementation, not dvp's - so both are definitely union
	// also, this means we must return union nodes for directories, because hfs_vnop_rmdir assumes that dvp is also HFS
	// I fail at VFS.
	int error, flags;
	vnode_t dvp = ap->a_dvp;
	struct union_node *un = VTOUNION(ap->a_vp);

	int uerror = 0, lerror = 0;
	char ok = 0;
	if(lower(dvp) != NULLVP && un->un_lowervp != NULLVP) {
		ok = 1;
		lerror = VNOP_RMDIR(lower(dvp), un->un_lowervp, ap->a_cnp, ap->a_context);
		if(lerror && lerror != ENOENT) return lerror;
	}
	if(upper(dvp) != NULLVP && un->un_uppervp != NULLVP) {
		ok = 1;
		uerror = VNOP_RMDIR(upper(dvp), un->un_uppervp, ap->a_cnp, ap->a_context);
	}
	if(!ok) {
		return EROFS;
	}
	if(uerror && !lerror) {
		union_lock();
		union_removed_lower(un);
		union_unlock();
	} else if(lerror && !uerror) {
		union_lock();
		union_removed_upper(un);
		union_unlock();
	}
	return uerror == ENOENT ? lerror : uerror;
}

int
union_symlink(struct vnop_symlink_args *ap)
/*
	struct vnop_symlink_args {
		struct vnode *a_dvp;
		struct vnode **a_vpp;
		struct componentname *a_cnp;
		struct vnode_attr *a_vap;
		char *a_target;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct union_node *un = VTOUNION(ap->a_dvp);
	struct vnode *dvp = un->un_uppervp;
	struct componentname *cnp = ap->a_cnp;

	if (dvp != NULLVP) {
		int error;
		struct vnode *vp;

		error = VNOP_SYMLINK(dvp, &vp, cnp, ap->a_vap, ap->a_target, ap->a_context);
		*ap->a_vpp = vp;
		return (error);
	}
	return (EROFS);
}

/*
 * union_readdir works in concert with getdirentries and
 * readdir(3) to provide a list of entries in the unioned
 * directories.  getdirentries is responsible for walking
 * down the union stack.  readdir(3) is responsible for
 * eliminating duplicate names from the returned data stream.
 */
int
union_readdir(struct vnop_readdir_args *ap)
/*
	struct vnop_readdir_args {
		struct vnodeop_desc *a_desc;
		struct vnode *a_vp;
		struct uio *a_uio;
		int a_flags;
		int *a_eofflag;
		int *a_numdirent;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct union_node *un = VTOUNION(ap->a_vp);
	struct vnode *uvp = un->un_uppervp;
	struct vnode *lvp = un->un_lowervp;
	stub("union_readdir", ap->a_vp);
	printf("upper:%p lower:%p\n", uvp, lvp);

	if (ap->a_flags & (VNODE_READDIR_EXTENDED | VNODE_READDIR_REQSEEKOFF))
		return (EINVAL);
	
	int result = EPWROFF;

	if (uvp != NULLVP) {
		ap->a_vp = uvp;
		result = (VCALL(uvp, VOFFSET(vnop_readdir), ap));
	} else if(lvp != NULLVP) {
		ap->a_vp = lvp;
		result = VCALL(lvp, VOFFSET(vnop_readdir), ap);
	}

	printf("readdir out: eofflag:%d numdirent:%d\n", *ap->a_eofflag, *ap->a_numdirent);

	return result;
}

STUB(union_readlink, struct vnop_readlink_args *, a_vp)

int
union_inactive(struct vnop_inactive_args *ap)
/*
	struct vnop_inactive_args {
		struct vnode *a_vp;
		vfs_context_t a_context;
	} *ap;
*/
{
	struct vnode *vp = ap->a_vp;
	struct union_node *un = VTOUNION(vp);

	/*
	 * Do nothing (and _don't_ bypass).
	 * Wait to vnode_put lowervp until reclaim,
	 * so that until then our union_node is in the
	 * cache and reusable.
	 *
	 * NEEDSWORK: Someday, consider inactive'ing
	 * the lowervp and then trying to reactivate it
	 * with capabilities (v_id)
	 * like they do in the name lookup cache code.
	 * That's too much work for now.
	 */

	union_lock();
	if (un->un_flags & UN_DELETED) {
		if(un->un_uppervp != NULLVP) {
			vnode_rele(un->un_uppervp);
		}
		union_removed_upper(un);
	}

	if (un->un_dircache != 0)  {
			union_dircache_free(un);
	}
	if (un->un_flags & UN_DIRENVN) {
		vnode_recycle(vp);
	}

	union_unlock();

	return (0);
}

int
union_reclaim(struct vnop_reclaim_args *ap)
/*
	struct vnop_reclaim_args {
		struct vnode *a_vp;
		vfs_context_t a_context;
	} *ap;
*/
{

	union_lock();
	union_freevp(ap->a_vp);
	union_unlock();

	return (0);
}

int
union_blockmap(struct vnop_blockmap_args *ap)
/*
	struct vnop_blockmap_args {
		struct vnode *a_vp;
		off_t a_offset;    
		size_t a_size;
		daddr64_t *a_bpn;
		size_t *a_run;
		void *a_poff;
		int a_flags;
	} *ap;
*/
{
	int error;
	struct vnode *vp = OTHERVP(ap->a_vp);

	ap->a_vp = vp;
	error = VCALL(vp, VOFFSET(vnop_blockmap), ap);

	return (error);
}

int
union_pathconf(struct vnop_pathconf_args *ap)
/*
	struct vnop_pathconf_args {
		struct vnode *a_vp;
		int a_name;
		int *a_retval;
		vfs_context_t a_context;
	} *ap;
*/
{
	int error;
	struct vnode *vp = OTHERVP(ap->a_vp);

	ap->a_vp = vp;
	error = VCALL(vp, VOFFSET(vnop_pathconf), ap);

	return (error);
}

int
union_advlock(struct vnop_advlock_args *ap)
/*
	struct vnop_advlock_args {
		struct vnode *a_vp;
		caddr_t  a_id;
		int  a_op;
		struct flock *a_fl;
		int  a_flags;
		vfs_context_t a_context;
	} *ap;
*/
{
	register struct vnode *ovp = OTHERVP(ap->a_vp);

	ap->a_vp = ovp;
	return (VCALL(ovp, VOFFSET(vnop_advlock), ap));
}


/*
 * XXX - vnop_strategy must be hand coded because it has no
 * vnode in its arguments.
 * This goes away with a merged VM/buffer cache.
 */
int
union_strategy(struct vnop_strategy_args *ap)
/*
	struct vnop_strategy_args {
		struct buf *a_bp;
	} *ap;
*/
{
	struct buf *bp = ap->a_bp;
	int error;
	struct vnode *savedvp;

	savedvp = buf_vnode(bp);
	buf_setvnode(bp, OTHERVP(savedvp));

#if DIAGNOSTIC
	if (buf_vnode(bp) == NULLVP)
		panic("union_strategy: nil vp");
	if (((buf_flags(bp) & B_READ) == 0) &&
	    (buf_vnode(bp) == LOWERVP(savedvp)))
		panic("union_strategy: writing to lowervp");
#endif

	error = VNOP_STRATEGY(bp);
	buf_setvnode(bp, savedvp);

	return (error);
}

/* Pagein */
STUB(union_pagein, struct vnop_pagein_args *, a_vp)

/* Pageout  */
STUB(union_pageout, struct vnop_pageout_args *, a_vp)

/* Blktooff derives file offset for the given logical block number */
STUB(union_blktooff, struct vnop_blktooff_args *, a_vp)

/* offtoblk derives file offset for the given logical block number */
STUB(union_offtoblk, struct vnop_offtoblk_args *, a_vp)

#define VOPFUNC int (*)(void *)

/*
 * Global vfs data structures
 */
int (**union_vnodeop_p)(void *);
struct vnodeopv_entry_desc union_vnodeop_entries[40];
struct vnodeopv_desc union_vnodeop_opv_desc =
	{ &union_vnodeop_p, union_vnodeop_entries };
void init_vnodeop_entries() {
	struct vnodeopv_entry_desc foo[] = {
		{ &vnop_default_desc, (VOPFUNC)vn_default_error },
		{ &vnop_lookup_desc, (VOPFUNC)union_lookup },		/* lookup */
		{ &vnop_create_desc, (VOPFUNC)union_create },		/* create */
		{ &vnop_whiteout_desc, (VOPFUNC)union_whiteout },	/* whiteout */
		{ &vnop_mknod_desc, (VOPFUNC)union_mknod },		/* mknod */
		{ &vnop_open_desc, (VOPFUNC)union_open },		/* open */
		{ &vnop_close_desc, (VOPFUNC)union_close },		/* close */
		{ &vnop_access_desc, (VOPFUNC)union_access },		/* access */
		{ &vnop_getattr_desc, (VOPFUNC)union_getattr },		/* getattr */
		{ &vnop_setattr_desc, (VOPFUNC)union_setattr },		/* setattr */
		{ &vnop_read_desc, (VOPFUNC)union_read },		/* read */
		{ &vnop_write_desc, (VOPFUNC)union_write },		/* write */
		{ &vnop_ioctl_desc, (VOPFUNC)union_ioctl },		/* ioctl */
		{ &vnop_select_desc, (VOPFUNC)union_select },		/* select */
		{ &vnop_revoke_desc, (VOPFUNC)union_revoke },		/* revoke */
		{ &vnop_mmap_desc, (VOPFUNC)union_mmap },		/* mmap */
		{ &vnop_mnomap_desc, (VOPFUNC)union_mnomap },		/* mnomap */
		{ &vnop_fsync_desc, (VOPFUNC)union_fsync },		/* fsync */
		{ &vnop_remove_desc, (VOPFUNC)union_remove },		/* remove */
		{ &vnop_link_desc, (VOPFUNC)union_link },		/* link */
		{ &vnop_rename_desc, (VOPFUNC)union_rename },		/* rename */
		{ &vnop_mkdir_desc, (VOPFUNC)union_mkdir },		/* mkdir */
		{ &vnop_rmdir_desc, (VOPFUNC)union_rmdir },		/* rmdir */
		{ &vnop_symlink_desc, (VOPFUNC)union_symlink },		/* symlink */
		{ &vnop_readdir_desc, (VOPFUNC)union_readdir },		/* readdir */
		{ &vnop_readlink_desc, (VOPFUNC)union_readlink },	/* readlink */
		{ &vnop_inactive_desc, (VOPFUNC)union_inactive },	/* inactive */
		{ &vnop_reclaim_desc, (VOPFUNC)union_reclaim },		/* reclaim */
		{ &vnop_strategy_desc, (VOPFUNC)union_strategy },	/* strategy */
		{ &vnop_pathconf_desc, (VOPFUNC)union_pathconf },	/* pathconf */
		{ &vnop_advlock_desc, (VOPFUNC)union_advlock },		/* advlock */
	#ifdef notdef
		{ &vnop_bwrite_desc, (VOPFUNC)union_bwrite },		/* bwrite */
	#endif
		{ &vnop_pagein_desc, (VOPFUNC)union_pagein },		/* Pagein */
		{ &vnop_pageout_desc, (VOPFUNC)union_pageout },		/* Pageout */
		{ &vnop_copyfile_desc, (VOPFUNC)eopnotsupp },		/* Copyfile */
		{ &vnop_blktooff_desc, (VOPFUNC)union_blktooff },	/* blktooff */
		{ &vnop_offtoblk_desc, (VOPFUNC)union_offtoblk },	/* offtoblk */
		{ &vnop_blockmap_desc, (VOPFUNC)union_blockmap },	/* blockmap */
		{ (struct vnodeop_desc*)NULL, (int(*)())NULL }
	};
	memcpy(union_vnodeop_entries, foo, sizeof(foo));
}
