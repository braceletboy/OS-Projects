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
	fileObj = fileSystem->Open(fileName);  // TODO: fileName should have ../test
}

//------------------------------------------------------------------------
// VNode::~VNode
//  Destructor.
//------------------------------------------------------------------------
VNode::~VNode()
{
	delete fileObj;
}

//------------------------------------------------------------------------
// VNode::IncreaseRef
//  Increase the reference count of the VNode.
//------------------------------------------------------------------------
void VNode::IncreaseRef()
{
	refCount++;
}

//------------------------------------------------------------------------
// VNode::DecreaseRef
//  Decrease the reference count of the VNode.
//------------------------------------------------------------------------
void VNode::DecreaseRef()
{
	ASSERT(refCount > 0);
	refCount--;
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
