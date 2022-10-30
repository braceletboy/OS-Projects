

#include "memorymanager.h"
#include "machine.h"


//----------------------------------------------------------------------
// MemoryManager::MemoryManager
//  Create a memory manager for the Operating system to use. The job of
//  the memory manager is to manage, allocate and deallocate the
//  physical memory in the system.
//
//  The memory manager uses a bitmap internally to keep track of the
//  pages in the physical memory.
//----------------------------------------------------------------------

MemoryManager::MemoryManager() {

    bitmap = new BitMap(NumPhysPages);

}

//----------------------------------------------------------------------
// MemoryManager::~MemoryManager
//  Destructor. Delete the bitmap that was being used for tracking the
//  pages.
//----------------------------------------------------------------------

MemoryManager::~MemoryManager() {

    delete bitmap;

}

//----------------------------------------------------------------------
// MemoryManager::AllocatePage
//  Allocate a single page to the address space of the process that
//  is requesting the allocation.
//
//  Returns the page number
//----------------------------------------------------------------------

int MemoryManager::AllocatePage() {

    return bitmap->Find();

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

    if(bitmap->Test(which) == false) return -1;
    else {
        bitmap->Clear(which);
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

