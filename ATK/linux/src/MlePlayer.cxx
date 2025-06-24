/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file MlePlayer.h
 * @ingroup MleATK
 *
 * This file contains the implementation of a class that provides utility for
 * managing a Magic Lantern Rehearsal Player.
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
//  For information concerning this source file, contact Mark S. Millard,
//  of Wizzer Works at msm@wizzerworks.com.
//
//  More information concerning Wizzer Works may be found at
//
//      http://www.wizzerworks.com
//
// COPYRIGHT_END

#if defined(__linux__) || defined (__APPLE__)
#include <signal.h>
#if defined(MLE_QT)
#include <QWindow>
#endif
#endif

#include "mle/MlePlayer.h"

#include "mle/AtkWire.h"
#include "mle/AtkWireMsg.h"
#include "mle/AtkCommonStructs.h"

#include <mle/mlFileio.h>

#include <math/transfrm.h>

#include <mle/DwpItem.h>
#include <mle/DwpActor.h>
#include <mle/DwpInput.h>
#include <mle/DwpStrKeyDict.h>
#include <mle/DwpDatatype.h>
#include <mle/DwpDSOFile.h>
#include <mle/DwpMediaRef.h>
#include <mle/DwpFinder.h>
#include <mle/DwpGroup.h>
#include <mle/DwpScene.h>
#include <mle/DwpActorDef.h>
#include <mle/DwpDataUnion.h>
#include <mle/DwpOutput.h>


#include "mle/MleStageFuncs.h"
#include "mle/MleActor.h"
#include "mle/MleActorClass.h"
#include "mle/MleRole.h"
#include "mle/MleSet.h"
#include "mle/MleGroup.h"
#include "mle/MleGroupClass.h"
#include "mle/MleScene.h"
#include "mle/MleSceneClass.h"


#include "mle/MleLoad.h"
#include "mle/MleStage.h"

#include "math/3dmath.h"

#include "mle/iv3dset.h"
//#include "mle/br3dfrm.h" //Temporary only.
#include "mle/3dcamc.h"


// Todo: might want to change hardcoded strings to the name of the method
//       if the compiler allows this.

#define MAX_NAME_LENGTH 200

/*****************************************************************************
* Constructor, destructor, creation
*****************************************************************************/

#if defined(_WINDOWS)
typedef void (__cdecl *SIG_PF)(int signal);
#endif /* _WINDOWS */
#if defined(__linux__)
#define SIG_PF sighandler_t
#endif /* __linux__ */
typedef void (*SIG_PF) (int);
#if defined(__APPLE__)
#endif /* __APPLE__ */

MlePlayer::MlePlayer(AtkWire* wire, void* objID) 
 : AtkWired("Player", wire, objID)
{
    m_running = 0;
    refreshCB = NULL;
    frameAdvanceCB = NULL;
    toolsInitFinishedCB = NULL;
    m_quit = 0;

    m_prefWidth = m_prefHeight = 0;

    m_curStageEditMode = 1;
    m_curPlacementState = FLOATING;

    m_sendStats = 0;

    // Trap fatal signals to fflush diagnostic (stdout, stderr) pipes to tools.
#if defined(__linux__) || defined(__APPLE__)
    signal(SIGBUS, (SIG_PF) signalHandler);
	signal(SIGSEGV, (SIG_PF) signalHandler);
    signal(SIGABRT, (SIG_PF) signalHandler);
	signal(SIGSYS, (SIG_PF) signalHandler);
#endif /* __linux__ */
#if defined(_WINDOWS)
	signal(SIGSEGV, (SIG_PF) signalHandler);
    signal(SIGABRT, (SIG_PF) signalHandler);
#endif /* _WINDOWS */
}

void MlePlayer::signalHandler(int signal, ...)
{
    printf("Player caught signal %d.\n", signal);
    mlFFlush(stdout);
    mlFFlush(stderr);

    abort();
}

MlePlayer::~MlePlayer()
{
    while (m_propArray.getLength() > 0)
	{
        MlePropStruct *current = m_propArray[0];
		m_propArray.remove(0);
		mlFree(current->m_property); // Allocated in strdup.
		mlFree(current->m_data);
		delete current;
    }
}

#if defined(_WINDOWS)
#define STDOUT_FILENO _fileno(stdout)
#define STDERR_FILENO _fileno(stderr)
#endif /* _WINDOWS */

/*****************************************************************************
* Creating a player
*****************************************************************************/
MlePlayer*
MlePlayer::create(int argc, char* argv[])
{
    // Check argc.
    if (argc < 5) return(NULL);

    // Get params
    int readFD= -1;
    int writeFD= -1;
    int errorFD = -1;
    void* m_objID = 0;
    sscanf(argv[1], "%d", &writeFD);
	MLE_DEBUG_CAT("ATK",
		printf("Write FD = %d\n", writeFD);
	);
    sscanf(argv[2], "%d", &readFD);
	MLE_DEBUG_CAT("ATK",
		printf("Read FD = %d\n", readFD);
	);
    sscanf(argv[3], "%d", &errorFD);
	MLE_DEBUG_CAT("ATK",
		printf("ErrorFD = %d\n", errorFD);
	);
    sscanf(argv[4], "%p", &m_objID);
	MLE_DEBUG_CAT("ATK",
		printf("ObjID = %d\n", m_objID);
	);

    // Create wire.
    AtkWire* wire = new AtkWire(readFD, writeFD);

    // Create a player and set up callbacks.
    MlePlayer* player = new MlePlayer(wire, m_objID);
    player->setErrorFD(errorFD);

    // dup off error FD to stdin and stderr.
    if (dup2(errorFD, STDOUT_FILENO) != STDOUT_FILENO)
	{
		printf("Player Error: Could not dup stdout\n");
    }
    if (dup2(errorFD, STDERR_FILENO) != STDERR_FILENO)
	{
		printf("Player Error: Could not dup sterr\n");
    }
    close(errorFD);

    return(player);
}

/*****************************************************************************
* Delivering msgs
*****************************************************************************/
AtkWireMsg*
MlePlayer::deliverMsg(AtkWireMsg* msg)
{
    // Check for errors.
    if (!msg) return(0);

    // XXX - shouldn't all these strings be constants instead? Why not
    // include the header files from authoring/wirefuncs?

    if (!strcmp("Init", msg->m_msgName))
	{
		int w, h;
		int ret = msg->getParam(w);
		if (ret >=0) ret = msg->getParam(h);

		// Check parameters
		if (ret < 0)
		{
			// Error - data length incorrect
			printf("MlePlayer::deliverMsg - Init msg wrong length: %d\n",
			   msg->getDataLength());
			return(0);
		}

		// Get window parameter and call init proc
		recvInit(w,h);

    } else if (!strcmp("Play", msg->m_msgName))
	{
		recvPlay();

    } else if (!strcmp("Pause", msg->m_msgName))
	{
		recvPause();

    } else if (!strcmp("MoveToTarget", msg->m_msgName))
	{
		recvMoveToTarget();

    } else if (!strcmp("Nudge", msg->m_msgName))
	{
		int dir, numPixels;
		int ret = msg->getParam(dir);
		if (ret >=0) ret = msg->getParam(numPixels);

		// Check parameters.
		if (ret < 0)
		{
			// Error - data length incorrect.
			printf("MlePlayer::deliverMsg - Nudge msg wrong length: %d\n",
			   msg->getDataLength());
			return(0);
		}
		recvNudge(dir, numPixels);

    } else if (!strcmp("FindActor", msg->m_msgName))
	{
		// Check parameters.
		if (msg->getDataLength() <= 0 )
		{
			// Error no data passed in.
			printf("MlePlayer::deliverMsg - FindActor msgData < 0\n");
			return(0);
		}

		// Call find.
		recvFindActor((char*) msg->m_msgData);

    } else if (!strcmp("GetActorPropertyNames", msg->m_msgName))
	{
		char actorName[MAX_NAME_LENGTH];
		char propDataset[MAX_NAME_LENGTH];
		int ret;

        actorName[0] = propDataset[0] = '\0';
		ret = msg->getParam(actorName);
		if (ret >= 0)
			ret = msg->getParam(propDataset);
		if (ret < 0)
		{
			printf("MlePlayer::deliverMsg - GetActorPropertyNames failed\n");
			return 0;
		}

		// Get all the property names of a property dataset.
		recvGetActorPropertyNames(actorName, propDataset);

    } else if (!strcmp("GetActorProperty", msg->m_msgName))
	{
		char actorClass[MAX_NAME_LENGTH];
		char actorName[MAX_NAME_LENGTH];
		char propName[MAX_NAME_LENGTH];
		actorClass[0] = actorName[0] = propName[0] = 0;

		// Get and Check parameters.
		int ret = msg->getParam(actorClass);
		if (ret >=0) ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(propName);
		if (ret < 0)
		{
			// Error could not get data.
			printf("MlePlayer::deliverMsg - getActorProperty failed\n");
			return(0);
		}

		// Get the properties.
		recvGetActorProperty(actorClass, actorName, propName);

    } else if (!strcmp("SetActorProperty", msg->m_msgName))
	{
		char actorClass[MAX_NAME_LENGTH];
		char actorName[MAX_NAME_LENGTH];
		char propName[MAX_NAME_LENGTH];
		void* data = 0;
		actorClass[0] = actorName[0] = propName[0] = 0;

		// Check parameters.
		int len;
		int ret = msg->getParam(actorClass);
		if (ret >=0) ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(propName);
		if (ret >=0) ret = msg->getParam(data, len);
		if (ret < 0)
		{
			printf("ERROR MlePlayer::deliverMsg - setActorProperty failed\n");
			return(0);
		}

		MLE_DEBUG_CAT("ATK",
			printf("Msg %s  Len: %d, AC: %s, AN: %s,  PN: %s\n", msg->m_msgData, msg->getDataLength(), actorClass, actorName, propName);
		);

		// Set the properties.
		recvSetActorProperty(actorClass, actorName, propName, data);
		if (data) mlFree(data);

    } else if (!strcmp("SetActorName", msg->m_msgName))
	{
		char actorName[MAX_NAME_LENGTH];
		char newActorName[MAX_NAME_LENGTH];
		actorName[0] = newActorName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(newActorName);
		if (ret < 0)
		{
			printf("ERROR MlePlayer::deliverMsg - setActorName failed\n");
			return(0);
		}

		MLE_DEBUG_CAT("ATK",
			printf("Msg %s  Len: %d, AN: %s, new AN: %s\n", msg->m_msgData, msg->getDataLength(), actorName, newActorName);
		);

		// Set the properties
		recvSetActorName(actorName, newActorName);

    } else if (!strcmp("GetActorIsA", msg->m_msgName))
	{
		char actorName[MAX_NAME_LENGTH];
		char actorClass[MAX_NAME_LENGTH];
		int ret;

		actorName[0] = actorClass[0] = 0;
		ret = msg->getParam(actorName);
		if(ret >= 0)
			ret = msg->getParam(actorClass);
		if(ret < 0) {
			printf("MlePlayer::deliverMsg - GetActorIsA failed\n");
			return 0;
		}

		// Check if actor is type of actorClass or subclass of actorClass.
		recvGetActorIsA(actorName, actorClass);

    } else if (!strcmp("SetTransform", msg->m_msgName))
	{
		char actorName[MAX_NAME_LENGTH];
		MlTransform t;
		
		int ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(t);
		if (ret < 0)
		{
			printf("ERROR MlePlayer::deliverMsg - SetTransform failed\n");
			return(0);
		}

		recvSetTransform(actorName, t);

    } else if (!strcmp("GetTransform", msg->m_msgName))
	{
		char actorName[MAX_NAME_LENGTH];
		
		int ret = msg->getParam(actorName);
		if (ret < 0)
		{
			printf("ERROR MlePlayer::deliverMsg - SetTransform failed\n");
			return(0);
		}

		recvGetTransform(actorName);

    } else if (!strcmp("Pick", msg->m_msgName))
	{
		// Get and check parameters.
		int x, y;
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;
		int ret = msg->getParam(x);
		if (ret >=0) ret = msg->getParam(y);
		if (ret >=0) ret = msg->getParam(setName);
		if (ret < 0)
		{
			printf("ERROR FWPlayer::deliverMsg - pick params incorrect\n");
			return(0);
		}

		// Pick actors
		recvPick(setName, x, y);

    } else if (!strcmp("Refresh", msg->m_msgName))
	{
		recvRefresh();

    } else if (!strcmp("FrameAdvance", msg->m_msgName))
	{
		recvFrameAdvance();

    } else if (!strcmp("SetCamera", msg->m_msgName))
	{
		if (!msg->m_msgData) return(0);
		char setName[MAX_NAME_LENGTH];
		char cameraName[MAX_NAME_LENGTH];
		cameraName[0] = setName[0] = 0;
		int ret = msg->getParam(setName);
		if (ret >=0) ret = msg->getParam(cameraName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - setCamera params incorrect\n");
			return(0);
		}

		recvSetCamera(setName, cameraName);

    } else if (!strcmp("LoadGroup", msg->m_msgName))
	{
		recvLoadGroup((char*) msg->m_msgData);

    } else if (!strcmp("UnloadGroup", msg->m_msgName))
	{
		recvUnloadGroup((char*) msg->m_msgData);

    } else if (!strcmp("LoadScene", msg->m_msgName))
	{
		recvLoadScene((char*) msg->m_msgData);

    } else if (!strcmp("UnloadScene", msg->m_msgName))
	{
		recvUnloadScene((char*) msg->m_msgData);

    } else if (!strcmp("UnloadActor", msg->m_msgName))
	{
		recvUnloadActor((char*) msg->m_msgData);

    } else if (!strcmp("ActivateManip", msg->m_msgName))
	{
		recvActivateManip((char*) msg->m_msgData);

    } else if (!strcmp("DeactivateManip", msg->m_msgName))
	{
		recvDeactivateManip((char*) msg->m_msgData);

    } else if (!strcmp("Quit", msg->m_msgName))
	{
		recvQuit();

    } else if (!strcmp("WorkprintItem", msg->m_msgName))
	{
		recvWorkprintItem((char*) msg->m_msgData);

    } else if (!strcmp("StageEditMode", msg->m_msgName)) 
	{
		int mode;
		int ret = msg->getParam(mode);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - stageEditMode params incorrect\n");
			return(0);
		}
		recvStageEditMode(mode);

    } else if (!strcmp("PlacementState", msg->m_msgName))
	{
		int state;
		int ret = msg->getParam(state);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - stageEditMode params incorrect\n");
			return(0);
		}
		recvPlacementState(state);

    } else if (!strcmp("Resize", msg->m_msgName))
	{
		// Get and check parameters.
		int w, h;
		int ret = msg->getParam(w);
		if (ret >=0) ret = msg->getParam(h);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - resize params incorrect\n");
			return(0);
		}

		recvResize(w, h);

    } else if (!strcmp("SetPosition", msg->m_msgName))
	{
		// Get and check parameters.
		int x, y;
		char actorName[MAX_NAME_LENGTH];
		char setName[MAX_NAME_LENGTH];
		actorName[0] = 0;
		setName[0] = 0;
		int ret = msg->getParam(setName);
		if (ret >=0) ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(x);
		if (ret >=0) ret = msg->getParam(y);
		if (ret < 0)
		{
			printf("ERROR MlePlayer::deliverMsg - setPositions params incorrect\n");
			return(0);
		}

		recvSetPosition(setName, actorName, x, y);

    } else if (!strcmp("ResolveEdit", msg->m_msgName))
	{
		// Get and check parameters.
		char actorName[MAX_NAME_LENGTH];
		actorName[0] = 0;
		int ret = msg->getParam(actorName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - resolveEdit params incorrect\n");
			return(0);
		}

		recvResolveEdit(actorName);

    } else if (!strcmp("ResolveEditProperty", msg->m_msgName))
	{
		// Get and check parameters.
		char actorName[MAX_NAME_LENGTH];
		char propName[MAX_NAME_LENGTH];
		actorName[0] = 0;
		int ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(propName);
		if (ret < 0)
		{
			printf("ERROR MlePlayer::deliverMsg - resolveEditProperty params incorrect\n");
			return(0);
		}

		recvResolveEdit(actorName, propName);

    } else if (!strcmp("GetCameraPosition", msg->m_msgName))
	{
		// Get and check parameters.
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;
		int ret = msg->getParam(setName);
		if (ret < 0)
		{
			printf("ERROR MlePlayer::deliverMsg - getCameraPosition params incorrect\n");
			return(0);
		}

		recvGetCameraPosition(setName);

    } else if (!strcmp("SetCameraPosition", msg->m_msgName))
	{
		// Get and check parameters.
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;
		MlTransform t;
		
		int ret = msg->getParam(setName);
		if (ret >=0) ret = msg->getParam(t);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - setCameraPosition params incorrect\n");
			return(0);
		}

		recvSetCameraPosition(setName, &t);

    } else if (!strcmp("StartStats", msg->m_msgName))
	{
		recvStartStats();

    } else if (!strcmp("EndStats", msg->m_msgName))
	{
		recvEndStats();

    } else if (!strcmp("RegisterProp", msg->m_msgName))
	{
		char actorName[MAX_NAME_LENGTH];
		char propName[MAX_NAME_LENGTH];
		actorName[0] = propName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(propName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - registerProp failed\n");
			return(0);
		}

		// Set the properties.
		recvRegisterProperty(actorName, propName);

    } else if (!strcmp("UnregisterProp", msg->m_msgName))
	{
		char actorName[MAX_NAME_LENGTH];
		char propName[MAX_NAME_LENGTH];
		actorName[0] = propName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(propName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - unregisterProp failed\n");
			return(0);
		}

		// Set the properties.
		recvUnregisterProperty(actorName, propName);

    } else if (!strcmp("GetFunctions", msg->m_msgName))
	{
		char objectType[MAX_NAME_LENGTH], objectName[MAX_NAME_LENGTH];
		objectName[0] = objectType[0] = 0;

		// Check parameters.
		int ret = msg->getParam(objectType);
		if (ret >= 0) ret = msg->getParam(objectName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - Functions failed\n");
			return(0);
		}

		// Get the functions.
		recvGetFunctions(objectType, objectName);

    } else if (!strcmp("GetFunctionAttributes", msg->m_msgName))
	{
		char objectType[MAX_NAME_LENGTH];
		char objectName[MAX_NAME_LENGTH];
		char functionName[MAX_NAME_LENGTH];
		objectType[0] = objectName[0] = functionName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(objectType);
		if (ret >= 0) ret = msg->getParam(objectName);
		if (ret >= 0) ret = msg->getParam(functionName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - FunctionAttributes failed\n");
			return(0);
		}

		// Get the functions.
		recvGetFunctionAttributes(objectType, objectName, functionName);

    } else if (!strcmp("SetViewer", msg->m_msgName))
	{
		char viewerName[MAX_NAME_LENGTH];
		viewerName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(viewerName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetViewer failed\n");
			return(0);
		}

		// Get the functions.
		recvSetViewer(viewerName);

    } else if (!strcmp("GetViewer", msg->m_msgName))
	{
		// Get the functions.
		recvGetViewer();

    } else if (!strcmp("SetEditMode", msg->m_msgName))
	{
		char editMode[MAX_NAME_LENGTH];
		editMode[0] = 0;

		// Check parameters.
		int ret = msg->getParam(editMode);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetEditMode failed\n");
			return(0);
		}

		// Get the functions
		recvSetEditMode(editMode);

    } else if (!strcmp("GetEditMode", msg->m_msgName))
	{
		// Get the functions
		recvGetEditMode();

    } else if (!strcmp("HasSnappingTarget", msg->m_msgName))
	{
		// Get the functions.
		recvHasSnappingTarget();

    } else if (!strcmp("GetSnapping", msg->m_msgName))
	{
		// Get the functions.
		recvGetSnapping();

    } else if (!strcmp("SetSnapping", msg->m_msgName))
	{
		int mode;

		// Check parameters.
		int ret = msg->getParam(mode);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetSnapping failed\n");
			return(0);
		}

		// Get the functions.
		recvSetSnapping(mode);

    } else if (!strcmp("PushSet", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - PushSet failed\n");
			return(0);
		}

		// Get the functions.
		recvPushSet(setName);

    } else if (!strcmp("PushSetToBottom", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - PushSetToBottom failed\n");
			return(0);
		}

		// Get the functions.
		recvPushSetToBottom(setName);

    } else if (!strcmp("PopSet", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - PopSet failed\n");
			return(0);
		}

		// Get the functions.
		recvPopSet(setName);

    } else if (!strcmp("PopSetToTop", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - PopSetToTop failed\n");
			return(0);
		}

		// Get the functions.
		recvPopSetToTop(setName);

    } else if (!strcmp("SetBackgroundColor", msg->m_msgName))
	{
		float f[3];

		// Check parameters.
		int ret = msg->getParam(f);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetBackgroundColor failed\n");
			return(0);
		}

		recvSetBackgroundColor(f);

    } else if (!strcmp("GetBackgroundColor", msg->m_msgName))
	{
		recvGetBackgroundColor();

    } else if (!strcmp("SetHorizonGrid", msg->m_msgName))
	{
		int onOff;
		
		// Check parameters
		int ret = msg->getParam(onOff);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetHorizonGrid failed\n");
			return(0);
		}

		recvSetHorizonGrid(onOff);

    } else if (!strcmp("GetHorizonGrid", msg->m_msgName))
	{
		recvGetHorizonGrid();

    } else if (!strcmp("OpenPrefsDialog", msg->m_msgName))
	{
		recvOpenPrefsDialog();

    } else if (!strcmp("ViewAll", msg->m_msgName))
	{
		recvViewAll();

    } else if (!strcmp("ShowDecoration", msg->m_msgName))
	{
		int onOff;

		// Check parameters.
		int ret = msg->getParam(onOff);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - ShowDecoration failed\n");
			return(0);
		}

		recvShowDecoration(onOff);

    } else if (!strcmp("PushActor", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		char actorName[MAX_NAME_LENGTH];
		setName[0] = actorName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret >= 0)  msg->getParam(actorName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - PushActor failed\n");
			return(0);
		}

		// Get the functions.
		recvPushActor(setName,actorName);

    } else if (!strcmp("PushActorToBottom", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		char actorName[MAX_NAME_LENGTH];
		setName[0] = actorName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret >= 0)  msg->getParam(actorName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - PushActorToBottom failed\n");
			return(0);
		}

		// Get the functions.
		recvPushActorToBottom(setName, actorName);

    } else if (!strcmp("PopActor", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		char actorName[MAX_NAME_LENGTH];
		setName[0] = actorName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret >= 0)  msg->getParam(actorName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - PopActor failed\n");
			return(0);
		}

		// Get the functions.
		recvPopActor(setName, actorName);

    } else if (!strcmp("PopActorToTop", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		char actorName[MAX_NAME_LENGTH];
		setName[0] = actorName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret >= 0)  msg->getParam(actorName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - PopActorToTop failed\n");
			return(0);
		}

		// Get the functions.
		recvPopActorToTop(setName, actorName);

    } else if (!strcmp("SetRenderMode", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		char actorName[MAX_NAME_LENGTH];
		char mode[MAX_NAME_LENGTH];
		actorName[0] = mode[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret >=0) ret = msg->getParam(actorName);
		if (ret >=0) ret = msg->getParam(mode);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetRenderMode failed\n");
			return(0);
		}

		// Get the functions.
		recvSetRenderMode(setName, actorName, mode);

    } else if (!strcmp("GetRenderMode", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		char actorName[MAX_NAME_LENGTH];
		setName[0] = actorName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret >=0) ret = msg->getParam(actorName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - GetRenderMode failed\n");
			return(0);
		}

		// Get the functions.
		recvGetRenderMode(setName, actorName);

    } else if (!strcmp("SetGlobalRenderMode", msg->m_msgName))
	{
		char mode[MAX_NAME_LENGTH];
		mode[0] = 0;

		// Check parameters.
		int ret = msg->getParam(mode);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetGlobalRenderMode failed\n");
			return(0);
		}

		// Get the functions.
		recvSetGlobalRenderMode(mode);

    } else if (!strcmp("GetGlobalRenderMode", msg->m_msgName))
	{
		// Get the functions.
		recvGetGlobalRenderMode();

    } else if (!strcmp("GetPerspective", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - GetPerspective failed\n");
			return(0);
		}

		recvGetPerspective(setName);

    } else if (!strcmp("SetPerspective", msg->m_msgName))
	{
		char setName[MAX_NAME_LENGTH];
		setName[0] = 0;
		int perspective;

		// Check parameters.
		int ret = msg->getParam(setName);
		if (ret >=0) ret = msg->getParam(perspective);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetPerspective failed\n");
			return(0);
		}

		recvSetPerspective(setName, perspective);

    } else if (!strcmp("GetSets", msg->m_msgName))
	{
		int x, y;

		// Check parameters.
		int ret = msg->getParam(x);
		if (ret >=0) ret = msg->getParam(y);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - GetSets failed\n");
			return(0);
		}

		// Go get the functions.
		recvGetSets(x, y);

    } else if (!strcmp("LoadSet", msg->m_msgName))
	{
        char setName[256];

		int ret = msg->getParam(setName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg = LoadSet failed.\n");
			return(0);
		}

		recvLoadSet(setName);

    } else if (!strcmp("ReparentWindow", msg->m_msgName))
	{
#if defined(__linux__) || defined(__APPLE__)
#ifdef Q_OS_UNIX
    	WId w;

    	// Check parameters.
    	int param;   // XXX passing an int as a window.
    	int ret = msg->getParam(param);
    	if (ret < 0) {
    		printf("ERROR MlePlayer::deliverMsg - ReparentWindow failed\n");
    		return(0);
    	}

    	// Go get the functions.
    	w = (WId) param;
    	recvReparentWindow(w);
#else
		Window w;

		// Check parameters.
		int param;   // XXX passing an int as a window.
		int ret = msg->getParam(param);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - ReparentWindow failed\n");
			return(0);
		}

		// Go get the functions.
		w = (Window) param;
		recvReparentWindow(w);
#endif /* ! Q_OS_UNIX */
#endif /* __linux__ */

    } else if (!strcmp("SetSetName", msg->m_msgName))
	{
		char oldSetName[MAX_NAME_LENGTH];
		char newSetName[MAX_NAME_LENGTH];
		oldSetName[0] = newSetName[0] = 0;

		// Check parameters.
		int ret = msg->getParam(oldSetName);
		if (ret >=0) ret = msg->getParam(newSetName);
		if (ret < 0) {
			printf("ERROR MlePlayer::deliverMsg - SetSetName failed\n");
			return(0);
		}

		recvSetSetName(oldSetName, newSetName);

    } else if (!strcmp("ToolsInitFinished", msg->m_msgName))
	{
		recvToolsInitFinished();

    } else
	{
		return(AtkWired::deliverMsg(msg));
    }

    // Default return value.
    return(0);
}

/*****************************************************************************
* Initializing the player
*****************************************************************************/
void 
MlePlayer::recvInit(int w, int h)
{
	MLE_DEBUG_CAT("ATK",
		printf("PLAYER: Window size: %d, %d\n", w, h);
	);
    m_prefWidth = w;
    m_prefHeight = h;

    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setSize(w,h);
}

/*****************************************************************************
* Controlling the player
*****************************************************************************/
int 
MlePlayer::recvPlay()
{

    // Set play flag and return.
    m_running = 1;

    // Set the editing mode on the stage.
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setEditing(0);

    return(0);
}

int 
MlePlayer::recvPause()
{
    // Set play flag and return.
    m_running = 0;

    // Set the editing mode on the stage.
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setEditing(m_curStageEditMode);

    return(0);
}

/*****************************************************************************
* Moving current actor to the target
*****************************************************************************/
int 
MlePlayer::recvMoveToTarget()
{
    // Call stage's move to target function.
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->moveToTarget();

    return(0);
}

/*****************************************************************************
* Nudging current actor
*****************************************************************************/

int 
MlePlayer::recvNudge(int dir, int numPixels)
{
    // Call stage's move to target function.
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->nudge(dir, numPixels);

    return(0);
}

/*****************************************************************************
* Finding actors
*****************************************************************************/
void
MlePlayer::recvFindActor(const char* name)
{
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(name);
    if (actor)
	{
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME, &actor, sizeof(MleActor*));
    } else
	{
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME);
    }
}

/*****************************************************************************
* Getting actor property names
*****************************************************************************/
void
MlePlayer::recvGetActorPropertyNames(const char *actorName,
	const char *propDataset)
{
    char **propName;
    int i, numPropName;
    MleDwpStrKeyDict *actorInstanceReg;
    MleActor *actor;
    AtkWireMsg *retMsg;
    MlePtrArray *propNameArray;

    actorInstanceReg = MleActor::getInstanceRegistry();
    actor = (MleActor *) actorInstanceReg->find(actorName);

    if (actor)
	{
		propNameArray = actor->getPropNames(propDataset);
		numPropName = propNameArray->getSize();

		// Note the addParam() call expects propName to have a terminator
		// string with NULL in it.
		propName = new char *[numPropName + 1];
		for(i = 0; i < numPropName; i++)
			propName[i] = (char *) ((*propNameArray)[i]);
        propName[i] = const_cast<char *>("");

		retMsg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
		retMsg->addParam((const char**)propName);

		// Send a list of property names for the property dataset back to
		// the tool side.
		if (m_wire->sendMsg(retMsg) < 0)
		{
			printf("PLAYER ERROR: problem sending property names,actor '%s'\n",
			   (actorName) ? actorName : "");
		}

		delete retMsg;
		delete [] propName;

	} else
	{
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME);
		printf("PLAYER ERROR:  cound not find actor '%s'\n",
			   (actorName) ? actorName : "");
    }
}

/*****************************************************************************
* Getting/setting properties
*****************************************************************************/
void
MlePlayer::recvGetActorProperty(const char* actorClass, const char* actorName, 
	const char* propName)
{
    // Eventually, get Actor Member object by finding class, member.
    MleActorClass* ac = MleActorClass::find(actorClass);
    if (ac)
	{
		// find member
		const MleActorMember* am = ac->findMember(propName);
		if (am)
		{
			// Now find actor.
			MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
			MleActor* actor = (MleActor*) actorInstances->find(actorName);

			// If actor, send back message.
			if (actor)
			{
				// Get datatype and dataunion.
				const MleDwpDatatype* datatype = am->getType();
				MleDwpDataUnion dataunion;
				//datatype->set(&dataunion, (((char*) actor) + am->getOffset()));
				MlePropertyEntry *entry = am->getEntry();
				char *value;
				entry->getProperty(actor, entry->name, (unsigned char **)&value);
				datatype->set(&dataunion, value);

				// Set up DwpOutput.
				MleDwpOutput out;

				// Transcribe into buffer.
				datatype->write(&out, &dataunion);

				// Send across wire.
				int size;
				char* buf;
				out.getBuffer(&buf, &size);
				m_wire->sendMsg(m_objID, REPLY_MSG_NAME, buf, size);

#ifdef notdefined
				// XXX special case on string
				if (!strcmp(am->getType()->getName(), "string"))
				{
					char* p = *((char**) (((char*) actor) + am->getOffset()));
					m_wire->sendMsg(objID, REPLY_MSG_NAME, p, strlen(p)+1);
				} else {
					m_wire->sendMsg(objID, REPLY_MSG_NAME, 
						((char*) actor) + am->getOffset(),
						am->getType()->getSize());
				}
#endif
				return;
			} else
			{
				printf("PLAYER: actor problem\n");
			}
		} else
		{
			printf("PLAYER: member problem\n");
		}
    } else
	{
		printf("PLAYER: class problem\n");
    }

    // Something wrong - send back NULL
	printf("PLAYER: Could not find actor class: %s,  name:  %s,  prop: %s\n",
       actorClass, actorName, propName);
    m_wire->sendMsg(m_objID, REPLY_MSG_NAME);
}


int
MlePlayer::recvSetActorProperty(const char* actorClass, const char* actorName, 
	const char* propName, void* data)
{
	MLE_DEBUG_CAT("ATK",
		printf("RECV Set Prop\n");
	);

    // Eventually, get Actor Member object by finding class, member.
    MleActorClass* ac = MleActorClass::find(actorClass);
    if (ac)
	{
		// Find member.
		const MleActorMember* am = ac->findMember(propName);
		if (am)
		{
			// Now find actor.
			MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
			MleActor* actor = (MleActor*) actorInstances->find(actorName);

			// If actor, set the field.
			if (actor)
			{
				// Get datatype 
				const MleDwpDatatype* datatype = am->getType();

				// Read into dataunion.
				MleDwpInput in;
				MleDwpDataUnion dataunion;
				in.setBuffer((char*) data);
				datatype->read(&in, &dataunion);

				// Poke value into actor.
				actor->poke(propName, &dataunion);

				// Call the resolve edit function.
				actor->resolveEdit(propName);

#ifdef notdefined
				if (!strcmp(am->getType()->getName(), "string"))
				{
					char* p = *((char**) (((char*) actor) + am->getOffset()));

					// XXX should we free this?
					// if (p) free(p)
					p = strdup((char*) data);
				} else
				{
					bcopy(data, ((char*) actor) + am->getOffset(),
					  am->getType()->getSize());
				}
#endif
				return(0);
			} else
			{
				printf("actor failed\n");
			}
		} else
		{
			printf("member failed\n");
		}
    }
	printf("class failed\n");

    // Failure.
	printf("RECV Set Prop failed\n");
    return(-1);
}

int
MlePlayer::recvSetActorName(char* actorName, 
	char* newActorName)
{
	MLE_DEBUG_CAT("ATK",
		printf("RECV Set Actor Name\n");
	);

	// Find actor.
	MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
	MleActor* actor = (MleActor*) actorInstances->find(actorName);

	// If actor, set the field.
	if (actor)
	{
		actor->setName(newActorName);
		return(0);
	} else
	{
		printf("actor failed\n");
	}

	// Failure.
	printf("RECV Set Actor Name failed\n");
	return(-1);
}

/*****************************************************************************
* Check type of actor
*****************************************************************************/
void
MlePlayer::recvGetActorIsA(const char *actorName, const char *actorClass)
{
    int status;
    MleDwpStrKeyDict *actorInstanceReg;
    MleActor *actor;
    AtkWireMsg *retMsg;

    actorInstanceReg = MleActor::getInstanceRegistry();
    actor = (MleActor *) actorInstanceReg->find(actorName);

    if (actor)
	{
		status = actor->isa(actorClass);

		retMsg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
		retMsg->addParam(status);

		//
		// Send the "is a" status back to the tool side.
		//
		if (m_wire->sendMsg(retMsg) < 0)
		{
			printf("PLAYER ERROR: problem sending isa status, actor '%s'\n",
			   (actorName) ? actorName : "");
		}

		delete retMsg;
    } else
	{
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME);
		printf("PLAYER ERROR:  cound not find actor '%s'\n",
			   (actorName) ? actorName : "");
	}
}

int MlePlayer::recvSetTransform(char *actorName, MlTransform& t)
{
	MLE_DEBUG_CAT("ATK",
		printf("RECV Set Transform");
	);

    // Find actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);

    // If actor, send back message.
    if (actor)
    {
		actor->setPropDataset(MLE_PROP_DATASET_TRANSFORM, &t);
		actor->resolveEdit();
		return 0;
    }

	printf("PLAYER setTransform problem - actor '%s' failed\n", actorName);
    return -1;
}

int MlePlayer::recvGetTransform(char *actorName)
{
	MLE_DEBUG_CAT("ATK",
		printf("RECV Get Transform\n");
	);

    // Find actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);

    // If actor, send back message.
    if (actor)
    {
		MlTransform t;
		actor->getPropDataset(MLE_PROP_DATASET_TRANSFORM, &t);

		AtkWireMsg* m = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
		m->addParam(t);
		if (m_wire->sendMsg(m) < 0) 
		{
			printf("PLAYER ERROR: sending transform, actor '%s'\n", 
			(actorName) ? actorName: "");
			delete m;
			return -1;
		}
		
		return 0;
    }

    // Send back a null msg.
    m_wire->sendMsg(m_objID, REPLY_MSG_NAME);

	printf("PLAYER getTransform problem - actor '%s' failed\n", actorName);
    return -1;
}


/*****************************************************************************
* Recving find
*****************************************************************************/

void 
MlePlayer::recvFind(MleDwpType /*t*/, const char* name, int findAll)
{
    // XXX - Currently, we can only find actors - we are ignoring t.
    // Now find actor
    char* data=0;
    int dataLen = 0;

    // Loop through actors.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    for (MleDwpDictIter iter(*actorInstances); iter.getValue(); iter.next())
	{
		// Compare name with key.
		char* key = (char*) iter.getKey();
		if (!key) continue;

		// If a match, alloc space and copy.
		if (!name || !strcmp(key, name) )
		{
			if (!data)
			{
				data = (char*) mlMalloc(strlen(key));
			} else
			{
				data = (char*) mlRealloc(data, dataLen+strlen(key)+1);
			}
			strcpy(data+dataLen, key);

			// Keep a running count of data len; 
			dataLen += strlen(key) + 1;

			// If we only want first item, pass that back
			if (!findAll) break;
		}
    }

    // Send back data.
    if (data)
	{
		data = (char*) mlRealloc(data, ++dataLen);
		data[dataLen-1] = 0;
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME, data, dataLen);
    } else
	{
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME);
    }
    return;
}

/*****************************************************************************
* Recving picking
*****************************************************************************/
void
MlePlayer::recvPick(char* setName, int x, int y)
{
    // Locate set.
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set) return;

	MLE_DEBUG_CAT("ATK",
		printf("RECVPICK: fname: %s, x: %d,  y %d\n", setName, x, y);
	);

    // Pick actor.
    MleActor* actor = set->pick(x,y);

    // Reply.
    if (actor && actor->getName())
	{
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME, actor->getName(), strlen(actor->getName()));
    } else
	{
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME);
    }
    return;
}



/*****************************************************************************
* Refresh
*****************************************************************************/
void
MlePlayer::recvRefresh()
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->render();

    //if (refreshCB) (*refreshCB)(refreshData);
}


/*****************************************************************************
* Frame Advance
*****************************************************************************/
void
MlePlayer::recvFrameAdvance()
{
    if (frameAdvanceCB) (*frameAdvanceCB)(frameAdvanceData);
}


/*****************************************************************************
* Setting the camera for a set
*****************************************************************************/
// XXX this code is all obsolete
void
MlePlayer::recvSetCamera(char* setName, char* cameraName)
{
    // Locate set.
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set) return;

    // Find Actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* camera = (MleActor*) actorInstances->find(cameraName);
    if (!camera) return;

}

/*****************************************************************************
* Loading/Unloading a group
*****************************************************************************/
void
MlePlayer::recvLoadGroup(void* data)
{
    //mlLoadGroup(groupName, NULL, NULL);

    // Create an input object - read objects into it
    MleDwpInput in;
    in.setBuffer((char*) data);

    // Before we read, we need to activate DSO loading if a DSOFile
    //   specification comes in.
    // XXX - why don't we just have DSO loading active all the time?
    MleDwpDSOFile::setLoadOnSet(1);

    // Now read the input buffer.
    MleDwpItem* items = MleDwpItem::readAll(&in);

    // and deactivate DSO loading.
    MleDwpDSOFile::setLoadOnSet(0);

    if (!items)
	{
		printf("PLAYER: null item in loadWorkprintGroup - name\n");
    }

    // prints out to stdout an actor tree (wp) - dumps actors
#if 0
{
    MleDwpOutput out;
    items->write(&out);
    
    char *buf;
    int len;
    out.getBuffer(&buf, &len);

    // Assuming buf is null terminated (DwpOutput code shows it is).
    printf("PLAYER recvLoadGroup: tree received:\n%s\n", buf);
    free(buf);
}
#endif

    // Create msg that we will send back.
    AtkWireMsg* m = new AtkWireMsg(m_objID, REPLY_MSG_NAME);

    // mvo 11/4/95: data being passed over is more than a group now - 
    // container has a group plus possibly an actorDef in it. We need
    // to find the group and load it. The actordef we ignore - it's found
    // because its in the tree w/the group!
    MleDwpFinder finder(MleDwpGroup::typeId);
    MleDwpGroup *wpGroup = (MleDwpGroup *) finder.find(items);
    if (wpGroup)
	{
        MleGroup *group = mlLoadGroup((MleDwpGroup*) wpGroup);
		createLoadGroupRetMsg(m, (MleDwpGroup*) wpGroup, group);

	// Which scene to add this group to in the player?  
		// Would prefer the current scene if active, else the
		// global.  otherwise I guess we\'ll have to make a current
		// scene and add to it.
		MleScene *s;
		if (NULL != (s = MleScene::getCurrentScene())) 
		{
			s->add(group);
		}
		else if (NULL != (s = MleScene::getGlobalScene())) 
		{
			s->add(group);
		}
		else 
		{
			s = new MleScene;
			s->setCurrentScene();
			s->add(group);
		}

#if 0
		// Would really like to do this, but then must find all the
		// right places to remove it from the dict so we don\'t die
		// on dict destruction.
		
		// Put it into the dictionary of the scene so that 
		// user code can find it
		int index = s->find(group);
		s->dict.set(wpGroup->getName(), (void*)(1+index));
#endif /* 0 */

	} else
	{
		printf("PLAYER no group to load\n");
		m->addParam(-1);  // Error.
	}

    // Send return msg
    if (m_wire->sendMsg(m) < 0)
	{
		printf("PLAYER ERROR: sending back loadGroup info, group '%s'\n", 
			(wpGroup) ? wpGroup->getName(): "");
    }
    delete m;

    // Don't delete items because need any actordef to stay around.
}

void
MlePlayer::recvUnloadActor(char* actorName)
{
    // Find Actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor) 
    {
		printf("ERROR: Player - can't find actor '%s' to unload\n", 
		    actorName);
		return;
    }
    
    // Before deleting actor, remove its manip, if any; since
    // tools requested actor unloading, don't do callback (FALSE).
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->deactivateManipulator(actor, FALSE);

    // Believe that we can only receive unload messages pertaining
    // to actors visible in the current scene, though the actor might
    // have been put there by another scene, or maybe deleted from
    // the scene\'s groups by user code.  So hunt for the actor
    // in current and global scene, but be tolerant if it\'s not there.

    MleScene *scenes[2];
    scenes[0] = MleScene::getCurrentScene();
    scenes[1] = MleScene::getGlobalScene();

    MleScene *s;
    MleGroup *g;
    int found = FALSE;
    for (int i=0; i<2 && !found; i++)
	{
	    s = scenes[i];
	    if (NULL == s)
		{
		    continue;
	    }
	    // Have a scene to examine;
	    for (int j=0; j<s->getSize() && !found; j++)
		{
		    g = (*s)[j];
		    if (NULL == g)
			{
			    continue;
		    }
		    int index = g->find(actor);
		    if (-1 != index)
			{
			    found = TRUE;

			    // Get rid of actor and 
			    // make sure the group has no record of it
			    g->clear(index);
			    break;
		    }
	    }

	    // If not found, continue iterating.
    }

    // Whether we found it in the scene/group or not, we can still delete it.
    delete actor;
}

void
MlePlayer::recvUnloadGroup(char* groupName)
{
	// Find Group.
	MleScene *s = NULL;
	MleGroup *g = NULL;

	// Keep trying until we find a group, then unload it.
	if (NULL == g && NULL != (s = MleScene::getCurrentScene()))
	{
		g = s->find(groupName);
	}
	if (NULL == g && NULL != (s = MleScene::getGlobalScene()))
	{
		g = s->find(groupName);
	}

	if (NULL != g)
	{
		int index = s->find(g);
		if (-1 != index)
		{
			delete (*s)[index];
			(*s)[index] = NULL;
		}
	}

	// Else it didn\'t exist in the first place.
}

/*****************************************************************************
* Loading/Unloading an entire scene
*****************************************************************************/
void
MlePlayer::recvLoadScene(void* data)
{
    //mlLoadScene(sceneName);

    // Create an input object - read objects into it.
    MleDwpInput in;
    in.setBuffer((char*) data);

    // Before we read, we need to activate DSO loading if a DSOFile
    //   specification comes in.
    // XXX - why don't we just have DSO loading active all the time?
    MleDwpDSOFile::setLoadOnSet(1);

    // Now read the input buffer.
    MleDwpItem* items = MleDwpItem::readAll(&in);

    // and deactivate DSO loading.
    MleDwpDSOFile::setLoadOnSet(0);

    if (!items)
	{
		printf("PLAYER: null item in loadWorkprintScene - name\n");
    }

	// Prints out to stdout an actor tree (wp) - dumps actors.
#if 0
{
    MleDwpOutput out;
    items->write(&out);
    
    char *buf;
    int len;
    out.getBuffer(&buf, &len);

    // Assuming buf is null terminated (DwpOutput code shows it is).
    printf("PLAYER recvLoadScene: tree received:\n%s\n", buf);
    mlFree(buf);
}
#endif

    // Create msg that we will send back.
    AtkWireMsg* m = new AtkWireMsg(m_objID, REPLY_MSG_NAME);

    MleDwpFinder finder(MleDwpScene::typeId);
    MleDwpScene *wpScene = (MleDwpScene *) finder.find(items);
    if (wpScene)
	{
	    // Logic is to call the global scene (manager scene)
	    // to allow it to handle the change.
	    // If there\'s no global scene, let the current
	    // scene handle the logic.
	    // if no scene is active, then just load it.
	    MleScene *scene, *s;
	    if (NULL != (s = MleScene::getGlobalScene()))
		{
		    scene = s->changeCurrentScene( wpScene );
	    }
	    else if (NULL != (s = MleScene::getCurrentScene()))
		{
		    scene = s->changeCurrentScene( wpScene );
	    }
	    else
		{
		    scene = mlLoadScene( wpScene);
	    }

	    // Ack the message.
	    createLoadSceneRetMsg(m, (MleDwpScene*) wpScene, scene);

    } else
	{
	    printf("PLAYER no scene to load\n");
	    m->addParam(-1);  // Error
    }

    // Send return msg.
    if (m_wire->sendMsg(m) < 0)
	{
		printf("PLAYER ERROR: sending back loadScene info, scene '%s'\n", 
			(wpScene) ? wpScene->getName(): "");
    }
    delete m;


    // Don't delete items because need any actordef to stay around.
}

void
MlePlayer::recvLoadBootScene(void* data)
{
    //mlLoadBootScene(workprint);

    // Create an input object - read objects into it.
    MleDwpInput in;
    in.setBuffer((char*) data);

    // before we read, we need to activate DSO loading if a DSOFile
    //   specification comes in
    // XXX - why don't we just have DSO loading active all the time?
    MleDwpDSOFile::setLoadOnSet(1);

    // Now read the input buffer
    MleDwpItem* items = MleDwpItem::readAll(&in);

    // and deactivate DSO loading.
    MleDwpDSOFile::setLoadOnSet(0);

    if (!items)
	{
		printf("PLAYER: null item in loadWorkprintScene - name\n");
    }

	// Prints out to stdout an actor tree (wp) - dumps actors.
#if 0
{
    MleDwpOutput out;
    items->write(&out);
    
    char *buf;
    int len;
    out.getBuffer(&buf, &len);

    // Assuming buf is null terminated (DwpOutput code shows it is).
    printf("PLAYER recvLoadScene: tree received:\n%s\n", buf);
    mlFree(buf);
}
#endif

    // Create msg that we will send back.
    AtkWireMsg* m = new AtkWireMsg(m_objID, REPLY_MSG_NAME);

    MleDwpFinder finder(MleDwpScene::typeId);
    MleDwpScene *wpScene = (MleDwpScene *) finder.find(items);

    if (wpScene)
	{
	    // Logic is to delete global and current scenes so that
	    // we start completely fresh.
	    MleScene::deleteGlobalScene();
	    MleScene::deleteCurrentScene();

	    MleScene *scene = mlLoadBootScene(wpScene);
	    // Ack the message.
	    createLoadBootSceneRetMsg(m, wpScene, scene);

    } else
	{
	    printf("PLAYER no boot scene to load\n");
	    m->addParam(-1);  // Error
    }

    // Send return msg.
    if (m_wire->sendMsg(m) < 0)
	{
	    printf("PLAYER ERROR: sending back loadBootScene info\n" );
    }
    delete m;

    // don't delete items because need any actordef to stay around.
}

void
MlePlayer::recvUnloadScene(char* sceneName)
{
	// Find scene.
	MleScene *s = NULL;

	if (NULL != (s = MleScene::getCurrentScene()))
	{
		if (!strcmp(s->getName(), sceneName))
		{
			MleScene::deleteCurrentScene();
			return;
		}
	}
	if (NULL != (s = MleScene::getGlobalScene()))
	{
		if (!strcmp(s->getName(), sceneName))
		{
			MleScene::clearGlobalScene();
			delete s;
			return;
		}
	}
	// Else it didn\'t exist in the first place.
}

void
MlePlayer::recvActivateManip(char* actorName)
{
    // Find Actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor)
    {
		printf("ERR PLAYER couldn't activate manip on '%s'\n", actorName);
		return;
    }

    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->activateManipulator(actor);
}

void
MlePlayer::recvDeactivateManip(char* actorName)
{
    // Find Actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor) 
    {
		printf("ERR PLAYER couldn't deactivate manip on '%s'\n", actorName);
		return;
    }

    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->deactivateManipulator(actor);
}

/*****************************************************************************
* Quit message
*****************************************************************************/
void
MlePlayer::recvQuit()
{
    m_quit = 1;
}

// XXX - receive configuration workprint items
//   Eventually this should be broken out into all the different types
//   of items.
void
MlePlayer::recvWorkprintItem(void* data)
{
    // Create an input object - read objects into it.
    MleDwpInput in;
    in.setBuffer((char*) data);
    MleDwpItem* item = MleDwpItem::readAll(&in);

    if (!item)
	{
		printf("PLAYER: null item in loadWorkprintGroup - name\n");
	return;
    }

    // The transcription process adds a level of hierarchy, so
    //   switch on the (should be single) child.
    MleDwpItem *child = item->getFirstChild();

    if ( 0 )
		;
    else if ( child->getTypeId() == MleDwpDSOFile::typeId )
    {
		// Load the DSO.
		const char *dsoFile = ((MleDwpDSOFile *)child)->getDSOFile();
		if ( MleDSOLoader::loadFile(dsoFile) == NULL )
			printf("error loading DSO %s.\n",dsoFile);
    }
    else if ( child->getTypeId() == MleDwpActorDef::typeId )
    {
		// Load the actor def class.
		MleActorClass::find(child->getName());
    }
    else
	{
		printf("item %s received.\n",child->getTypeName());
    }

    // Delete from the top level.
    delete item;
}

/*****************************************************************************
* Stats modes
*****************************************************************************/
void
MlePlayer::recvStartStats()
{
    m_sendStats = 1;
}

void 
MlePlayer::recvEndStats()
{
    m_sendStats = 0;
}

/*****************************************************************************
* Stage input mode
*****************************************************************************/
void
MlePlayer::recvStageEditMode(int mode)
{
    MLE_ASSERT(MleStage::g_theStage);
    m_curStageEditMode = mode;
    MleStage::g_theStage->setEditing(mode);
}

/*****************************************************************************
* Placement state
*****************************************************************************/
void
MlePlayer::recvPlacementState(int state)
{
    m_curPlacementState = state;
}

/*****************************************************************************
* Resize
*****************************************************************************/
void
MlePlayer::recvResize(int w, int h)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setSize(w, h);
}

/*****************************************************************************
* Setting an actors position
*****************************************************************************/
void
MlePlayer::recvSetPosition(char* setName, char* actorName, int x, int y)
{
	MLE_DEBUG_CAT("ATK",
		printf("PLAYER: Set actor Position Actor: %s  x:%d, y:%d\n", actorName, x, y);
	);

    // Find Actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor)
	{
		printf("PLAYER: setPosition, Could not find actor name: %s\n", 
	       actorName);
		return;
    }

    // Find set.
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet *f = (MleSet *) setInstances->find(setName);
    if (!f)
	{
		printf("PLAYER: setPosition, Could not find set name: %s\n", 
	       setName);
	return;
    }

    MlScalar p[3];

    if(f->isa("Mle3dSet"))
	{
		Mle3dSet *set = (Mle3dSet *) f;

		// Project screen coords.
		if (m_curPlacementState == FLOATING)
		{
			set->projectScreenCoordinates(x, y, p);
		}
		else if (m_curPlacementState == ON_BACKGROUND)
		{
			set->intersectScreenCoordinates(x, y, p);

			// Use floating method if we found nothing to intersect with.
			if (p[0] == ML_SCALAR_ZERO &&
				p[1] == ML_SCALAR_ZERO &&
				p[2] == ML_SCALAR_ZERO)
			{
				set->projectScreenCoordinates(x, y, p);
			}
		}
		else
		{
            printf("PLAYER: setPosition: bad placement state %d\n",
				m_curPlacementState);
			return;
		}
	}
	else if(f->isa("Mle2dSet"))
	{
		int w, h;

		MLE_ASSERT(MleStage::g_theStage);
		MleStage::g_theStage->getSize(&w, &h);

		p[0] = mlLongToScalar(x);
		p[1] = mlLongToScalar(h - y);
		p[2] = ML_SCALAR_ZERO;
	}
	else
		printf("PLAYER: setPosition: neither Mle2dSet nor Mle3dSet\n");

	MLE_DEBUG_CAT("ATK",
		printf("PLAYER: new position x:%f, y:%f, z:%f\n", mlScalarToFloat(p[0]), mlScalarToFloat(p[1]), mlScalarToFloat(p[2]));
	);
    if (!actor) return;

    // Build matrix.
    MlTransform m;

/*
    m[0][0] = ML_SCALAR_ONE;  m[0][1] = ML_SCALAR_ZERO;  m[0][2] = ML_SCALAR_ZERO;
    m[1][0] = ML_SCALAR_ZERO;  m[1][1] = ML_SCALAR_ONE;  m[1][2] = ML_SCALAR_ZERO;
    m[2][0] = ML_SCALAR_ZERO;  m[2][1] = ML_SCALAR_ZERO;  m[2][2] = ML_SCALAR_ONE;
*/
    actor->getTransform(m);
    m[3][0] = p[0]; m[3][1] = p[1]; m[3][2] = p[2];


    // Set transformation.
    actor->setTransform(m);
    actor->resolveEdit();
}

/*****************************************************************************
* Resolving editing
*****************************************************************************/
void
MlePlayer::recvResolveEdit(char* actorName, char* propName)
{
    // Find Actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor) return;

    // Call it!
    actor->resolveEdit(propName);
}

/*****************************************************************************
* Getting camera position
*****************************************************************************/
void
MlePlayer::recvGetCameraPosition(char* setName)
{
    // Locate set.
    // XXX - casting to 3d set here.
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    Mle3dSet* set = (Mle3dSet*) setInstances->find(setName);
    if (!set)
	{
		printf("PLAYER ERROR: could not find set %s\n", setName);
		m_wire->sendMsg(m_objID, REPLY_MSG_NAME);
		return;
    }

    // get set's camera xform.
    MlTransform t;
	// Mle3dCameraDelegate::getTransform(set, &t);
    set->getCameraTransform(&t);
    AtkWireMsg* m = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    m->addParam(t);

    m_wire->sendMsg(m);
    delete m;
}

/*****************************************************************************
* Setting camera position
*****************************************************************************/
void
MlePlayer::recvSetCameraPosition(char* setName, MlTransform *t)
{
    // Locate set.
    // XXX - casting to 3d set here.
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    Mle3dSet* set = (Mle3dSet*) setInstances->find(setName);
    if (!set)
	{
		printf("PLAYER ERROR: could not find set %s\n", setName);
		return;
    }

    // Set set's camera xform.
	// Mle3dCameraDelegate::setTransform(set, &t);
    set->setCameraTransform(t);
}

/*****************************************************************************
* Register actor property for actor editor updates.
*****************************************************************************/
void
MlePlayer::recvRegisterProperty(char* actorName, char *propName)
{
    // Find Actor.
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor) return;

    registerProp(actor, propName);
}

/*****************************************************************************
* Cancel actor editor updates.
*****************************************************************************/
void
MlePlayer::recvUnregisterProperty(char* actorName, char *propName)
{
    // Find Actor
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor) return;

    unregisterProp(actor, propName);
}

/*****************************************************************************
* Getting the functions of an object.
*****************************************************************************/
void
MlePlayer::recvGetFunctions(char* objectType, char* objectName)
{
    MLE_ASSERT(MleStage::g_theStage);
    const char** strArray = NULL;

    if (!strcmp(objectType, STAGE_OBJTYPE))
    {
		// Object name ignored for stages - only one stage.
		strArray = MleStage::g_theStage->getFunctions();
    }
    else if (!strcmp(objectType, SET_OBJTYPE))
    {
		// Find the named set.
		MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
		MleSet* set = (MleSet*) setInstances->find(objectName);
		if (!set) 
		{
			printf("ERROR MlePlayer::recvGetFunctions: "
			"Could not find Set named: %s\n", objectName);
		}
		else
		{
			strArray = set->getFunctions();
		}
    }
    else
    {
		printf("WARNING Player getFunctions: unknown obj type '%s'\n",
			objectType);
    }

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(strArray);
    m_wire->sendMsg(msg);
    delete msg;
}

/*****************************************************************************
* Getting the attributes of a function.
*****************************************************************************/
void
MlePlayer::recvGetFunctionAttributes(char *objectType, char* objectName,
	char* functionName)
{
    MLE_ASSERT(MleStage::g_theStage);
    const char** attrArray = NULL;

    if (!strcmp(objectType, STAGE_OBJTYPE))
    {
		// Object name ignored for stages - only one stage.
		attrArray = MleStage::g_theStage->getFunctionAttributes(functionName);
    }
    else if (!strcmp(objectType, SET_OBJTYPE))
    {
		// Find the named set.
		MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
		MleSet* set = (MleSet*) setInstances->find(objectName);
		if (!set) 
		{
			printf("ERROR MlePlayer::recvGetFunctionAttributes: "
			"Could not find Set named '%s'\n", objectName);
		}
		else
		{
			attrArray = set->getFunctionAttributes(functionName);
		}
    }

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(attrArray);
    m_wire->sendMsg(msg);
    delete msg;
}

/*****************************************************************************
* Viewers
*****************************************************************************/
void 
MlePlayer::recvSetViewer(char* viewerName)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setViewer(viewerName);
}

void 
MlePlayer::recvGetViewer()
{
    MLE_ASSERT(MleStage::g_theStage);
    const char* viewerName = MleStage::g_theStage->getViewer();

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(viewerName);
    m_wire->sendMsg(msg);
    delete msg;
}


/*****************************************************************************
* Edit modes
*****************************************************************************/
void 
MlePlayer::recvSetEditMode(char* editMode)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setEditMode(editMode);
}

void 
MlePlayer::recvGetEditMode()
{
    MLE_ASSERT(MleStage::g_theStage);
    char* editMode = MleStage::g_theStage->getEditMode();

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(editMode);
    m_wire->sendMsg(msg);
    delete msg;
}


/*****************************************************************************
*  Snapping
*****************************************************************************/
void 
MlePlayer::recvHasSnappingTarget()
{
    MLE_ASSERT(MleStage::g_theStage);
    int hasSnapping = MleStage::g_theStage->hasSnappingTarget();

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(hasSnapping);
    m_wire->sendMsg(msg);
    delete msg;
}

void 
MlePlayer::recvGetSnapping()
{
    MLE_ASSERT(MleStage::g_theStage);
    int hasSnapping = MleStage::g_theStage->hasSnappingTarget();

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(hasSnapping);
    m_wire->sendMsg(msg);
    delete msg;
}

void 
MlePlayer::recvSetSnapping(int mode)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setSnapping(mode);
}


/*****************************************************************************
*  Push sets
*****************************************************************************/
void 
MlePlayer::recvPushSet(char* setName)
{
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::PushSet Could not find Set named: %s\n", setName);
		return;
    }

    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->pushSet(set);
}

void 
MlePlayer::recvPushSetToBottom(char* setName)
{
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::PushSetToBottom Could not find Set named: %s\n", setName);
		return;
    }

    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->pushSetToBottom(set);
}

void 
MlePlayer::recvPopSet(char* setName)
{
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::recvPopSet: Could not find Set named: %s\n", setName);
		return;
    }

    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->popSet(set);
}

void
MlePlayer::recvPopSetToTop(char* setName)
{
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::recvPopSetToTop: Could not find Set named: %s\n", setName);
		return;
    }

    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->popSetToTop(set);
}


/*****************************************************************************
*  Setting background colors
*****************************************************************************/
void 
MlePlayer::recvSetBackgroundColor(float* f)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setBgndColor(f);
}

void 
MlePlayer::recvGetBackgroundColor()
{
    MLE_ASSERT(MleStage::g_theStage);
    float backgroundColor[3];
    MleStage::g_theStage->getBgndColor(backgroundColor);

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(backgroundColor);
    m_wire->sendMsg(msg);
    delete msg;
}

/*****************************************************************************
*  Setting horizon grid
*****************************************************************************/
void 
MlePlayer::recvSetHorizonGrid(int onOff)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->setHorizonGrid(onOff);
}

void 
MlePlayer::recvGetHorizonGrid()
{
    MLE_ASSERT(MleStage::g_theStage);
    int onOff = MleStage::g_theStage->getHorizonGrid();

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(onOff);
    m_wire->sendMsg(msg);
    delete msg;
}

/*****************************************************************************
*  misc inventor controls
*****************************************************************************/

void 
MlePlayer::recvOpenPrefsDialog()
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->openPrefsDialog();
}

void 
MlePlayer::recvViewAll()
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->viewAll();
}

void 
MlePlayer::recvShowDecoration(int onOff)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->showDecoration(onOff);
}


/*****************************************************************************
*  push/pop actors
*****************************************************************************/
void 
MlePlayer::recvPushActor(char* setName, char* actorName)
{
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::recvPopSet: Could not find Set named: %s\n", setName);
		return;
    }

    // Get actor.
    if (!actorName) return;
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor)
	{
		printf("MlePlayer::recvPushActor: Could not find actor named: %s\n", actorName);
		return;
    }
    set->pushActor(actor);
}

void 
MlePlayer::recvPushActorToBottom(char* setName, char* actorName)
{
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::recvPopSet: Could not find Set named: %s\n", setName);
		return;
    }

    // Get actor.
    if (!actorName) return;
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor)
	{
		printf("MlePlayer::recvPushActorToBottom: Could not find actor named: %s\n", actorName);
		return;
    }
    set->pushActorToBottom(actor);
}

void 
MlePlayer::recvPopActor(char* setName, char* actorName)
{
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::recvPopActor: Could not find Set named: %s\n", setName);
		return;
    }

    // Get actor.
    if (!actorName) return;
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor)
	{
		printf("MlePlayer::recvPopActor: Could not find actor named: %s\n", actorName);
		return;
    }
    set->popActor(actor);
}

void 
MlePlayer::recvPopActorToTop(char* setName, char* actorName)
{
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::recvPopSet: Could not find Set named: %s\n", setName);
		return;
    }

    // Get actor.
    if (!actorName) return;
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor)
	{
		printf("MlePlayer::recvPopActorToTop: Could not find actor named: %s\n", actorName);
		return;
    }
    set->popActorToTop(actor);
}


/*****************************************************************************
*  render modes
*****************************************************************************/
void 
MlePlayer::recvSetRenderMode(char* setName, char* actorName, char* mode)
{
    // Get set.
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::recvSetRenderMode: Could not find Set named: %s\n", setName);
		return;
    }

    // Get actor.
    if (!actorName) return;
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor)
	{
		printf("MlePlayer::recvSetRenderMode: Could not find actor named: %s\n", actorName);
		return;
    }

    set->setRenderMode(actor, mode);
}

void 
MlePlayer::recvGetRenderMode(char* setName, char* actorName)
{
    // Get Set.
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set)
	{
		printf("MlePlayer::recvGetRenderMode: Could not find Set named: %s\n", setName);
		return;
    }

    // Get actor.
    if (!actorName) return;
    MleDwpStrKeyDict* actorInstances = MleActor::getInstanceRegistry();
    MleActor* actor = (MleActor*) actorInstances->find(actorName);
    if (!actor)
	{
		printf("MlePlayer::recvGetRenderMode: Could not find actor named: %s\n", actorName);
		return;
    }

    char* renderMode = set->getRenderMode(actor);
    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(renderMode);
    m_wire->sendMsg(msg);
    delete msg;
}

void 
MlePlayer::recvSetGlobalRenderMode(char* mode)
{
    MleStage::g_theStage->setRenderMode(mode);
}

void 
MlePlayer::recvGetGlobalRenderMode()
{
    const char* renderMode = MleStage::g_theStage->getRenderMode();
    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam((char*) renderMode);
    m_wire->sendMsg(msg);
    delete msg;
}

/*****************************************************************************
* Load set
*****************************************************************************/
void
MlePlayer::recvLoadSet(const char *setName)
{
    mlLoadSet(setName);
}

/*****************************************************************************
* Getting sets
*****************************************************************************/
void
MlePlayer::recvGetSets(int /*x*/, int /*y*/)
{
    MleDwpStrKeyDict * setRegistry;
    MleDwpDictIter * iterator;
    MleSet * set;
    char ** setNames;
    unsigned int	i;

    // Iterate set registry searching for hits on sets.
    setRegistry = MleSet::getInstanceRegistry();
    setNames = new char*[100 /*setRegistry->getLength()*/];
    iterator = new MleDwpDictIter(*setRegistry);
    for (i = 0; iterator->getKey() != NULL; iterator->next(), i++)
	{
        set = ((MleSet *) iterator->getValue());
		MLE_ASSERT(set != NULL);

		// XXX - should compare X-Y location to set dimensions, but
		// XXX - coordinate system for set dimensions not yet defined
		// XXX - just return all sets for now
		setNames[i] = set->getName();
    }
    setNames[i] = (char *)""; // Terminator needed?
    delete iterator;

    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam((const char**)setNames);
    m_wire->sendMsg(msg);
    delete msg;
    delete setNames;
}

/*****************************************************************************
* Perspective/orthograpic
*****************************************************************************/
void
MlePlayer::recvGetPerspective(char *setName)
{
    // Locate set.
    if (!setName) 
    {
		printf("ERROR MlePlayer getPerspective: no set name\n");
		AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
		msg->addParam((int) 0);
		m_wire->sendMsg(msg);
		delete msg;
		return;
    }

    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set) 
    {
		printf("ERROR MlePlayer getPerspective: set '%s' not found\n", 
			setName);
		AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
		msg->addParam((int) 0);
		m_wire->sendMsg(msg);
		delete msg;
		return;
    }

    // It's perspective if has a persp. FOV.
    int perspectiveOnOff =
		(Mle3dCameraCarrier::getPerspectiveFieldOfView(set) != ML_SCALAR_ZERO);
    
    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(perspectiveOnOff);
    m_wire->sendMsg(msg);
    delete msg;
}

void
MlePlayer::recvSetPerspective(char *setName, int perspectiveOnOff)
{
    // Locate set.
    if (!setName) return;
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(setName);
    if (!set) return;

    // XXX - for now use hardwired 45 degrees and 20 world units.
    if (perspectiveOnOff)
    {
        Mle3dCameraCarrier::setPerspectiveFieldOfView(set, 
			mlFloatToScalar(45.0));
    }
    else
    {
		Mle3dCameraCarrier::setOrthographicViewHeight(set, 
			mlFloatToScalar(20.0));
    }

#if 0
	// Didn't get this code working. Calc's grabbed from SoXtViewer
	// code.

    // Hack approx. of focal distance: 1/2 way between near and far
    // clipping planes.
    MlScalar near = Mle3dCameraCarrier::getNearClipping(set);
    MlScalar far = Mle3dCameraCarrier::getFarClipping(set);
    MlScalar focalDist = mlDiv(near+far, MLE_SCALAR_TWO);

    // XXX - for now use hardwired 45 degrees and 20 world units.
    if (perspectiveOnOff)
    {
		// Going from ortho to perspective.
		MlScalar height = 
			mlDiv(Mle3dCameraCarrier::getOrthographicViewHeight(set), 
				MLE_SCALAR_TWO);
		// ((SoOrthographicCamera *)camera)->height.getValue() / 2;
		MlScalar angle = mlAtan2(height, focalDist);
		// float angle = fatan(height / camera->focalDistance.getValue());

		Mle3dCameraCarrier::setPerspectiveFieldOfView(set, 
			mlMul(MLE_SCALAR_TWO, angle));
		//	((SoPerspectiveCamera *)newCam)->heightAngle = 2 * angle;

		printf("setPersp: to persp, height %g, angle %g, fov %g\n", 
			mlScalarToFloat(height), 
			mlScalarToFloat(angle), 
			mlScalarToFloat(mlMul(MLE_SCALAR_TWO, angle)));
    }
    else
    {
		// Going from perspective to ortho.
		MlScalar angle = Mle3dCameraCarrier::getPerspectiveFieldOfView(set);
		// float angle = ((SoPerspectiveCamera *)camera)->heightAngle.getValue();
		MlScalar halfAngle = mlDiv(angle, MLE_SCALAR_TWO);
		MlScalar tangent = mlDiv(fwSin(halfAngle), mlCos(halfAngle));
		MlScalar height = mlMul(focalDist, tangent);
		// float height = camera->focalDistance.getValue() * ftan(angle/2);

		Mle3dCameraCarrier::setOrthographicViewHeight(set, 
			mlMul(MLE_SCALAR_TWO, height));
		// ((SoOrthographicCamera *)newCam)->height = 2 * height;

		printf("setPersp: to ortho, height %g, angle %g, tangent %g, view height %g\n", 
		MleObstacle.wpa mlScalarToFloat(mlMul(MLE_SCALAR_TWO, height)), 
			mlScalarToFloat(angle), 
			mlScalarToFloat(tangent), 
			mlScalarToFloat(fwMul(MLE_SCALAR_TWO, height)));
    }
#endif	/* 0 */

}

#if defined(__linux__) || defined(__APPLE__)
/*****************************************************************************
* Reparenting a window
*****************************************************************************/
#ifdef Q_OS_UNIX
void
MlePlayer::recvReparentWindow(WId w)
{
    MLE_ASSERT(MleStage::g_theStage);
    QWindow *window = QWindow::fromWinId(w);
    MleStage::g_theStage->reparentWindow(window);
}
#else
void
MlePlayer::recvReparentWindow(Window w)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->reparentWindow(w);
/*
    AtkWireMsg* msg = new AtkWireMsg(m_objID, REPLY_MSG_NAME);
    msg->addParam(1);
    m_wire->sendMsg(msg);
    delete msg;
*/
}
#endif /* !Q_OS_UNIX */
#endif /* __linux__ */
#if defined(_WINDOWS)
void
MlePlayer::recvReparentWindow(HWND w)
{
    MLE_ASSERT(MleStage::g_theStage);
    MleStage::g_theStage->reparentWindow(w);
}
#endif /* _WINDOWS */

/*****************************************************************************
* Setting a sets name
*****************************************************************************/
void
MlePlayer::recvSetSetName(char* oldSetName, char* newSetName)
{
    MleDwpStrKeyDict *setInstances = MleSet::getInstanceRegistry();
    MleSet* set = (MleSet*) setInstances->find(oldSetName);
    if (!set)
	{
		printf("PLAYER ERROR - recvSetSetName: Could not find set name%s\n", 
	       oldSetName);
		return;
    }

    // XXX - fill in code here to change the sets name.
    set->setName(newSetName);
}

/*****************************************************************************
* Tools finished initializing
*****************************************************************************/
void
MlePlayer::recvToolsInitFinished()
{
    if (toolsInitFinishedCB) (*toolsInitFinishedCB)(toolsInitFinishedData);
}


/*****************************************************************************
* Getting groups from the tools
*****************************************************************************/
MleDwpGroup* 
MlePlayer::sendGetWorkprintGroup(const char* id)
{
    if (!id) return(NULL);

    // Send msg over wire.
    // PSPS - Is this correct to change the group to item.
    AtkWireMsg* msg = m_wire->sendSyncMsg(this, m_objID, "GetGroup", (void*) id,
		strlen(id)+1);
    if (!msg)
	{
		printf("PLAYER ERROR: could not get workprint group %s\n", id);
		return(NULL);
    }

    // Create an input object - read objects into it.
    MleDwpInput* in = new MleDwpInput;
    in->setBuffer((char*) msg->m_msgData);
    MleDwpItem* item = MleDwpItem::readAll(in);

    if (!item)
	{
		printf("PLAYER: null item in sendGetWorkprintGroup - name: %s\n", id);
    }

    // Cleanup and return item.
    delete in;
    delete msg;
    return((MleDwpGroup*) item);
}

/*****************************************************************************
* Getting scenes from the tools
*****************************************************************************/
MleDwpScene* 
MlePlayer::sendGetWorkprintScene(const char* id)
{
    if (!id) return(NULL);

    // Send msg over wire.
    // PSPS - Is this correct to change the scene to item.
    AtkWireMsg* msg = m_wire->sendSyncMsg(this, m_objID, "GetScene", (void*) id,
		strlen(id)+1);
    if (!msg)
	{
		printf("PLAYER ERROR: could not get workprint scene %s\n", id);
		return(NULL);
    }

    // Create an input object - read objects into it.
    MleDwpInput* in = new MleDwpInput;
    in->setBuffer((char*) msg->m_msgData);
    MleDwpItem* item = MleDwpItem::readAll(in);

    if (!item)
	{
		printf("PLAYER: null item in sendGetWorkprintScene - name: %s\n", id);
    }

    // Cleanup and return item.
    delete in;
    delete msg;
    return((MleDwpScene*) item);
}

int 
MlePlayer::sendLoadWorkprint(const char *filename)
{
    if (m_wire->sendMsg(m_objID, "LoadWorkprint", (void*) filename, 
		strlen(filename)) < 0)
	{
       printf("PLAYER ERROR: loading workprint %s\n", filename);
       return(-1);
    }
    return(0);
}

/*****************************************************************************
* Getting media refs from the tools.
*****************************************************************************/
MleDwpMediaRef* 
MlePlayer::sendGetWorkprintMediaRef(const char* id)
{
    if (!id) return(NULL);

    // Send msg over wire.
    AtkWireMsg* msg = m_wire->sendSyncMsg(this, m_objID, "GetMediaRef", (void*) id,
		strlen(id)+1);
    if (!msg || !msg->m_msgData)
	{
		printf("PLAYER ERROR: could not get workprint MediaRef %s\n", id);
		return(NULL);
    }

    // Create an input object - read objects into it
    MleDwpInput* in = new MleDwpInput;
    in->setBuffer((char*) msg->m_msgData);
    MleDwpItem* item = MleDwpItem::readAll(in);

    if (!item)
	{
		printf("PLAYER: null item in sendGetMediaRef - name: %s\n", id);
    }

    // Cleanup and return item.
    delete in;
    delete msg;
    if (!item) return(NULL);
    // XXX problem - we will only end up freeing the first child
    // XXX I hav gone back an forth between returning item and item getFirstChild
    // Need to make sure that the current code is correct
    //return((MleDwpMediaRef*) item->getFirstChild());
    return((MleDwpMediaRef*) item);
}

#if defined(__linux__) || defined(__APPLE__)
/*****************************************************************************
* Sending a window
*****************************************************************************/
#ifdef Q_OS_UNIX
int
MlePlayer::sendWindow(WId wid)
{
	if (m_wire->sendMsg(m_objID, "Window", &wid, sizeof(WId)) < 0)
	{
        printf("PLAYER ERROR: sending window %llu\n", wid);
	    return (-1);
	}
	return(0);
}
#else
int
MlePlayer::sendWindow(Window wid)
{
    if (m_wire->sendMsg(m_objID, "Window", &wid, sizeof(Window)) < 0)
	{
        printf("PLAYER ERROR: sending window %ld\n", wid);
        return (-1);
    }
    return(0);
}
#endif /* ! Q_OS_UNIX */
#endif /* __linux__ */
#if defined(_WINDOWS)
int
MlePlayer::sendWindow(HWND wid)
{
    if (m_wire->sendMsg(m_objID, "Window", &wid, sizeof(HWND)) < 0)
	{
        printf("PLAYER ERROR: sending window %d\n", wid);
        return (-1);
    }
    return(0);
}
#endif /* _WINDOWS */

/*****************************************************************************
* Sending back pick information
*****************************************************************************/
int
MlePlayer::sendPick(char* setName, char* actorName)
{
	MLE_DEBUG_CAT("ATK",
		printf("PLAYER sendPick()\n");
	);

    AtkWireMsg* msg = new AtkWireMsg(m_objID, "Pick");
    msg->addParam(setName ? setName: "");
    msg->addParam(actorName);

    if (m_wire->sendMsg(msg) < 0)
	{
		printf("PLAYER ERROR: sending pick Set:%s  actor:%s\n",
			(setName) ? setName: "", (actorName) ? actorName: "");
		delete msg;
		return(-1);
    }
    delete msg;
    return(0);
}

/*****************************************************************************
* Sending back unpick information
*****************************************************************************/
int
MlePlayer::sendUnpick(char* setName, char* actorName)
{
    AtkWireMsg* msg = new AtkWireMsg(m_objID, "Unpick");
    msg->addParam(setName ? setName: "");
    msg->addParam(actorName);

    if (m_wire->sendMsg(msg) < 0)
	{
		printf("PLAYER ERROR: sending unpick Set:%s  actor:%s\n",
			(setName) ? setName: "", (actorName) ? actorName: "");
		delete msg;
		return(-1);
    }
    delete msg;
    return(0);
}

/*****************************************************************************
* Sending back start, continue, end manip markers
*****************************************************************************/

// is3d defaults to true - ignored on tools side except for manipCB case

int MlePlayer::sendManip(char *manipTypeStr, char* actorName, 
	MlTransform *t, int is3d)
{
    AtkWireMsg* msg = new AtkWireMsg(m_objID, manipTypeStr);
    msg->addParam(actorName);
    msg->addParam(*t);
    msg->addParam(is3d);

	//fprintf(stderr, "MlePlayer::sendManip %s, actor %s, is3d %d\n", 
	// manipTypeStr, actorName, is3d);

	//msg->print("MlePlayer::sendManip()");

    if (m_wire->sendMsg(msg) < 0) 
    {
		fprintf(stderr, "PLAYER ERROR: sending %s actor:%s\n",
			manipTypeStr, actorName);
		delete msg;
		return(-1);
    }
    delete msg;
    return(0);
}

/*****************************************************************************
* Sending back property Change info
*****************************************************************************/
int
MlePlayer::sendPropertyChange(MleActor* actor, char* propName)
{
    // Checks.
    if (!actor || !propName) return(-1);

    // Eventually, get Actor Member object by finding class, member.
    const MleActorClass* ac = actor->getClass();
    if (ac)
	{
		// Find member.
		const MleActorMember* am = ac->findMember(propName);
		if (am)
		{
			// Send back message.
			AtkWireMsg* msg = new AtkWireMsg(m_objID, "PropertyChange");
			msg->addParam(actor->getName());
			msg->addParam(propName);
			// XXX - we are assuming no string properties.
			//msg->addParam(((char*) actor) + am->getOffset(), am->getType()->getSize());
			MlePropertyEntry *entry = am->getEntry();
			char *value;
			entry->getProperty(actor, entry->name, (unsigned char **)&value);
			msg->addParam(value, am->getType()->getSize());

			m_wire->sendMsg(msg);

			delete msg;
			return(0);;
		}
    }
    return(-1);
}

/*****************************************************************************
* Sending back double click
*****************************************************************************/
int
MlePlayer::sendDoubleClick(MleActor* actor, int keymask)
{
    // Check actor.
    if (!actor) return(-1);

    // Send message.
    AtkWireMsg* msg = new AtkWireMsg(m_objID, "DoubleClick");
    msg->addParam(actor->getName());
    msg->addParam(keymask);

    if (m_wire->sendMsg(msg) < 0)
	{
		printf("PLAYER ERROR: sending doubleClick actor:%s  keymask: %d\n",
			   actor->getName(), keymask);
		delete msg;
		return(-1);
    }
    delete msg;
    return(0);
}

/*****************************************************************************
* Getting a set
*****************************************************************************/

MleDwpSet* 
MlePlayer::sendGetWorkprintSet(const char* setName)
{
    if (!setName) return(NULL);

    // Send msg over wire
    AtkWireMsg* msg = m_wire->sendSyncMsg(this, m_objID, "GetSet", (void*) setName,
		strlen(setName)+1);
    if (!msg)
	{
		printf("PLAYER ERROR: could not get set %s\n", setName);
		return(NULL);
    }

    // Create an input object - read objects into it
    MleDwpInput* in = new MleDwpInput;
    in->setBuffer((char*) msg->m_msgData);
    MleDwpItem* item = MleDwpItem::readAll(in);

    if (!item)
	{
		printf("PLAYER: null item in sendGetSet - name: %s\n", setName);
    }

    // Cleanup and return item.
    delete in;
    delete msg;
    return((MleDwpSet*) item);
}

/*****************************************************************************
* Stats
*****************************************************************************/

int 
MlePlayer::sendStats(int stats)
{
    // Send message.
    AtkWireMsg* msg = new AtkWireMsg(m_objID, "Stats");
    msg->addParam(stats);

    if (m_wire->sendMsg(msg) < 0)
	{
		printf("PLAYER ERROR: sending stats stats: %d\n", stats);
		delete msg;
		return(-1);
    }
    delete msg;
    return(0);
}

#if defined(__linux__) || defined(__APPLE__)
/*****************************************************************************
* Right mouse callback
*****************************************************************************/
#ifdef Q_OS_UNIX
int 
MlePlayer::sendRightMouse(QEvent* ev)
{
	printf("Sending Right mouse\n");

    // Send message.
    AtkWireMsg* msg = new AtkWireMsg(m_objID, "RightMouse");
    msg->addParam(ev, sizeof(QEvent));

    if (m_wire->sendMsg(msg) < 0)
	{
		printf("PLAYER ERROR: sending right Mouse\n");
		delete msg;
		return(-1);
    }
    delete msg;
    return(0);
}
#else
int
MlePlayer::sendRightMouse(XEvent* ev)
{
	printf("Sending Right mouse\n");

    // Send message.
    AtkWireMsg* msg = new AtkWireMsg(m_objID, "RightMouse");
    msg->addParam(ev, sizeof(XEvent));

    if (m_wire->sendMsg(msg) < 0)
	{
		printf("PLAYER ERROR: sending right Mouse\n");
		delete msg;
		return(-1);
    }
    delete msg;
    return(0);
}
#endif /* ! Q_OS_UNIX */
#endif /* __linux__ */
#if defined(_WINDOWS)
int 
MlePlayer::sendRightMouse(DWORD *ev)
{
	printf("Sending Right mouse\n");

    // Send message.
    AtkWireMsg* msg = new AtkWireMsg(m_objID, "RightMouse");
    msg->addParam(ev, sizeof(DWORD));

    if (m_wire->sendMsg(msg) < 0)
	{
		printf("PLAYER ERROR: sending right Mouse\n");
		delete msg;
		return(-1);
    }
    delete msg;
    return(0);
}
#endif /* _WINDOWS */

/*****************************************************************************
* Registering with the stage
*****************************************************************************/

void
MlePlayer::registerWithStage()
{
    MLE_ASSERT(MleStage::g_theStage);

    MleStage::g_theStage->setPickCallback(pickCB, this);
    MleStage::g_theStage->setUnpickCallback(unpickCB, this);
    MleStage::g_theStage->setStartManipCallback(startManipCB, this);
    MleStage::g_theStage->setManipCallback(manipCB, this);
    MleStage::g_theStage->setFinishManipCallback(endManipCB, this);
    MleStage::g_theStage->setOpenCallback(doubleClickCB, this);
#if defined(__linux__) || defined(__APPLE__)
#ifdef Q_OS_UNIX
#else
    MleStage::g_theStage->setRightMouseCallback(rightMouseCB, this);
#endif /* ! Q_OS_UNIX */
#endif /* __linux__ */
}

void
MlePlayer::pickCB(MleActor* actor, void* clientData)
{
    MlePlayer* player = (MlePlayer*) clientData;

    // XXX - fill out set.
    player->sendPick(NULL, (actor) ? actor->getName() : NULL);
}

void
MlePlayer::unpickCB(MleActor* actor, void* clientData)
{
    MlePlayer* player = (MlePlayer*) clientData;

    // XXX - fill out set.
    player->sendUnpick(NULL, (actor) ? actor->getName() : NULL);
}

void MlePlayer :: startManipCB(MleActor* actor, void* clientData)
{
    MlePlayer* player = (MlePlayer*) clientData;
    MlTransform t;
    actor->getTransform(t);
    player->sendManip(const_cast<char*>("StartManip"), (actor) ? actor->getName() : NULL, &t);
}

void
MlePlayer::manipCB(MleActor* actor, void* clientData)
{
    MLE_ASSERT(actor);
    MlePlayer* player = (MlePlayer*) clientData;
    MlTransform t;
    actor->getTransform(t);
    
    // See if it's a 2d set. We're checking for 2d rather than
    // 3d so that if someone doesn't use our class hierarchy at
    // all, this code will assume 3d, and tools will assume transform
    // is a 3d one, which is just extra info if it turns out to be 2d.
    MleRole *role = actor->getRole();
    int is2d = (role ? role->m_set->isa("Mle2dSet") : 0);
    int is3d = (is2d ? 0 : 1);

    player->sendManip(const_cast<char*>("Manip"), actor->getName(), &t, is3d);
}

void MlePlayer :: endManipCB(MleActor* actor, void* clientData)
{
    MlePlayer* player = (MlePlayer*) clientData;
    MlTransform t;
    actor->getTransform(t);
    player->sendManip(const_cast<char*>("EndManip"), (actor) ? actor->getName() : NULL, &t);

    // XXX - Currently, we are just sending a list of properties that we
    // believe might change.
    player->sendPropertyChange(actor, const_cast<char*>("position"));
    player->sendPropertyChange(actor, const_cast<char*>("orientation"));
    player->sendPropertyChange(actor, const_cast<char*>("transform"));
}

void
MlePlayer::doubleClickCB(MleActor* actor, void* clientData)
{
    MlePlayer* player = (MlePlayer*) clientData;

    // XXX - Currently, we are just sending a list of properties that we
    // believe might change.
    player->sendDoubleClick(actor, 0);
}

#if defined(__linux__) || defined(__APPLE__)
#ifdef Q_OS_UNIX
void
MlePlayer::rightMouseCB(QEvent* e, void* clientData)
{
    MlePlayer* player = (MlePlayer*) clientData;

    // XXX  - Currently, we are just sending a list of properties that we
    // believe might change.
    player->sendRightMouse(e);
}
#else
void
MlePlayer::rightMouseCB(XEvent* e, void* clientData)
{
    MlePlayer* player = (MlePlayer*) clientData;

    // XXX  - Currently, we are just sending a list of properties that we
    // believe might change.
    player->sendRightMouse(e);
}
#endif /* ! Q_OS_UNIX */
#endif /* __linux__ */


void MlePlayer::registerProp(MleActor *actor, const char *prop)
{
    MlePropStruct * propStruct = new MlePropStruct;
    propStruct->m_actor = actor;
    propStruct->m_property = strdup(prop);

    char *temp;
    if (getPropInfo(actor, prop, (void **) &temp, propStruct->m_length) == 0)
	{
        propStruct->m_data  = mlMalloc(propStruct->m_length);
		//bcopy(temp, propStruct->m_data, propStruct->m_length);
		memcpy(propStruct->m_data, temp, propStruct->m_length);

		m_propArray.add(propStruct);
    } else
	{
        printf("MlePlayer::registerProp(): Couldn't find actor %s's "
	       "property %s.\n", actor->getName(), prop);
    }
}


void MlePlayer::unregisterProp(MleActor *actor, const char *prop)
{
    for (int i = 0; i < m_propArray.getLength(); i++)
	{
        MlePropStruct *current = m_propArray[i];
		if (current->m_actor == actor &&
			strcmp(current->m_property, prop) == 0)
		{
			m_propArray.remove(i);
			mlFree(current->m_property); // allocated in strdup
			mlFree(current->m_data);
			delete current;
			break;
		}
    }
}

int MlePlayer::getPropInfo(MleActor *actor, const char *property, void **data,
                           int &length) const
{
	MLE_ASSERT(actor != NULL);
    MLE_ASSERT(property != NULL);

    // Make sure the actor class is present.
    MLE_ASSERT( actor->getClass() != NULL );

    // Get the member corresponding to the property name
    const MleActorMember *member = actor->getClass()->findMember(property);
    if ( member == NULL )
	{
		return(-1);
	}

	length = member->getType()->getSize();
	//*data = ((char *) actor) + member->getOffset();
	MlePropertyEntry *entry = member->getEntry();
	char *value;
	entry->getProperty(actor, entry->name, (unsigned char **)&value);
	*data = value;

	return(0);
}

void MlePlayer::notifyPropChanged(void)
{
    char *data;
    int length;

    for (int i = 0; i < m_propArray.getLength(); i++)
	{
        MlePropStruct *current = m_propArray[i];
		if (getPropInfo(current->m_actor, current->m_property, (void **) &data,
				length) == 0)
		{
			if (length != current->m_length ||
				//bcmp(data, current->m_data, current->m_length) != 0)
				memcmp(data, current->m_data, current->m_length) != 0)
			{

				sendPropertyChange(current->m_actor, current->m_property);

				// save copy of new value
				if (length != current->m_length)
				{
					if (current->m_data != NULL)
					{
						current->m_data = mlRealloc(current->m_data, length);
					} else {
						current->m_data = mlMalloc(length);
					}
					current->m_length = length;
				}
				//bcopy(data, current->m_data, length);
				memcpy(current->m_data, data, length);
			}
		} else
		{
			printf("MlePlayer::notifyPropChanged(): Couldn't find property %s "
			   "in actor %s to send change notification.\n",
			   current->m_actor->getName(), current->m_property);
		}
    }
}

/*****************************************************************************
* Creating load scene return msg
*****************************************************************************/
int
MlePlayer::createLoadSceneRetMsg(AtkWireMsg* msg, MleDwpScene* wpScene, MleScene *scene)
{
    MLE_ASSERT(NULL != msg);
    MLE_ASSERT(NULL != wpScene);

    if (NULL == scene)
	{
		// We must have a scene here, so lack is an error in scene loading.
		// Set up a non-zero error count as first param
		return(1);
    }

    // XXX - maybe this should be done with a callback - this seems 
    // a little flimsy the way we are checking for errors.
    // Find all groups that should be in scene.
    MleDwpFinder groupFinder(MleDwpGroup::typeId, NULL, 1);
    groupFinder.find(wpScene);
    MleDwpGroup** groups = (MleDwpGroup**) groupFinder.getItems();

    // Set up number of errors as first param.
    msg->addParam(groupFinder.getNumItems() - scene->getSize());
	//printf("CLDGRM:  Num Errors: %d\n", groupFinder.getNumItems() - scene->getSize());

    // Loop through these checking to see if they actually made it in scene.
    for (int i=0; i<groupFinder.getNumItems(); i++)
	{
		// Not found.
		if (!findGroupInScene(groups[i], scene))
		{
			msg->addParam(groups[i]->getName());
			printf("CLGRM: error on group %s\n", groups[i]->getName());
		}
    }
    return(groupFinder.getNumItems() - scene->getSize());
}

int
MlePlayer::createLoadBootSceneRetMsg(AtkWireMsg* msg, MleDwpItem* items, MleScene *scene)
{
    MLE_ASSERT(NULL != msg);
    MLE_ASSERT(NULL != items);

    if (NULL == scene)
	{
		// We must have a scene here, so lack is an error in scene loading.
		// Set up a non-zero error count as first param
		return(1);
    }

    // XXX - maybe this should be done with a callback - this seems 
    // a little flimsy the way we are checking for errors.
    // Find all groups that should be in scene
    //   first, find the root
    MleDwpItem *root = items;
    MleDwpItem *parent;
    while ( NULL != (parent = root->getParent()) )
	{
	    root = parent;
    }

    MleDwpFinder sceneFinder(MleDwpScene::typeId, NULL, 1);
    sceneFinder.find(root);
    MleDwpScene* wpScene = (MleDwpScene*) sceneFinder.getItem();

    MleDwpFinder groupFinder(MleDwpGroup::typeId, NULL, 1);
    groupFinder.find(wpScene);
    MleDwpGroup** groups = (MleDwpGroup**) groupFinder.getItems();

    // Set up number of errors as first param.
    msg->addParam(groupFinder.getNumItems() - scene->getSize());
	//printf("CLDGRM:  Num Errors: %d\n", groupFinder.getNumItems() - scene->getSize());

    // Loop through these checking to see if they actually made it in scene
    for (int i=0; i<groupFinder.getNumItems(); i++)
	{
		// Not found.
		if (!findGroupInScene(groups[i], scene))
		{
			msg->addParam(groups[i]->getName());
			printf("CLGRM: error on group %s\n", groups[i]->getName());
		}
    }
    return(groupFinder.getNumItems() - scene->getSize());
}

MleGroup* 
MlePlayer::findGroupInScene(MleDwpGroup* group, MleScene *scene)
{
    // Just loop through groups in list.
    for (int i=0; i<scene->getSize(); i++)
	{
		if (!strcmp(group->getName(), (*scene)[i]->getName()))
		{
			return((*scene)[i]);
		}
    }

    // Not found.
    return(NULL);
}

/*****************************************************************************
* Creating load group return msg
*****************************************************************************/
int
MlePlayer::createLoadGroupRetMsg(AtkWireMsg* msg, MleDwpGroup* wpGroup, MleGroup *group)
{
    MLE_ASSERT(NULL != msg);
    MLE_ASSERT(NULL != wpGroup);

    if (NULL == group)
	{
		// We must have a group here, so lack is an error in group loading.
		// Set up a non-zero error count as first param
		return(1);
    }

    // XXX - maybe this should be done with a callback - this seems 
    // a little flimsy the way we are checking for errors.
    // Find all actors that should be in group.
    MleDwpFinder actorFinder(MleDwpActor::typeId, NULL, 1);
    actorFinder.find(wpGroup);
    MleDwpActor** actors = (MleDwpActor**) actorFinder.getItems();

    // Set up number of errors as first param
    msg->addParam(actorFinder.getNumItems() - group->getSize());
	//printf("CLDGRM:  Num Errors: %d\n", actorFinder.getNumItems() - group->getSize());

    // Loop through these checking to see if they actually made it in group.
    for (int i=0; i<actorFinder.getNumItems(); i++)
	{
		// Not found.
		if (!findActorInGroup(actors[i], group))
		{
			msg->addParam(actors[i]->getName());
			printf("CLGRM: error on actor %s\n", actors[i]->getName());
		}
    }
    return(actorFinder.getNumItems() - group->getSize());
}

MleActor* 
MlePlayer::findActorInGroup(MleDwpActor* actor, MleGroup *group)
{
    // Just loop through actors in list.
    for (int i=0; i<group->getSize(); i++)
	{
		if (!strcmp(actor->getName(), (*group)[i]->getName()))
		{
			return((*group)[i]);
		}
    }

    // Not found.
    return(NULL);
}
