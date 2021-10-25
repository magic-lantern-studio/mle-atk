/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkWired.h
 * @ingroup MleATK
 *
 * This file contains a class that manages messages sent and
 * recieved over a wire.
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

#ifndef __ATK_WIRED_H_
#define __ATK_WIRED_H_

// Include system header files.
#if defined(sgi)
#include <unistd.h>
#endif

// Include Authoring 
#include <mle/mleatk_rehearsal.h>
#include <mle/AtkBasicArray.h>

// Declare classes.
class AtkWire;
class AtkWireMsg;
class AtkWireFunc;

MLE_DECLARE_ARRAY(AtkWireFuncArray, AtkWireFunc*);


class MLEATK_REHEARSAL_API AtkWired 
{
  public:
    
    AtkWired(const char* name, AtkWire* wire, void* objID);

    ~AtkWired();

    // delivering msgs
    virtual AtkWireMsg* deliverMsg(AtkWireMsg* msg);

    virtual AtkWireMsg* recvAndDeliverMsg(); 

    // Getting the FD
    virtual int getFD();

    // Sending an ID msg
    virtual void sendID();

    // check for pending msgs
    virtual int pendingMsgs();

    // find wire func from list
    virtual AtkWireFunc* find(const char* name, int recv = 0);

    virtual AtkWireFunc* findRecv(const char* name);

    virtual AtkWireFunc* findInArray(const char* name, int recv);

    // adding wire to array
    virtual void addToArray(AtkWireFunc* wf);

    virtual void addToRecvArray(AtkWireFunc* wf);

    // getting data fields
    char* getName() { return m_name; }

    AtkWire* getWire() { return m_wire; }
 
    void setWire(AtkWire* w) { m_wire = w; }

#if defined(WIN32)
	int getPID() { return m_pid; }
    void setPID(int p) { m_pid = p; }
#else
    pid_t getPID() { return m_pid; }
    void setPID(pid_t p) { m_pid = p; }
#endif

    void* getObjID() { return m_objID; }

    void* getUserData() { return m_userData; }

    void* getUserData2() { return m_userData2; }

    void setUserData(void* d) { m_userData = d; }

    void setUserData2(void* d) { m_userData2 = d; }

    void setParentData(void* d) { m_parentData = d; }

    void* getParentData() { return m_parentData; }

    void setWindowData(int d) { m_windowData = d; }

    int getWindowData() { return m_windowData; }

    void printWireFuncs(int recv);

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

    AtkWire* m_wire;
    void* m_objID;
    char* m_name;
#if defined(WIN32)
	int m_pid;
#else
    pid_t m_pid;
#endif
    void* m_userData;
    void* m_userData2;

    void* m_parentData;

    int m_windowData;

    AtkWireFuncArray m_wireFuncs;
    AtkWireFuncArray m_recvWireFuncs;
};

#endif /* __ATK_WIRED_H_ */
