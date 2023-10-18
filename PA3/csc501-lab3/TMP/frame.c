/* frame.c - manage physical frames */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{	
	STATWORD ps;
	disable(ps);
	int i;
	for(i=0; i<NFRAMES; i++) {
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = -1;
		frm_tab[i].fr_dirty = 0;
		frm_tab[i].fr_acc = 0;
		frm_tab[i].fr_wr = -1;
	}
	restore(ps);
	return OK;
}


/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	STATWORD ps;
	disable(ps);
	int i;
	for(i=0; i<NFRAMES; i++) {
		if (frm_tab[i].fr_status == FRM_MAPPED)
			continue;
		*avail = i;
		restore(ps);
		return OK;
	}

	int frame = -1;
	if(page_replace_policy == SC)
		frame = page_replacement_SC();
	else if(page_replace_policy == FIFO)
		frame = queue_firstitem(fifohead);
	else
		return SYSERR;

	if(frame < 0)
		return SYSERR;
	if(debugging_mode)
		kprintf("Page being replaced = %d\n",FRAME0+frame);
	free_frm(frame);
	*avail = frame;
	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{	
	STATWORD ps;
	disable(ps);

	if(frm_tab[i].fr_type == FR_PAGE)
	{
		delete_pt_entry(i);

		if(dequeue_fifo(&fifohead, i) == SYSERR){
			restore(ps);
			return SYSERR;
		}
		if((--num_sc) == 0)
			sc_curr = -1;
	}
	frm_tab[i].fr_status = FRM_UNMAPPED;
	frm_tab[i].fr_pid = -1;
	frm_tab[i].fr_vpno = -1;
	frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_type = -1;
	frm_tab[i].fr_dirty = 0;
	frm_tab[i].fr_acc = 0;
	frm_tab[i].fr_wr = -1;

	restore(ps);
	return OK;
}

void delete_pt_entry(int i){

	pd_t *pd_entry;
	pt_t *pt_entry;
	unsigned long addr, pdbr, pt;
	virt_addr_t v_addr;
	int pid, vpno, wr;
	unsigned int p,q,bs_id,page_offset;

	pid = frm_tab[i].fr_pid;
	vpno = frm_tab[i].fr_vpno;
	wr = frm_tab[i].fr_wr;
	
	addr = vpno * NBPG;	
	get_virt_addr(addr, &v_addr);        
	p = v_addr.pd_offset;
	q = v_addr.pt_offset; 

	pdbr = proctab[pid].pdbr;
	pt = pdbr + sizeof(pd_t) * p;   
	pd_entry = (pd_t *) pt;
	pt_entry = (pt_t *) (pd_entry->pd_base * NBPG + sizeof(pt_t) * q );

	pt_entry->pt_pres = 0;

	/* invalidate the TLB entry for this page if it belongs to the current process */
	// if(pid == currpid)
	// 		;                               /* invlpg instruction */

	if(!(--frm_tab[pd_entry->pd_base].fr_refcnt))
 	   pd_entry->pd_pres = 0;

	if(pt_entry->pt_dirty){
		if(bsm_lookup(pid, addr, &bs_id, &page_offset) == SYSERR){

			if(wr != WR_FORCED){
				kprintf("Backing Store lookup failed! Killing process %d!\n",pid);
				kill(pid);
			}
		}
		write_bs((char *)(pt_entry->pt_base * NBPG), bs_id, page_offset);
	
	}
	

                                                  
}

int dequeue_fifo(struct fifo_queue **phead, int item){

	struct fifo_queue *temp, *prev;
	if(queue_isempty(*phead))
		return SYSERR;
        
	temp = *phead;
	prev = *phead;

	if((*phead)->key == item){
		*phead = (*phead)->next;
		freemem((struct mblock *)temp, sizeof(struct fifo_queue));
		return item;
	}

	while(temp != NULL && temp->key != item){
		prev = temp;
		temp = temp->next;
	}

	if(temp->key == item){
		prev->next = temp->next;
		freemem((struct mblock *)temp, sizeof(struct fifo_queue));
		return item;
	}
	return SYSERR;
}

int page_replacement_SC() {
	int i = sc_curr;
	while (1){
		if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE){
			if(!frm_tab[i].fr_acc){
				increment_sc_curr();
				return i;
			}
			frm_tab[i].fr_acc = 0;
		}
		i = (i+1) % NFRAMES;
	}
	return SYSERR;
}

void increment_sc_curr(){
	int i = (sc_curr + 1) % NFRAMES;
	while(i != sc_curr){
		if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE){
			sc_curr = i;
			break;
		}
		i = (i+1) % NFRAMES;
	}
}


