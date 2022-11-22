#ifndef PCB_H
#define PCB_H

#include "list.h"

class PCB
{
public:
    PCB(int id);
    ~PCB();

    void AddChild(PCB *pcb);
    int RemoveChild(PCB *pcb);
    bool HasExited();
    void DeleteExitedChildrenSetParentNull();

    int GetPID();
    PCB* GetParent();
    List* GetChildren();
    void SetParent(PCB *new_parent);
    int exitStatus;  //  -9999 means not exited, 9999 means exited by a kill
                     //  from another process, -1 means exited unsuccessfully,
                     //  any other value means exited successfully

private:
    int pid;
    PCB *parent;
    List *children;
};

#endif // PCB_H