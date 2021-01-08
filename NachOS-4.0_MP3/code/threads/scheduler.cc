// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

//MP3

int SJFCompare(Thread *a, Thread *b){
    //cout << "SJFCompare"<< endl;
    if(a->getApproximated_CPUBurst() == b->getApproximated_CPUBurst() ){
        //if(a->getID() > b->getID()) return 1;
        //else if(a->getID() < b->getID()) return-1;
        return 0;
    }
    return a->getApproximated_CPUBurst() > b->getApproximated_CPUBurst()? 1: -1;
}



int  PriorityCompare(Thread *a, Thread *b){
    //cout << "PriorityCompare" << endl;
    if(a->getPriority() == b->getPriority()){
        //if(a->getID() > b->getID()) return 1;
        //else if(a->getID() < b->getID()) return-1;
        return 0;
    }
    return a->getPriority() < b->getPriority()? 1: -1;
}


//MP3
Scheduler::Scheduler()
{ 
    //readyList = new List<Thread *>; 

    //MP3
    //cout << "scheduler::scheduler()"<<endl;
    readyList1 = new SortedList<Thread *>(SJFCompare);
    readyList2 = new SortedList<Thread *>(PriorityCompare);
    readyList3 = new List<Thread *>;
    //MP3
    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
 //   delete readyList;
//MP3
    delete readyList1;
    delete readyList2;
    delete readyList3;
 //MP3
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------
//MP3
bool
Scheduler::CheckPreemptive(){
    if( !readyList1->IsEmpty() ){
	Thread *first = readyList1->Front();
	Thread *now = kernel->currentThread;
	if( first->getApproximated_CPUBurst() < now->getApproximated_CPUBurst()){
	    return true;
	}
	else {
	    return false;
	}
    }
    else {
	return false;
    }

}

//MP3
void
Scheduler::ReadyToRun (Thread *thread)
{
    //cout << "Scheduler::ReadyToRun"<<endl;
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
    //cout << "Putting thread on ready list: " << thread->getID() << endl ;//MP3
    //cout << "Thread Priority: " << thread->getPriority() << endl;//MP3
    //cout << "Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is put in ReadyToRun with Priority :[" << thread->getPriority() << "]" << endl;
    thread->setStatus(READY);
   // readyList->Append(thread);//MP3
//MP3

    thread->setStatus(READY);
    //thread->setWaitingTime(0);
    int priority = thread->getPriority();
    if(priority >= 100 && priority < 150){
        DEBUG(dbgCPU, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[1]");
        readyList1->Insert(thread);
    }
    else if(priority >= 50 && priority < 100){
        DEBUG(dbgCPU, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[2]");
        readyList2->Insert(thread);
    }
    else {
        DEBUG(dbgCPU, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[3]");
        readyList3->Append(thread);
    }


//MP3
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    //cout << "scheduler::FindNextToRun" << endl;
    ASSERT(kernel->interrupt->getLevel() == IntOff);
/*
    if (readyList->IsEmpty()) {
		return NULL;
    } else {
    	return readyList->RemoveFront();
    }
*/
//MP3
//    cout << "Tick [" << kernel->stats->totalTicks << "]: In FindNextToRun " << endl;

    Thread *thread = NULL;
    if (!readyList1->IsEmpty()) {
        thread = readyList1->RemoveFront();
        //thread->reset_age();
	DEBUG(dbgCPU, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[1]" );
	return thread;
   	//return readyList1->RemoveFront();
    } 
    else if (!readyList2->IsEmpty()) {
        thread = readyList2->RemoveFront();
        //thread->reset_age();
        DEBUG(dbgCPU, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[2]" );
        return thread;
	//return readyList2->RemoveFront();
    } 
    else if (!readyList3->IsEmpty()) {
        thread = readyList3->RemoveFront();
        //thread->reset_age();
        DEBUG(dbgCPU, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[3]" );
        return thread;
	//return readyList3->RemoveFront();
    } 

    return NULL;

//MP3
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    //cout << "Scheduler::Run" <<endl;
    Thread *oldThread = kernel->currentThread;
    //MP3
    //nextThread->setStartTime(kernel->stats->totalTicks);
    //

    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	 toBeDestroyed = oldThread;
    }
    
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	oldThread->space->SaveState();
    }
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    //SWITCH(oldThread, nextThread);
    //MP3
    DEBUG(dbgCPU,"[E] Tick [" << kernel->stats->totalTicks <<"]: Thread [" << nextThread->getID() << "] is now selected for execution" << ", thread [" << oldThread->getID() << "] is replaced, and it has executed [" << kernel->stats->totalTicks - oldThread->getStartTime()<< "] ticks" );
    
    nextThread->setStartTime(kernel->stats->totalTicks );
    
    SWITCH(oldThread, nextThread);

    //MP3

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
	toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    readyList->Apply(ThreadPrint);
}


//MP3
void 
Scheduler::N_Aging(){
    SortedList<Thread *> *new_readyList1 = new SortedList<Thread *> (SJFCompare);
    SortedList<Thread *> *new_readyList2 = new SortedList<Thread *> (PriorityCompare);
    List<Thread *> *new_readyList3 = new List<Thread *>;
    
    while(!readyList1->IsEmpty()){
	Thread *t = readyList1->RemoveFront();
	t->aging();
	new_readyList1->Insert(t);
    }
    while(!readyList2->IsEmpty()){
        Thread *t = readyList2->RemoveFront();
        t->aging();
	if(t->getPriority() >= 100 ){
            DEBUG(dbgCPU, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] is removed from queue L[2]" );
            DEBUG(dbgCPU, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] is inserted into queue L[1]" );
	    new_readyList1->Insert(t);
	}
	else{
	    new_readyList2->Insert(t);
	}
    }
    while(!readyList3->IsEmpty()){
        Thread *t = readyList3->RemoveFront();
        t->aging();
        if(t->getPriority() >= 50 ){
            DEBUG(dbgCPU, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] is removed from queue L[3]" );
            DEBUG(dbgCPU, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] is inserted into queue L[2]" );
            new_readyList2->Insert(t);
        }
        else{
            new_readyList3->Append(t);
        }
    }
    delete readyList1;
    delete readyList2;
    delete readyList3;

    readyList1 = new_readyList1;
    readyList2 = new_readyList2;
    readyList3 = new_readyList3;

}

void
Scheduler::Aging() {
    ListIterator<Thread *> *it1 = new ListIterator<Thread *>(readyList1);
    ListIterator<Thread *> *it2 = new ListIterator<Thread *>(readyList2);
    ListIterator<Thread *> *it3 = new ListIterator<Thread *>(readyList3);

    for (; !it1->IsDone(); it1->Next()) {
       	it1->Item()->setWaitingTime(it1->Item()->getWaitingTime() + 100);
	cout << "Thread [" << it1->Item()->getID() <<  "] **waitingtime:" << it1->Item()->getWaitingTime() << endl;
	//int count_time = kernel->stats->totalTicks - it1->Item()->getWaitingTime();
	//count_time /= 1500;
	//it1->Item()->setWaitingTime( kernel->stats->totalTicks );
	//if(count_time > 0){
       	if (it1->Item()->getWaitingTime() >= 1500) {//getWaitingTime()
            int oldPriority = it1->Item()->getPriority();
            it1->Item()->setPriority(oldPriority + 10);
            it1->Item()->setWaitingTime(0);//0
            //count_time = 1;
	    //it1->Item()->setPriority(oldPriority + count_time * 10);
            //it1->Item()->setWaitingTime( kernel->stats->totalTicks );
            int newPriority = it1->Item()->getPriority();
            DEBUG(dbgCPU, "Tick [" << kernel->stats->totalTicks <<"]: Thread [" << it1->Item()->getID() << "] changes its priority from [" << oldPriority << "] to [" << newPriority <<"]" << endl);
        }
    }

    for (; !it2->IsDone(); it2->Next()) {
       	it2->Item()->setWaitingTime(it2->Item()->getWaitingTime() + 100);
       	cout << "Thread [" << it2->Item()->getID() <<  "] **waitingtime:" << it2->Item()->getWaitingTime() << endl;
	//int count_time = kernel->stats->totalTicks - it2->Item()->getWaitingTime();
	//count_time /= 1500;
	//it2->Item()->setWaitingTime(kernel->stats->totalTicks);
	//if(count_time > 0){
	if (it2->Item()->getWaitingTime() >= 1500) {
            int oldPriority = it2->Item()->getPriority();
            it2->Item()->setPriority(oldPriority +  10);
            it2->Item()->setWaitingTime( 0 );
            //count_time = 1;
 	    //it2->Item()->setPriority(oldPriority + count_time * 10);
            //it2->Item()->setWaitingTime( kernel->stats->totalTicks );
            int newPriority = it2->Item()->getPriority();
            DEBUG(dbgCPU, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << it2->Item()->getID() << "] changes its priority from [" << oldPriority << "] to [" << newPriority <<"]" << endl);
            if (newPriority >= 100) {
                readyList2->Remove(it2->Item());
                DEBUG(dbgCPU, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << it2->Item()->getID() << "] is removed from queue L[2]" << endl);
                ReadyToRun(it2->Item());
            }
        }
    }
    for (; !it3->IsDone(); it3->Next()) {
       	it3->Item()->setWaitingTime(it3->Item()->getWaitingTime() + 100);
       	cout << "Thread [" << it3->Item()->getID() <<  "] **waitingtime:" << it3->Item()->getWaitingTime() << endl;
        //int count_time = kernel->stats->totalTicks - it3->Item()->getWaitingTime();
	//count_time /= 1500;
	//it3->Item()->setWaitingTime( kernel->stats->totalTicks );
	//if(count_time > 0){
	if (it3->Item()->getWaitingTime() >= 1500) {
            int oldPriority = it3->Item()->getPriority();
            it3->Item()->setPriority(oldPriority + 10);
	    it3->Item()->setWaitingTime(0);
	    //count_time = 1;
	    //it3->Item()->setPriority(oldPriority + count_time * 10);
            //it3->Item()->setWaitingTime( kernel->stats->totalTicks );
            int newPriority = it3->Item()->getPriority();
            DEBUG(dbgCPU, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << it3->Item()->getID() << "] changes its priority from [" << oldPriority << "] to [" << newPriority <<"]" << endl);
            if (newPriority >= 50) {
                readyList3->Remove(it3->Item());
                DEBUG(dbgCPU, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << it3->Item()->getID() << "] is removed from queue L[3]" << endl);
                ReadyToRun(it3->Item());
            }
        }
    }
}



//MP3
