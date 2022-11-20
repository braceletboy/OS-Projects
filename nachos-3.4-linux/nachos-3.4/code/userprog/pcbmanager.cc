#include "pcbmanager.h"

//---------------------------------------------------------------------
// PCBManager::PCBManager
//  Create a pcb manager that can manage the given number of processes
//
//  The pcb manager is synchronized - it's implemented like a monitor.
//  Different processes can access the pcb manager and the various
//  resources it holds in a synchronized fashion.
//
//  "maxProcesses" is the number of processes that it can manage
//---------------------------------------------------------------------

PCBManager::PCBManager(int maxProcesses)
{
    bitmap = new BitMap(maxProcesses);
    pcbs = new PCB*[maxProcesses];

    for (int i = 0; i < maxProcesses; i++)
    {
        pcbs[i] = NULL;
    }
    pcbManagerLock = new Lock("pcb manager lock");
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
    // assign a pcb to the process in a synchronized manner
    pcbManagerLock->Acquire();

    int pid = bitmap->Find();
    ASSERT(pid != -1);  // TODO - don't use assert
    pcbs[pid] = new PCB(pid);

    pcbManagerLock->Release();

    return pcbs[pid];
}

//--------------------------------------------------------------------
// PCBManager::DeallocatePCB
//  Deallocate (delete) the pcb instance and clean up after it
//
//  If the pcb pointer is given as NULL, does nothing
//
//  "pcb" is the pcb you want to deallocate
//--------------------------------------------------------------------

void PCBManager::DeallocatePCB(PCB *pcb)
{
    // TODO: Let the pcb manager also take care of modifying
    // the pcb tree when a pcb is deleted.
    if (pcb == NULL) return;

    // remove the pcb in a synchronized manner
    pcbManagerLock->Acquire();

    int process_id = pcb->GetPID();
    bitmap->Clear(process_id);
    pcbs[process_id] = NULL;
    delete pcb;

    pcbManagerLock->Release();
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
