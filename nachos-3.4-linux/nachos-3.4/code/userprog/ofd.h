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
        OFD(int id);
        OFD(const char *fileName, int id);
        virtual ~OFD();

        int GetID();
        void IncreaseRef();
        void DecreaseRef();
        bool IsActive();

        virtual int Read(unsigned int virtAddr, unsigned int nBytes);
        virtual int Write(unsigned int virtAddr, unsigned int nBytes);

    private:
        int ofdID;  // index of the OFD in the (Global) Open File Table
        int refCount;  // the number of file descriptors pointing to the OFD
        unsigned int fileOffSet;  // the file offset in bytes

    protected:
        const char *name;  // name of the file
        VNode *fileVNode;  // Associated VNode
        Lock *syncLock;  // lock for synchronized access - two processes
        // sharing an OFD should access it synchronously
};

class ConsoleOFD : public OFD
{
    public:
        ConsoleOFD(const char *fileName, int id);

        virtual int Read(unsigned int virtAddr, unsigned int nBytes);
        virtual int Write(unsigned int virtAddr, unsigned int nBytes);
};

#endif  // OFD_H
