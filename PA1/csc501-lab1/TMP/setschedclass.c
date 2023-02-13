#include<lab1.h>

int sch = AGESCHED;

void setschedclass(int sched_class) {
	sch = sched_class;
}

int getschedclass() {
	return sch;
}