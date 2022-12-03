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
        VNode(char *fileName);
        ~VNode();

        void IncreaseRef();
        void DecreaseRef();
        char *GetFileName();
        bool IsActive();

        int ReadAt(char *into, int nBytes, int offset);
        int WriteAt(char *from, int nBytes, int offset);

    private:
        char *name;  // name of the file corresponding to the VNode Object
        OpenFile *fileObj;
        int refCount;  // the number of open connections to the file
        Lock *syncLock;  // lock for synchronized access - two connections
        // sharing an VNode should access it synchronously
};

#endif  // VNODE_H
