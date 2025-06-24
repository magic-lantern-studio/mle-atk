/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkWireFunc.cxx
 * @ingroup MleATK
 *
 * This file contains utilities for managing functions that
 * send and recieve messages over a wire.
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

#if defined(_WINDOWS)
#include <windows.h>
#endif
#include <string.h>

//#include <mle/types.h>
#include <mle/MleDsoLoader.h>
#include <mle/mlDebug.h>

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#include <sys/time.h>
#endif

#include "mle/AtkWireFunc.h"

#include "mle/AtkWired.h"
#include "mle/AtkWire.h"

AtkCreateWireFuncArray AtkWireFunc::g_wireFuncs;
AtkCreateWireFuncArray AtkWireFunc::g_recvWireFuncs;

#if defined(__linux__) || defined(__APPLE__)
#define WIRE_FUNC_DSO_PATH "/usr/share/mle/plug-ins/wirefuncs"
#else
#define WIRE_FUNC_DSO_PATH "C:\\Program Files\\Wizzer Works\\Auteur\\Magic Lantern\\plug-ins\\wirefuncs"
#define PATH_MAX 1024
#endif



AtkWireFunc::AtkWireFunc()
{
    m_name = 0;
    m_sendSynced = 0;
    m_recvCB=0;
}

AtkWireFunc::~AtkWireFunc()
{
}

AtkWireFunc*
AtkWireFunc::find(const char* msgName, int recv)
{
    // First search through array.
    AtkWireFunc* wireFunc = findInArray(msgName, recv);
    if (wireFunc) return(wireFunc);

    // Get Class name ...
    char className[500];
    sprintf(className, "Mle%sWireFunc", msgName);

    // and path
    char path[PATH_MAX];
#if defined(_WINDOWS)
	sprintf(path, "%s\\%s.dll", WIRE_FUNC_DSO_PATH, className);
#else
    sprintf(path, "%s/%s.so", WIRE_FUNC_DSO_PATH, className);
#endif

#ifdef TIMELOAD
struct timeval starttime, endtime, dlOpenTime;
gettimeofday(&starttime);
#endif

    // OK, try to load through DSO.
    //MlDSOLoader loader;
    //MlDSOLoader::loadFile(path);
#ifdef _WINDOWS
	HMODULE handle = LoadLibrary((LPCTSTR)path);
#endif

    char dso_func[1024];
    void (*initClass)(void);// initClass function pointer
    initClass = NULL;

#if defined(_WINDOWS)
	sprintf(dso_func,"initClass");
    initClass = (void (*)(void))GetProcAddress(handle,dso_func);
#else
    sprintf(dso_func,"initClass__%ld%sSFv", strlen(className),className);
    //void* handle = dlopen(NULL,RTLD_NOW);
#ifdef TIMELOAD
gettimeofday(&dlOpenTime);
#endif
    initClass = (void (*)(void))dlsym(0,dso_func);
    //initClass = (void (*)(void))dlsym(handle,dso_func);
#endif
    if (!initClass)
	{
		printf("WIREFUNC ERROR: could not load wirefunc class: %s\n", className);
		return(NULL);
    } else
	{
		(*initClass)();
		printf("Init class\n");
    }

    //if (loader.loadClass(className)) {
    //    printf("WIREFUNC ERROR: could not load wirefunc class: %s\n", className);
    //    return(NULL);
    //}

#ifdef TIMELOAD
gettimeofday(&endtime);
float totalTime = (endtime.tv_sec - starttime.tv_sec) +
		 (float(endtime.tv_usec - starttime.tv_usec)/1000000.0);
printf("TotalTime to load wirefunc:  %s: %f\n", className, totalTime);
totalTime = (dlOpenTime.tv_sec - starttime.tv_sec) +
		 (float(dlOpenTime.tv_usec - starttime.tv_usec)/1000000.0);
printf("TotalTime to dlopen:  %s: %f\n", className, totalTime);
#endif

	MLE_DEBUG_CAT("ATK",
		printf("WIREFUNC: Loaded wirefunc class from DSO: %s  for %s array\n", className, recv ? "RECV": "NORMAL");
	);

    // search through array again
    wireFunc = findInArray(msgName, recv);
    if (wireFunc) return(wireFunc);

    // error
    printf("WIREFUNC ERROR: could not find wirefunc class (but loaded dso): %s\n", className);
    return(NULL);
}

AtkWireFunc*
AtkWireFunc::findInArray(const char* name, int recv)
{
    int len = (recv) ? g_recvWireFuncs.getLength() : g_wireFuncs.getLength();
    for (int i=0; i<len; i++)
	{
        struct AtkCreateWireFunc* cwf = (recv) ? g_recvWireFuncs[i]: g_wireFuncs[i];
        if (!strcmp(cwf->name, name))
		{
		    // Create the func
		    AtkWireFunc* wireFunc = (*cwf->createFunc)();
	        return(wireFunc);
        }
	}
    return(NULL);
}

AtkWireFunc*
AtkWireFunc::findRecv(const char* name)
{
    return(find(name, 1));
}

void
AtkWireFunc::addToArray(const char* name, AtkCreateFunc createFunc)
{
    struct AtkCreateWireFunc* cwf = new AtkCreateWireFunc;
    cwf->createFunc = createFunc;
#if defined(_WINDOWS)
	cwf->name = _strdup(name);
#else
    cwf->name = strdup(name);
#endif
    g_wireFuncs.add(cwf);
    //printf("Added: %s to array - len %d\n", name, wireFuncs.getLength());
}

void
AtkWireFunc::addToRecvArray(const char* name, AtkCreateFunc createFunc)
{
    struct AtkCreateWireFunc* cwf = new AtkCreateWireFunc;
    cwf->createFunc = createFunc;
#if defined(_WINDOWS)
	cwf->name = _strdup(name);
#else
    cwf->name = strdup(name);
#endif
    g_recvWireFuncs.add(cwf);
    //printf("Added: %s to recv array - len %d\n", name, recvWireFuncs.getLength());
}

AtkWireMsg* 
AtkWireFunc::sendMsg(AtkWired* wired, AtkWireMsg* msg)
{
    MLE_ASSERT(wired);
    MLE_ASSERT(msg);

    if (m_sendSynced)
	{
		return(wired->getWire()->sendSyncMsg(wired, msg));
    } else
	{
		wired->getWire()->sendMsg(msg);
    }
    return(NULL);

}

AtkWireMsg*
AtkWireFunc::recvMsg(AtkWired*, AtkWireMsg* msg)
{
    MLE_ASSERT(msg);

    if (m_recvCB) (*m_recvCB)(m_recvData, msg);

    return(0);
}

void
AtkWireFunc::printWireFuncs(int recv)
{
    int len = (recv) ? g_recvWireFuncs.getLength() : g_wireFuncs.getLength();
    for (int i=0; i<len; i++)
	{
       struct AtkCreateWireFunc* cwf = (recv) ? g_recvWireFuncs[i]: g_wireFuncs[i];
       printf("WF: %s\n", cwf->name);
    }
}

void *
AtkWireFunc::operator new(size_t tSize)
{
	void *p = mlMalloc(tSize);
	return p;
}

void
AtkWireFunc::operator delete(void *p)
{
	mlFree(p);
}
