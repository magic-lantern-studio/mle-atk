/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkWired.cxx
 * @ingroup MleATK
 *
 * This file contains the implementation of a class that manages
 * messages sent and recieved over a wire.
 */

// COPYRIGHT_BEGIN
//
// The MIT License (MIT)
//
// Copyright (c) 2015-2021 Wizzer Works
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//  For information concerning this source file, contact Mark S. Millard,
//  of Wizzer Works at msm@wizzerworks.com.
//
//  More information concerning Wizzer Works may be found at
//
//      http://www.wizzerworks.com
//
// COPYRIGHT_END

// Include system header files.
#include <string.h>

// Include Magic Lantern header files.
#include <mle/mlAssert.h>
#include <mle/mlDebug.h>

// Include Authoring Toolkit header files.
#include "mle/AtkWired.h"
#include "mle/AtkWire.h"
#include "mle/AtkWireMsg.h"
#include "mle/AtkWireFunc.h"

static AtkWired* g_firstWired = 0;

AtkWired::AtkWired(const char* name, AtkWire* wire, void* objID)
{
    this->m_wire = wire;
    this->m_objID = objID;
#if defined(WIN32)
	this->m_name = name ? _strdup(name) : _strdup("");
#else
    this->m_name = name ? strdup(name) : strdup("");
#endif

    m_userData2 = m_userData = 0;
    m_parentData = 0;
    m_windowData = 0;
    g_firstWired = this;
}

AtkWired::~AtkWired()
{
    if (m_wire) delete m_wire;
}

AtkWireMsg*
AtkWired::recvAndDeliverMsg()
{
    // Get Msg.
    if (!m_wire) return(0);
    AtkWireMsg* msg = m_wire->recvMsg();

    // Error - recvMsg failed.
    if (!msg) return(0);

    //
    AtkWired* w = (AtkWired*) msg->m_destObj;

	MLE_DEBUG_CAT("ATK",
		printf("WIRED (%s): delivering %s msg to %d obj\n", m_name,
		   msg->m_msgName ? msg->m_msgName : "UNKNOWN", w);
	);

    // If no id - deliver to itself - otherwise deliver to.
    if (!w) return(deliverMsg(msg));
    return(w->deliverMsg(msg));


}

AtkWireMsg*
AtkWired::deliverMsg(AtkWireMsg* msg)
{
	// XXX - Until Player is fixed.
	if (!strcmp(m_name, "Player"))
	{
		// The only message the base class will take care of.
		if (!strcmp(msg->m_msgName, "ID"))
		{
			int id;
			int ret = msg->getParam(id);
			if (ret < 0) {
				printf("WIRED (%s): Error in deliverMsg - could not get ID param\n", m_name);
			}
			m_objID = (void*) id;
			return(0);
		} 

		printf("WIRED (%s): No one took care of %d   --- Exitting\n", m_name, msg ? msg->m_msgName : "NULL MESSAGE");
		MLE_ASSERT(0);
	}

    // Try to find a wire func for it.
    AtkWireFunc* wireFunc = findRecv(msg->m_msgName);
    if (!wireFunc)
	{
		printf("WIRED (%s): Could not find WireFunc for: %s\n", m_name, msg->m_msgName);
		MLE_ASSERT(0);
    }

    return(wireFunc->recvMsg(this, msg));
}

int
AtkWired::getFD()
{
    if (m_wire) return(m_wire->getFD());
    return(-1);
}

void
AtkWired::sendID()
{
    void* test = this;
    m_wire->sendMsg(m_objID, "ID", &test, sizeof(AtkWired*));
}

int 
AtkWired::pendingMsgs()
{
   return(m_wire->getNumMsgs());
}

AtkWireFunc*
AtkWired::find(const char* name, int recv)
{
    // First look in our array.
    AtkWireFunc* wf = findInArray(name, recv);
    if (wf) return(wf);

    // Next go to wire funcs and see if it is there.
    wf = AtkWireFunc::find(name, recv);
    if (recv)
	{
		addToRecvArray(wf);
    } else
	{
		addToArray(wf);
    }
    return(wf);
}

AtkWireFunc*
AtkWired::findRecv(const char* name)
{
    return(find(name, 1));
}

AtkWireFunc*
AtkWired::findInArray(const char* name, int recv)
{
    MLE_ASSERT(name);
    int len = (recv) ? m_recvWireFuncs.getLength() : m_wireFuncs.getLength();
    for (int i=0; i<len; i++)
	{
       AtkWireFunc* wf = (recv) ? m_recvWireFuncs[i]: m_wireFuncs[i];
//printf("WF name: %s\n", wf->getName());
       if (!strcmp(wf->getName(), name))
	   {
		return(wf);
       }
    }
    return(NULL);
}

void
AtkWired::addToArray(AtkWireFunc* wf)
{
    if (!wf) return;
//printf("Wired:  Adding %s to wirefuncs array\n", wf->getName());
	MLE_ASSERT(wf->getName());
    m_wireFuncs.add(wf);
}

void
AtkWired::addToRecvArray(AtkWireFunc* wf)
{
    if (!wf) return;
//printf("Wired:  Adding %s to recvwirefuncs array\n", wf->getName());
	MLE_ASSERT(wf->getName());
    m_recvWireFuncs.add(wf);
}

void
AtkWired::printWireFuncs(int recv)
{
printf("Printing WFs\n");
    int len = (recv) ? m_recvWireFuncs.getLength() : m_wireFuncs.getLength();
    for (int i=0; i<len; i++) {
       AtkWireFunc* wf = (recv) ? m_recvWireFuncs[i]: m_wireFuncs[i];
printf("PWF name: %s\n", wf->getName());
    }
printf("\n");
}

void
printWireFuncs(int recv)
{
    if (!g_firstWired) printf("No Wired\n");
    else g_firstWired->printWireFuncs(recv);
}

void *
AtkWired::operator new(size_t tSize)
{
	void *p = mlMalloc(tSize);
	return p;
}

void
AtkWired::operator delete(void *p)
{
	mlFree(p);
}
