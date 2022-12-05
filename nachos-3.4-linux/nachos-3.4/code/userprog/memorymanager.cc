#include "memorymanager.h"
#include "machine.h"


//----------------------------------------------------------------------
// MemoryManager::MemoryManager
//  Create a memory manager for the Operating system to use. The job of
//  the memory manager is to manage, allocate and deallocate the
//  physical memory in the system. It does this in a synchronized
//  fashion.
//
//  The memory manager uses a bitmap internally to keep track of the
//  pages in the physical memory. Also, it does the synchronization
//  using a lock and the memory manager is implemented like a monitor
//  (synchronization primitive) whose methods different processes can
//  use.
//----------------------------------------------------------------------

MemoryManager::MemoryManager() {

    mmLock = new Semaphore("memory manager lock", 1);
    bitmap = new BitMap(NumPhysPages);

}

//----------------------------------------------------------------------
// MemoryManager::~MemoryManager
//  Destructor. Delete the bitmap that was being used for tracking the
//  pages.
//----------------------------------------------------------------------

MemoryManager::~MemoryManager() {

    delete bitmap;
    delete mmLock;

}

//----------------------------------------------------------------------
// MemoryManager::AllocatePage
//  Allocate a single page to the address space of the process that
//  is requesting the allocation.
//
//  Returns the page number
//----------------------------------------------------------------------

int MemoryManager::AllocatePage() {

    // allocate page in a synchronized fashion
    mmLock->P();
    int page_number = bitmap->Find();
    ASSERT(page_number != -1);  // TODO - don't use assert
    mmLock->V();

    return page_number;

}

//----------------------------------------------------------------------
// MemoryManager::DeallocatePage
//  Deallocate the single page whose page number was given.
//
//  "which" is the page number
//
//  Returns 0 if the page was cleared otherwise -1
//----------------------------------------------------------------------

int MemoryManager::DeallocatePage(int which) {

    // trying to deallocate a page that was not allocated
    if(bitmap->Test(which) == false) return -1;
    else {
        // deallocate the page in a synchronized way
        mmLock->P();
        bitmap->Clear(which);
        mmLock->V();
        return 0;
    }

}

//----------------------------------------------------------------------
// MemoryManager::GetFreePageCount
//  Return the number of free pages in the physical memory.
//----------------------------------------------------------------------

unsigned int MemoryManager::GetFreePageCount() {

    return bitmap->NumClear();

}

