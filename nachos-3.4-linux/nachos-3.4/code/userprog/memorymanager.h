#ifndef MEMORY_H
#define MEMORY_H

#include "bitmap.h"
#include "synch.h"

class MemoryManager {

    public:
        MemoryManager();
        ~MemoryManager();

        int AllocatePage();
        int DeallocatePage(int which);
        unsigned int GetFreePageCount();

    private:
        BitMap *bitmap;
        Lock *mmLock;
};



#endif // MEMORY_H