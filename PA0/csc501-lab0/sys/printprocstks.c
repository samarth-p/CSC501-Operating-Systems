#include <stdio.h>
#include <kernel.h>
#include <proc.h>

static unsigned long	*esp;

void printprocstks(int priority) {

	kprintf("\nvoid printprocstks(int priority)\n");
	int process_id = 0;

	while(process_id < NPROC) {
		struct	pentry *process = &proctab[process_id++];

		if((process->pstate == PRFREE) || (process->pprio < priority))
			continue;

		if(process_id == currpid) {
			asm("movl %esp, esp");
		}
		else
			*esp = process->pesp;

		kprintf("Process [%s]\n", process->pname);
		kprintf("\tpid; %d\n", process_id);
		kprintf("\tpriority: %d\n", process->pname);
		kprintf("\tbase: 0x%08x\n", process->pbase);
		kprintf("\tlimit: 0x%08x\n", process->plimit);
		kprintf("\tlen: %d\n", process->pstklen);
		kprintf("\tpointer: %d\n", *esp);
	}
	kprintf("\n");

}