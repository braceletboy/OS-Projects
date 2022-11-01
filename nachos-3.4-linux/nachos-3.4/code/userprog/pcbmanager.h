#ifndef PCBMANAGER_H
#define PCBMANAGER_H

#include "bitmap.h"
#include "pcb.h"
#include "synch.h"

class PCBManager {

    public:
        PCBManager(int maxProcesses);
        ~PCBManager();

        PCB* AllocatePCB();
        int DeallocatePCB(PCB* pcb);
        PCB* GetPCB(int pid);
        int NumProc();

    private:
        BitMap* bitmap;
        PCB** pcbs;
        Lock* pcbManagerLock;

};

#endif // PCBMANAGER_H