/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkWire.cxx
 * @ingroup MleATK
 *
 * This file contains the implementation of a class that provides utility
 * for sending and recieving messages over the network.
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
//  For information concerning this source code, contact Mark S. Millard,
//  of Wizzer Works at msm@wizzerworks.com.
//
//  More information concerning Wizzer Works may be found at
//
//      http://www.wizzerworks.com
//
// COPYRIGHT_END

// Include Magic Lantern header files.
#include <mle/mlErrno.h>
#include <mle/mlFileio.h>
#include <mle/mlAssert.h>
#include <mle/mlDebug.h>

// Include Authoring Toolkit header files.
#include "mle/AtkWire.h"
#include "mle/AtkWired.h"    // I hate the fact that I use this class here
#include "mle/AtkWireMsg.h"


AtkWire::AtkWire(int readFD, int writeFD)
{
    this->m_readFD = readFD;
    this->m_writeFD = writeFD;
    m_head = m_tail = 0;
    m_lostConnection = 0;
}

AtkWire::~AtkWire()
{
    // Delete all msgs in queue.
    for (; m_head; )
	{
		AtkWireMsg* next = m_head->m_next;
		delete m_head;
		m_head = next;
    }

    // Close the fds.
    close(m_readFD);
    close(m_writeFD);
}

int 
AtkWire::sendMsg(void* destObj, const char* msgName, void* msgData, 
	int msgDataLen)
{
    AtkWireMsg msg(destObj, msgName, 0, msgData, msgDataLen);
    return(sendMsg(&msg));
}

int 
AtkWire::sendMsg(AtkWireMsg* msg)
{
    // Must have a valid connection.
    if (m_lostConnection)
	{
		printf("WIRE: Lost Connection\n");
		return(-1);
    }

    // Must have a valid msg.
    if (!msg)
	{
		printf("WIRE: Null msg\n");
		return(-1);
    }

    // Make sure writeFD is valid.
    if (m_writeFD < 0 )
	{
		printf("WIRE: bad write FD\n");
		return(-2);
    }

    // Write out msg header.
    if (mlWrite(m_writeFD, msg->getStartAddress(), msg->getHeaderLength()) < 0)
	{
		printf("WIRE: Could not write header.  Errno: %d\n", g_mlErrno);
		return(-3);
    }

    // Write out data.
    if (msg->getDataLength() > 0)
	{
		int wlen;
		if ((wlen = mlWrite(m_writeFD, msg->m_msgData, msg->getDataLength())) < 0)
		{
			printf("WIRE: Could not write data.  Errno: %d\n", g_mlErrno);
			return(-4);
		}
		if (wlen != msg->getDataLength())
		{
			printf("WIRE: write len != data len    %d, %d\n", wlen, 
			   msg->getDataLength());
			MLE_ASSERT(0);
		}
    }

	MLE_DEBUG_CAT("ATK",
		printf("WIRE: Sent %s msg to %x object\n", msg->m_msgName, msg->m_destObj);
		/*
		printf("     Detail: ");
		for (int h=0; h<msg->getHeaderLength(); h++) printf("%x ", msg->msgName[h]);
		printf("\n");
		*/
	);

    // Success.
    return(0);
}

AtkWireMsg* 
AtkWire::recvMsg()
{
    // Check read FD
    if (m_readFD < 0)
	{
		printf("WIRE: Bad read FD\n");
		return(NULL);
    }

    // If any msgs are on the queue - return those.
    if (m_head)
	{
		AtkWireMsg* ret = m_head;
		if (m_head == m_tail)
		{
			m_head = m_tail = 0;
		} else
		{
			m_head = m_head->m_next;
		}
		return(ret);
    }

    // OK, find it from the FD.
    return(recvMsgFromFD());
}

AtkWireMsg* 
AtkWire::recvMsgFromFD()
{
    // Allocate a msg.
    AtkWireMsg* msg = new AtkWireMsg();

    // Read msg header.
    int len;
    if ((len = mlRead(m_readFD, msg->getStartAddress(), msg->getHeaderLength())) < 0)
	{
		printf("WIRE: Could not read header.  Errno: %d\n", g_mlErrno);
		delete msg;
		return(NULL);
    } else if (len == 0)
	{
//printf("WIRE: ZERO len in read:    %d\n", m_readFD);     
		m_lostConnection = 1;
		mlSetErrno(MLE_E_ATKLIB_CONNECTION_LOST);
		delete msg;
		return(NULL);
    } else if (len != msg->getHeaderLength())
	{
printf("WIRE: len != msgHeaderLen   %d, %d\n", len, msg->getHeaderLength());
		MLE_ASSERT(0);
    }

    // Allocate space.
    msg->allocMsgData();

    // Read data
    if (msg->getDataLength() > 0)
	{
		for (int totalLen=0; totalLen <msg->getDataLength(); totalLen+=len)
		{
			if ((len = mlRead(m_readFD, ((char*) msg->m_msgData)+totalLen, 
				msg->getDataLength()-totalLen)) < 0)
			{
				printf("WIRE: could not read msg data.  Errno: %d\n", g_mlErrno);
				delete msg;
				return(NULL);
			} else if (len == 0)
			{
				printf("ZERO len in read 2:    %d\n", m_readFD);     
				exit(-1);
				mlSetErrno(MLE_E_ATKLIB_CONNECTION_LOST);
				m_lostConnection = 1;
				delete msg;
				return(NULL);
			}
		}
    }

// printf("WIRE: Recved %s msg from %x object\n", msg->msgName, msg->destObj);
/*
printf("     Detail: ");
for (int h=0; h<msg->getHeaderLength(); h++) printf("%x ", msg->msgName[h]);
printf("\n");
*/

    // Success.
    return(msg);
}

AtkWireMsg* 
AtkWire::sendSyncMsg(AtkWired* wired, void* destObj, const char* msgName, 
	void* msgData, int msgDataLen)
{
    AtkWireMsg msg(destObj, msgName, 1, msgData, msgDataLen);
    return sendSyncMsg(wired, &msg);
}

AtkWireMsg* 
AtkWire::sendSyncMsg(AtkWired* wired, AtkWireMsg* msg)
{
    // Must have a valid connection.
    if (m_lostConnection)
	{
		printf("WIRE: Lost Connection\n");
		return(NULL);
    }

    // First send the msg.
    if (sendMsg(msg) < 0)
	{
		// Error.
		printf("WIRE: send msg from sendSyncMsg failed\n");
		return(NULL);
    }

    // Now we have to wait for a reply
    for (;;)
	{
		AtkWireMsg* msg = recvMsgFromFD();

		// If no message  - we have lost connection.
		if (!msg) return(NULL);

		// If a reply msg return it.
		if (msg->isReplyMsg())
		{
			return(msg);
		}

		// If a sync msg - respond to it - otherwise we might have deadlock 
		// situations.
		if (msg->isSyncMsg())
		{

			AtkWired* w = (AtkWired*) msg->m_destObj;

			MLE_DEBUG_CAT("ATK",
				printf("WIRED (inside sendSyncMsg): delivering %s msg to %d obj\n",
					msg->m_msgName ? msg->m_msgName : "UNKNOWN", w);
			);

			// If no id - deliver msg to itself.
			if (!w)
			{
				wired->deliverMsg(msg);
			} else
			{
				// Dtherwise deliver to obj.
				w->deliverMsg(msg);
			}
		} else
		{
			// Otherwise, place msg on queue.
			if (m_tail) m_tail->m_next = msg;
			m_tail = msg;
			if (!m_head) m_head = msg;
		}
    }
}

int
AtkWire::getNumMsgs()
{
    AtkWireMsg* m= m_head;
	int count;
    for (count=0; m; m=m->m_next) count++;
    return(count);
}

void *
AtkWire::operator new(size_t tSize)
{
	void *p = mlMalloc(tSize);
	return p;
}

void
AtkWire::operator delete(void *p)
{
	mlFree(p);
}
