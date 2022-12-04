#include "vnodemanager.h"
#include "system.h"

//------------------------------------------------------------------------
// VNodeManager::VNodeManager
//  Constructor.
//------------------------------------------------------------------------
VNodeManager::VNodeManager()
{
	vnodes = new List();
	vnmLock = new Lock("vnode manager lock");

    // initialize the vnode for the console
    ConsoleVNode *console = new ConsoleVNode();
}

//------------------------------------------------------------------------
// VNodeManager::~VNodeManager
//  Destructor.
//------------------------------------------------------------------------
VNodeManager::~VNodeManager()
{
	delete vnodes;
    delete vnmLock;
}

//------------------------------------------------------------------------
// VNodeManager::AssignVNode
//  Associate VNode with the given file name to the current process.
//
//  "fileName" the file name corresponding to the VNode.
//
//  Returns a pointer the associated VNode.
//------------------------------------------------------------------------
VNode *VNodeManager::AssignVNode(char *fileName)
{
	vnmLock->Acquire();

    VNode *allocated_vnode;

    // if vnode already exists - return the already allocated vnode
    // iterate through the items in the vnode list by calling remove
    List *tempList = new List();
    VNode *removed_vnode = (VNode *) vnodes->Remove();
    while(removed_vnode != NULL)
    {
        //removed vnode is the allocated vnode
        if(strcmp(fileName, removed_vnode->GetFileName()) == 0)
        {
            removed_vnode->IncreaseRef();  // another proc wants to access
            allocated_vnode = removed_vnode;

            // put back the removed vnodes in the correct order
            vnodes->Prepend((void *) removed_vnode);
            while(!tempList->IsEmpty())
            {
                void *item = tempList->Remove();
                vnodes->Prepend(item);
            }

            return allocated_vnode;
        }

        // store removed item in the temp list
        tempList->Prepend((void *) removed_vnode);

        removed_vnode = (VNode *) vnodes->Remove();
    }

    // vnode doesn't already exist - reset the vnodes list
    delete vnodes;
    vnodes = tempList;

    // allocate new vnode and store it
    allocated_vnode = new VNode(fileName);
    vnodes->Append((void *) allocated_vnode);

    vnmLock->Release();

    return allocated_vnode;
}

//------------------------------------------------------------------------
// VNodeManager::RelieveVNode
//  Dis-associate VNode from the current process.
//
//  Does nothing if the vnode provided is not associated with the current
//  process.
//
//  "vnode" is the VNode of the current process.
//------------------------------------------------------------------------
void VNodeManager::RelieveVNode(VNode* vnode)
{
    vnmLock->Acquire();

    vnode->DecreaseRef();
    if(!vnode->IsActive()) vnodes->RemoveItem((void *) vnode);

    vnmLock->Release();
}

ConsoleVNode *VNodeManager::GetConsoleVNode()
{
    return console;
}
