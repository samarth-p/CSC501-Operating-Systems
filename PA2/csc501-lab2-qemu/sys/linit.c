#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

struct lentry locks[NLOCKS];
int nextlock;
int locks_traverse;

void linit(){
    // locks_traverse = 0;
    struct lentry *lptr;
    int i=0, j=0;
    nextlock = NLOCKS - 1;

	for (i=0; i < NLOCKS; i++) {
		lptr = &locks[i];
		lptr->lstate = LFREE;
		lptr->lprio = -1;
		lptr->nr = 0;
		lptr->nw = 0;
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
		for (j=0; j < NPROC; j++)
            lptr->proc_lock[j] = 0;
    }
}
