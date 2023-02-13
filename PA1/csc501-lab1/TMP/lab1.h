#define AGESCHED 1 
#define LINUXSCHED 2

extern int sch;
void setschedclass(int sched_class);
int getschedclass();