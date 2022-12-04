// vnodemanager.h
//  The VNode Manager is similar to the VNode table in the Unix operating
//  system. In addition to having the VNode entries in the form of an
//  array of pointer (to the VNodes), the manager also support creation and
//  deallocation of VNodes.
//
//  The vnodes in the vnode manager are stored as a linked list and not as
//  an array marked by a bitmap because we need to iterate over all the
//  valid entries, whenever a new process asks for a file's vnode, to
//  determine that we are creating any duplicate VNodes. As a design choice,
//  I agree that a Hashmap would be the best data structure with the filenames
//  as the keys, but that would be an overkill for the small number of files
//  we will actually handle in Nachos.
//

#ifndef VNM_H
#define VNM_H

#include "bitmap.h"
#include "synch.h"
#include "vnode.h"

class VNodeManager
{
    public:
        VNodeManager();
        ~VNodeManager();

        VNode *AssignVNode(const char *fileName);
        void RelieveVNode(VNode *vnode);
        ConsoleVNode *GetConsoleVNode();

    public:
        List *vnodes;  // file vnodes
        Semaphore *vnmLock;
        ConsoleVNode *console;  // vnode for the console
};

#endif  // VNM_H
