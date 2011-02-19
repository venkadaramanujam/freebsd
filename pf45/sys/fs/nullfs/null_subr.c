/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
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
 *	@(#)null_subr.c	8.7 (Berkeley) 5/14/95
 *
 * $FreeBSD$
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/vnode.h>

#include <fs/nullfs/null.h>

#define LOG2_SIZEVNODE 8		/* log2(sizeof struct vnode) */
#define	NNULLNODECACHE 16

/*
 * Null layer cache:
 * Each cache entry holds a reference to the lower vnode
 * along with a pointer to the alias vnode.  When an
 * entry is added the lower vnode is VREF'd.  When the
 * alias is removed the lower vnode is vrele'd.
 */

#define	NULL_NHASH(vp) \
	(&null_node_hashtbl[(((uintptr_t)vp)>>LOG2_SIZEVNODE) & null_node_hash])

static LIST_HEAD(null_node_hashhead, null_node) *null_node_hashtbl;
static u_long null_node_hash;
struct mtx null_hashmtx;

static MALLOC_DEFINE(M_NULLFSHASH, "nullfs_hash", "NULLFS hash table");
MALLOC_DEFINE(M_NULLFSNODE, "nullfs_node", "NULLFS vnode private part");

static struct vnode * null_hashget(struct mount *, struct vnode *);
static struct vnode * null_hashins(struct mount *, struct null_node *);

/*
 * Initialise cache headers
 */
int
nullfs_init(vfsp)
	struct vfsconf *vfsp;
{

	NULLFSDEBUG("nullfs_init\n");		/* printed during system boot */
	null_node_hashtbl = hashinit(NNULLNODECACHE, M_NULLFSHASH, &null_node_hash);
	mtx_init(&null_hashmtx, "nullhs", NULL, MTX_DEF);
	return (0);
}

int
nullfs_uninit(vfsp)
	struct vfsconf *vfsp;
{

	mtx_destroy(&null_hashmtx);
	free(null_node_hashtbl, M_NULLFSHASH);
	return (0);
}

/*
 * Return a VREF'ed alias for lower vnode if already exists, else 0.
 * Lower vnode should be locked on entry and will be left locked on exit.
 */
static struct vnode *
null_hashget(mp, lowervp)
	struct mount *mp;
	struct vnode *lowervp;
{
	struct null_node_hashhead *hd;
	struct null_node *a;
	struct vnode *vp;

	ASSERT_VOP_LOCKED(lowervp, "null_hashget");

	/*
	 * Find hash base, and then search the (two-way) linked
	 * list looking for a null_node structure which is referencing
	 * the lower vnode.  If found, the increment the null_node
	 * reference count (but NOT the lower vnode's VREF counter).
	 */
	hd = NULL_NHASH(lowervp);
	mtx_lock(&null_hashmtx);
	LIST_FOREACH(a, hd, null_hash) {
		if (a->null_lowervp == lowervp && NULLTOV(a)->v_mount == mp) {
			/*
			 * Since we have the lower node locked the nullfs
			 * node can not be in the process of recycling.  If
			 * it had been recycled before we grabed the lower
			 * lock it would not have been found on the hash.
			 */
			vp = NULLTOV(a);
			vref(vp);
			mtx_unlock(&null_hashmtx);
			return (vp);
		}
	}
	mtx_unlock(&null_hashmtx);
	return (NULLVP);
}

/*
 * Act like null_hashget, but add passed null_node to hash if no existing
 * node found.
 */
static struct vnode *
null_hashins(mp, xp)
	struct mount *mp;
	struct null_node *xp;
{
	struct null_node_hashhead *hd;
	struct null_node *oxp;
	struct vnode *ovp;

	hd = NULL_NHASH(xp->null_lowervp);
	mtx_lock(&null_hashmtx);
	LIST_FOREACH(oxp, hd, null_hash) {
		if (oxp->null_lowervp == xp->null_lowervp &&
		    NULLTOV(oxp)->v_mount == mp) {
			/*
			 * See null_hashget for a description of this
			 * operation.
			 */
			ovp = NULLTOV(oxp);
			vref(ovp);
			mtx_unlock(&null_hashmtx);
			return (ovp);
		}
	}
	LIST_INSERT_HEAD(hd, xp, null_hash);
	mtx_unlock(&null_hashmtx);
	return (NULLVP);
}

static void
null_insmntque_dtr(struct vnode *vp, void *xp)
{
	vp->v_data = NULL;
	vp->v_vnlock = &vp->v_lock;
	free(xp, M_NULLFSNODE);
	vp->v_op = &dead_vnodeops;
	(void) vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
	vgone(vp);
	vput(vp);
}

/*
 * Make a new or get existing nullfs node.
 * Vp is the alias vnode, lowervp is the lower vnode.
 * 
 * The lowervp assumed to be locked and having "spare" reference. This routine
 * vrele lowervp if nullfs node was taken from hash. Otherwise it "transfers"
 * the caller's "spare" reference to created nullfs vnode.
 */
int
null_nodeget(mp, lowervp, vpp)
	struct mount *mp;
	struct vnode *lowervp;
	struct vnode **vpp;
{
	struct null_node *xp;
	struct vnode *vp;
	int error;

	/* Lookup the hash firstly */
	*vpp = null_hashget(mp, lowervp);
	if (*vpp != NULL) {
		vrele(lowervp);
		return (0);
	}

	/*
	 * We do not serialize vnode creation, instead we will check for
	 * duplicates later, when adding new vnode to hash.
	 *
	 * Note that duplicate can only appear in hash if the lowervp is
	 * locked LK_SHARED.
	 */

	/*
	 * Do the MALLOC before the getnewvnode since doing so afterward
	 * might cause a bogus v_data pointer to get dereferenced
	 * elsewhere if MALLOC should block.
	 */
	xp = malloc(sizeof(struct null_node),
	    M_NULLFSNODE, M_WAITOK);

	error = getnewvnode("null", mp, &null_vnodeops, &vp);
	if (error) {
		free(xp, M_NULLFSNODE);
		return (error);
	}

	xp->null_vnode = vp;
	xp->null_lowervp = lowervp;
	vp->v_type = lowervp->v_type;
	vp->v_data = xp;
	vp->v_vnlock = lowervp->v_vnlock;
	if (vp->v_vnlock == NULL)
		panic("null_nodeget: Passed a NULL vnlock.\n");
	error = insmntque1(vp, mp, null_insmntque_dtr, xp);
	if (error != 0)
		return (error);
	/*
	 * Atomically insert our new node into the hash or vget existing 
	 * if someone else has beaten us to it.
	 */
	*vpp = null_hashins(mp, xp);
	if (*vpp != NULL) {
		vrele(lowervp);
		vp->v_vnlock = &vp->v_lock;
		xp->null_lowervp = NULL;
		vrele(vp);
		return (0);
	}
	*vpp = vp;

	return (0);
}

/*
 * Remove node from hash.
 */
void
null_hashrem(xp)
	struct null_node *xp;
{

	mtx_lock(&null_hashmtx);
	LIST_REMOVE(xp, null_hash);
	mtx_unlock(&null_hashmtx);
}

#ifdef DIAGNOSTIC

struct vnode *
null_checkvp(vp, fil, lno)
	struct vnode *vp;
	char *fil;
	int lno;
{
	struct null_node *a = VTONULL(vp);

#ifdef notyet
	/*
	 * Can't do this check because vop_reclaim runs
	 * with a funny vop vector.
	 */
	if (vp->v_op != null_vnodeop_p) {
		printf ("null_checkvp: on non-null-node\n");
		panic("null_checkvp");
	}
#endif
	if (a->null_lowervp == NULLVP) {
		/* Should never happen */
		int i; u_long *p;
		printf("vp = %p, ZERO ptr\n", (void *)vp);
		for (p = (u_long *) a, i = 0; i < 8; i++)
			printf(" %lx", p[i]);
		printf("\n");
		panic("null_checkvp");
	}
	VI_LOCK_FLAGS(a->null_lowervp, MTX_DUPOK);
	if (a->null_lowervp->v_usecount < 1) {
		int i; u_long *p;
		printf("vp = %p, unref'ed lowervp\n", (void *)vp);
		for (p = (u_long *) a, i = 0; i < 8; i++)
			printf(" %lx", p[i]);
		printf("\n");
		panic ("null with unref'ed lowervp");
	}
	VI_UNLOCK(a->null_lowervp);
#ifdef notyet
	printf("null %x/%d -> %x/%d [%s, %d]\n",
	        NULLTOV(a), vrefcnt(NULLTOV(a)),
		a->null_lowervp, vrefcnt(a->null_lowervp),
		fil, lno);
#endif
	return (a->null_lowervp);
}
#endif