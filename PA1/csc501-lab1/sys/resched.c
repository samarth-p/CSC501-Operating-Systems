/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	if (sch == LINUXSCHED) {
		optr = &proctab[currpid];
		optr->counter = preempt;
		// kprintf("Init - currpid = %d, quantum = %d, counter = %d\n", currpid, optr->quantum, optr->counter);

		int p, new_epoch = 1;
		for (p = 0; p < NPROC; p++) {
			if ((proctab[p].pstate == PRCURR || proctab[p].pstate == PRREADY) && proctab[p].counter != 0) {
				new_epoch = 0;
				break;
			}
			p++;
		}

		if (new_epoch) {
			for (p = 0; p < NPROC; p++) {
				if (proctab[p].pstate != PRFREE) {
					proctab[p].quantum = proctab[p].pprio;
					if (proctab[p].counter > 0 && proctab[p].counter < proctab[p].quantum)
						proctab[p].quantum += (proctab[p].counter / 2);
					proctab[p].counter = proctab[p].quantum;
				}
			}
		}

		int max_goodness = 0, new_proc = NULLPROC, next;
		next = q[rdyhead].qnext;
		while (q[next].qkey < MAXINT) {
			int goodness = (proctab[next].counter)? proctab[next].pprio + proctab[next].counter : 0;
			if (goodness > max_goodness) {
				new_proc = next;
				max_goodness = goodness;
			}
			next = q[next].qnext;
		}

		if (new_proc == NULLPROC && currpid == NULLPROC)
			return(OK);

		if (new_proc == currpid && optr->pstate == PRCURR) {
			preempt = optr->counter;
			return(OK);
		}
		
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}
		nptr = &proctab[currpid = dequeue(new_proc)];
		nptr->pstate = PRCURR;
		preempt = (new_proc == NULLPROC)? QUANTUM : nptr->counter;
		// kprintf("Final - currpid = %d, quantum = %d, counter = %d, preempt =\n", currpid, optr->quantum, optr->counter, preempt);

	}
	else if (sch == AGESCHED) {

		// Age based scheduler
		int next;
		next = q[rdyhead].qnext;
		while (q[next].qkey < MAXINT) {
			q[next].qkey++;
			next = q[next].qnext;
		}

		/* no switch needed if current process priority higher than next*/

		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
		
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	}
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
