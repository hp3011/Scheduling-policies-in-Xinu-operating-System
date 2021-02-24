#define EXPDISTSCHED 1
#define LINUXSCHED 2

extern int sched_set;
extern int lnxschRR;

extern void setschedclass(int sched_class);
extern int getschedclass();