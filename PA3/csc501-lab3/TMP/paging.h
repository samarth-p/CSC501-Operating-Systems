/* paging.h */

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			     */
  unsigned int pt_offset : 10;		/* page table offset		  */
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
  int bs_status;			/* BSM_MAPPED or BSM_UNMAPPED	*/
  int bs_pid[NPROC];			/* process ids using this slot   */
  int bs_vpno[NPROC];			/* starting virtual page numbers for all processes sharing the BS */
  int bs_npages;			/* number of pages in the store */
  int bs_ispriv;			/* BSM_SHARED OR BSM_PRIVATE	*/
  int bs_numprocs;			/* count of process that have acquire this bs  */
  int bs_sem;				/* semaphore mechanism ?	*/
} bs_map_t;

typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no*/
  int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;	
  int fr_acc;
  int fr_wr;
}fr_map_t;

// FIFO Queue for Page replacement
struct fifo_queue{
	int key;
	struct fifo_queue *next;
};

#define queue_isempty(list) (list == NULL)
#define queue_firstitem(list) (list->key)

void insert_into_queue(struct fifo_queue **, int);
int dequeue_fifo(struct fifo_queue **, int);

extern int page_replace_policy;
extern int debugging_mode;

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
extern int 	gpt_tab[];
extern struct fifo_queue *fifohead;
extern int sc_curr;
extern int num_sc;

SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);

/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

/* paging helper functions */
int get_virt_page_num(virt_addr_t, int *);
int get_virt_addr(unsigned long, virt_addr_t *);
int create_page_dir(int , int *);
void init_page_dir(int);
void initialize_pt(int);
void print_debug_frame_tab();
void delete_pt_entry(int);


#define NBPG		4096	/* number of bytes per page	*/
#define NPBS		128	/* number of pages per BS	*/
#define FRAME0		1024	/* zero-th frame		*/
#define NFRAMES 	1024	/* number of frames		*/
// #define NFRAMES		13	/* to test page replacement	*/
#define NBS		16	/* number of backing stores	*/

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1

#define BSM_SHARED	0
#define BSM_PRIVATE	1

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define SC 3
#define FIFO 4

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00080000

#define WR_NORMAL 1
#define WR_FORCED 2

#define	isbadbs(x)	(x < 0 || x >= NBS)
#define isbadvpno(x)	(x < 4096)
