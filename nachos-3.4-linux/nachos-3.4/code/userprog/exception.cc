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

//---------------------------------------------------------------------
// doExit
//  Helper function for performing the Exit system call
//
//  "status" is the status with which the process will exit
//---------------------------------------------------------------------

void doExit(int status) {
    int pid = currentThread->space->pcb->GetPID();
    printf("System Call: %d invoked Exit\n", pid);

    // 1. Set the exit status
    currentThread->space->pcb->exitStatus = status;

    // 2. Make changes to the PCB tree
    PCB* pcb = currentThread->space->pcb;
    pcb->DeleteExitedChildrenSetParentNull();

    // 3. Delete PCB if necessary
    if(pcb->GetParent() == NULL) pcbManager->DeallocatePCB(pcb);

    // 4. Delete address space
    delete currentThread->space;

    // 5. Delete thread of execution
    printf("Process %d exits with status %d\n", pid, status);
    currentThread->Finish();

}

//---------------------------------------------------------------------
// startChildProcess
//  Helper function to get the forked processes to also simulate
//  instructions
//
//  "dummy" is a dummy int argument required by the Thread::Fork
//  function
//---------------------------------------------------------------------

void startChildProcess(int dummy) {
    machine->Run();
}

//---------------------------------------------------------------------
// doFork
//  Helper function for performing the Fork system call
//
//  "functionAddr" is the function that immediately gets executed in
//  the forked process
//---------------------------------------------------------------------

int doFork(int functionAddr) {
    int pid = currentThread->space->pcb->GetPID();
    printf("System Call: %d invoked Fork\n", pid);

    // 1. Allocate a pcb for the forked process
    PCB* pcb = pcbManager->AllocatePCB();
    if(pcb == NULL)
    {
        DEBUG(
            'e', "Process %d Fork: failed. Maximum PCB limit reached.", pid
        );
        return -1;
    }

    // 2. Allocate an address space for the forked process
    AddrSpace* childAddrSpace = new AddrSpace(*(currentThread->space));
    if (!childAddrSpace->IsValid())
    {
        DEBUG(
            'e',
            "Process %d Fork: failed. Insufficient memory for address space",
            pid
        );

        // clean up
        pcbManager->DeallocatePCB(pcb);
        delete childAddrSpace;

        return -1;
    }
    printf("Process %d Fork: start at address 0x%08X with %d pages memory\n",
            pid, functionAddr, childAddrSpace->GetNumPages()
    );
    childAddrSpace->pcb = pcb;

    // 3. Allocate a thread for the forked process
    char threadName[20];
    sprintf(threadName, "childThread_%d", pcb->GetPID());
    Thread* childThread = new Thread(threadName);
    childThread->space = childAddrSpace;

    // 4. Setup the machine state for forked process
    currentThread->SaveUserState();

    machine->WriteRegister(PCReg, functionAddr);
    machine->WriteRegister(PrevPCReg, functionAddr - 4);
    machine->WriteRegister(NextPCReg, functionAddr + 4);
    childThread->SaveUserState();

    currentThread->RestoreUserState();

    // 5. Setup the execution switch stack for the forked process
    childThread->Fork(startChildProcess, 0);

    // 6. Add the forked pcb to the current pcb as child
    currentThread->space->pcb->AddChild(pcb);

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
    int pid = currentThread->space->pcb->GetPID();
    printf("System Call: %d invoked Exec\n", pid);
    AddrSpace *current_addrspace = currentThread->space;

    // 1. Read the executable
    OpenFile *executable = fileSystem->Open(filename);
    if (executable == NULL)
    {
        DEBUG('e', "Process %d Exec: failed. Unable to open file %s\n",
                pid, filename);
        return -1;
    }

    printf("Exec Program: %d loading %s\n", pid, filename);

    // 2. Replace the process memory with the content of the executable
    PCB *current_pcb = current_addrspace->pcb;
    delete current_addrspace;

    AddrSpace *executable_addrspace = new AddrSpace(executable);
    executable_addrspace->pcb = current_pcb;
    currentThread->space = executable_addrspace;
    if(!executable_addrspace->IsValid())
    {
        DEBUG(
            'e',
            "Process %d Exec: failed. Insufficient memory for address space",
            pid
        );

        // clean up
        doExit(-1);

        return -1;
    }

    delete executable;

    // 3. Set the registers, pageTable & pageTableSize for the machine
    executable_addrspace->InitRegisters();
    executable_addrspace->RestoreState();
    return 0;
}

//---------------------------------------------------------------------
// doJoin
//  Helper function for making current process join on another process
//
//  "join_pid" is the process we want to join on
//
//  Returns -9999 if the join was unsuccessful else the exit status of
//  of the joined process
//---------------------------------------------------------------------

int doJoin(int join_pid) {
    int pid = currentThread->space->pcb->GetPID();
    printf("System Call: %d invoked Join\n", pid);

    // 1. Check if process joining on itself
    if(join_pid == pid)
    {
        printf("Process %d trying to join on itself: note allowed\n",
               join_pid);
        return -9999;
    }

    // 2. Check if process to join exists
    PCB *join_pcb = pcbManager->GetPCB(join_pid);
    if(join_pcb == NULL)
    {
        DEBUG('e', "Process %d cannot join process %d: doesn't exist\n",
                pid, join_pid);
        return -1;
    }

    // 3. Check if parent is calling join on child
    PCB *jp_parent_pcb = join_pcb->GetParent();
    if((jp_parent_pcb  == NULL) || (jp_parent_pcb->GetPID() != pid))
    {
        DEBUG('e', "Non parent %d trying to join on %d: not allowed\n",
              pid, join_pid);
        return -9999;
    }
    while(!join_pcb->HasExited()) currentThread->Yield();
    DEBUG('e', "Process %d joined on %d\n", pid, join_pid);
    return join_pcb->exitStatus;
}

//--------------------------------------------------------------------
// doKill
//  Helper function for performing the Kill system call
//
//  Returns 0 if successful else -1
//--------------------------------------------------------------------

int doKill (int kill_pid) {
    int pid = currentThread->space->pcb->GetPID();
    printf("System Call: %d invoked Kill\n", pid);

    // 1. Call Exit if the to be killed process is same as current process
    if(kill_pid == pid)
    {
        doExit(9999);
        return 0;
    }
    else
    {
        PCB *killed_pcb = pcbManager->GetPCB(kill_pid);

        // 2. Check if the process to be killed exists
        if(killed_pcb == NULL)
        {
            printf("Process %d cannot kill process %d: doesn't exist\n",
                   pid, kill_pid);
            return -1;
        }

        // 3. Set the exit status
        killed_pcb->exitStatus = 9999;

        // 3. Make changes to the PCB tree
        killed_pcb->DeleteExitedChildrenSetParentNull();
        PCB *kp_parent_pcb = killed_pcb->GetParent();

        // 4. Delete PCB if necessary
        if(kp_parent_pcb == NULL) pcbManager->DeallocatePCB(killed_pcb);

        // 5. Delete address space
        Thread *kp_thread = scheduler->UnSchedule(kill_pid);
        delete kp_thread->space;

        // 6. Delete thread of execution
        delete kp_thread;

        printf("Process %d killed process %d\n", pid, kill_pid);
        return 0;
    }
}

//--------------------------------------------------------------------
// doYield
//  Helper function for performing the Yield system call
//--------------------------------------------------------------------

void doYield() {
    int pid = currentThread->space->pcb->GetPID();
    printf("System Call: %d invoked Yield\n", pid);
    currentThread->Yield();
}

//--------------------------------------------------------------------
// incrementPC
//  Increment the program counter by one instruction.
//
//  Required for system calls because this update, that usually happens
//  at the end of an instruction inside the machine - refer to the
//  OneInstruction function, is not reached when the machine
//  interrupts due to a system call and traps into the NachOS kernel.
//--------------------------------------------------------------------

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

//----------------------------------------------------------------------
// doCreate
//  Helper function for the close system call
//----------------------------------------------------------------------

void doCreate(char* fileName)
{
    printf("Syscall Call: [%d] invoked Create.\n",
            currentThread->space->pcb->GetPID());
    char *fileName = strcat("../test/", fileName);
    fileSystem->Create(fileName, 0);
}

//----------------------------------------------------------------------
// doOpen
//  Helper function for the close system call
//
//  "fileName" is the filename to open
//
//  Return the file id of the opened file if successful else -1
//----------------------------------------------------------------------

OpenFileId doOpen(char* fileName)
{
    printf("Syscall Call: [%d] invoked Open.\n",
            currentThread->space->pcb->GetPID());
    char *fileName = strcat("../test/", fileName);
    int fid = currentThread->space->pcb->AllocateFD(fileName);
    return (OpenFileId) fid;
}

//----------------------------------------------------------------------
// doRead
//  Helper function for the close system call
//----------------------------------------------------------------------

void doRead(char *buffer, int size, OpenFileId id)
{
    printf("Syscall Call: [%d] invoked Read.\n",
            currentThread->space->pcb->GetPID());
}

//----------------------------------------------------------------------------------------------------------------
// doWrite
//  Helper function for the close system call
//----------------------------------------------------------------------

void doWrite(char *buffer, int size, OpenFileId id)
{
    printf("Syscall Call: [%d] invoked Write.\n",
            currentThread->space->pcb->GetPID());
}

//----------------------------------------------------------------------
// doClose
//  Helper function for the close system call
//----------------------------------------------------------------------

void doClose(OpenFileId id)
{
    printf("Syscall Call: [%d] invoked Close.\n",
            currentThread->space->pcb->GetPID());
    currentThread->space->pcb->DeallocateFD((int) id);
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
	DEBUG('e', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    } else  if ((which == SyscallException) && (type == SC_Exit)) {
        doExit(machine->ReadRegister(4));
        incrementPC();  // should never reach here
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
        char *fileName = readString(virtAddr);
        doCreate(fileName);
        incrementPC();
    } else if((which == SyscallException) && (type == SC_Open)) {
        int virtAddr = machine->ReadRegister(4);
        char *fileName = readString(virtAddr);
        OpenFileId fid = doOpen(fileName);
        machine->WriteRegister(2, fid);
        incrementPC();
    } else if((which == SyscallException) && (type == SC_Write)) {
        ;
    } else if((which == SyscallException) && (type == SC_Read)) {
        ;
    } else if((which == SyscallException) && (type == SC_Close)) {
        OpenFileId fid = machine->ReadRegister(4);
        doClose(fid);
        incrementPC();
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
