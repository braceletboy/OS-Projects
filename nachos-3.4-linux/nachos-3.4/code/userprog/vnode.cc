// vnode.cc
//  Implementation of the System-wide Open File object mention in the
//  assignment.

#include "vnode.h"
#include "system.h"
#include "unistd.h"

//------------------------------------------------------------------------
// VNode::VNode
//  Default Constructor.
//------------------------------------------------------------------------
VNode::VNode()
{
	name = NULL;
	refCount = 1;
	fileObj = NULL;
	syncLock = NULL;
}

//------------------------------------------------------------------------
// VNode::VNode
//  Constructor.
//
//  "fileName" is the name of the file corresponding to the VNode.
//------------------------------------------------------------------------
VNode::VNode(const char *fileName)
{
    char* newName = new char[strlen(fileName) + 1];
    strcpy(newName, fileName);
	name = newName;
	refCount = 1;  // at vnode creation, file is actively being used
	fileObj = fileSystem->Open(fileName);

	char lockName[300];
	snprintf(lockName, sizeof(lockName), "vnode lock: '%s'", fileName);
	syncLock = new Semaphore(lockName, 1);
}

//------------------------------------------------------------------------
// VNode::~VNode
//  Destructor.
//------------------------------------------------------------------------
VNode::~VNode()
{
	delete fileObj;
	delete syncLock;
}

//------------------------------------------------------------------------
// VNode::IncreaseRef
//  Increase the reference count of the VNode.
//------------------------------------------------------------------------
void VNode::IncreaseRef()
{
	syncLock->P();
	refCount++;
	syncLock->V();
}

//------------------------------------------------------------------------
// VNode::DecreaseRef
//  Decrease the reference count of the VNode.
//------------------------------------------------------------------------
void VNode::DecreaseRef()
{
	syncLock->P();
	ASSERT(refCount > 0);
	refCount--;
	syncLock->V();
}

//------------------------------------------------------------------------
// VNode::GetFileName
//  Return the name of the file corresponding to the vnode.
//------------------------------------------------------------------------
const char *VNode::GetFileName()
{
	return name;
}

//------------------------------------------------------------------------
// VNode::IsActive()
//  Return whether the VNode is actively being used.
//
//  Active usage means presence of atleast one open connection.
//------------------------------------------------------------------------
bool VNode::IsActive()
{
	return (refCount > 0);
}

//------------------------------------------------------------------------
// VNode::ReadAt()
//  Read bytes from the file into the given buffer in the main memory.
//
//  In linux, we have a VNode operation that uses the uio structure and
//  the ureadc kernel service to read from file to a buffer. The uio structure
//  describes a buffer that is not contiguous in main memory. The ureadc uses
//  the uio structure and reads into the buffer byte by byte.
//
//  Similarly, here I have a virtual address for a buffer that's not
//  contiguous in main memory. We shall use the Translate function from the
//  address space instance to read from the file into our buffer byte by byte.
//
//  Our (virtAddr + AddrSpace::Translate) combination is analogous to the
//  (uio + ureadc) combination in linux.
//
//  "virtAddr" is the virtual address of the buffer.
//  "nBytes" is the number of bytes to read.
//  "offset" is the file offset from where we will perform the read.
//
//  Return the number of bytes read if successful else -1
//------------------------------------------------------------------------
int VNode::ReadAt(unsigned int virtAddr, unsigned int nBytes,
					unsigned int offset)
{
	// TODO: Handle overflow of the buffer we are reading into

	// read file synchronously byte by byte
	syncLock->P();
	int totalBytes = 0;
	for(unsigned int idx = 0; idx < nBytes; virtAddr++, idx++, offset++)
    {
        unsigned int physAddr = currentThread->space->Translate(virtAddr);
		int bytesRead = fileObj->ReadAt(
			&machine->mainMemory[physAddr], 1, offset);

		if(bytesRead == 1) totalBytes++;

		else if (bytesRead == 0) break;  // file end reached

		else
		{
			// byte read failed
			syncLock->V();
			return -1;
		}
    }
	syncLock->V();
	return totalBytes;
}

//------------------------------------------------------------------------
// VNode::WriteAt()
//  Write bytes from the given buffer in the main memory into the file.
//
//  In linux, we have a VNode operation that uses the uio structure and
//  the uwritec kernel service to write from a buffer to the file. The uio
//  structure describes a buffer that is not contiguous in main memory. The
//  uwritec uses the uio structure and writes into the file byte by byte.
//
//  Similarly, here I have a virtual address for a buffer that's not
//  contiguous in main memory. We shall use the Translate function from the
//  address space instance to write into the file from our buffer byte by
//  byte.
//
//  Our (virtAddr + AddrSpace::Translate) combination is analogous to the
//  (uio + uwritec) combination in linux.
//
//  "virtAddr" is the buffer.
//  "nBytes" is the number of bytes to write.
//  "offset" is the file offset where we are going to write.
//
//  Return the number of bytes written else -1.
//------------------------------------------------------------------------
int VNode::WriteAt(unsigned int virtAddr, unsigned int nBytes,
					unsigned int offset)
{
	// TODO: Handle what happens if disk has no sufficient space for write

	// write to file synchronously byte by byte
	syncLock->P();
	int totalBytes = 0;
	for(unsigned int idx = 0; idx < nBytes; virtAddr++, idx++, offset++)
	{
		unsigned int physAddr = currentThread->space->Translate(virtAddr);
        if(machine->mainMemory[physAddr] == '\0') break;  // end of buffer
		int bytesWritten = fileObj->WriteAt(
			&machine->mainMemory[physAddr], 1, offset);

		if (bytesWritten == 1) totalBytes++;  // write successful
		else
		{
			// write failed
			syncLock->V();
			return -1;
		}
	}
	syncLock->V();
	return totalBytes;
}

//------------------------------------------------------------------------
// ConsoleVNode::ConsoleVNode
//  Default Constructor.
//------------------------------------------------------------------------
ConsoleVNode::ConsoleVNode() : VNode()
{
	name = (char *) "Console";
	syncLock = new Semaphore("vnode lock: Console", 1);
}

//------------------------------------------------------------------------
// ConsoleVNode::ReadAt
//  Read from console into a buffer.
//
//  "virtAddr" is the starting virtual address of the buffer.
//  "nBytes" is the number of bytes to write.
//  "offset" (unused)
//
//  Returns the bytes read if successful else -1
//------------------------------------------------------------------------
int ConsoleVNode::ReadAt(unsigned int virtAddr, unsigned int nBytes,
							unsigned int offset)
{
	syncLock->P();
	int totalBytes = 0;
	for(unsigned int idx = 0; idx < nBytes; virtAddr++, idx++, offset++)
    {
        unsigned int physAddr = currentThread->space->Translate(virtAddr);
		int bytesRead = read(STDIN_FILENO, &machine->mainMemory[physAddr], 1);

		if(bytesRead == 1) totalBytes++;
		else
		{
			// byte read failed
			syncLock->V();
			return -1;
		}
        if(machine->mainMemory[physAddr] == '\n') break;  // end of line
    }
	syncLock->V();
	return totalBytes;
}

//------------------------------------------------------------------------
// ConsoleVNode::WriteAt
//  Write to console from a buffer.
//
//  "virtAddr" is the starting virtual address of the buffer.
//  "nBytes" is the number of bytes to write.
//  "offset" (unused)
//
//  Returns the bytes written if successful else -1
//------------------------------------------------------------------------
int ConsoleVNode::WriteAt(unsigned int virtAddr, unsigned int nBytes,
							unsigned int offset)
{
	syncLock->P();
	int totalBytes = 0;
	for(unsigned int idx = 0; idx < nBytes; virtAddr++, idx++, offset++)
	{
		unsigned int physAddr = currentThread->space->Translate(virtAddr);
        if(machine->mainMemory[physAddr] == '\0') break;  // end of buffer
		int bytesWritten = write(
			STDOUT_FILENO, &machine->mainMemory[physAddr], 1);

		if (bytesWritten == 1) totalBytes++;  // write successful
		else
		{
			// write failed
			syncLock->V();
			return -1;
		}
	}
    //********************************************************************
    // Add a new life after every line because the test program expects it
    //********************************************************************
    write(STDOUT_FILENO, "\n",1);

	syncLock->V();
	return totalBytes;
}
