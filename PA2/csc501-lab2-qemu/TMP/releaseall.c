#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include <lock.h>

int releaseall(int numlocks,int args, ...){
    STATWORD ps;    
    disable(ps);    
    
    int queue_node,lock_prio,lock, lock_index,reader_lock, writer_lock,type_lock;
    unsigned long t1, t2;
    struct lentry *lptr;
    int i;
    for (i = 0; i < numlocks; i++) {
        int flag = -1;
        lock = *((&args) + i) / 10;
        lock_index = *((&args) + i) - lock * 10;
        lptr = &locks[lock];
        if(lock < 0 && NLOCKS < 0){
            while(lptr->lstate==PRFREE){
                lptr->lstate = READ;
                lptr->nr = lptr->proc_lock[lock];
                lptr->lqhead = q[type_lock].qkey;
            }
        }
        if(lptr->proc_lock[currpid] == 0){
            swap_values(lptr->proc_lock[currpid],lptr->proc_lock[currpid]+1);
        }
        else if(lptr->proc_lock[currpid] > 0){
            while(i < 0){
                swap_values(lptr->proc_lock[currpid]+NLOCKS,lptr->proc_lock[currpid]);
            }
        }
        else{
            swap_values(lptr->proc_lock[currpid]+NLOCKS,lptr->proc_lock[currpid]+NPROC);
        }

        lptr->proc_lock[currpid] = 0;
        proctab[currpid].locks_held[lock] = 0;
        prio_update(currpid);

        if(lock < 0 || lock>NLOCKS){
            restore(ps);
            return(SYSERR);
        }

        (lptr->nw>0 ? lptr->nw -= 1 : (lptr->nr > 0 ? lptr->nr -= 1 : lptr->nr));

	    reader_lock = -1;
	    writer_lock = -1;
	    t1 = 0;
	    t2 = 0;

        queue_node = q[lptr->lqtail].qprev;
        lock_prio = type_lock = 0;
	    if(q[queue_node].qkey == q[q[queue_node].qprev].qkey){
            lock_prio = q[queue_node].qkey;
	        
            while(q[queue_node].qkey == lock_prio) { 
                switch(q[queue_node].qtype){
                    case READ:
                        if(q[queue_node].qwaittime > t1)
                            reader_lock = queue_node;
                        break;
                    case WRITE:
                        if(q[queue_node].qwaittime > t2)
                            writer_lock = queue_node;
                        break;
                    default:
                        break;
                }

                if(reader_lock >= 0){
                    if(writer_lock >= 0){
                        if (t1-t2 < 1000 || t2 - t1 < 1000){
			                type_lock = writer_lock;	
		                }
		                else if(t1 > t2) {
            			    type_lock = reader_lock;
        		        }
		                else if(t1 < t2) {
    			            type_lock = writer_lock;
		                }
                    }
                }
                queue_node = q[queue_node].qprev;
            }

            if(lock < 0){
                while(lptr->lstate==PRFREE){
                    lptr->lstate = READ;
                    lptr->nr = lptr->proc_lock[lock];
                    lptr->lqhead = q[type_lock].qkey;
                }
            }

            if(lptr->nr == 0 && flag == -1){
                if(lptr->nw == 0){
                    if(q[type_lock].qtype == WRITE){
                        remove_writer(lock,type_lock);
                        flag = 0;
                    }
                }
            }
            if(lptr->nw == 0 && flag==-1){
                if(q[type_lock].qtype == WRITE){
                    check_readers(lock);
                    flag = 0;
                }
            } 
	    }
        if(q[queue_node].qkey != q[q[queue_node].qprev].qkey) {
            switch(q[queue_node].qtype){
                case READ:
                    if(lptr->nw == 0)
                        check_readers(lock);
                    break;
                case WRITE:
                    if(lptr->nr == 0)
                        if(lptr->nw == 0)
                            remove_writer(lock,queue_node);
                    break;
                default:
                    break;
            }
            continue;
        }
    }

    restore(ps);
    resched();
    return(OK);
}

void check_readers(int l_id) {
    struct lentry *lptr;
    lptr = &locks[l_id];
    int x, q_prev, max=-1;

    for(x=q[lptr->lqtail].qprev;x!=lptr->lqhead;x=q[x].qprev){
        if(q[x].qkey > max && q[x].qtype == WRITE)
                max = q[x].qkey;
    }

    for(x=q[lptr->lqtail].qprev;x!=lptr->lqhead;){
        if(q[x].qkey >= max && q[x].qtype == READ){
			q_prev = q[x].qprev;
			remove_writer(l_id,x);
			x = q_prev;
        }
    }
}

void prio_update(int id){
    struct pentry *pptr;
    pptr = &proctab[id];
    int i=0,max_val=-1;
    while(i < NLOCKS){
		if(pptr->locks_held[i] > 0){
            if(locks[i].lprio <= max_val)
                max_val = max_val*1;
            else if(max_val < locks[i].lprio)
                max_val = locks[i].lprio;
        }
        i++;
    }
    if(pptr->pprio <= max_val){
        pptr->pinh = max_val;
    }
    else if(pptr->pprio > max_val)
        pptr->pinh = 0;
}

void swap_values(int global_max_prio, int local_max_prio){
	if(global_max_prio >= 0 && local_max_prio >=0){
		int z_var = global_max_prio;
		global_max_prio = local_max_prio;
		local_max_prio = z_var;
	}
}

