#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>


#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }

/* Locks - Problem of priority inversion */
void lockwriter1 (char *msg, int lck)
{
	kprintf ("  %s: to acquire lock\n", msg);
	lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock\n", msg);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void lockwriter2 (char *msg, int lck)
{
	kprintf ("  %s: to acquire lock\n", msg);
	lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep(2);
	kprintf("  %s: writer is running again\n", msg);
	int i;
	
	// wasting CPU cycles
	for(i=0; i<200000; i++){
	}

	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void cpuboundproc (char *msg)
{
	int i;
	kprintf ("  %s: begins execution\n", msg);
	sleep(1);

	// wasting CPU cycles
	for(i=0; i<200000; i++){
	}
	   
	kprintf ("  %s: ends execution\n", msg);
}
void testlock ()
{
	int     wr1,wr2;
	int 	cpu1,cpu2,cpu3;
	int 	lck;

	kprintf("\n\n------------------Locks - Problem of priority inversion------------------\n\n");
	lck  = lcreate ();

	wr1 = create(lockwriter1, 2000, 100, "wr1", 2, "writer1", lck);
	wr2 = create(lockwriter2, 2000, 20, "wr2", 2, "writer2", lck);
	cpu1 = create(cpuboundproc, 2000, 40, "cpu1", 1, "cpu1");
	cpu2 = create(cpuboundproc, 2000, 70, "cpu2", 1, "cpu2");
	cpu3 = create(cpuboundproc, 2000, 80, "cpu3", 1, "cpu3");


	kprintf("-start writer 2, then sleep 1s. lock granted to writer 2 (priority =  20)\n");
	resume(wr2);
	sleep (1);

	kprintf("-start writer 1. writer 1 (priority =  100) blocked on the lock\n");
	resume(wr1);

	kprintf("-start cpu1 (priority =  40)\n");
	resume (cpu1);

	kprintf("-start cpu2 (priority = 70)\n");
	resume (cpu2);

	kprintf("-start cpu3 (priority = 80)\n");
	resume (cpu3);

	sleep (10);
}

/* Semaphore - Problem of priority inversion */

void semwriter1(char *msg, int sem)
{ 
	kprintf ("  %s: to acquire sem\n", msg);
	wait (sem);
	kprintf ("  %s: acquired sem\n", msg);
	kprintf ("  %s: to release sem\n", msg);
	signal (sem);
}

void semwriter2 (char *msg, int sem)
{
	int i;
	kprintf ("  %s: to acquire sem\n", msg);
	wait (sem);
	kprintf ("  %s: acquired sem, sleep 2s\n", msg);
	sleep(2);
	kprintf("  %s: writer is running again\n", msg);
	// wasting CPU cycles
	for(i=0; i<200000; i++){
	}
	kprintf ("  %s: ends execution\n", msg);
	signal (sem);
}

void testsem ()
{
	int     cpu1, cpu2,cpu3;
	int     wr1,wr2;
	int     sem;

	kprintf("\n\n------------------Semaphore - Problem of priority inversion------------------\n\n");
	sem  = screate (1);

	wr1 = create(semwriter1, 2000, 100, "wr1", 2, "writer1", sem);
	wr2 = create(semwriter2, 2000, 20, "wr2", 2, "writer2", sem);
	cpu1 = create(cpuboundproc, 2000, 40, "cpu1", 1, "cpu1");
	cpu2 = create(cpuboundproc, 2000, 70, "cpu2", 1, "cpu2");
	cpu3 = create(cpuboundproc, 2000, 80, "cpu3", 1, "cpu3");


	kprintf("-start writer 2, then sleep 1s. semaphore granted to writer 2 (priority =  20)\n");
	resume(wr2);
	sleep (1);

	kprintf("-start writer 1. writer 1 (priority =  100) blocked on the semaphore\n");
	resume(wr1);

	kprintf("-start cpu1 (priority =  40)\n");
	resume (cpu1);

	kprintf("-start cpu2 (priority = 70)\n");
	resume (cpu2);

	kprintf("-start cpu3 (priority = 80)\n");
	resume (cpu3);

	sleep (10);
}




