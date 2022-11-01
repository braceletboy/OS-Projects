#include "pcb.h"

//--------------------------------------------------------------------
// PCB::PCB
//  Create a PCB for a process
//
//  "id" is the PID of the process
//--------------------------------------------------------------------

PCB::PCB(int id)
{
    pid = id;
    parent = NULL;
    children = new List();
    exitStatus = -9999;
}

//--------------------------------------------------------------------
// PCB::~PCB
//  Destructor.
//--------------------------------------------------------------------

PCB::~PCB()
{
    delete children;
}

//--------------------------------------------------------------------
// PCB::AddChild
//  Append the child pcb to the end of the list of children
//
//  "pcb" is the pointer to the pcb of the child process
//--------------------------------------------------------------------

void PCB::AddChild(PCB *pcb)
{
    children->Append(pcb);
}

//--------------------------------------------------------------------
// PCB::RemoveChild
//  Remove the child with the given pcb pointer from the list of children
//
//  "pcb" is the pointer to the pcb of the child process
//
//  Returns 0 if removal was successful otherwise -1
//--------------------------------------------------------------------

int PCB::RemoveChild(PCB *pcb)
{
    return children->RemoveItem(pcb);
}

//--------------------------------------------------------------------
// PCB::HasExited
//  Returns true if the process corresponding to the pcb has exited
//  and returns false otherwise
//--------------------------------------------------------------------

bool PCB::HasExited()
{
    return exitStatus == -9999 ? false : true;
}

//--------------------------------------------------------------------
// PCB::getPID
//  Return the pid of the process corresponding to this pcb
//--------------------------------------------------------------------
int PCB::getPID()
{
    return pid;
}

//--------------------------------------------------------------------
// PCB::getParent
//  Return the pointer to the parent of the process corresponding to
//  this pcb
//--------------------------------------------------------------------
PCB* PCB::getParent()
{
    return parent;
}

//--------------------------------------------------------------------
// PCB::setParentNull
//  Set the pointer of the parent to NULL 
//--------------------------------------------------------------------
void PCB::setParentNull()
{
    parent = NULL;
}