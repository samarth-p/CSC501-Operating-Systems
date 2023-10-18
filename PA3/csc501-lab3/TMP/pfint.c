/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	disable(ps);

	virt_addr_t v_addr;	
	unsigned int p,q;
	pd_t *pd_entry;
	pt_t *pt_entry;
	unsigned long addr, pdbr, pt;
	int vpno, new_frame1, new_frame2, bs_num, page_offset, i;

	addr = read_cr2();
	get_virt_addr(addr, &v_addr);
	get_virt_page_num(v_addr, &vpno);
	pdbr = proctab[currpid].pdbr;

	p = v_addr.pd_offset;
	q = v_addr.pt_offset;
	
	pt = pdbr + (sizeof(pd_t)) * p;
 	pd_entry = (pd_t *) pt;
	
	if(!pd_entry->pd_pres){

		// Initialize frames
		if( get_frm(&new_frame1) == SYSERR)
				return SYSERR;

		frm_tab[new_frame1].fr_type = FR_TBL;
		frm_tab[new_frame1].fr_status = FRM_MAPPED;
		frm_tab[new_frame1].fr_pid = currpid;

		// initialize page tables
		pt_t *pt_entry;

		pt_entry = (pt_t *)((FRAME0 + new_frame1) * NBPG);
		for(i=0; i < 1024; i++){
				pt_entry->pt_pres = 0;
				pt_entry->pt_write = 1;
				pt_entry->pt_user = 0;
				pt_entry->pt_pwt = 0;
				pt_entry->pt_pcd = 0;
				pt_entry->pt_acc = 0;
				pt_entry->pt_dirty = 0;
				pt_entry->pt_mbz = 0;
				pt_entry->pt_global = 0;
				pt_entry->pt_avail = 0;
				pt_entry++;

		}
		
		// update page directory entry
		pd_entry->pd_pres = 1;
	    pd_entry->pd_write = 1;
    	pd_entry->pd_user = 0;
    	pd_entry->pd_pwt = 0;
    	pd_entry->pd_pcd = 0;
    	pd_entry->pd_acc = 0;
    	pd_entry->pd_mbz = 0;
    	pd_entry->pd_fmb = 0;
    	pd_entry->pd_global = 0;
    	pd_entry->pd_avail = 0;
		pd_entry->pd_base = FRAME0 + new_frame1;
	}

	bsm_lookup(currpid, addr, &bs_num, &page_offset);

	frm_tab[pd_entry->pd_base - FRAME0].fr_refcnt++;
	get_frm(&new_frame2);
	read_bs((char*)((FRAME0+new_frame2)*NBPG),bs_num,page_offset);


	frm_tab[new_frame2].fr_status = FRM_MAPPED;
	frm_tab[new_frame2].fr_pid = currpid;
	frm_tab[new_frame2].fr_type = FR_PAGE;
	frm_tab[new_frame2].fr_vpno = vpno;
	frm_tab[new_frame2].fr_wr = WR_NORMAL;
	
	frm_tab[new_frame2].fr_acc = 1;	
	num_sc++;

	if(num_sc == 1)	
		sc_curr = new_frame2;

	insert_into_queue(&fifohead, new_frame2);

	pt_entry = (pt_t *) ((pd_entry->pd_base * NBPG) + (sizeof(pt_t) * q));

	pt_entry->pt_pres = 1;
	pt_entry->pt_base = FRAME0 + new_frame2;
	restore(ps);
	return OK;
	
}

void insert_into_queue(struct fifo_queue ** phead, int item){
	struct fifo_queue *p,*temp;
	p = (struct fifo_queue *)getmem(sizeof(struct fifo_queue));
	p->key = item;
	p->next = NULL;
    
	if(queue_isempty(*phead)){
		*phead = p;
		return; 
    }
    
	temp = *phead;
    while(temp->next != NULL)
   		temp = temp->next;
   	temp->next = p;
}


int get_virt_addr(unsigned long addr, virt_addr_t *v_addr){
	v_addr->pg_offset = addr & 4095UL;
	v_addr->pt_offset = (addr & 4190208UL)>>12;
	v_addr->pd_offset = (addr & 4290772992UL)>>22;
	
	return OK;
}

int get_virt_page_num(virt_addr_t v_addr, int *vpno){
	*vpno = (((int)v_addr.pd_offset) << 10) | ((int)v_addr.pt_offset);
	return OK;
}
