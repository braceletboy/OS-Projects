// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "addrspace.h"
#include "thread.h"
#include "noff.h"

extern void SwapHeader(NoffHeader *noffH);

void doExit(int status) {
    int pid = currentThread->space->pcb->GetPID();
    printf("System Call: %d invoked Exit\n", pid);
    printf("Process %d exits with status %d\n", pid, status);

    currentThread->space->pcb->exitStatus = status;

    // Manage PCB memory As a parent process
    PCB* pcb = currentThread->space->pcb;
    pcb->DeleteExitedChildrenSetParentNull();

    // Manage PCB memory As a child process
    if(pcb->GetParent() == NULL)
        pcbManager->DeallocatePCB(pcb);
    delete currentThread->space;

    currentThread->Finish();
}


void startChildProcess(int dummy) {
    currentThread->RestoreUserState();
    currentThread->space->RestoreState();
    machine->Run();
}

int doFork(int functionAddr) {
    printf("System Call: %d invoked Fork\n", currentThread->space->pcb->GetPID());
    AddrSpace* childAddrSpace = new AddrSpace(*(currentThread->space));    
    if (!childAddrSpace->IsValid())
    {
        return -1;
    }

    Thread* childThread = new Thread("childThread");
    childThread->space = childAddrSpace;

    PCB* pcb = pcbManager->AllocatePCB();
    childAddrSpace->pcb = pcb;
    currentThread->space->pcb->AddChild(pcb);

    currentThread->SaveUserState();

    machine->WriteRegister(PCReg, functionAddr);
    machine->WriteRegister(PrevPCReg, functionAddr - 4);
    machine->WriteRegister(NextPCReg, functionAddr + 4);
    childThread->SaveUserState();

    currentThread->RestoreUserState();

    childThread->Fork(startChildProcess, 0);
    printf("Process %d Fork: start at address 0x%08X with %d pages memory\n",
            currentThread->space->pcb->GetPID(),
            functionAddr,
            childAddrSpace->GetNumPages()
    );

    return pcb->GetPID();
}

//---------------------------------------------------------------------
// doExec
//  Helper function for performing the exec system call
//
//  This function assumes that the current thread already has an address
//  space. This is a reasonable assumption because new processes are
//  created using fork and a fork always creates a new address space for
//  the forked process.
//
//  "filename" is the name of the executable that needs to be loaded
//  into the current address space.
//
//  Returns 0 if successful else -1
//---------------------------------------------------------------------

int doExec(char* filename) {
    AddrSpace *current_addrspace = currentThread->space;

    // 1. Read the executable
    OpenFile *executable = fileSystem->Open(filename);
    if (executable == NULL)
    {
        printf("Unable to open file %s\n", filename);
        return -1;
    }

    // 2. Replace the process memory with the content of the executable
    delete current_addrspace;

    AddrSpace *executable_addrspace = new AddrSpace(executable);
    if(!executable_addrspace->IsValid()) return -1;
    currentThread->space = executable_addrspace;

    delete executable;

    // 3. Set the registers, pageTable & pageTableSize for the machine
    executable_addrspace->InitRegisters();
    executable_addrspace->RestoreState();
    return 0;
}


int doJoin(int pid) {

}


int doKill (int pid) {

}



void doYield() {
    currentThread->Yield();
}

void incrementPC() {
    int oldPCReg = machine->ReadRegister(PCReg);

    machine->WriteRegister(PrevPCReg, oldPCReg);
    machine->WriteRegister(PCReg, oldPCReg + 4);
    machine->WriteRegister(NextPCReg, oldPCReg + 8);
}


// perform MMU translation to access physical memory
char* readString(int virtualAddr) {
    int i = 0;
    char* str = new char[256];
    unsigned int physicalAddr = currentThread->space->Translate(virtualAddr);

    // Need to get one byte at a time since the string may straddle multiple non-contiguous pages
    bcopy(&(machine->mainMemory[physicalAddr]),&str[i],1);
    while(str[i] != '\0' && i != 256-1)
    {
        virtualAddr++;
        i++;
        physicalAddr = currentThread->space->Translate(virtualAddr);
        bcopy(&(machine->mainMemory[physicalAddr]),&str[i],1);
    }
    if(i != 256-1 && str[i] != '\0')
    {
        str[i] = '\0';
    }

    return str;
}

void doCreate(char* fileName)
{
    printf("Syscall Call: [%d] invoked Create.\n", currentThread->space->pcb->GetPID());
    fileSystem->Create(fileName, 0);
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    } else  if ((which == SyscallException) && (type == SC_Exit)) {
        // Implement Exit system call
        doExit(machine->ReadRegister(4));
    } else if ((which == SyscallException) && (type == SC_Fork)) {
        int ret = doFork(machine->ReadRegister(4));
        machine->WriteRegister(2, ret);
        incrementPC();
    } else if ((which == SyscallException) && (type == SC_Exec)) {
        int virtAddr = machine->ReadRegister(4);
        char* fileName = readString(virtAddr);
        int ret = doExec(fileName);
        machine->WriteRegister(2, ret);
        incrementPC();
    } else if ((which == SyscallException) && (type == SC_Join)) {
        int ret = doJoin(machine->ReadRegister(4));
        machine->WriteRegister(2, ret);
        incrementPC();
    } else if ((which == SyscallException) && (type == SC_Kill)) {
        int ret = doKill(machine->ReadRegister(4));
        machine->WriteRegister(2, ret);
        incrementPC();
    } else if ((which == SyscallException) && (type == SC_Yield)) {
        doYield();
        incrementPC();
    } else if((which == SyscallException) && (type == SC_Create)) {
        int virtAddr = machine->ReadRegister(4);
        char* fileName = readString(virtAddr);
        doCreate(fileName);
        incrementPC();
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
