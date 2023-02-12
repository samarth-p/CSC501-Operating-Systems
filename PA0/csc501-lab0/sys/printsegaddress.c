#include <stdio.h>

void printsegaddress() {

	extern long etext, edata, end;
	kprintf("\nvoid printsegaddress()\n");
	kprintf("\n");
	kprintf("Current: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", 
	&etext, etext, &edata, edata, &end, end);
	kprintf("Preceding: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", 
	&etext-1, etext-1, &edata-1, edata-1, &end-1, end-1);
	kprintf("After: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", 
	&etext+1, etext+1, &edata+1, edata+1, &end+1, end+1);

	kprintf("\n");
}
