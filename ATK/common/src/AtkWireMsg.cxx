/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkWireMsg.h
 * @ingroup MleATK
 *
 * This file contains the implementation of a class that defines a message.
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
//  For information concerning this source code, contact Mark S. Millard,
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
#include <mle/mlMalloc.h>
#include <math/transfrm.h>

// Include Authoring Toolkit header files.
#include "mle/AtkWire.h"
#include "mle/AtkWireMsg.h"


AtkWireMsg::AtkWireMsg(void* destObj, const char* msgName, int waitForReply, 
	void* msgData, int msgDataLen)
{
    if (msgName) strncpy(this->m_msgName, msgName, MAX_MSG_NAME_LEN);
    this->m_msgName[MAX_MSG_NAME_LEN - 1] = 0;
    this->m_waitForReply = waitForReply;
    this->m_destObj = destObj;

    // Copy data - note that this is a little controversial since you must
    // allocate a data slot just to send a msg.
    this->m_msgData = 0;
    if (msgData && msgDataLen > 0)
	{
		this->m_msgData = mlMalloc(msgDataLen);
		//bcopy(msgData, this->msgData, msgDataLen);
		memcpy(this->m_msgData, msgData, msgDataLen);
	}
    this->m_totalMsgLen = msgDataLen+getHeaderLength();

    // Initialize next field.
    m_next = NULL;
    m_curParamOffset = 0;
}

AtkWireMsg::~AtkWireMsg()
{
    // Warning if data not consumed by receiver.
    MLE_DEBUG_CAT("ATK",
		if (m_msgData && m_curParamOffset != 0 && m_curParamOffset != getDataLength())
		{
			fprintf(stderr, "*** WARNING: AtkWireMsg: data not consumed evenly:\n"
				"name %s, curParamOffset %d, dataLength: %d\n",
				m_msgName, m_curParamOffset, getDataLength());
		}
    );

    if (m_msgData) mlFree(m_msgData);
}

void
AtkWireMsg::setMsgName(const char* name)
{
    MLE_ASSERT(name);
    MLE_ASSERT(strlen(name) < MAX_MSG_NAME_LEN);

    strcpy(m_msgName, name);
}

void
AtkWireMsg::setDestObj(void* obj)
{
    m_destObj = obj;
}


void* 
AtkWireMsg::getStartAddress()
{
    return((void *) (&m_totalMsgLen));
}


// XXX - this relies on the data packing 
int 
AtkWireMsg::getHeaderLength()
{
    return(sizeof(int) + MAX_MSG_NAME_LEN + sizeof(void*) + sizeof(char));
    //return(((char*) &m_msgData) - ((char*) &m_totalMsgLen));
}

int 
AtkWireMsg::getDataLength()
{
    return(m_totalMsgLen - getHeaderLength());
}

void
AtkWireMsg::allocMsgData()
{
    if (m_msgData) mlFree(m_msgData);
	m_msgData = 0;
    if (getDataLength() > 0)
	{
		m_msgData = mlMalloc(getDataLength());
    }
}

void
AtkWireMsg::setMsgData(void* data, int len)
{
    if (m_msgData) mlFree(m_msgData);
    m_msgData = 0;
    if (len > 0) {
		m_msgData = mlMalloc(len);
		//if (data) bcopy(data, msgData, len);
		if (data) memcpy(m_msgData, data, len);
    }
    m_totalMsgLen = len+getHeaderLength();
}

int 
AtkWireMsg::isReplyMsg()
{
#if defined(_WINDOWS)
	return(!_stricmp(m_msgName, REPLY_MSG_NAME));
#else
    return(!strcasecmp(m_msgName, REPLY_MSG_NAME));
#endif /* ! _WINDOWS */
}

void 
AtkWireMsg::addParam(int i)
{
    m_totalMsgLen += sizeof(int);
    m_msgData = mlRealloc(m_msgData, getDataLength());
    //bcopy(&i, ((char*) msgData) + getDataLength() - sizeof(int), sizeof(int));
    memcpy(((char*) m_msgData) + getDataLength() - sizeof(int), &i, sizeof(int));
}

void 
AtkWireMsg::addParam(const char* s)
{
    if (!s) s = "";
    m_totalMsgLen += strlen(s)+1;
    m_msgData = mlRealloc(m_msgData, getDataLength());
    strcpy(((char*) m_msgData) + getDataLength() - strlen(s)-1, s);
}

void 
AtkWireMsg::addParam(void* data, int len)
{
    if (!data || !len) return;
    m_totalMsgLen += len + sizeof(int);
    m_msgData = mlRealloc(m_msgData, getDataLength());
    //bcopy(&len, ((char*) msgData) + getDataLength()-len-sizeof(int), sizeof(int));
    //bcopy(data, ((char*) msgData) + getDataLength() - len, len);
	memcpy(((char*) m_msgData) + getDataLength()-len-sizeof(int), &len, sizeof(int));
    memcpy(((char*) m_msgData) + getDataLength() - len, data, len);
}

// NOTE: This is important to be pass by value (&xform) because
// otherwise a caller can mistakenly pass a MlTransform, which will
// get cast to a float [], which is same as float*, and will run a
// wrong version of addParam()!
void
AtkWireMsg::addParam(MlTransform &xform)
{
    int len = sizeof(MlTransform);
    m_totalMsgLen += len;
    m_msgData = mlRealloc(m_msgData, getDataLength());
    //bcopy(&xform, ((char*) msgData) + getDataLength() - len, len);
	memcpy(((char*) m_msgData) + getDataLength() - len, &xform, len);
}

void
AtkWireMsg::addParam(const char** strArray)
{
    // Calc length of str array.
    int len=0;
    for (int i=0; strArray && strArray[i] && *(strArray[i]); i++)
	{
		len += strlen(strArray[i]) + 1;
    }
    len++;

    // Realloc buffer length.
    m_totalMsgLen += len;
    m_msgData = mlRealloc(m_msgData, getDataLength());

    // Copy data.
    int index = getDataLength() - len;
    for (int j=0; strArray && strArray[j] && *(strArray[j]); j++)
	{
		//bcopy(strArray[j], ((char*) msgData) + index, strlen(strArray[j])+1);
		memcpy(((char*) m_msgData) + index, strArray[j], strlen(strArray[j])+1);
		index += strlen(strArray[j]) + 1;
    }
    *(((char*) m_msgData)+index) = 0;
}

void AtkWireMsg::addParam(const float f[3])
{
    m_totalMsgLen += sizeof(float) * 3;
    m_msgData = mlRealloc(m_msgData, getDataLength());
    //bcopy(f, ((char*) msgData) + getDataLength() - sizeof(float)*3, 
	//sizeof(float)*3);
	memcpy(((char*) m_msgData) + getDataLength() - sizeof(float)*3, f,
	sizeof(float)*3);
}

void 
AtkWireMsg::resetParam()
{
    m_curParamOffset = 0;
}

int 
AtkWireMsg::getParam(int &i)
{
    if (m_curParamOffset + sizeof(int) > (unsigned int) getDataLength())
	{
		printf("WM: getParam (int) error curParamOffset: %d  dataLength: %d\n",
			 m_curParamOffset, getDataLength());
		return(-1);
    }
    //bcopy(((char*) msgData) + curParamOffset, &i, sizeof(int));
	memcpy(&i, ((char*) m_msgData) + m_curParamOffset, sizeof(int));
    m_curParamOffset += sizeof(int);
    return(0);
}

int 
AtkWireMsg::getParam(char* s)
{
    strcpy(s, ((char*) m_msgData) + m_curParamOffset);
    if (m_curParamOffset + strlen(s) + 1 > (unsigned int) getDataLength())
	{
        printf("WM: getParam (char*) error curParamOffset: %d  str: %s, %zu  dataLength: %d\n",
			 m_curParamOffset, s, strlen(s), getDataLength());
		return(-1);
    }
    m_curParamOffset += strlen(s)+1;
    return(0);
}

int 
AtkWireMsg::getParam(void* &data, int& len)
{
    if (m_curParamOffset + sizeof(int) > (unsigned int) getDataLength())
	{
		printf("WM: getParam (void*) error curParamOffset: %d  dataLength: %d\n",
			 m_curParamOffset, getDataLength());
		return(-1);
    }
    //bcopy(((char*) msgData) + curParamOffset, &len, sizeof(int));
	memcpy(&len, ((char*) m_msgData) + m_curParamOffset, sizeof(int));
    m_curParamOffset += sizeof(int);

    data = NULL;
    if (len <= 0) return -1;
    data = mlMalloc(len);
    if (m_curParamOffset + len > getDataLength())
	{
		printf("WM: getParam (void*) error curParamOffset: %d  len: %d  dataLength: %d\n",
			 m_curParamOffset, len, getDataLength());
		return(-1);
    }
    //bcopy(((char*) msgData) + curParamOffset, data, len);
	memcpy(data, ((char*) m_msgData) + m_curParamOffset, len);
    m_curParamOffset += len;
    return(0);
}

int
AtkWireMsg::getParam(void* data)
{
    int len;
    if (m_curParamOffset + sizeof(int) > (unsigned int) getDataLength())
	{
		printf("WM: getParam (void*) error curParamOffset: %d  dataLength: %d\n",
			 m_curParamOffset, getDataLength());
		return(-1);
    }
    //bcopy(((char*) msgData) + curParamOffset, &len, sizeof(int));
	memcpy(&len, ((char*) m_msgData) + m_curParamOffset, sizeof(int));
    m_curParamOffset += sizeof(int);

    if (len <= 0) return -1;
    if (m_curParamOffset + len > getDataLength())
	{
		printf("WM: getParam (void*) error curParamOffset: %d  len: %d  dataLength: %d\n",
			 m_curParamOffset, len, getDataLength());
		return(-1);
    }
    //bcopy(((char*) msgData) + curParamOffset, data, len);
	memcpy(data, ((char*) m_msgData) + m_curParamOffset, len);
    m_curParamOffset += len;
    return(len);
}

// NOTE: This is important to be pass by value (&xform) because
// otherwise a caller can mistakenly pass a MlTransform, which will
// get cast to a float [], which is same as float*, and will run a
// wrong version of getParam()!

int
AtkWireMsg::getParam(MlTransform &t)
{
    int len = sizeof(MlTransform);

    if (m_curParamOffset + len > getDataLength())
	{
		printf("WM: getParam (FwTransform*) error curParamOffset: %d  len: %d  dataLength: %d\n",
			 m_curParamOffset, len, getDataLength());
		return(-1);
    }
    //bcopy(((char*) msgData) + curParamOffset, &t, len);
	memcpy(&t, ((char*) m_msgData) + m_curParamOffset, len);
    m_curParamOffset += len;
    return(len);
}

// Caller frees strArray.
int 
AtkWireMsg::getParam(char*** strArray)
{
    // Find number and length.
    char* p= ((char*) m_msgData) + m_curParamOffset;
    int len = 0, numStrs = 0;
    for (; *p;  numStrs++,p += strlen(p)+1) len += strlen(p)+1;

    // Check.
    if (m_curParamOffset + len > getDataLength())
	{
		printf("WM: getParam (char**) error curParamOffset: %d  len: %d  dataLength: %d\n",
			 m_curParamOffset, len, getDataLength());
		return(-1);
    }
    
    // Allocate space.
    *strArray = (char**) mlMalloc(sizeof(char*) * (numStrs+1));

    // get data.
    p = ((char*) m_msgData) + m_curParamOffset;
	int i;
    for (i=0; *p; p+=strlen(p)+1, i++)
	{
		(*strArray)[i] = strdup(p);
    }
    (*strArray)[i] = NULL;

    return(len);
}

int AtkWireMsg::getParam(float f[3])
{
    int len = sizeof(float) * 3;

    if (m_curParamOffset + len > getDataLength())
	{
		printf("WM: getParam (float f[3]) error curParamOffset: %d  len: %d  dataLength: %d\n",
			 m_curParamOffset, len, getDataLength());
		return(-1);
    }
    
    //bcopy(((char*) msgData) + curParamOffset, f, len);
	memcpy(f, ((char*) m_msgData) + m_curParamOffset, len);
    m_curParamOffset += len;
    return(len);
}

//
// Print out wire msg for debugging purposes; headerMsg is optional
// string to print before dumping the msg itself.
//

void AtkWireMsg::print(char *headerMsg)
{
    if (headerMsg)
		fprintf(stderr, "AtkWireMsg: %s\n", headerMsg);

    fprintf(stderr, "AtkWireMsg: name '%s', DataLen %d, waitForReply %d\n", 
		m_msgName, getDataLength(), m_waitForReply);
    fprintf(stderr, "AtkWireMsg: curParamOffset %d", m_curParamOffset);

    for (int i = 0; i < getDataLength(); i++)
    {
		if (i % 16 == 0)
			fprintf(stderr, "\nAtkWireMsg (%x): ", ((char*)m_msgData)+i);
		fprintf(stderr, "%02hx ", *(((char*) m_msgData)+i));
    }
    fprintf(stderr, "\n");
}

void *
AtkWireMsg::operator new(size_t tSize)
{
	void *p = mlMalloc(tSize);
	return p;
}

void
AtkWireMsg::operator delete(void *p)
{
	mlFree(p);
}
