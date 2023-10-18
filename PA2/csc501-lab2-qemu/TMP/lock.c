#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <proc.h>
#include <lock.h>

int lock(int ldes1, int type, int priority) {

    STATWORD ps;    
    disable(ps);
    // int count = 10;
    struct lentry *lptr;
    struct pentry *pptr;
    
    int lock = (int)(ldes1/10);
    lptr = &locks[lock];

	if (isbadlock(lock) || (lptr->lstate==LFREE)) {
		restore(ps);
		return(SYSERR);
	}

    int i = 0;
    int wait = 0;

    if(lptr->nr<0 || lptr->nw<0){
        restore(ps);
        return(SYSERR);
    }

    if(lptr->nr == 0){
        if(lptr->nw == 0){
            wait = 0;
        }
        else if(lptr->nw!=0){
            wait = 1;
        }
    }
    if(lptr->nr != 0){
        if(lptr->nw == 0){
            if(type == WRITE){
                wait = 1;
            }
            else if(type == READ){
                int lock_desc = q[lptr->lqtail].qprev;
                while(priority < q[lock_desc].qkey){
                    if(q[lock_desc].qtype == WRITE){
                        wait = 1;
                    }
                    lock_desc = q[lock_desc].qprev;
                }
            }
        }
    }
    int check=OK;
    switch(wait){
        case 0:
            lptr->proc_lock[currpid] = 1;
            proctab[currpid].locks_held[lock] = 1;
            prio_update(currpid);
            switch(type){
                case READ:
                    lptr->nr++;
                    break;
                case WRITE:
                    lptr->nw++;
                    break;
                default:
                    break;    
            }
            restore(ps);
            return(OK);
        case 1:
            check = set_lock_values(currpid,priority,type,lock);
            if(check!=OK){
            }
            modify_lock_prio(lock);
            struct lentry *temp_lock;
            temp_lock = &locks[lock];
            i=0;
            while(i < NPROC){
                if(temp_lock->proc_lock[i] > 0)
                    prio_update(i);
                i++;
            }

            resched();
            restore(ps);
            return(pptr->plockret);
        
        default:
            break;
    }
    if(lptr->proc_lock[currpid] == 0){
        swap_values(lptr->proc_lock[currpid],lptr->proc_lock[currpid]+1);
    }
    else if(lptr->proc_lock[currpid] > 0){
        i=0;
        while(i < 0){
            swap_values(lptr->proc_lock[currpid]+NLOCKS,lptr->proc_lock[currpid]);
        }
    }
    else{
        swap_values(lptr->proc_lock[currpid]+NLOCKS,lptr->proc_lock[currpid]+NPROC);
    }


    restore(ps);
    return(OK);
}

int get_max(int lock_id){
    struct lentry *lptr;
    lptr = &locks[lock_id];
    int x = q[lptr->lqtail].qprev,max=0;
    for(x = q[lptr->lqtail].qprev; x!=lptr->lqhead; x = q[x].qprev){
        if(proctab[x].pprio > max){
            max = proctab[x].pprio;
        }
    }
    return max;

}

void modify_lock_prio(int l_id){
    struct lentry *lptr;
    lptr = &locks[l_id];
    lptr->lprio = get_max(l_id);
}

int set_lock_values(int proc_id,int prio,int type,int lock){
    struct pentry *pptr;
    struct lentry *lptr;
    lptr = &locks[lock];
    pptr = &proctab[proc_id];
    pptr->pstate = PROC_LOCK;
    pptr->l_proc = lock;
    pptr->plockret = OK;
    insert(proc_id,lptr->lqhead,prio);
    q[proc_id].qtype = type;
    q[proc_id].qwaittime = ctr1000;
    return(OK);
}

void remove_writer(int l_id, int p) {
    struct lentry *lptr;
    lptr = &locks[l_id];
    lptr->proc_lock[p] = 1;
    proctab[currpid].locks_held[l_id] = 1;
    switch(q[p].qtype){
        case READ:
            lptr->nr += 1;
            break;
        case WRITE:
            lptr->nw += 1;
            break;
        default:
            break;
    }
    modify_lock_prio(l_id);
    int i=0;
    while(i < NPROC){
        if(lptr->proc_lock[i] > 0)
            prio_update(i);
        i++;
    }
    dequeue(p);
    ready(p, RESCHNO);
}
