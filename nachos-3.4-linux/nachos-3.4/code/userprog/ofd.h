// ofd.h
//  This file contains the class for a User Open File Object mentioned in the
//  assignment. This concept is analogous to the concept of an Open File
//  Description (OFD) in Unix. An OFD is also called an open file table entry
//  i.e. an entry in the Open File Table in unix.
//
//  In Unix, every open() system call will have its own OFD. Similarly, in our
//  Nachos implementation, we shall have one OFD for every Open system call.
//  Multiple OFD instances can point to the same VNode instance.
//

#include "vnode.h"

class OFD
{
    public:
        OFD(char *fileName, int id);
        ~OFD();

        int GetID();

    private:
        int ofdID;  // index of the OFD in the Open File Table
        char *name;  // name of the file
        VNode *fileVNode;  // Associated VNode
};
