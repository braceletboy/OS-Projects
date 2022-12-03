// ofd.h
//  This file contains the class for a User Open File Object mentioned in the
//  assignment. This concept is analogous to the concept of an Open File
//  Description (OFD) in Unix. An OFD is also called an open file table entry
//  i.e. an entry in the Open File Table in unix. An OFD represents an open
//  connection between a process and a file.
//
//  In Unix, every open() system call will have its own OFD. Similarly, in our
//  Nachos implementation, we shall have one OFD for every Open system call.
//  Multiple OFD instances can point to the same VNode instance. Such a
//  scenario arises when multiple open system calls are made to the same file
//  either by different processes or the same process i.e. there are multiple
//  connections to the file from different or same process respectively.
//
//  Similarly, there can be multiple file descriptors (FDs) pointing to the
//  same OFD. This happens when a process forks and the child process inherits
//  the same file descriptors.
//

#ifndef OFD_H
#define OFD_H

#include "vnode.h"
#include "synch.h"

class OFD
{
    public:
        OFD(char *fileName, int id);
        ~OFD();

        int GetID();
        void IncreaseRef();
        void DecreaseRef();
        bool IsActive();

        int Read(char *into, int nBytes);
        int Write(char *from, int nBytes);

    private:
        int ofdID;  // index of the OFD in the (Global) Open File Table
        char *name;  // name of the file
        VNode *fileVNode;  // Associated VNode
        int refCount;  // the number of file descriptors pointing to the OFD
        int fileOffSet;  // the file offset in bytes
        Lock *syncLock;  // lock for synchronized access - two processes
        // sharing an OFD should access it synchronously
};

#endif  // OFD_H
