// openfiletable.cc
//  The implementation for managing User Open File objects. This is similar
//  to the Open File Table in Unix.

#include "openfiletable.h"

//------------------------------------------------------------------------
// OpenFileTable::OpenFileTable
//  Constructor.
//
//  "maxOFDs" The maximum number of open file descriptors that the open
//  file table can store.
//------------------------------------------------------------------------
OpenFileTable::OpenFileTable(int maxOFDs)
{
    bitmap = new BitMap(maxOFDs);
    entries = new OFD*[maxOFDs];

    for(int i = 0; i < maxOFDs; i++)
    {
        entries[i] = NULL;
    }
    oftLock = new Semaphore("open file table lock", 1);
}

//------------------------------------------------------------------------
// OpenFileTable::~OpenFileTable
//  Destructor.
//------------------------------------------------------------------------
OpenFileTable::~OpenFileTable()
{
    delete bitmap;
    delete entries;
    delete oftLock;
}

//------------------------------------------------------------------------
// OpenFileTable::AllocateOFD
//  Allocate an open file descriptor to the invoking process.
//
//  "fileName" The file that the process wants to open.
//  "consoleOFD" if we want to allocate a console ofd or not.
//
//  Returns a pointer to the allocated OFD.
//------------------------------------------------------------------------
OFD *OpenFileTable::AllocateOFD(const char *fileName, bool consoleOFD)
{
    oftLock->P();

    int id = bitmap->Find();
    if (id != -1)
    {
        if(consoleOFD) entries[id] = new ConsoleOFD(fileName, id);

        else entries[id] = new OFD(fileName, id);

        oftLock->V();
        return entries[id];
    }
    else
    {
        // no free entry in the table
        oftLock->V();
        return NULL;
    }
}

//------------------------------------------------------------------------
// OpenFileTable::DeallocateOFD
//  Delete the allocated given open file descriptor if unused.
//
//  If the given open file descriptor is being used by other processes,
//  then this function only decreases the reference count of the OFD.
//  When the reference count reaches zero (i.e.) no process using it, then
//  the OFD gets deleted.
//------------------------------------------------------------------------
void OpenFileTable::DeallocateOFD(OFD *ofd)
{
    if (ofd == NULL) return;

    oftLock->P();

    ofd->DecreaseRef();
    if(!ofd->IsActive())
    {
        int ofdID = ofd->GetID();
        bitmap->Clear(ofdID);
        entries[ofdID] = NULL;
        delete ofd;
    }

    oftLock->V();
}
