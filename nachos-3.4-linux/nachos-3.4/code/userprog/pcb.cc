#include "pcb.h"
#include "system.h"

//--------------------------------------------------------------------
// PCB::PCB
//  Create a PCB for a process
//
//  "id" is the PID of the process
//--------------------------------------------------------------------

PCB::PCB(int id)
{
    pid = id;
    if (currentThread->space == NULL)
        parent = NULL;
    else
        parent = currentThread->space->pcb;
    children = new List();
    exitStatus = -9999; // hasn't exited

    bitmap = new BitMap(MAX_PROC_OFDS);
    ofds = new OFD*[MAX_PROC_OFDS];

    for(int i = 0; i < MAX_PROC_OFDS; i++)
    {
        ofds[i] = NULL;
    }
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
    children->Append((void *)pcb);
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
// PCB::GetPID
//  Return the pid of the process corresponding to this pcb
//--------------------------------------------------------------------
int PCB::GetPID()
{
    return pid;
}

//--------------------------------------------------------------------
// PCB::GetParent
//  Return the pointer to the parent of the process corresponding to
//  this pcb
//--------------------------------------------------------------------
PCB* PCB::GetParent()
{
    return parent;
}

//---------------------------------------------------------------------
// PCB::GetChildren
//  Return a pointer to the list of children PCB pointers
//----------------------------------------------------------------------
List* PCB::GetChildren()
{
    return children;
}

//----------------------------------------------------------------------
// PCB::SetParent
//  Set the pointer of the parent to the given pointer
//
//  "new_parent" is the new parent pointer
//----------------------------------------------------------------------
void PCB::SetParent(PCB *new_parent)
{
    parent = new_parent;
}

//----------------------------------------------------------------------
// PCB::DeleteExitedChildrenSetParentNull
//  Delete the exited children of the pcb and set the parent of the rest
//  to NULL.
//----------------------------------------------------------------------
void PCB::DeleteExitedChildrenSetParentNull()
{
    PCB *child = (PCB *)children->Remove();
    while (child != NULL)
    {
        if (child->HasExited())
            delete child;
        else
            child->SetParent(NULL);
        child = (PCB *)children->Remove();
    }
}

//----------------------------------------------------------------------
// PCB::AllocateFD
//  Return the file descriptor (file id) allocated
//
//  "fileName" is the name of the file
//----------------------------------------------------------------------
int PCB::AllocateFD(char *fileName)
{
    int fid = bitmap->Find();
    if(fid != -1)
    {
        ofds[fid] = oft->AllocateOFD(fileName);
    }
    return fid;
}

//----------------------------------------------------------------------
// PCB::DeallocateFD
//  Remove the given file descriptor (file id)
//----------------------------------------------------------------------
void PCB::DeallocateFD(int fid)
{
    if(fid < 0) return;

    bitmap->Clear(fid);
    OFD *ofd = ofds[fid];
    oft->DeallocateOFD(ofd);
    ofds[fid] = NULL;
}

//----------------------------------------------------------------------
// PCB::GetOFD
//  Get the  (file id)
//----------------------------------------------------------------------
OFD *PCB::GetOFD(int fid)
{
    if(fid < 0) return NULL;
    return ofds[fid];
}
