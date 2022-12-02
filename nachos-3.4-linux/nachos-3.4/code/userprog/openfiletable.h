// openfiletable.h
//  This file contains a class for managing the open file descriptors.
//  In linux all the open file descriptors are entries in the open file table.
//  This class represents that table in addition to having methods to interact
//  with the table.
//

#include "bitmap.h"
#include "ofd.h"
#include "synch.h"

class OpenFileTable
{
    public:
        OpenFileTable(int maxOFDs);
        ~OpenFileTable();

        OFD *AllocateOFD(char *fileName);
        void DeallocateOFD(OFD *ofd);

    private:
        BitMap *bitmap;  // bitmap to indicate if an entry is already filled
        OFD **entries;  // the table of entries
        Lock *oftLock;
};
