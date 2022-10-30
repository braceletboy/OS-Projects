// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

#if defined(HW1_SEMAPHORES) || defined(HW1_LOCKS) || defined(HW1_CONDITIONS) \
    || defined(HW1_ELEVATOR)
#include "synch.h"
#endif

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

#if defined(HW1_SEMAPHORES)
int SharedVariable;  // global int variables are Zero-initialized
Semaphore *binary_semaphore = new Semaphore("semaphore-sharedVariable", 1);

int nthreads;
int completed_threads;  // global int variables are Zero-initialized
Semaphore *binary_semaphore_ct = new Semaphore("semaphore-completed_threads", 1);
Semaphore *barrier = new Semaphore("barrier", 0);

void
SimpleThread(int which)
{
    int num, val;
    for(num = 0; num < 5; num++){
        binary_semaphore->P();  // wait for resource to be freed
        val = SharedVariable;
        printf("*** thread %d sees value %d\n", which, val);
        currentThread->Yield();
        SharedVariable = val + 1;
        binary_semaphore->V();  // free resource
        currentThread->Yield();
    }

    // check for completion of all threads
    binary_semaphore_ct->P();
    completed_threads++;
    binary_semaphore_ct->V();
    if(completed_threads < nthreads){barrier->P();}
    barrier->V();

    // print the final value
    val = SharedVariable;
    printf("Thread %d sees final value %d\n", which, val);
}
#elif defined(HW1_LOCKS) || defined(HW1_CONDITIONS)
int SharedVariable;  // global int variables are Zero-initialized
Lock *mutex = new Lock("mutex-SharedVariable");

int nthreads;
int completed_threads;
Lock *mutex_ct = new Lock("mutex-completed_threads");

#ifdef HW1_CONDITIONS
Condition *condvar_ct = new Condition("condition variable-completed_threads");
#endif

void
SimpleThread(int which)
{
    int num, val;
    for(num = 0; num < 5; num++){
        mutex->Acquire();
        val = SharedVariable;
        printf("*** thread %d sees value %d\n", which, val);
        currentThread->Yield();
        SharedVariable = val + 1;
        mutex->Release();
        currentThread->Yield();
    }

#ifdef HW1_CONDITIONS
    // check for completion of all threads
    mutex_ct->Acquire();
    completed_threads++;

    // blocking barrier using a condition variable
    while(completed_threads < nthreads) { condvar_ct->Wait(mutex_ct); }
    condvar_ct->Broadcast(mutex_ct);
    mutex_ct->Release();
#else
    // check for completion of all threads
    mutex_ct->Acquire();
    completed_threads++;
    mutex_ct->Release();

    // busy-waiting barrier without using a semaphore
    while(completed_threads < nthreads) { currentThread->Yield(); }
#endif

    // print the final value
    val = SharedVariable;
    printf("Thread %d sees final value %d\n", which, val);
}
#elif defined(HW1_MODIFIED_SIMPLETHREAD)
int SharedVariable;  // global int variables are Zero-initialized
void
SimpleThread(int which)
{
    int num, val;
    for(num = 0; num < 5; num++){
        val = SharedVariable;
        printf("*** thread %d sees value %d\n", which, val);
        currentThread->Yield();
        SharedVariable = val+1;
        currentThread->Yield();
    }
    val = SharedVariable;
    printf("Thread %d sees final value %d\n", which, val);
}
#else
void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}
#endif

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

#if defined(HW1_MULTIPLE_THREADS) || defined(HW1_SEMAPHORES) || \
		defined(HW1_LOCKS) || defined(HW1_CONDITIONS)
void
ThreadTest(int num_child_threads)
{
#if defined(HW1_SEMAPHORES) || defined(HW1_LOCKS) || defined(HW1_CONDITIONS)
    nthreads = num_child_threads;
#endif
    DEBUG('h', "Entering homework ThreadTest");

    Thread** thread_tracker = new Thread*[num_child_threads];
    for(int idx = 1; idx < num_child_threads; idx++)
    {
        DEBUG('h', "Forking thread %d\n", idx);
        char = new threadName[100];
        sprintf(threadName, "Forked Thread %d", idx);
        thread_tracker[idx] = new Thread(threadName);
        thread_tracker[idx]->Fork(SimpleThread, idx);
    }
    SimpleThread(0);
}
#else
void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}
#endif