/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	STATWORD ps;    
	struct	mblock	*p, *q, *leftover;
	int bs_num, vpno;
	unsigned long bs_addr;

	disable(ps);
	if (nbytes==0 || proctab[currpid].vmemlist.mnext== (struct mblock *) NULL) {
		restore(ps);
		return( (WORD *)SYSERR);
	}
	nbytes = (unsigned int) roundmb(nbytes);

	for (q= &proctab[currpid].vmemlist,p=proctab[currpid].vmemlist.mnext; p != (struct mblock *) NULL ; q=p,p=p->mnext){
	
		bs_num = ((long)p - BACKING_STORE_BASE)/(long)BACKING_STORE_UNIT_SIZE;
		bs_addr = BACKING_STORE_BASE + bs_num * BACKING_STORE_UNIT_SIZE;
		vpno = bsm_tab[bs_num].bs_vpno[currpid];
	
		if ( p->mlen == nbytes) {
			q->mnext = p->mnext;
			restore(ps);	
			return( (WORD *)((char *)p - (char *)bs_addr + (char *)(vpno * NBPG) ));
		} 
		else if ( p->mlen > nbytes ) {
			leftover = (struct mblock *)( (unsigned)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			restore(ps);
			return( (WORD *)((char *)p -(char *) bs_addr + (char *)(vpno * NBPG)) );
		}
	}
	restore(ps);
	return( (WORD *)SYSERR );
}


