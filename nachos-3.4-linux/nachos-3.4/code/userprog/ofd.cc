// ofd.cc
//  The implementation of the User Open File class which is analogous
//  to the open file descriptor in linux.

#include "ofd.h"
#include "system.h"

//------------------------------------------------------------------------
// OFD::OFD
//  Constructor.
//
//  An OFD doesn't own a file. It's only an abstraction of the connection
//  between a process and the file it's accessing.
//
//  "fileName" is the name of the file associated.
//  "id" is the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
OFD::OFD(char *fileName, int id)
{
    ofdID = id;
    name = fileName;
    fileVNode = vnm->AssignVNode(fileName);
}

//------------------------------------------------------------------------
// OFD::~OFD
//  Destructor.
//
//  Disassociate a VNode.
//------------------------------------------------------------------------
OFD::~OFD()
{
    vnm->RelieveVNode(fileVNode);
}

//------------------------------------------------------------------------
// OFD::GetID
//  Return the index of the OFD in the Open File Table.
//------------------------------------------------------------------------
int OFD::GetID()
{
    return ofdID;
}
