// ofd.cc
//  The implementation of the User Open File class which is analogous
//  to the open file descriptor in linux.

#include "ofd.h"
#include "system.h"

//------------------------------------------------------------------------
// OFD::OFD
//  Constructor
//
//  "id" is the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
OFD::OFD(int id)
{
    ofdID = id;
    name = NULL;
    fileVNode = NULL;
    refCount = 1;
    fileOffSet = 0;
    syncLock = NULL;
}

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
OFD::OFD(const char *fileName, int id)
{
    ofdID = id;
    name = fileName;
    fileVNode = vnm->AssignVNode(fileName);
    refCount = 1;  // one FD points to this OFD at creation
    fileOffSet = 0;

    char lockName[100];
    snprintf(lockName, sizeof(lockName), "ofd-%d sync lock", id);
    syncLock = new Semaphore(lockName, 1);
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
    syncLock->P();
    refCount++;
    syncLock->V();
}

//------------------------------------------------------------------------
// OFD::DecreaseRef
//  Return the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
void OFD::DecreaseRef()
{
    syncLock->P();
    ASSERT(refCount > 0);
    refCount--;
    syncLock->V();
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
//  "virtAddr" is the virtual address of the start of the buffer
//  "nBytes" is the number of bytes to read
//
//  Returns the number of bytes read if successful else -1
//------------------------------------------------------------------------
int OFD::Read(unsigned int virtAddr, unsigned int nBytes)
{
    // the reading and updating of file offset need to happen atomically
    syncLock->P();

    int bytesRead = fileVNode->ReadAt(virtAddr, nBytes, fileOffSet);
    if(bytesRead != -1) fileOffSet += bytesRead;  // read successful

    syncLock->V();
    return bytesRead;
}

//------------------------------------------------------------------------
// OFD::Write
//  Write from the give buffer into the file.
//
//  "virtAddr" is the virtual address of the start of the buffer
//  "nBytes" is the number of bytes to write
//
//  Returns the number of bytes written if successful else -1
//------------------------------------------------------------------------
int OFD::Write(unsigned int virtAddr, unsigned int nBytes)
{
    // the writing and updating of file offset need to happen atomically
    syncLock->P();

    int bytesWritten = fileVNode->WriteAt(virtAddr, nBytes, fileOffSet);
    if(bytesWritten != -1) fileOffSet += bytesWritten;  // write successful

    syncLock->V();
    return bytesWritten;
}

//------------------------------------------------------------------------
// ConsoleOFD::ConsoleOFD
//  Constructor
//
//  "id" is the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
ConsoleOFD::ConsoleOFD(const char *fileName, int id) : OFD(id)
{
    name = fileName;
    fileVNode = vnm->GetConsoleVNode();

    char lockName[100];
    snprintf(lockName, sizeof(lockName), "ofd-%d sync lock", id);
    syncLock = new Semaphore(lockName, 1);
}

//------------------------------------------------------------------------
// ConsoleOFD::Read
//  Read from the console into the given buffer.
//
//  "virtAddr" is the virtual address of the start of the buffer
//  "nBytes" is the number of bytes to read
//
//  Returns the number of bytes read if successful else -1
//------------------------------------------------------------------------
int ConsoleOFD::Read(unsigned int virtAddr, unsigned int nBytes)
{
    // the reading and updating of file offset need to happen atomically
    syncLock->P();

    int bytesRead = fileVNode->ReadAt(virtAddr, nBytes, 0);

    syncLock->V();
    return bytesRead;
}

//------------------------------------------------------------------------
// ConsoleOFD::Write
//  Write from the give buffer into the console.
//
//  "virtAddr" is the virtual address of the start of the buffer
//  "nBytes" is the number of bytes to write
//
//  Returns the number of bytes written if successful else -1
//------------------------------------------------------------------------
int ConsoleOFD::Write(unsigned int virtAddr, unsigned int nBytes)
{
    // the writing and updating of file offset need to happen atomically
    syncLock->P();

    int bytesWritten = fileVNode->WriteAt(virtAddr, nBytes, 0);

    syncLock->V();
    return bytesWritten;
}
