/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkWire.h
 * @ingroup MleATK
 *
 * This file contains a class that provides utility for sending and
 * recieving messages over the network.
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
//  For information concerning this header file, contact Mark S. Millard,
//  of Wizzer Works at msm@wizzerworks.com.
//
//  More information concerning Wizzer Works may be found at
//
//      http://www.wizzerworks.com
//
// COPYRIGHT_END

#ifndef __ATK_WIRE_H_
#define __ATK_WIRE_H_

// Include Magic Lantern header files.
#include <mle/mlDebug.h>
#include <mle/mleatk_rehearsal.h>

// Class declarations
class AtkWireMsg;
class AtkWired;

/**
 * This class is used for sending and recieving messages over the network.
 */
class MLEATK_REHEARSAL_API AtkWire 
{
  public:
    /**
	 * A constructor that specifies the read and write file descriptors.
	 *
	 * @param readFD The read file descriptor.
	 * @param writeFD The write file descriptor.
	 */
    AtkWire(int readFD, int writeFD);

    /**
	 * The destructor.
	 */
    virtual ~AtkWire();

    /**
	 * Send a message to the specified destination.
	 *
	 * @param destObj The destination Object to send the message to.
	 * @param msgName The name of the message.
	 * @param msgData A pointer to the message payload.
	 * @param msgDataLen The size of the message payload.
	 *
	 * @return Upon success, <b>0</b> will be returned. If the message is
	 * not sent successfully, then a negative value will be returned.
	 */
    virtual int sendMsg(void* destObj, const char* msgName, 
        void* msgData=0, int msgDataLen=0);
    
	/**
	 * Send a message.
	 *
	 * @param msg A pointer to the message package.
	 *
	 * @return Upon success, <b>0</b> will be returned. If the message is
	 * not sent successfully, then a negative value will be returned.
	 */
    virtual int sendMsg(AtkWireMsg* msg);
    
	/**
	 * Recieve a message.
	 *
	 * @return If a message is successfully recieved, then a pointer
	 * to a message package is returned. Otherwise, <b>NULL</b>
	 * will be returned.
	 */
    virtual AtkWireMsg* recvMsg();
    
	/**
	 * Recieve a message from the read file descriptor.
	 *
	 * @return If a message is successfully recieved, then a pointer
	 * to a message package is returned. Otherwise, <b>NULL</b>
	 * will be returned.
	 */	
    virtual AtkWireMsg* recvMsgFromFD();

	/**
	 * Send a synchronous message to the specified destination.
	 *
	 * @param wired A pointer to the "network", or wire, to
	 * transmit the message on.	 * @param destObj The destination Object to send the message to.
	 * @param msgName The name of the message.
	 * @param msgData A pointer to the message payload.
	 * @param msgDataLen The size of the message payload.
	 *
	 * @return Upon success, a pointer to the reply message package
	 * is returned. Otherwise, <b>null</b> will be returned.
	 */
    virtual AtkWireMsg* sendSyncMsg(AtkWired* wired, void* destObj, 
	    const char* msgName, void* msgData=0, int msgDataLen=0);

	/**
	 * Send a synchronous message.
	 *
	 * @param wired A pointer to the "network", or wire, to
	 * transmit the message on.
	 * @param msg A pointer to the message package to send.
	 *
	 * @return Upon success, a pointer to the reply message package
	 * is returned. Otherwise, <b>null</b> will be returned.
	 */
    virtual AtkWireMsg* sendSyncMsg(AtkWired* wired, AtkWireMsg* msg);

    /**
     * Get the read file descriptor.
	 *
	 * @return The file dscriptor is returned.
	 */
    int getFD() { return m_readFD; }

    /**
	 * Check to see if any messages are pending in the queue.
	 */
    virtual int getNumMsgs();

    /**
	 * Check to see if the connection is lost.
	 */
    int getLostConnection() { return m_lostConnection; }

	/**
	 * Override operator new.
	 *
	 * @param tSize The size, in bytes, to allocate.
	 */
	void* operator new(size_t tSize);

	/**
	 * Override operator delete.
	 *
	 * @param p A pointer to the memory to delete.
	 */
    void  operator delete(void *p);

  protected:

	/** The read file descriptor. */
    int m_readFD;
	/** The write file descriptor. */
	int m_writeFD;
	/** The head of the message queue. */
    AtkWireMsg* m_head;
	/** The tail of the message queue. */
	AtkWireMsg* m_tail;
	/** Flag indicating whether network connection is lost. */
    int m_lostConnection;
};

#endif /* __ATK_WIRE_H_ */
