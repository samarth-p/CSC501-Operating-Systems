/* sdelete.c - sdelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * sdelete  --  delete a semaphore by releasing its table entry
 *------------------------------------------------------------------------
 */
int ldelete (int lockdescriptor)
{
	STATWORD ps;    
	int	pid;
	struct	lentry	*lptr;

	disable(ps);
	if (isbadlock(lockdescriptor) || locks[lockdescriptor].lstate == LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &locks[lockdescriptor/10];
	lptr->lstate = LFREE;
	if (nonempty(lptr->lqhead)) {
		while( (pid=getfirst(lptr->lqhead)) != EMPTY)
		  {
		    proctab[pid].plockret = DELETED;
		    ready(pid,RESCHNO);
		  }
		resched();
	}
	restore(ps);
	return(OK);
}
