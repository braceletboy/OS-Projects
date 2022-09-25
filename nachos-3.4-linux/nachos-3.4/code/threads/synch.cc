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

#ifdef HW1_LOCKS
//----------------------------------------------------------------------
// Lock::Lock
//  Initialize a lock, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Lock::Lock(const char* debugName)
{
    name = debugName;
    acquiredThread = NULL;
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

    while (acquiredThread != NULL)
    {
        queue->Append((void *) currentThread);  // lock is BUSY
        currentThread->Sleep();                 // so put to sleep
    }
    acquiredThread = currentThread;             // lock available

    (void) interrupt->SetLevel(oldLevel);  // ignore output - so cast to void
}

//----------------------------------------------------------------------
// Lock::Release()
//  Release the lock if called by the lock's owner. If any other thread
//  that doesn't hold the lock tries to release the lock, nothing happens.
//----------------------------------------------------------------------

void Lock::Release()
{
    // release lock only if the thread owning the lock releases it
    // otherwise do nothing
    if (isHeldByCurrentThread())
    {
        Thread *thread;
        IntStatus oldLevel = interrupt->SetLevel(IntOff);

        thread = (Thread *)queue->Remove();
        if (thread != NULL) scheduler->ReadyToRun(thread);
        acquiredThread = NULL;   // lock is free

        (void) interrupt->SetLevel(oldLevel);
    }
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread()
//  If the current thread is the thread that acquired the lock,
//  then return true. Otherwise return false.
//----------------------------------------------------------------------

bool Lock::isHeldByCurrentThread()
{
    return (currentThread == acquiredThread);
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

Condition::Condition(const char* debugName) { }
Condition::~Condition() { }
void Condition::Wait(Lock* conditionLock) { ASSERT(FALSE); }
void Condition::Signal(Lock* conditionLock) { }
void Condition::Broadcast(Lock* conditionLock) { }
