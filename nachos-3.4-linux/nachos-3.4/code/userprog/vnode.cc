// vnode.cc
//  Implementation of the System-wide Open File object mention in the
//  assignment.

#include "vnode.h"
#include "system.h"

//------------------------------------------------------------------------
// VNode::VNode
//  Constructor.
//
//  "fileName" is the name of the file corresponding to the VNode.
//------------------------------------------------------------------------
VNode::VNode(char *fileName)
{
	name = fileName;
	refCount = 1;  // at vnode creation, file is actively being used
	fileObj = fileSystem->Open(fileName);

	char lockName[300];
	snprintf(lockName, sizeof(lockName), "vnode lock: '%s'", fileName);
	syncLock = new Lock(lockName);
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
	syncLock->Acquire();
	refCount++;
	syncLock->Release();
}

//------------------------------------------------------------------------
// VNode::DecreaseRef
//  Decrease the reference count of the VNode.
//------------------------------------------------------------------------
void VNode::DecreaseRef()
{
	syncLock->Acquire();
	ASSERT(refCount > 0);
	refCount--;
	syncLock->Release();
}

//------------------------------------------------------------------------
// VNode::GetFileName
//  Return the name of the file corresponding to the vnode.
//------------------------------------------------------------------------
char *VNode::GetFileName()
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
//  Read bytes from the file into the given buffer.
//
//  "into" is the buffer.
//  "nBytes" is the number of bytes to read.
//  "offset" is the file offset from where we will perform the read.
//
//  Return the number of bytes read.
//------------------------------------------------------------------------
int VNode::ReadAt(char *into, int nBytes, int offset)
{
	// read file synchronously
	syncLock->Acquire();
	int bytesRead = fileObj->ReadAt(into, nBytes, offset);
	syncLock->Release();
	return bytesRead;
}

//------------------------------------------------------------------------
// VNode::WriteAt()
//  Write bytes from the given buffer into the file.
//
//  "from" is the buffer.
//  "nBytes" is the number of bytes to write.
//  "offset" is the file offset where we are going to write.
//
//  Return the number of bytes written.
//------------------------------------------------------------------------
int VNode::WriteAt(char *from, int nBytes, int offset)
{
	// read file synchronously
	syncLock->Acquire();
	int bytesWritten = fileObj->WriteAt(from, nBytes, offset);
	syncLock->Release();
	return bytesWritten;
}
