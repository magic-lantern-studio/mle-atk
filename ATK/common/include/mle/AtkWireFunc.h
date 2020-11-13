/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkWireFunc.h
 * @ingroup MleATK
 *
 * This file contains utilities for managing functions that
 * send and recieve messages over a wire.
 *
 * @author Mark S. Millard
 */

// COPYRIGHT_BEGIN
//
// The MIT License (MIT)
//
// Copyright (c) 2015-2020 Wizzer Works
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

#ifndef __ATK_WIREFUNC_H_
#define __ATK_WIREFUNC_H_

// Include Authoring Toolkit header fiels.
#include <mle/AtkBasicArray.h>

// Declare classes.
class AtkWired;
class AtkWireMsg;


#define ATK_WIREFUNC_HEADER(CLASS)         \
  public:                                  \
	CLASS();                               \
	static void initClass(void);           \
	static AtkWireFunc* createObj(void);

#define ATK_WIREFUNC_RECV_HEADER(CLASS)    \
  public:                                  \
	CLASS();                               \
	static void initClass(void);           \
	static AtkWireFunc* createObj(void);   \
	virtual AtkWireMsg* recvMsg(AtkWired*, AtkWireMsg*);

#define ATK_WIREFUNC_SIMPLE_HEADER(CLASS)  \
class CLASS : public AtkWireFunc {         \
  public:                                  \
	CLASS();                               \
	static void initClass(void);           \
	static AtkWireFunc* createObj(void);   \
};

#define ATK_WIREFUNC_SIMPLE_RECV_HEADER(CLASS)           \
class CLASS : public AtkWireFunc {                       \
  public:                                                \
	CLASS();                                             \
	static void initClass(void);                         \
	static AtkWireFunc* createObj(void);                 \
	virtual AtkWireMsg* recvMsg(AtkWired*, AtkWireMsg*); \
};

#define ATK_WIREFUNC_SOURCE(CLASS, NAME)          \
    CLASS##::CLASS##() { name = NAME; }           \
                                                  \
    AtkWireFunc* CLASS##::createObj()             \
	{                                             \
		return(new CLASS);                        \
	}                                             \
					                              \
    void CLASS##::initClass(void)                 \
	{                                             \
		AtkWireFunc::addToArray(NAME, createObj); \
    }

#define ATK_WIREFUNC_SYNC_SOURCE(CLASS, NAME)     \
    CLASS##::CLASS##()                            \
	{                                             \
		m_name = NAME;                            \
		m_sendSynced = 1;                         \
	}                                             \
					                              \
    AtkWireFunc* CLASS##::createObj()             \
	{                                             \
		return(new CLASS);                        \
	}                                             \
					                              \
    void CLASS##::initClass(void)                 \
	{                                             \
		AtkWireFunc::addToArray(NAME, createObj); \
    }

#define ATK_WIREFUNC_RECV_SOURCE(CLASS, NAME)         \
    CLASS##::CLASS##() { name = NAME; }               \
                                                      \
    AtkWireFunc* CLASS##::createObj()                 \
	{                                                 \
	    return(new CLASS);                            \
	}                                                 \
					                                  \
    void CLASS##::initClass(void)                     \
	{                                                 \
	    AtkWireFunc::addToRecvArray(NAME, createObj); \
    }



// Array of wire funcs structs.
class AtkWireFunc;
typedef AtkWireFunc* (*AtkCreateFunc)();

struct AtkCreateWireFunc
{
    char* name;
    AtkCreateFunc createFunc;
};

MLE_DECLARE_ARRAY(AtkCreateWireFuncArray, AtkCreateWireFunc*);


typedef void (*RecvCallback)(void*, AtkWireMsg*);

class AtkWireFunc
{
  public:
    // Constructor/Destructor
    AtkWireFunc();
    ~AtkWireFunc();

    // Find wire func from list.
    static AtkWireFunc* find(const char* name, int fRecv = 0);
    static AtkWireFunc* findInArray(const char* name, int fRecv);

    static AtkWireFunc* findRecv(const char* name);

    // Adding to array.
    static void addToArray(const char* name, AtkCreateFunc createFunc);
    static void addToRecvArray(const char* name, AtkCreateFunc createFunc);

    // Sending to other wire.
    virtual AtkWireMsg* sendMsg(AtkWired* wired, AtkWireMsg* msg);

    // Recieving.
    virtual AtkWireMsg* recvMsg(AtkWired* wired, AtkWireMsg* msg);

    // Callbacks.
    void setRecvCallback(RecvCallback func, void* data=NULL)
	{ m_recvCB = func; m_recvData = data; }

    RecvCallback getRecvCallback(void **data)
    {
		*data = m_recvData;
		return m_recvCB;
    }

    // Getting the name.
    char* getName() { return m_name; }

    // Printing wirefuncs.
    static void printWireFuncs(int recv=0);

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

    RecvCallback m_recvCB;
    void* m_recvData;

    char* m_name;
    int m_sendSynced;

    // Add a function to list.
    // Array of create funcs.
    static AtkCreateWireFuncArray g_wireFuncs;
    static AtkCreateWireFuncArray g_recvWireFuncs;
};

#endif /* __ATK_WIREFUNC_H_ */
