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
    pcbManagerLock = new Semaphore("pcb manager lock", 1);
}

//--------------------------------------------------------------------
// PCBManager::~PCBManager
//  Destructor.
//--------------------------------------------------------------------

PCBManager::~PCBManager()
{
    delete bitmap;
    delete pcbs;
    delete pcbManagerLock;
}

//--------------------------------------------------------------------
// PCBManager::AllocatePCB
//  Allocate a pcb to the calling process
//
//  Returns a pointer to the allocated pcb. NULL pointer if unable to
//  allocate
//--------------------------------------------------------------------

PCB *PCBManager::AllocatePCB()
{
    // assign a pcb to the process in a synchronized manner
    pcbManagerLock->P();

    int pid = bitmap->Find();
    if(pid != -1)
    {
        pcbs[pid] = new PCB(pid);
        pcbManagerLock->V();
        return pcbs[pid];
    }
    else
    {
        // max pcb limit reached
        pcbManagerLock->V();
        return NULL;
    }
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
    pcbManagerLock->P();

    int process_id = pcb->GetPID();
    bitmap->Clear(process_id);
    pcbs[process_id] = NULL;
    delete pcb;

    pcbManagerLock->V();
}

//--------------------------------------------------------------------
// PCBManager::GetPCB
//  Return a pointer to the pcb instance for the given pid
//
//  If the pid is invalid then a NULL pointer is returned to indicate
//  invalid PCB.
//
//  "pid" is the given pid
//--------------------------------------------------------------------

PCB *PCBManager::GetPCB(int pid)
{
    if(pid < 0) return NULL;
    return pcbs[pid];
}
