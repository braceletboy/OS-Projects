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
    void SetParentNull();
    int exitStatus;

private:
    int pid;
    PCB *parent;
    List *children;
};

#endif // PCB_H