// openfiletable.cc
//  The implementation for managing User Open File objects.

#include "openfiletable.h"

OpenFileTable::OpenFileTable(int maxOFDs)
{
    bitmap = new BitMap(maxOFDs);
    entries = new OFD*[maxOFDs];

    for(int i = 0; i < maxOFDs; i++)
    {
        entries[i] = NULL;
    }
    oftLock = new Lock("open file table lock");
}

OpenFileTable::~OpenFileTable()
{
    delete bitmap;
    delete entries;
    delete oftLock;
}

OFD *OpenFileTable::AllocateOFD(char *fileName)
{
    oftLock->Acquire();

    int id = bitmap->Find();
    if (id != -1)
    {
        entries[id] = new OFD(fileName, id);
        oftLock->Release();
        return entries[id];
    }
    else
    {
        // no free entry in the table
        oftLock->Release();
        return NULL;
    }
}

void OpenFileTable::DeallocateOFD(OFD *ofd)
{
    if (ofd == NULL) return;

    oftLock->Acquire();

    int ofdID = ofd->GetID();
    bitmap->Clear(ofdID);
    entries[ofdID] = NULL;
    delete ofd;

    oftLock->Release();
}
