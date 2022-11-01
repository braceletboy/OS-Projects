#include "pcbmanager.h"

//---------------------------------------------------------------------
// PCBManager::PCBManager
//  Create a pcb manager that can manage the given number of processes
//
//  "maxProcesses" is the number of processes that it can manage
//---------------------------------------------------------------------

PCBManager::PCBManager(int maxProcesses)
{
    bitmap = new BitMap(maxProcesses);
    pcbs = new PCB *[maxProcesses];
    printf("pcbs.size() = %d\n", sizeof(pcbs));

    for (int i = 0; i < maxProcesses; i++)
    {
        pcbs[i] = NULL;
    }
}

//--------------------------------------------------------------------
// PCBManager::~PCBManager
//  Destructor.
//--------------------------------------------------------------------

PCBManager::~PCBManager()
{
    delete bitmap;

    delete pcbs;
}

//--------------------------------------------------------------------
// PCBManager::AllocatePCB
//  Allocate a pcb to the calling process
//
//  Returns a pointer to the allocated pcb
//--------------------------------------------------------------------

PCB *PCBManager::AllocatePCB()
{
    // Aquire pcbManagerLock

    int pid = bitmap->Find();

    // Release pcbManagerLock

    ASSERT(pid != -1);

    pcbs[pid] = new PCB(pid);

    return pcbs[pid];
}

//--------------------------------------------------------------------
// PCBManager::DeallocatePCB
//  Deallocate (delete) the pcb instance and clean up after it
//
//  "pcb" is the pcb you want to deallocate
//
//  Returns 0 if successful otherwise -1
//--------------------------------------------------------------------

int PCBManager::DeallocatePCB(PCB *pcb)
{
    // Check is pcb is valid -- check pcbs for pcb->pid

    // Aquire pcbManagerLock

    int process_id = pcb->getPID();
    bitmap->Clear(process_id);

    // Release pcbManagerLock

    delete pcbs[process_id];

    pcbs[process_id] = NULL;
}

//--------------------------------------------------------------------
// PCBManager::GetPCB
//  Return a pointer to the pcb instance for the given pid
//
//  "pid" is the given pid
//--------------------------------------------------------------------

PCB *PCBManager::GetPCB(int pid)
{
    return pcbs[pid];
}