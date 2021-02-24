/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

void setschedclass(int sched_class){
    sched_set = sched_class;
}

int getschedclass(){
    return sched_set;
}

int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	double randnum;
	int expschprocess, expprevprocess,counter, lnxcurprocess, lnxschprocess, maxgoodness,i,epochend, firstinsertRR;

	if (sched_set == 1){
		//If the state of current process is PRREADY, then inserting it to the ready queue 
		optr= &proctab[currpid];
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		//calculating the random number
		randnum = expdev(0.1);

		//selecting a process to schedule that is just greater than random number
		expschprocess = q[rdytail].qprev;
		while (randnum < q[expschprocess].qkey && expschprocess != rdyhead) {
			expschprocess = q[expschprocess].qprev;
		}
		if (expschprocess != q[rdytail].qprev){
			expschprocess = q[expschprocess].qnext;
		}

		//selecting first inserted process for same priority (Round Robin) 
		while (q[expschprocess].qnext != rdytail  && q[q[expschprocess].qnext].qkey == q[expschprocess].qkey){
			expschprocess = q[expschprocess].qnext;
		}
		
		currpid = expschprocess;
		nptr = &proctab[currpid];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		dequeue(currpid);
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		return(OK);
	}
	else if (sched_set==2){
		//If the state of current process is PRREADY, then inserting it to the ready queue 
		optr= &proctab[currpid];
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}
		
		counter = preempt;
		if (counter <= 0) {
			proctab[currpid].goodness = 0;
			proctab[currpid].quantum = 0;
		}
		else {
			proctab[currpid].quantum = counter;
			proctab[currpid].goodness = counter + proctab[currpid].pprio;
		}

		//checking if the epoch is ended or not
		epochend = 1;
		lnxcurprocess = q[rdytail].qprev;
		while (lnxcurprocess != rdyhead){
			if (proctab[lnxcurprocess].quantum !=0 || proctab[lnxcurprocess].pstate != PRREADY ){
				epochend = 0;
				break;
			}
			lnxcurprocess = q[lnxcurprocess].qprev;
		}

		//if the epoch is ended then calcutating the goodness and quantum value for every process
		if (epochend == 1){
			for (i=0;i<NPROC;i++){
				proctab[i].goodness = proctab[i].quantum + proctab[i].pprio;
				proctab[i].quantum = proctab[i].pprio;
			}
		}

		//selecting a process to be schedule based on the goodness value of the process
		lnxcurprocess = q[rdytail].qprev;
		maxgoodness = 0;
		firstinsertRR = MAXINT;
		while (lnxcurprocess != rdyhead){
			if (proctab[lnxcurprocess].goodness >= maxgoodness && proctab[lnxcurprocess].pstate == PRREADY) {
				//checks if the more than one process has same goodness value then, select the which one was inserted first in the ready queue.
				//proctab[lnxcurprocess].roundrobin keeps track of when was the process inserted in the queue
				if (proctab[lnxcurprocess].goodness == maxgoodness && proctab[lnxcurprocess].roundrobin<firstinsertRR){
					maxgoodness = proctab[lnxcurprocess].goodness;
					firstinsertRR = proctab[lnxcurprocess].roundrobin;
				}
				if (proctab[lnxcurprocess].goodness != maxgoodness) {
					maxgoodness = proctab[lnxcurprocess].goodness;
					firstinsertRR = proctab[lnxcurprocess].roundrobin;
				}
				lnxschprocess = lnxcurprocess;
			}
			lnxcurprocess = q[lnxcurprocess].qprev;
		}

		//Handling null process
		if (lnxcurprocess == q[rdyhead].qnext){
			currpid = 0;
		}else{
			currpid = lnxschprocess;
		}

		nptr = &proctab[currpid];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		dequeue(currpid);
		preempt = nptr->quantum;		/* reset preemption counter	*/
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		return(OK);
	}
	else{
		/* no switch needed if current process priority higher than next*/

		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
		
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
	#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
	#endif
		
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		
		/* The OLD process returns here when resumed. */
		return OK;
	}

	
}
