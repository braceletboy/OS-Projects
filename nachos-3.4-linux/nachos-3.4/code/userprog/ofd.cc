// ofd.cc
//  The implementation of the User Open File class which is analogous
//  to the open file descriptor in linux.

#include "ofd.h"
#include "system.h"

//------------------------------------------------------------------------
// OFD::OFD
//  Constructor.
//
//  An OFD doesn't own a file. It's only an abstraction of the connection
//  between a process and the file it's accessing.
//
//  "fileName" is the name of the file associated.
//  "id" is the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
OFD::OFD(char *fileName, int id)
{
    ofdID = id;
    name = fileName;
    fileVNode = vnm->AssignVNode(fileName);
    refCount = 1;  // one FD points to this OFD at creation
    fileOffSet = 0;

    char lockName[100];
    snprintf(lockName, sizeof(lockName), "ofd-%d sync lock", id);
    syncLock = new Lock(lockName);
}

//------------------------------------------------------------------------
// OFD::~OFD
//  Destructor.
//
//  Disassociate a VNode.
//------------------------------------------------------------------------
OFD::~OFD()
{
    vnm->RelieveVNode(fileVNode);
    delete syncLock;
}

//------------------------------------------------------------------------
// OFD::GetID
//  Return the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
int OFD::GetID()
{
    return ofdID;
}

//------------------------------------------------------------------------
// OFD::IncreaseRef
//  Return the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
void OFD::IncreaseRef()
{
    syncLock->Acquire();
    refCount++;
    syncLock->Release();
}

//------------------------------------------------------------------------
// OFD::DecreaseRef
//  Return the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
void OFD::DecreaseRef()
{
    syncLock->Acquire();
    ASSERT(refCount > 0);
    refCount--;
    syncLock->Release();
}

//------------------------------------------------------------------------
// OFD::IsActive
//  Return whether the OFD is being used by processes or not.
//
//  If there are no FDs pointing to the OFD then it's not active.
//------------------------------------------------------------------------
bool OFD::IsActive()
{
    return (refCount > 0);
}

//------------------------------------------------------------------------
// OFD::Read
//  Read from the file into the given buffer.
//
//  "into" is the buffer
//  "nBytes" is the number of bytes to read
//------------------------------------------------------------------------
int OFD::Read(char *into, int nBytes)
{
    // the reading and updating of file offset need to happen atomically
    syncLock->Acquire();
    int bytesRead = fileVNode->ReadAt(into, nBytes, fileOffSet);
    fileOffSet += bytesRead;
    syncLock->Release();
    return bytesRead;
}

//------------------------------------------------------------------------
// OFD::Write
//  Write from the give buffer into the file.
//
//  "from" is the buffer
//  "nBytes" is the number of bytes to read
//------------------------------------------------------------------------
int OFD::Write(char *from, int nBytes)
{
    // the writing and updating of file offset need to happen atomically
    syncLock->Acquire();
    int bytesWritten = fileVNode->WriteAt(from, nBytes, fileOffSet);
    fileOffSet += bytesWritten;
    syncLock->Release();
    return bytesWritten;
}
