// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

#if defined(HW1_LOCKS) || defined(HW1_CONDITIONS) || defined(HW1_ELEVATOR) ||\
    defined(USER_PROGRAM) || defined(FILESYS)
//----------------------------------------------------------------------
// Lock::Lock
//  Initialize a lock, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Lock::Lock(const char* debugName)
{
    name = debugName;
#ifdef USER_PROGRAM
    acquiredPID = -1;
#else
    acquiredThread = NULL;
#endif
    queue = new List;
}

//----------------------------------------------------------------------
// Lock::~Lock()
//  De-allocate lock, when no longer needed. Assume no one
//  is still waiting on the lock.
//----------------------------------------------------------------------

Lock::~Lock()
{
    delete queue;
}

//----------------------------------------------------------------------
// Lock::Acquire()
//  Acquire the lock if free. Otherwise the thread trying to
//  acquire the lock is put to sleep until it free again.
//  So this lock is not a spin lock.
//
//  Some corner cases:
//   A thread owning a lock should not try to acquire it again.
//   Otherwise, it will lead to a deadlock.
//   This specification is similar to std::mutex.
//   We can say that this Lock is not a re-entrant lock.
//----------------------------------------------------------------------

void Lock::Acquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

#ifdef USER_PROGRAM
    while(acquiredPID != -1)
#else
    while (acquiredThread != NULL)
#endif
    {
        queue->Append((void *) currentThread);  // lock is BUSY
        currentThread->Sleep();                 // so put to sleep
    }
#ifdef USER_PROGRAM
    acquiredPID = currentThread->space->pcb->GetPID();
#else
    acquiredThread = currentThread;             // lock available
#endif

    (void) interrupt->SetLevel(oldLevel);  // ignore output - so cast to void
}

//----------------------------------------------------------------------
// Lock::Release()
//  Release the lock if called by the lock's owner. If any other thread
//  that doesn't hold the lock tries to release the lock, throw error.
//----------------------------------------------------------------------

void Lock::Release()
{
    // release lock only if the thread owning the lock releases it
    // otherwise do nothing
    ASSERT(isHeldByCurrentThread())
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL) scheduler->ReadyToRun(thread);
#ifdef USER_PROGRAM
    acquiredPID = -1;
#else
    acquiredThread = NULL;   // lock is free
#endif

    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread()
//  If the current thread is the thread that acquired the lock,
//  then return true. Otherwise return false.
//----------------------------------------------------------------------

bool Lock::isHeldByCurrentThread()
{
#ifdef USER_PROGRAM
    return (currentThread->space->pcb->GetPID() == acquiredPID);
#else
    return (currentThread == acquiredThread);
#endif
}

#else
// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(const char* debugName) {}
Lock::~Lock() {}
void Lock::Acquire() {}
void Lock::Release() {}
#endif

#if defined(HW1_CONDITIONS) || defined(HW1_ELEVATOR) ||\
    defined(USER_PROGRAM) || defined(FILESYS)
//----------------------------------------------------------------------
// Condition::Condition(const char* debugName)
// 	Initialize a condition variable, so that it can be used for
//  synchronization.
//
//  The purpose of a condition variable is to allow threads to
//  put themselves in a queue until a 'condition' (defined by
//  the state of execution) is met.
//
//  When the 'condition' is satisfied a different thread will wake
//  one of the sleeping threads and thus allow them to continue.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Condition::Condition(const char* debugName)
{
    name = debugName;
    queue = new List;
}

//----------------------------------------------------------------------
// Condition::~Condition()
//  De-allocate condition when no longer needed. Assume no one is
//  still waiting on the condition.
//----------------------------------------------------------------------

Condition::~Condition()
{
    delete queue;
}

//----------------------------------------------------------------------
// Condition::Wait(Lock* conditionLock)
//  This condition variable is similar in specification to the
//  std::condition_variable. A thread should only call wait when the
//  lock passed to the wait function is owned by the same thread.
//  Otherwise its undefined behaviour - in this specific implementation
//  an error is thrown
//
//  The following are the steps in wait:
//   a) Atomically - Release the lock and put the calling thread
//      the wait to sleep
//   b) Reacquire the same lock before returning to the calling thread
//
//  "conditionLock" is the lock associated with the 'condition'
//----------------------------------------------------------------------

void Condition::Wait(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread())
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // release lock and sleep thread
    conditionLock->Release();
    queue->Append((void *)currentThread);
    currentThread->Sleep();

    (void) interrupt->SetLevel(oldLevel);

    // re-acquire lock
    conditionLock->Acquire();
}

//----------------------------------------------------------------------
// Condition::Signal(Lock* conditionLock)
//  Wake up a thread waiting on the condition and put it in ready into
//  the ready to run queue. Of course, this thread will also be removed
//  from waiting queue maintained by the condition.
//
//  Signalling is done by a currently running thread if it sees that
//  the state of the condition is changed. Of course, the change in the
//  state of the condition doesn't guarantee that the woken up thread,
//  when it runs, sees the condition state it desires.
//
//  A thread can only call signal if it owns the lock being passed to
//  the signal function. Otherwise the behaviour is undefined similar
//  to the specification of std::condition_variable (in this specific
//  implementation an error is thrown)
//
//  "conditionLock" is the lock associated with the 'condition'
//----------------------------------------------------------------------

void Condition::Signal(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread())
    Thread* thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL) scheduler->ReadyToRun(thread);

    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Condition::Broadcast(Lock* conditionLock)
//  Signal all the waiting threads. Of course only a thread holding the
//  conditon lock should call broadcast. Otherwise, the behaviour is
//  undefined and in this specific implementation, it throws an error.
//
//  "conditionLock" is the lock associated with the 'condition'
//----------------------------------------------------------------------

void Condition::Broadcast(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread())
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // signal all the waiting threads
    while(!queue->IsEmpty())
    {
        Thread* thread;
        thread = (Thread *)queue->Remove();
        if(thread != NULL) scheduler->ReadyToRun(thread);
    }

    (void) interrupt->SetLevel(oldLevel);
}

#else
Condition::Condition(const char* debugName) { }
Condition::~Condition() { }
void Condition::Wait(Lock* conditionLock) { ASSERT(FALSE); }
void Condition::Signal(Lock* conditionLock) { }
void Condition::Broadcast(Lock* conditionLock) { }
#endif
