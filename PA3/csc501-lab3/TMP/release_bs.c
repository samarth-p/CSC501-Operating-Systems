#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
	STATWORD ps; 
	disable(ps);

	if(isbadbs(bs_id)){ 
		restore(ps);
		return SYSERR;
	}

	int i;
	for(i=0; i < NPROC; i++){
		if(bsm_tab[bs_id].bs_pid[i]){
			restore(ps);
			return SYSERR;
		}
	}	
	
	free_bsm(bs_id);	
	
	restore(ps);
	return OK;

}

