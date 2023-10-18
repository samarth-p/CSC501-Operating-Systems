#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


int create_page_dir(int pid, int *pd_frame){
	if( get_frm(pd_frame) == SYSERR)
		return SYSERR;

	frm_tab[*pd_frame].fr_pid = pid;
	frm_tab[*pd_frame].fr_type = FR_DIR;
	frm_tab[*pd_frame].fr_status = FRM_MAPPED;
	return OK;
}

void init_page_dir(int pd_frame){
	int i;
	pd_t *pd_entry;

	pd_entry = (pd_t *)((FRAME0 + pd_frame) * NBPG);
        for(i=0; i < 1024; i++){

		if(i < 4){
			pd_entry->pd_pres = 1;
			pd_entry->pd_base = FRAME0 + i;
		}
		else{
			pd_entry->pd_pres = 0;
		}
		pd_entry->pd_write = 1;
		pd_entry->pd_user = 0;
		pd_entry->pd_pwt = 0;
		pd_entry->pd_pcd = 0;
		pd_entry->pd_acc = 0;
		pd_entry->pd_mbz = 0;
		pd_entry->pd_fmb = 0;
		pd_entry->pd_global = 0;
		pd_entry->pd_avail = 0;
		pd_entry++;

        }

}

void initialize_pt(int pt_frame){
	int i;
	pt_t *pt_entry;

	pt_entry = (pt_t *)((FRAME0 + pt_frame) * NBPG);
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

}

void print_debug_frame_tab(){
	int i;
	for(i=0; i < NFRAMES; i++)
	{
		if(frm_tab[i].fr_status)	
			kprintf("FRAME i = %d, status = %d, vpno = %d, pid = %d\n",i,frm_tab[i].fr_status, frm_tab[i].fr_vpno, frm_tab[i].fr_pid);

	}
	kprintf("\n");
}
