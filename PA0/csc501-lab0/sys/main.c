/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

int prX;
void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
prch(c)
char c;
{
	int i;
	sleep(5);	
}

long is_little_endian() {
	unsigned long x = 1;
	char *p = (char*) &x;
	return (int)*p;
}

int main()
{
	kprintf("\n\nIs Little Endian? - %d\n", is_little_endian());
	//Task 1
	long param = 0xaabbccdd;
	kprintf("\nlong zfunction(long param)\n");
	long output = zfunction(param);
	kprintf("Input: 0x%08x, Output: 0x%08x\n", param, output);
	kprintf("\n");

	//Task 2
	printsegaddress();

	//Task 3
	printtos();

	//Task 4
	printprocstks(1);

	//Task 5
	syscallsummary_start();
	resume(prX = create(prch,2000,20,"proc X",1,'A'));
	sleep(10);
	syscallsummary_stop();
	printsyscallsummary();

	return 0;
}

