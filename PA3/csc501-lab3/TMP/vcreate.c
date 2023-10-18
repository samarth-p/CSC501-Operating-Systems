/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD 	ps;    
	int 		bs;		/* private backing store	*/
	unsigned long 	unmap_vpno;	/* first unmapped virtual page for the current process	*/
	struct mblock 	*mptr;		/* pointer to free memory block */
	
	disable(ps);

	struct	pentry	*pptr;
	// struct	mblock	*mptr;
	int pid = create(procaddr,ssize,priority,name,nargs,args);

	unmap_vpno = 4096;
	mptr = &proctab[pid].vmemlist;

	if(get_bsm(&bs) == SYSERR){
		restore(ps);
		return SYSERR;
	}
	bsm_map(pid, 4096, bs, hsize);
	bsm_tab[bs].bs_ispriv = BSM_PRIVATE;

	mptr->mnext = (struct mblock*)  (BACKING_STORE_BASE + bs * BACKING_STORE_UNIT_SIZE);
	mptr->mnext->mlen = hsize * NBPG;
	mptr->mnext->mnext = NULL;

	restore(ps);
	return(pid);
	
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
