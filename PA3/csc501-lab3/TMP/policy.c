/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>

extern int page_replace_policy;
extern int debugging_mode;

/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
	STATWORD ps;
	disable(ps);
	
	debugging_mode = 1;
	page_replace_policy = policy;	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
	return page_replace_policy;
}
