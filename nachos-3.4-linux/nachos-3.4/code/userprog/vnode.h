// vnode.h
//  This file contains a class for System-wide Open File (SOF) Object.
//  The SOF object is similar to the VNode in Unix. Hence I name it as
//  VNode in this file.

#ifndef VNODE_H
#define VNODE_H

#include "openfile.h"
#include "synch.h"

class VNode
{
    public:
        VNode();
        VNode(const char *fileName);
        virtual ~VNode();

        void IncreaseRef();
        void DecreaseRef();
        const char *GetFileName();
        bool IsActive();

        virtual int ReadAt(unsigned int virtAddr, unsigned int nBytes,
                    unsigned int offset);
        virtual int WriteAt(unsigned int virtAddr, unsigned int nBytes,
                    unsigned int offset);

    private:
        OpenFile *fileObj;
        int refCount;  // the number of open connections to the file

    protected:
        const char *name;  // name of the file corresponding to the VNode Object
        Lock *syncLock;  // lock for synchronized access - two connections
        // sharing an VNode should access it synchronously
};

class ConsoleVNode: public VNode
{
    public:
        ConsoleVNode();

        virtual int ReadAt(unsigned int virtAddr, unsigned int nBytes,
                    unsigned int offset);
        virtual int WriteAt(unsigned int virtAddr, unsigned int nBytes,
                    unsigned int offset);
};

#endif  // VNODE_H
