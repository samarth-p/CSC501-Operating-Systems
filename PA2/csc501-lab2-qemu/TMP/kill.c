/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;
	disable(ps);
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	struct  lentry *lptr;
	int	dev;
	int i=0;
	struct lentry *lock_ptr;
	int temp =0;

	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

    case PROC_LOCK:
		lock_ptr = &locks[pptr->l_proc];
	    if(lock_ptr->proc_lock[currpid] == 0){
        	swap_values(lock_ptr->proc_lock[currpid],lock_ptr->proc_lock[currpid]+1);
    	}
    	else if(lock_ptr->proc_lock[currpid] > 0){
			temp=0;
        	while(temp < 0){
            	swap_values(lock_ptr->proc_lock[currpid]+NLOCKS,lock_ptr->proc_lock[currpid]);
        	}
    	}
    	else{
        	swap_values(lock_ptr->proc_lock[currpid]+NLOCKS,lock_ptr->proc_lock[currpid]+NPROC);
    	}

		kill_lock(pid);
		break;
	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}

void kill_lock(int pid){
	struct	pentry	*pptr;	
	struct  lentry *lptr;
	dequeue(pid);
	pptr= &proctab[pid];
	lptr = &locks[pptr->l_proc];

	locks[pptr->l_proc].proc_lock[pid] = 0;
	modify_lock_prio(pptr->l_proc);
	int i=0;
	while(i < NPROC){
		if(lptr->proc_lock[i] > 0)
			prio_update(i);
		i++;
	}
	pptr->pstate = PRFREE;
}
