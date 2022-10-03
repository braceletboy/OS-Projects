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
    nthreads = num_child_threads + 1;
#endif
    DEBUG('h', "Entering homework ThreadTest");

    Thread** thread_tracker = new Thread*[num_child_threads];
    for(int idx = 0; idx < num_child_threads; idx++)
    {
        DEBUG('h', "Forking thread %d\n", idx);
        char threadName[100];
        sprintf(threadName, "Forked Thread %d", idx);
        thread_tracker[idx] = new Thread(threadName);
        thread_tracker[idx]->Fork(SimpleThread, idx+1);
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

#ifdef HW1_ELEVATOR
int elevatorFloor = 1;       // always tracks elevator's current floor
bool goingUp = true;         // always tracks if elevator is going up or down
bool elevatorOpen = false;   // always tracks if the elevator is open
int elevatorOccupancy = 0;   // always tracks current occupancy (total is 5)

// mutex and conditional variable for the above five shared variables
Lock *mutexElevator = new Lock("elevator-mutex");
Condition *condvarElevator = new Condition("elevator-condition");

// number of persons actively inside the system
// this includes the ones waiting and the ones inside the elevator
int personsActive = 0;

// mutex and conditional variable for elevator wakeup
Lock *mutexWakeup = new Lock("wakeup-mutex");
Condition *condvarWakeup = new Condition("wakeup-condition");


//-----------------------------------------------------------------------
// elevator_subroutine
//  The job of the elevator subroutine is to make the elevator go up and
//  down the building.
//-----------------------------------------------------------------------
void elevator_subroutine(int numFloors)
{
    DEBUG('h', "Elevator started on floor %d\n", elevatorFloor);
    while(true)
    {
        // go to sleep if there are no persons to serve
        mutexWakeup->Acquire();
        while(personsActive == 0) condvarWakeup->Wait(mutexWakeup);
        mutexWakeup->Release();

        mutexElevator->Acquire();
        // decide whether to go up or down
        if (elevatorFloor == numFloors)
        {
            goingUp = false;
            DEBUG('h', "Elevator will go down now\n");
        }
        if (elevatorFloor == 1)
        {
            goingUp = true;
            DEBUG('h', "Elevator will go up now\n");
        }

        // loop for 50 ticks - going to next floor
        for(int i=0; i<50;)
        {
            // the elevator is now closed - its moving
            if (i == 0)
            {
                elevatorOpen = false;
                DEBUG('h', "Elevator is now closed\n");
            }

            i++;  // spend one tick

            // elevator is on next floor - its open
            if (i == 50)
            {
                elevatorOpen = true;
                DEBUG('h', "Elevator is now open\n");
            }
        }

        // elevator has reached the next floor
        if (goingUp)
        {
            elevatorFloor++;
            printf("Elevator arrives on floor %d\n", elevatorFloor);
        }
        else
        {
            elevatorFloor--;
            printf("Elevator arrives on floor %d\n", elevatorFloor);
        }

        // announce you have reached the next floor to all persons
        DEBUG('h', "All persons told about elevator arrival\n");
        condvarElevator->Broadcast(mutexElevator);
        mutexElevator->Release();
        currentThread->Yield();
    }
}

struct PersonAttributes
{
    int which;
    int toFloor;
    int atFloor;
};


void person_subroutine(int arg)
{
    PersonAttributes *struct_ptr = reinterpret_cast<PersonAttributes*>(arg);
    int which = struct_ptr->which;
    int toFloor = struct_ptr->toFloor;
    int atFloor = struct_ptr->atFloor;

    DEBUG('h', "Person %d has entered the system\n", which);
    printf("Person %d wants to go to floor %d from floor %d\n",
           which, toFloor, atFloor);

    // wakeup the elevator if it's sleeping
    // a person has entered the system
    mutexWakeup->Acquire();
    // if(personsActive == 0) goingUp = (atFloor > elevatorFloor);
    personsActive++;
    condvarWakeup->Signal(mutexWakeup);
    mutexWakeup->Release();

    // wait till the elevator arrives at the floor and the occupancy is not
    // more than 4 (only 5 can fit in the elevator)
    mutexElevator->Acquire();
    while(
        (
            !(elevatorOpen && goingUp && (toFloor > atFloor)) &&
            !(elevatorOpen && !goingUp && (toFloor < atFloor))
        ) ||
        !(elevatorFloor == atFloor) ||
        !(elevatorOccupancy < 5)
    ) condvarElevator->Wait(mutexElevator);

    // person gets into elevator
    elevatorOccupancy++;
    printf("Person %d got into the elevator\n", which);

    mutexElevator->Release();

    // wait till the elevator reaches the destination floor
    mutexElevator->Acquire();
    while(
        !(elevatorFloor == toFloor) || !elevatorOpen
    ) condvarElevator->Wait(mutexElevator);

    // person gets out of the elevator
    elevatorOccupancy--;
    printf("Person %d got out of the elevator\n", which);
    mutexElevator->Release();

    // person leaves the system
    mutexWakeup->Acquire();
    DEBUG('h', "Person %d has left the system\n", which);
    personsActive--;
    mutexWakeup->Release();
}

Thread* Elevator(int numFloors)
{
    Thread *elevatorThread = new Thread("Elevator");

    elevatorThread->Fork(elevator_subroutine, numFloors);
    return elevatorThread;
}

Thread* ArrivingFromGoingTo(int which, int atFloor, int toFloor)
{
    char threadName[100];
    sprintf(threadName, "Person Thread %d", which);
    Thread *personThread = new Thread(threadName);

    PersonAttributes *struct_ptr = new PersonAttributes;
    struct_ptr->which = which;
    struct_ptr->atFloor = atFloor;
    struct_ptr->toFloor = toFloor;
    int arg = reinterpret_cast<int>(struct_ptr);

    personThread->Fork(person_subroutine, arg);
    return personThread;
}
#endif

