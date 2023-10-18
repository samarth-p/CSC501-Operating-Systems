/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(vblock, size)
	struct	mblock	*vblock;
	unsigned size;
{
	STATWORD ps;
	disable(ps);
	int bs_id, page_offset;
	unsigned long bs_addr;
	struct  mblock  *p, *q, *block;
	unsigned top;
	
	bsm_lookup(currpid, (long) vblock, &bs_id, &page_offset);
	bs_addr = BACKING_STORE_BASE + bs_id * BACKING_STORE_UNIT_SIZE;
	block = (struct mblock *) (bs_addr + page_offset * NBPG);

	if (size==0 || ((unsigned)block)<((unsigned) &end)){
		restore(ps);
		return(SYSERR);
	}

	size = (unsigned)roundmb(size);

	for( p=proctab[currpid].vmemlist.mnext,q= &proctab[currpid].vmemlist; p != (struct mblock *) NULL && p < block ; q=p,p=p->mnext );

	if ((( top = q->mlen + (unsigned)q ) > (unsigned)block && q!= &proctab[currpid].vmemlist) || (p!=NULL && (size + (unsigned)block) > (unsigned)p )) {
			restore(ps);
			return(SYSERR);
	}

	if ( q!= &proctab[currpid].vmemlist && top == (unsigned)block )
					q->mlen += size;
	else {
			block->mlen = size;
			block->mnext = p;
			q->mnext = block;
			q = block;
	}

	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
			q->mlen += p->mlen;
			q->mnext = p->mnext;
	}

	restore(ps);
	return(OK);

}

