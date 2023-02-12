#include <stdio.h>

static unsigned long 	*ebp;

void printtos() {

	long x = 0xabcd, y = 0xffff;

	asm("movl %ebp, ebp");

	kprintf("\nvoid printtos()\n");

	kprintf("Before[0x%08x]: 0x%08x\n", (ebp+2), *(ebp+2));
	kprintf("After [0x%08x]: 0x%08x\n", ebp, *ebp);

	kprintf("\telement[0x%08x]: 0x%08x\n", (ebp-1), *(ebp-1));
	kprintf("\telement[0x%08x]: 0x%08x\n", (ebp-2), *(ebp-2));
	kprintf("\telement[0x%08x]: 0x%08x\n", (ebp-3), *(ebp-3));
	kprintf("\telement[0x%08x]: 0x%08x\n", (ebp-4), *(ebp-4));
	kprintf("\n");
}
