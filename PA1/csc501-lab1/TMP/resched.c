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
		optr->counter = preempt;  // Set the current process counter to the unused CPU ticks
		int curr_proc_goodness = (optr->counter)? optr->pprio + optr->counter : 0;  // Calculate the goodness of the current process
	
		// kprintf("Init - currpid = %d, quantum = %d, counter = %d\n", currpid, optr->quantum, optr->counter);

		// Check if we need to start a new EPOCH based if there are any ready/current processes with non-zero goodness
		int p, new_epoch = 1;
		for (p = 1; p < NPROC; p++) {
			if ((proctab[p].pstate == PRCURR || proctab[p].pstate == PRREADY) && proctab[p].counter != 0) {
				new_epoch = 0;
				break;
			}
		}

		// If we need to start a new EPOCH, recalculate the quantum and counter for each process
		if (new_epoch) {
			// kprintf("Starting new EPOCH\n");
			for (p = 1; p < NPROC; p++) {
				if (proctab[p].pstate != PRFREE) {
					proctab[p].quantum = proctab[p].pprio;

					// If the process has executed but has some unused CPU ticks, we add part of this to the new quantum value
					if (proctab[p].counter > 0 && proctab[p].counter < proctab[p].quantum)
						proctab[p].quantum += (proctab[p].counter / 2);

					// Set process counter to be equal to the quantum at the start of an EPOCH
					proctab[p].counter = proctab[p].quantum;
				}
			}
		}

		// Find the process with the maximum goodness value
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

		// If the new process to be scheduled is the current running process, reset the preempt timer and continue running the same process
		// No need to context switch
		if (curr_proc_goodness >= max_goodness && optr->pstate == PRCURR) {

			// If current process is NULLPROC set preempt timer to default QUANTUM
			preempt = (currpid == NULLPROC)? QUANTUM : optr->counter;
			return(OK);
		}
		
		/* force context switch */
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		/* remove the new process with maximum goodness from the ready queue */
		nptr = &proctab[currpid = dequeue(new_proc)];
		nptr->pstate = PRCURR;

		// If new process is NULLPROC set preempt timer to default QUANTUM
		preempt = (new_proc == NULLPROC)? QUANTUM : nptr->counter;

	}
	else {

		// Age based scheduler
		if (sch == AGESCHED)  {
			int next = q[rdyhead].qnext;
			while (q[next].qkey < MAXINT) {
				q[next].qkey++;
				proctab[next].pprio++;
				next = q[next].qnext;
			}
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
