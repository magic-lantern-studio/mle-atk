/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file MlePlayer.h
 * @ingroup MleATK
 *
 * This file contains a class that provides utility for
 * managing a Magic Lantern Rehearsal Player.
 *
 * @author Mark S. Millard
 * @date May 5, 2003
 */

// COPYRIGHT_BEGIN
//
// The MIT License (MIT)
//
// Copyright (c) 2015 Wizzer Works
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

#ifndef __MLE_PLAYER_H_
#define __MLE_PLAYER_H_

// Declare classes.
class AtkWire;

class MleDwpGroup;
class MleDwpScene;
class MleDwpActor;
class MleDwpSet;
class MleDwpMediaRef;

class MleActor;
class MleGroup;
class MleScene;
class MlTransform;
//class MleProperty;

// Include system header files.
#if defined(sgi) || defined(__linux__)
#include <X11/X.h>
#include <X11/Xlib.h>
#endif
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers.
#include <windows.h>
#include <signal.h>
#endif

#include "mle/mleatk_rehearsal.h"

// Include Digital Workprint header files.
#include "mle/DwpItem.h"

// Include Authoring Toolkit header files.
#include "mle/AtkBasicArray.h"
#include "mle/AtkWired.h"


typedef struct 
{
    MleActor *m_actor;
    char *m_property;
    void *m_data;
    int m_length;
} MlePropStruct;

MLE_DECLARE_ARRAY(MlePropArray, MlePropStruct*);


class MLEATK_REHEARSAL_API MlePlayer : public AtkWired
{
  public:
    
    MlePlayer(AtkWire* wire, void* objID);

    ~MlePlayer();

    // Creating a player .
    static MlePlayer* create(int argc, char* argv[]);

    // Delivering messages.
    virtual AtkWireMsg* deliverMsg(AtkWireMsg* msg);

    /**************************************************************************
    *  Interface to player object - Recv
    **************************************************************************/

    // Initializing the player.
    virtual void recvInit(int w, int h);

    // Controlling the player.
    virtual int recvPlay();

    virtual int recvPause();

    // Snapping.
    virtual int recvMoveToTarget();

    // Nudging current actor.
    virtual int recvNudge(int dir, int numPixels);

    // Finding actors.
    virtual void recvFindActor(const char* name);

    // Property interface.
    virtual void recvGetActorPropertyNames(const char *actorName,
		const char *propDataSet);

    virtual void recvGetActorProperty(const char* actorClass, 
		const char* actorName, 
		const char* propName);

    virtual int recvSetActorProperty(const char* actorClass, 
		const char* actorName, 
		const char* propName, void* data);

    // Changing name of actor.
    int recvSetActorName(char* actorName, char* newActorName);

    // Check type of actor.
    virtual void recvGetActorIsA(const char *actorName,
		const char *actorClass);

    // Actor transformation matrix.
    int recvSetTransform(char *actorName, MlTransform& t);

    int recvGetTransform(char *actorName);

    // Finding objects.
    virtual void recvFind(MleDwpType t, const char* name, int findAll);

    // Picking.
    virtual void recvPick(char* setName, int x, int y);

    // Refresh.
    virtual void recvRefresh();

    // Frame advance.
    virtual void recvFrameAdvance();

    // Setting a camera.
    virtual void recvSetCamera(char* setName, char* cameraName);

    // Loading actor groups/ unloading actors
    virtual void recvLoadGroup(void* data);

    virtual void recvUnloadActor(char* actorName);

    virtual void recvUnloadGroup(char* groupName);

    // Loading and unloading scenes.
    virtual void recvLoadScene(void* data);

    virtual void recvLoadBootScene(void* data);

    virtual void recvUnloadScene(char* sceneName);

    // De/Activate manipulator on an actor
    virtual void recvActivateManip(char* actorName);

    virtual void recvDeactivateManip(char* actorName);

    // Quit message
    virtual void recvQuit();

    // XXX receive configuration workprint items.
    virtual void recvWorkprintItem(void *data);

    // Setting Stage editting mode.
    virtual void recvStageEditMode(int x);

    // Placement mode.
    virtual void recvPlacementState(int state);

    // Resize.
    virtual void recvResize(int w, int h);

    // Setting an actors position.
    virtual void recvSetPosition(char* setName, char* actorName, int x, int y);

    // Getting resolve edit.
    virtual void recvResolveEdit(char* actorName, char* propName = NULL);

    // Getting Camera transform.
    virtual void recvGetCameraPosition(char* setName);

    // Setting camera transform.
    virtual void recvSetCameraPosition(char* setName, MlTransform *t);

    // Title stats
    virtual void recvStartStats();

    virtual void recvEndStats();

    // Register for actor editor property change updates.
    virtual void recvRegisterProperty(char* actorName, char *propName);

    virtual void recvUnregisterProperty(char* actorName, char *propName);

    // Getting info on functions.
    virtual void recvGetFunctions(char* objectType, char* objectName);

    virtual void recvGetFunctionAttributes(char* objectType, 
				char* objectName, char* functionName);

    // Viewers.
    virtual void recvSetViewer(char* viewerName);

    virtual void recvGetViewer();

    // Edit modes.
    virtual void recvSetEditMode(char* editMode);

    virtual void recvGetEditMode();

    // Snapping.
    virtual void recvHasSnappingTarget();

    virtual void recvGetSnapping();

    virtual void recvSetSnapping(int mode);

    // Push/pop sets
    virtual void recvPushSet(char* setName);

    virtual void recvPushSetToBottom(char* setName);

    virtual void recvPopSet(char* setName);

    virtual void recvPopSetToTop(char* setName);

    // Background colors.
    virtual void recvSetBackgroundColor(float* f);

    virtual void recvGetBackgroundColor();
    
    // Horizon grid.
    virtual void recvSetHorizonGrid(int onOff);
    virtual void recvGetHorizonGrid();

    // Misc inventor controls.
    void recvOpenPrefsDialog();

    void recvViewAll();

    void recvShowDecoration(int onOff);

    // Push/pop actors.
    virtual void recvPushActor(char* setName, char* actorName);

    virtual void recvPushActorToBottom(char* setName, char* actorName);

    virtual void recvPopActor(char* setName, char* actorName);

    virtual void recvPopActorToTop(char* setName, char* actorName);

    // Render modes.
    virtual void recvSetRenderMode(char* setName, char* actorName, char* mode);

    virtual void recvGetRenderMode(char* setName, char* actorName);

    virtual void recvSetGlobalRenderMode(char* mode);

    virtual void recvGetGlobalRenderMode();

    // Perspective.
    virtual void recvSetPerspective(char *setName, int perspectiveOnOff);

    virtual void recvGetPerspective(char *setName);

    // Load a set from workprint.
    virtual void recvLoadSet(const char *setName);

    // Getting a set from x,y.
    virtual void recvGetSets(int x, int y);

    // Reparenting a window.
#if defined(sgi) || defined(__linux__)
    virtual void recvReparentWindow(Window w);
#endif
#if defined(WIN32)
	virtual void recvReparentWindow(HWND w);
#endif /* sgi */

    // Setting a sets name.
    virtual void recvSetSetName(char* oldSetName, char* newSetName);

    // Tools initialization finished.
    virtual void recvToolsInitFinished();

    /**************************************************************************
    *  Interface to player object - Send to tools
    **************************************************************************/

    // Loading groups across the wire.
    virtual MleDwpGroup* sendGetWorkprintGroup(const char* id);

    virtual MleDwpScene* sendGetWorkprintScene(const char* id);

    virtual int sendLoadWorkprint(const char *filename);

	// Sending the window over.
#if defined(sgi) || defined(__linux__)
    virtual int sendWindow(Window w);
#endif
#if defined(WIN32)
	virtual int sendWindow(HWND w);
#endif /* sgi */

    // Loading media refs across the wire.
    virtual MleDwpMediaRef* sendGetWorkprintMediaRef(const char* id);

    // Send a picked actor.
    virtual int sendPick(char* setName, char* actorName);

    virtual int sendUnpick(char* setName, char* actorName);

    // Send start/continue/end manip
    virtual int sendManip(char *manipTypeStr, char *actorName, 
		MlTransform *t, int is3d = 1);

    // Send a changed property.
    virtual int sendPropertyChange(MleActor* actor, char* propertyName);

    // Double click.
    virtual int sendDoubleClick(MleActor* actor, int keymask);

    // Getting a set.
    virtual MleDwpSet* sendGetWorkprintSet(const char* setName);

    // Sending title stats.
    virtual int sendStats(int time);

    // Sending right mouse callback
#if defined(sgi) || defined(__linux__)
    virtual int sendRightMouse(XEvent* e);
#endif
#if defined(WIN32)
	virtual int sendRightMouse(DWORD *e);
#endif


    /**************************************************************************
    *  Callbacks
    **************************************************************************/

    void setInitWindowCB(void (*func)(void*), void* data = NULL)
	{ initWindowCB = func; initWindowData = data; }

    void setRefreshCB(void (*func)(void*), void* data = NULL)
	{ refreshCB = func; refreshData = data; }

    void setFrameAdvanceCB(void (*func)(void*), void* data = NULL)
	{ frameAdvanceCB = func; frameAdvanceData = data; }

    void setToolsInitFinishedCB(void (*func)(void*), void* data = NULL)
	{ toolsInitFinishedCB = func; toolsInitFinishedData = data; }

    /**************************************************************************
    *  Misc routines
    **************************************************************************/

    // ds access
#if defined(sgi) || defined(__linux__)
    Window getWID() { return m_wid; }
    VisualID getVID() { return m_vid; }
#endif

    int isRunning() { return m_running; }

    int hasQuit() { return m_quit; }

    int getSendStats() { return m_sendStats; }

    int getErrorFD() { return m_errorFD; }

    void setErrorFD(int fd) { m_errorFD = fd; }

    // Registering with the stage & callback function.
    virtual void registerWithStage();

    static void pickCB(MleActor* actor, void* clientData);

    static void unpickCB(MleActor* actor, void* clientData);

    static void startManipCB(MleActor* actor, void* clientData);

    static void manipCB(MleActor* actor, void* clientData);

    static void endManipCB(MleActor* actor, void* clientData);

    static void doubleClickCB(MleActor* actor, void* clientData);

#if defined(sgi) || defined(__linux__)
    static void rightMouseCB(XEvent* e, void* clientData);
#endif
#if defined(WIN32)
	static void rightMouseCB(DWORD e, void* clientData);
#endif

    // Getting pref height and width.
    int getPrefWidth() { return m_prefWidth; }

    int getPrefHeight() { return m_prefHeight; }

    // Property registration.
    void registerProp(MleActor *actor, const char *prop);

    void unregisterProp(MleActor *actor, const char *prop);

    void notifyPropChanged(void);

    // Adding errors to return msg for loadgroup.
    virtual int createLoadSceneRetMsg(AtkWireMsg* msg, MleDwpScene* wpScene, 
				      MleScene* scene);

    virtual int createLoadBootSceneRetMsg(AtkWireMsg* msg, MleDwpItem* items, 
				      MleScene* scene);

    virtual MleGroup* findGroupInScene(MleDwpGroup* group, MleScene* scene);

    virtual int createLoadGroupRetMsg(AtkWireMsg* msg, MleDwpGroup* wpGroup, 
				      MleGroup* group);

    virtual MleActor* findActorInGroup(MleDwpActor* actor, MleGroup* group);



  protected:

    // Placement mode values.
    // XXX - how can we share this enum with playerMgr?
    enum MlePlacementState { ON_BACKGROUND, FLOATING };

#if defined(sgi) || defined(__linux__)
    Window m_wid;
    VisualID m_vid;
#endif
    int m_running;
    int m_quit;

    void (*initWindowCB)(void*);
    void (*refreshCB)(void*);
    void (*frameAdvanceCB)(void*);
    void (*toolsInitFinishedCB)(void*);

    void* initWindowData;
    void* refreshData;
    void* frameAdvanceData;
    void* toolsInitFinishedData;

    int m_prefWidth, m_prefHeight;

    int m_curStageEditMode;
    int m_curPlacementState;

    int m_sendStats;

    MlePropArray m_propArray;

    int getPropInfo(MleActor *actor, const char *property, void **data,
		    int &length) const;

    int m_errorFD;

    // Signal handler to flushing diagnostic pipes.
    static void signalHandler(int signal, ...);
};

#endif /* __MLE_PLAYER_H_ */
