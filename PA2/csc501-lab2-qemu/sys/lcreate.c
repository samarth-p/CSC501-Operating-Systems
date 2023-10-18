#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
int lcreate()
{
	STATWORD ps;    
	disable(ps);

	int ret_val=SYSERR;
	int lock;
	int i=0;
	while(i < NLOCKS && ret_val==-1){
		lock = nextlock--;
		if(nextlock<0){
			nextlock = NLOCKS - 1;
			locks_traverse += 1;
		}
		else if(locks[lock].lstate == LFREE){
			locks[lock].lstate = LUSED;
			locks[lock].nr = 0;
			locks[lock].nw = 0;
			ret_val = lock*10 + locks_traverse;
			break;
		}
		i++;
	}
	restore(ps);
	return(ret_val);
}

