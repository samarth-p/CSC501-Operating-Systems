/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int dev;

	disable(ps);
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

	free_frames(pid);

	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}

void free_frames(int pid) {
	int	i, j, frame;
	for(i=0; i < NFRAMES; i++){
		if(frm_tab[i].fr_pid != pid)
			continue;
		if(frm_tab[i].fr_type == FR_PAGE){
			frm_tab[i].fr_wr = WR_FORCED;
			free_frm(i);
		}
		else if(frm_tab[i].fr_type == FR_DIR)
			frame = i;
	}
	for(i=0; i < NBS; i++){
		if(bsm_tab[i].bs_pid[pid])
			bsm_unmap(pid, bsm_tab[i].bs_vpno[pid], 0);
		
	}
	free_frm(frame);
}
