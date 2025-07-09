/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkWireMsg.h
 * @ingroup MleATK
 *
 * This file contains a class that defines an ATK message.
 */

// COPYRIGHT_BEGIN
//
// The MIT License (MIT)
//
// Copyright (c) 2015-2025 Wizzer Works
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

#ifndef __ATK_WIREMSG_H_
#define __ATK_WIREMSG_H_

// Include Magic Lantern header files.
#include <mle/mleatk_rehearsal.h>

#define MAX_MSG_NAME_LEN 32

#define REPLY_MSG_NAME "Reply"

class MlTransform;


class MLE_ATK_API AtkWireMsg 
{
  public:

    AtkWireMsg(void* destObj=0, const char* msgName=0, int waitForReply=0, 
	      void* msgData=0, int msgDataLen=0);

    virtual ~AtkWireMsg();

    // Setting msgName and destination
    virtual void setMsgName(const char* name);

    virtual void setDestObj(void* destObj);

    // Getting start address and length
    virtual void* getStartAddress();

    virtual int getHeaderLength();

    virtual int getDataLength();

    // alloc buffer and copy data
    virtual void allocMsgData();

    virtual void setMsgData(void* msgData, int msgDataLen);

    // a  or sync message reply msg
    // Note: naming a little confusing - a reply message is something the 
    // other side sends to you as a result of a sync message, a sync message
    // is something that will result in a reply message
    virtual int isReplyMsg();

    int isSyncMsg() { return m_waitForReply; }


    // Building a message.
    virtual void addParam(int i);
 
    virtual void addParam(const char* s);

    virtual void addParam(void* data, int len);

    virtual void addParam(MlTransform &xform);

    virtual void addParam(const char**);

    virtual void addParam(const float f[3]);


    // Getting the parameters.
    virtual void resetParam();

    virtual int getParam(int &i);

    virtual int getParam(char* s);
 
    virtual int getParam(void* &data, int& len);

    virtual int getParam(void* data);

    virtual int getParam(MlTransform &xform1);

    virtual int getParam(char*** strArray);

    virtual int getParam(float f[3]);

	/**
	 * Override operator new.
	 *
	 * @param tSize The size, in bytes, to allocate.
	 */
	void* operator new(size_t tSize);

    /**
     * Override operator new array.
     *
     * @param tSize The size, in bytes, to allocate.
     */
    void* operator new[](size_t tSize);

	/**
	 * Override operator delete.
	 *
	 * @param p A pointer to the memory to delete.
	 */
    void  operator delete(void *p);

    /**
     * Override operator delete array.
     *
     * @param p A pointer to the memory to delete.
     */
    void  operator delete[](void* p);

    /**
	 * Print out wire msg for debugging purposes.
	 *
	 * @param headerMsg An optional string to print before dumping the msg itself.
	 */
    virtual void print(char *headerMsg = 0);
    
    /** The total size of the message. */
    int m_totalMsgLen;
	/** The name of the message. */
    char m_msgName[MAX_MSG_NAME_LEN];
	/** The object to send the message to. */
    void* m_destObj;
	/** Flag indicating whether to wait for the reply. */
    char m_waitForReply;
	/** A pointer to the message data. */
    void* m_msgData;
    /** A pointer to the next message. */
    AtkWireMsg* m_next;
	/** The current parameter offset. */
    int m_curParamOffset;
};

#endif /* __ATK_WIREMSG_H_ */
