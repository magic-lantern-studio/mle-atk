/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2005 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

// src/Inventor/Win/viewers/MleFlyViewer.cpp.  Generated from SoGuiFlyViewer.cpp.in by configure.

/*!
  \class MleFlyViewer Inventor/Win/viewers/MleFlyViewer.h
  \brief The MleFlyViewer class implements controls for moving
  the camera in a "flying" motion.
  \ingroup viewers

  Controls:
  <ul>

  <li>Left mouse button increases the speed.</li>

  <li>Middle mouse button decreases the speed.</li>

  <li>Left and middle mouse button together sets the speed to zero.</li>

  <li>"s" puts the viewer in seek mode. Click some geometry with the
      left mouse button to start the seek zoom animation. (Hitting "s"
      again before clicking will cancel the seek operation.)</li>

  <li>"u" puts the viewer in up-vector pick mode. Click some geometry
      with the left mouse button to set the camera's up-vector to the
      normal vector of the face you pick.
      (Hitting "u" again before clicking will cancel the pick operation.)</li>

  <li>The control key stops the flying and lets you tilt the camera by moving
      the pointer.</li>

  </ul>
*/

/*
  FIXME:
  - animate camera when setting up-vector so the scene doesn't just
    suddenly change.
*/

#include <sowindefs.h>
#include <mle/MleFlyViewer.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <string.h> // strlen() etc
#include <stdlib.h> // abs() 
#include <Inventor/Win/SoWinCursor.h>


// ************************************************************************

#ifndef DOXYGEN_SKIP_THIS

// FIXME: We should probably move this class out of this
// impl-file. There is code here that could be factored out and reused
// in other modules, for example camera handling. Now there is a
// duplication with some of the code in MleConstrainedViewerP, for
// example. 20021017 rolvs
class MleFlyViewerP {
public:
  MleFlyViewerP(MleFlyViewer * owner);
  ~MleFlyViewerP();

  enum ViewerMode {
    FLYING, TILTING, WAITING_FOR_SEEK, WAITING_FOR_UP_PICK
  };

  void constructor(SbBool build);

  void dolly(const float delta) const;
  void updateCursorRepresentation(void); // in SoNativeFlyViewer.cpp
  void setMode(ViewerMode newmode);
  int getMode(void) { return this->viewermode; }

#define SOWIN_MIN_STEP     0.2f
#define SOWIN_INC_FACTOR   1.2f
#define SOWIN_MAX_SPEED   20.0f

  SbTime * lastrender;

  float currentspeed;
  
  // Maximum speed, target for currentspeed during
  // acceleration/decceleration.
  float maxspeed; 

  // Scales speed. Calculated in updateSpeedScalingFactor.
  float speed_scaling_factor; 
  
  // Used to calculate a new max_speed, on the basis on 'where' we are
  // in the speed landscape, see {increment|decrement}MaxSpeed(). 
  int max_speed_factor; 

  // Speed
  void incrementMaxSpeed();
  void decrementMaxSpeed();
  void updateMaxSpeed();
  void updateSpeedScalingFactor();
  void stopMoving();

  void updateSpeedIndicator(void);
  void updateCameraPosition( SoCamera * camera, float speed, float dt ); 
  void updateCameraOrientation( SoCamera * camera, 
                                float d_tilt, 
                                float d_pan, 
                                float dt );
  float calculateChangeInTime();
  void  updateCurrentSpeed( float dt );


  // Current keyboard state.
  SbBool button1down;
  SbBool button3down;
  int lctrldown;
  int rctrldown;
  SbBool lshiftdown;
  SbBool rshiftdown;

  // View, speed display, renderingstate.
  SoSearchAction * searcher;

  SoNode * superimposition;
  SoCoordinate3 * sgeometry;

  SoScale * sscale;
  SoScale * crossscale;

  SoTranslation * stranslation;
  SoTranslation * crossposition;

  SoSwitch * smaxspeedswitch; 
  SoSwitch * scurrentspeedswitch;
  SoSwitch * crossswitch;

  SoNode * getSuperimpositionNode(const char * name);

  void superimpositionevent(SoAction * action);
  static void superimposition_cb(void * closure, SoAction * action);


  // 
  float tilt_increment; // Angle-adjustment between View-up and direction 
  float pan_increment;  // Rotation-adjustment around View-up

  SbVec2s mouseloc;
  SbVec2s lastpos;
  SbVec2s tiltpos;

  // FIXME: Refactor event handlers and PUBLIC(this)->processSoEvent
  // in a way similar to that in SoGuiExaminerViewer, where only
  // internal state is red/set. 20021017 rolvs
  SbBool processKeyboardEvent( const SoKeyboardEvent * const kevt );
  SbBool processMouseButtonEvent( const SoMouseButtonEvent * const mevt );
  SbBool processLocation2Event( const SoLocation2Event * const levt );
private:
  MleFlyViewer * publ;
  ViewerMode viewermode;
};

MleFlyViewerP::MleFlyViewerP( MleFlyViewer * owner )
{
  this->searcher = NULL;
  this->publ = owner;
  this->viewermode = FLYING;
  this->currentspeed = 0.0f;
  this->maxspeed = 0.0f;
  this->speed_scaling_factor = 0.4f;
  this->max_speed_factor = 0;
  this->stranslation = NULL;
  this->sscale = NULL;
  this->button1down = FALSE;
  this->button3down = FALSE;
  this->lctrldown = 0;
  this->rctrldown = 0;
  this->lshiftdown = FALSE;
  this->rshiftdown = FALSE;
  this->lastrender = new SbTime;
  this->tilt_increment = 0;
  this->pan_increment = 0;
}

MleFlyViewerP::~MleFlyViewerP(void)
{
  if ( this->searcher != NULL )
    delete this->searcher;
  delete this->lastrender;

  // superimposition unrefed in other destructor
}

#define PRIVATE(o) (o->pimpl)
#define PUBLIC(o) (o->publ)


// Common constructor code.
void
MleFlyViewerP::constructor(SbBool build)
{
  PUBLIC(this)->setClassName(PUBLIC(this)->getDefaultWidgetName());

  static const char * superimposed[] = {
    "#Inventor V2.1 ascii",
    "",
    "Separator {",
    "  MaterialBinding {",
    "    value OVERALL",
    "  }",
    "  OrthographicCamera {",
    "    height 1",
    "    nearDistance 0",
    "    farDistance 1",
    "  }",
    "  DEF sowin->callback Callback { }",
    "  Separator {",
    "    DEF sowin->translation Translation {",
    "      translation 0 0 0",
    "    }",
    "    DEF sowin->scale Scale {",
    "      scaleFactor 1 1 1",
    "    }",
    "    DEF sowin->geometry Coordinate3 {",
    "      point [",
    "       -0.8 -0.04 0,",
    "       -0.8  0    0,",
    "       -0.8  0.04 0,",
    "        0   -0.04 0,",
    "        0    0    0,",
    "        0    0.04 0,",
    "        0.8 -0.04 0,",
    "        0.8  0    0,",
    "        0.8  0.04 0,", 
    "        0    0.02 0,", // idx 9
    "        0.8  0.02 0,",
    "        0.8 -0.02 0,",
    "        0   -0.02 0,",
    "        0    0.01 0,", // idx 13
    "        0.4  0.01 0,",
    "        0.4 -0.01 0,",
    "        0   -0.01 0",
    "      ]",
    "    }",
    "    DEF sowin->maxspeedswitch Switch {",
    "      whichChild -3",
    // max speed indicator
    "      Material {",
    "        emissiveColor 1 0 0",
    "      }",
    "      IndexedFaceSet {",
    "        coordIndex [",
    "          12, 11, 10, 9, -1",
    "        ]",
    "      }",
    "    }",
    // the coordinate system
    "    BaseColor {",
    "      rgb 1 1 1",
    "    }",
    "    IndexedLineSet {",
    "      coordIndex [",
    "        0, 2, -1,",
    "        3, 5, -1,",
    "        6, 8, -1,",
    "        1, 7, -1",
    "      ]",
    "    }",
    // current speed indicator
    "    DEF sowin->currentspeedswitch Switch {",
    "      whichChild -3",
    "      Material {",
    "        emissiveColor 0 0 1",
    "      }",
    "      IndexedFaceSet {",
    "        coordIndex [",
    "          16, 15, 14, 13, -1",
    "        ]",
    "      }",
    "    }",
    "  }",
    // cross
    "  DEF sowin->crossswitch Switch {",
    "    whichChild -1",
    "    DEF sowin->crossposition Translation {",
    "      translation 0 0 0",
    "    }",
    "    DEF sowin->crossscale Scale {",
    "      scaleFactor 1 1 1",
    "    }",
    "    BaseColor {",
    "      rgb 1 0 0",
    "    }",
    "    Coordinate3 {",
    "      point [",
    "        0 -1  0,",
    "        0  1  0,",
    "       -1  0  0,",
    "        1  0  0",
    "      ]",
    "    }",
    "    IndexedLineSet {",
    "      coordIndex [",
    "        0, 1, -1,",
    "        2, 3, -1",
    "      ]",
    "    }",
    "  }",
    "}",
    NULL
  };

  int i, bufsize;
  for (i = bufsize = 0; superimposed[i]; i++)
    bufsize += strlen(superimposed[i]) + 1;
  char * buf = new char [bufsize + 1];
  for (i = bufsize = 0; superimposed[i]; i++) {
    strcpy(buf + bufsize, superimposed[i]);
    bufsize += strlen(superimposed[i]);
    buf[bufsize] = '\n';
    bufsize++;
  }
  SoInput * input = new SoInput;
  input->setBuffer(buf, bufsize);
  SbBool ok = SoDB::read(input, this->superimposition);
  assert(ok);
  delete input;
  delete [] buf;
  this->superimposition->ref();


  this->sscale = (SoScale *)
    this->getSuperimpositionNode("sowin->scale");
  this->stranslation = (SoTranslation *)
    this->getSuperimpositionNode("sowin->translation");
  this->sgeometry = (SoCoordinate3 *)
    this->getSuperimpositionNode("sowin->geometry");
  this->smaxspeedswitch = (SoSwitch *)
    this->getSuperimpositionNode("sowin->maxspeedswitch");
  this->scurrentspeedswitch = (SoSwitch *)
    this->getSuperimpositionNode("sowin->currentspeedswitch");
  this->crossswitch = (SoSwitch *)
    this->getSuperimpositionNode("sowin->crossswitch");
  this->crossposition = (SoTranslation *)
    this->getSuperimpositionNode("sowin->crossposition");
  this->crossscale = (SoScale *)
    this->getSuperimpositionNode("sowin->crossscale");

  SoCallback * cb = (SoCallback *)
    this->getSuperimpositionNode("sowin->callback");
  cb->setCallback(MleFlyViewerP::superimposition_cb, this);

  this->updateSpeedIndicator();

  PUBLIC(this)->addSuperimposition(this->superimposition);
  PUBLIC(this)->setSuperimpositionEnabled(this->superimposition,TRUE);

  if (build) {
    HWND viewer = PUBLIC(this)->buildWidget(PUBLIC(this)->getParentWidget());
    PUBLIC(this)->setBaseWidget(viewer);
  }
}

// This method dollies the camera back and forth in the scene.
void
MleFlyViewerP::dolly(const float delta) const
{
  SoCamera * const camera = PUBLIC(this)->getCamera();
  if (camera == NULL) { return; } // if there's no scenegraph, for instance

  SbPlane walkplane(PUBLIC(this)->getUpDirection(), 
		    camera->position.getValue());

  SbVec3f campos = camera->position.getValue();
  SbVec3f camvec;
  camera->orientation.getValue().multVec(SbVec3f(0, 0, -1), camvec);
  SbLine cross(campos + camvec,
                campos + camvec + PUBLIC(this)->getUpDirection());
  SbVec3f intersect;
  walkplane.intersect(cross, intersect);
  SbVec3f dir = intersect - campos;
  dir.normalize();

  camera->position = campos - dir * delta;
}

// The viewer is a state machine, and all changes to the current state
// are made through this call.
void
MleFlyViewerP::setMode(ViewerMode newmode)
{
  this->viewermode = newmode;
  this->updateCursorRepresentation();
}

// This method locates a named node in the superimposed scene.
SoNode *
MleFlyViewerP::getSuperimpositionNode(const char * name)
{
  if (! this->searcher)
    this->searcher = new SoSearchAction;
  searcher->reset();
  searcher->setName(SbName(name));
  searcher->setInterest(SoSearchAction::FIRST);
  searcher->setSearchingAll(TRUE);
  searcher->apply(this->superimposition);
  assert(searcher->getPath());
  return searcher->getPath()->getTail();
}

SbBool
MleFlyViewerP::processKeyboardEvent( const SoKeyboardEvent * const ke )
{
  assert( ke != NULL );
  switch (ke->getState()) {
  case SoButtonEvent::UP:
    switch (ke->getKey()) {
    case SoKeyboardEvent::U:
      do {
	// either to switch to up-vector pick mode, or back to fly
	// mode if pick-mode already activated (ie cancel the
	// up-vector pick operation)
	SbBool uppickmode =
	  this->getMode() == MleFlyViewerP::WAITING_FOR_UP_PICK;
	this->setMode(uppickmode ? MleFlyViewerP::FLYING :
			       MleFlyViewerP::WAITING_FOR_UP_PICK);

        this->stopMoving();

	this->updateSpeedIndicator();
	PUBLIC(this)->scheduleRedraw();
	return TRUE;
      } while (FALSE);
      break;
      
    case SoKeyboardEvent::S:
      this->stopMoving();
      this->updateSpeedIndicator();
      PUBLIC(this)->scheduleRedraw();
      return FALSE;
      
    case SoKeyboardEvent::LEFT_SHIFT:
      this->lshiftdown = FALSE;
      if (this->lshiftdown < 0) {
#if SOWIN_DEBUG
	SoDebugError::post("MleFlyViewerP::processKeyboardEvent",
			   "left shift key count < 0");
#endif
	this->lshiftdown = 0;
      }
      break;
    case SoKeyboardEvent::RIGHT_SHIFT:
      this->rshiftdown = FALSE;
      if (this->rshiftdown < 0) {
#if SOWIN_DEBUG
	SoDebugError::post("MleFlyViewerP::processKeyboardEvent",
			   "right shift key count < 0");
#endif
	this->rshiftdown = 0;
      }
      break;
    case SoKeyboardEvent::LEFT_CONTROL:
      this->lctrldown -= 1;
      if (this->lctrldown < 0) {
#if SOWIN_DEBUG
	SoDebugError::post("MleFlyViewerP::processKyeboardEvent",
			   "left control key count < 0");
#endif
	this->lctrldown = 0;
      }
      break;
    case SoKeyboardEvent::RIGHT_CONTROL:
      this->rctrldown -= 1;
      if (this->rctrldown < 0) {
#if SOWIN_DEBUG
	SoDebugError::post("MleFlyViewerP::processKyeboardEvent",
			   "right control key count < 0");
#endif
	this->rctrldown = 0;
      }
      break;
    default:
      break;
    }
    break;
  case SoButtonEvent::DOWN:
    switch (ke->getKey()) {
    case SoKeyboardEvent::LEFT_SHIFT:
      this->lshiftdown += 1;
      if (this->lshiftdown > 2) {
#if SOWIN_DEBUG
	SoDebugError::post("MleFlyViewerP::processKeyboardEvent",
			   "left shift key count > 2");
#endif
	this->lshiftdown = 2;
      }
      break;
    case SoKeyboardEvent::RIGHT_SHIFT:
      this->rshiftdown += 1;
      if (this->rshiftdown > 2) {
#if SOWIN_DEBUG
	SoDebugError::post("MleFlyViewerP::processKeyboardEvent",
			   "right shift key count > 2");
#endif
	this->rshiftdown = 2;
      }
      break;
    case SoKeyboardEvent::LEFT_CONTROL:
      this->lctrldown += 1;
      if (this->lctrldown > 2) {
#if SOWIN_DEBUG
	SoDebugError::post("MleFlyViewerP::processKeyboardEvent",
			   "left control key count > 2");
#endif
	this->lctrldown = 2;
      }
      break;
    case SoKeyboardEvent::RIGHT_CONTROL:
      this->rctrldown += 1;
      if (this->rctrldown > 2) {
#if SOWIN_DEBUG
	SoDebugError::post("MleFlyViewer::processSoEvent",
			   "right control key count > 2");
#endif
	this->rctrldown = 2;
      }
      break;
    default:
      break; 
    }
    break;
  default:
    break;
  }

  if ((this->getMode() == MleFlyViewerP::FLYING) &&
      (this->lctrldown || this->rctrldown)) {
    this->setMode(MleFlyViewerP::TILTING);
    
    this->tiltpos = this->mouseloc;
    this->lastpos = this->mouseloc;

    this->stopMoving();
    this->updateSpeedIndicator();
    this->crossswitch->whichChild.setValue(SO_SWITCH_ALL);
    PUBLIC(this)->scheduleRedraw();
    // NOTE; this could be optimized to only draw the superimposition in
    // question if speed is zero.
  } else if ((this->getMode() == MleFlyViewerP::TILTING) &&
	     !this->lctrldown && !this->rctrldown) {
    this->setMode(MleFlyViewerP::FLYING);
    assert(this->crossswitch != NULL);
    this->crossswitch->whichChild.setValue(SO_SWITCH_NONE);
    PUBLIC(this)->scheduleRedraw();
  }
  return FALSE;
}


SbBool
MleFlyViewerP::processMouseButtonEvent( const SoMouseButtonEvent * const me )
{
  assert( me != NULL );
  
  // FIXME: only for fly mode
  switch (this->getMode()) {
  case MleFlyViewerP::WAITING_FOR_UP_PICK:
    if ((me->getButton() == SoMouseButtonEvent::BUTTON1) &&
	(me->getState() == SoButtonEvent::DOWN)) {
      PUBLIC(this)->findUpDirection(me->getPosition());
      this->setMode(MleFlyViewerP::FLYING);
      return TRUE;
    }
    break;
  case MleFlyViewerP::FLYING:
    switch (me->getButton()) {
    case SoMouseButtonEvent::BUTTON1:

      switch (me->getState()) {
        
      case SoButtonEvent::DOWN:
        // Incrementing speed.
	this->button1down = TRUE;
	if (this->button3down) {
          this->stopMoving();
	} 
        else {
          this->incrementMaxSpeed();
	}
	this->updateSpeedIndicator();
	PUBLIC(this)->scheduleRedraw();
	return TRUE;
      case SoButtonEvent::UP:
	this->button1down = FALSE;
	return TRUE;
      default:
	break;
      }
      break;

    case SoMouseButtonEvent::BUTTON3:

      switch (me->getState()) {
      case SoButtonEvent::DOWN:
	this->button3down = TRUE;
	
        if (this->button1down) {
          this->stopMoving();
	} 
        else
          this->decrementMaxSpeed();
     
	this->updateSpeedIndicator();
	PUBLIC(this)->scheduleRedraw();
	return TRUE;
      case SoButtonEvent::UP:
	this->button3down = FALSE;
	return TRUE;
      default:
	break;
      }
      break;
    default:
      break;
    }
  default:
    break;
  }
  return FALSE;
}

SbBool
MleFlyViewerP::processLocation2Event( const SoLocation2Event * const lev )
{  
  this->mouseloc = lev->getPosition();

  if (this->getMode() == MleFlyViewerP::TILTING) {

    float pan = (this->lastpos[0] - this->mouseloc[0])/100.0f;
    float tilt = (this->lastpos[1] - this->mouseloc[1])/100.0f;
    
    SoCamera * camera = PUBLIC(this)->getCamera();
    if (camera == NULL) 
      return TRUE; // probably sceneless
    
    this->updateCameraOrientation( camera, tilt, pan, 1.0f );
    this->lastpos = this->mouseloc;
  }

  // FIXME: The size of the glcanvas only changes when the viewer is
  // resized. The GLSize should be set from the FlyViewer, to remove
  // the dependency on the PUBLIC(this) class. 20021021 rolvs
  SbVec2s glsize( PUBLIC(this)->getGLSize() );

  // NOTE: The values are normalized, so that the FlyViewer behaves
  // the same way no matter the screen-size. The old way to do it made
  // the possible range of pan and tilt increment depend on the canvas
  // size. 20021022 rolvs.
  this->pan_increment = 0.5f - float(this->mouseloc[0])/glsize[0]; 
  this->tilt_increment = 0.5f - float(this->mouseloc[1])/glsize[1];

  return TRUE;
}

void
MleFlyViewerP::superimpositionevent(SoAction * action)
{
  if (!action->isOfType(SoGLRenderAction::getClassTypeId())) return;
  SbViewportRegion vpRegion =
    ((SoGLRenderAction *) action)->getViewportRegion();
  SbVec2s viewport = vpRegion.getViewportSizePixels();
  float aspect = float(viewport[0]) / float(viewport[1]);
  float factorx = 1.0f/float(viewport[1]) * 220.0f;
  float factory = factorx;
  if (aspect > 1.0f) {
    this->stranslation->translation.setValue(SbVec3f(0.0f, -0.4f, 0.0f));
  } else {
    this->stranslation->translation.setValue(SbVec3f(0.0f, -0.4f / aspect, 0.0f));
    factorx /= aspect;
    factory /= aspect;
  }
  if (viewport[0] > 500)
    factorx *= 500.0f / 400.0f;
  else
    factorx *= float(viewport[0]) / 400.0f;
  this->sscale->scaleFactor.setValue(SbVec3f(factorx, factory, 1.0f));

  if (this->getMode() == TILTING) {
    assert(this->crossposition != NULL);
    assert(this->crossscale != NULL);
    float tx = float(this->tiltpos[0]-float(viewport[0])/2.0f)/(float(viewport[0]));
    float ty = float(this->tiltpos[1]-float(viewport[1])/2.0f)/(float(viewport[1]));
    if (aspect > 1.0f) tx *= aspect;
    else ty /= aspect;
    this->crossposition->translation.setValue(SbVec3f(tx, ty, 0));

    float sx = (1.0f/float(viewport[0])) * 15.0f;
    float sy = (1.0f/float(viewport[1])) * 15.0f;
    if (aspect > 1.0f) sx *= aspect;
    else sy /= aspect;
    this->crossscale->scaleFactor.setValue(SbVec3f(sx, sy, 0));
  }
}

void
MleFlyViewerP::superimposition_cb(void * closure, SoAction * action)
{
  assert(closure != NULL);
  ((MleFlyViewerP *) closure)->superimpositionevent(action);
}

void
MleFlyViewerP::updateSpeedIndicator(void)
{
  assert(this->sgeometry != NULL);

  SbVec3f * points = this->sgeometry->point.startEditing();

  if (points[10][0] == 0.0f)
    this->smaxspeedswitch->whichChild.setValue(SO_SWITCH_ALL);
  if (points[14][0] == 0.0f)
    this->scurrentspeedswitch->whichChild.setValue(SO_SWITCH_ALL);
  points[10][0] = this->maxspeed / (SOWIN_MAX_SPEED / 0.8f);
  points[11][0] = this->maxspeed / (SOWIN_MAX_SPEED / 0.8f);
  points[14][0] = this->currentspeed / (SOWIN_MAX_SPEED / 0.8f);
  points[15][0] = this->currentspeed / (SOWIN_MAX_SPEED / 0.8f);
  this->sgeometry->point.finishEditing();

  if (this->maxspeed == 0.0f)
    this->smaxspeedswitch->whichChild.setValue(SO_SWITCH_NONE);
  if (this->currentspeed == 0.0f)
    this->scurrentspeedswitch->whichChild.setValue(SO_SWITCH_NONE);
}

float MleFlyViewerP::calculateChangeInTime()
{
  SbTime thisrender; 
  thisrender.setToTimeOfDay();

  float t = 0.0f;

  if (this->currentspeed == 0.0f)
    this->lastrender->setValue(thisrender.getValue() - 0.01);

  t = float(thisrender.getValue() - this->lastrender->getValue()) * 10.0f;

  if (t >= 1.0f) 
    t = 1.0f;
  return t; 
}

void MleFlyViewerP::updateCurrentSpeed( float dt )
{
  float curveSpeedReductionFactor = 
    1.0f - (this->pan_increment * this->pan_increment 
            + this->tilt_increment * this->tilt_increment);  

  // NOTE: I don't believe that this boundary condition could ever
  // happen. 20021022 rolvs
  if( curveSpeedReductionFactor < 0 )
    curveSpeedReductionFactor = 0;

  this->currentspeed +=
    (((this->currentspeed +
      this->maxspeed * curveSpeedReductionFactor) / 2.0f) - 
     this->currentspeed) * dt;
}

void MleFlyViewerP::updateCameraPosition( SoCamera * camera, 
                                              float current_speed,
                                              float dt )
{
  assert( camera != NULL );

  SbRotation orientation = camera->orientation.getValue();
  SbVec3f dir;
  camera->orientation.getValue().multVec( SbVec3f(0,0,-1), dir );
  dir.normalize();
  camera->position.setValue(camera->position.getValue() +
                            dir * (current_speed * dt ));
}
 
void MleFlyViewerP::updateCameraOrientation( SoCamera * camera, 
                                                 float d_tilt, 
                                                 float d_pan, 
                                                 float dt )
{
  assert( camera != NULL );
  // FIXME: Make sure that the angle between direction and up-vector
  // stays larger than zero, or else it gets 'locked' in an undefined
  // state and starts to act weird. This should probably be done in
  // parent class. 20021017 rolvs
  PUBLIC( this )->tiltCamera( d_tilt*dt );

  camera->orientation = camera->orientation.getValue() *
    SbRotation( PUBLIC(this)->getUpDirection(), d_pan*dt );
}

void
MleFlyViewerP::incrementMaxSpeed()
{
  this->max_speed_factor++;
  updateMaxSpeed();
}


void
MleFlyViewerP::decrementMaxSpeed()
{	
  this->max_speed_factor--;
  updateMaxSpeed();
}


void MleFlyViewerP::updateSpeedScalingFactor()
{
  SoNode *n = PUBLIC(this)->getSceneGraph();
  if( n == NULL )
    return; // Scenegraph not set yet?

  SoGetBoundingBoxAction bbact( PUBLIC(this)->getViewportRegion() );
  bbact.apply( n );  
  
  SbBox3f bbox = bbact.getBoundingBox();
  float bbox_diagonal = (bbox.getMax() - bbox.getMin()).length();

  // FIXME: It should be possible to create a simple scaling function,
  // based on some logaritmic evaluation. 20021017 rolvs.
  if( bbox_diagonal>100 )
    this->speed_scaling_factor = 1.0f; // log(bbox_diagonal);
  else if( bbox_diagonal>10 && bbox_diagonal < 100 )
    this->speed_scaling_factor = 0.4f; 
  else if( bbox_diagonal>1 && bbox_diagonal<10 )
    this->speed_scaling_factor = 0.3f; 
  else if( bbox_diagonal>0.1 && bbox_diagonal<1)
    this->speed_scaling_factor = 0.1f; 
  else
    this->speed_scaling_factor = 0.1f*bbox_diagonal;
}

void MleFlyViewerP::stopMoving()
{
  maxspeed = 0;
  currentspeed = 0; 
  max_speed_factor = 0;
}

void MleFlyViewerP::updateMaxSpeed()
{
  if( this->max_speed_factor == 0 ){
    this->stopMoving();
    return;
  }

  // FIXME: Move this methodcall so that it is called only
  // once. (e.g. when scene graph is set) 20021021 rolvs
  this->updateSpeedScalingFactor();

  this->maxspeed = 
    this->max_speed_factor 
    * float(pow( SOWIN_INC_FACTOR, abs( this->max_speed_factor ) ))
    * this->speed_scaling_factor;

  if( this->maxspeed > SOWIN_MAX_SPEED )
    this->maxspeed = SOWIN_MAX_SPEED;
  else if( this->maxspeed < -1*SOWIN_MAX_SPEED )
    this->maxspeed = -1*SOWIN_MAX_SPEED;
}



// Set cursor graphics according to mode.
void
MleFlyViewerP::updateCursorRepresentation(void)
{
  if (!PUBLIC(this)->isCursorEnabled()) {
    PUBLIC(this)->setComponentCursor(SoWinCursor::getBlankCursor());
    return;
  }

  switch (this->viewermode) {
  case MleFlyViewerP::FLYING:
    PUBLIC(this)->setComponentCursor(SoWinCursor(SoWinCursor::DEFAULT));
    break;

  case MleFlyViewerP::WAITING_FOR_SEEK:
    PUBLIC(this)->setComponentCursor(SoWinCursor(SoWinCursor::CROSSHAIR));
    break;

  case MleFlyViewerP::WAITING_FOR_UP_PICK:
    PUBLIC(this)->setComponentCursor(SoWinCursor(SoWinCursor::UPARROW));
    break;

  case MleFlyViewerP::TILTING:
    PUBLIC(this)->setComponentCursor(SoWinCursor::getPanCursor());
    break;

  default:
    assert(0 && "unknown mode");
    break;
  }
}

#endif // DOXYGEN_SKIP_THIS

// ************************************************************************

SOWIN_OBJECT_SOURCE(MleFlyViewer);

// ************************************************************************

/*!
  Public constructor.
*/
MleFlyViewer::MleFlyViewer(HWND parent,
                                   const char * name, 
                                   SbBool embed, 
                                   MleFullViewer::BuildFlag flag,
                                   SoWinViewer::Type type)
  : inherited(parent, name, embed, flag, type, FALSE)
{
  PRIVATE(this) = new MleFlyViewerP(this);
  PRIVATE(this)->constructor(TRUE);
}

// ************************************************************************

/*!
  Protected constructor, used by viewer components derived from the
  MleFlyViewer.
*/
MleFlyViewer::MleFlyViewer(HWND parent,
                                   const char * const name,
                                   SbBool embed, 
                                   MleFullViewer::BuildFlag flag, 
                                   SoWinViewer::Type type, 
                                   SbBool build)
  : inherited(parent, name, embed, flag, type, FALSE)
{
  PRIVATE(this) = new MleFlyViewerP(this);
  PRIVATE(this)->constructor(build);
}

// ************************************************************************

/*!
  Virtual constructor.
*/
MleFlyViewer::~MleFlyViewer()
{
  if (PRIVATE(this)->superimposition != NULL) {
    this->removeSuperimposition(PRIVATE(this)->superimposition);
    PRIVATE(this)->superimposition->unref();
    PRIVATE(this)->superimposition = NULL;
  }
  delete PRIVATE(this);
}

// ************************************************************************

// doc in super
void
MleFlyViewer::setViewing(SbBool enable)
{
  if (enable != this->isViewing())
    PRIVATE(this)->stopMoving();

  inherited::setViewing(enable);
  this->setSuperimpositionEnabled(PRIVATE(this)->superimposition, enable);
  this->scheduleRedraw();
}

// ************************************************************************

// doc in super
void
MleFlyViewer::resetToHomePosition(void)
{
  PRIVATE(this)->stopMoving();
  inherited::resetToHomePosition();
}

// ************************************************************************

// doc in super
void
MleFlyViewer::viewAll(void)
{
  PRIVATE(this)->stopMoving();
  inherited::viewAll();
}

// ************************************************************************

// doc in super
void
MleFlyViewer::setCamera(SoCamera * camera)
{
  PRIVATE(this)->stopMoving();

  inherited::setCamera(camera);
  // FIXME: do something with up-direction? 
}

// ************************************************************************

// doc in super
void
MleFlyViewer::setCursorEnabled(SbBool enable)
{
  inherited::setCursorEnabled(enable);
  PRIVATE(this)->updateCursorRepresentation();
}

// ************************************************************************

// doc in super
void
MleFlyViewer::setCameraType(SoType type)
{
  PRIVATE(this)->stopMoving();
  inherited::setCameraType(type);
  // FIXME: what else? 20010907 mortene.
}

// ************************************************************************

// doc in super
const char *
MleFlyViewer::getDefaultWidgetName(void) const
{
  static const char defaultWidgetName[] = "MleFlyViewer";
  return defaultWidgetName;
}

// ************************************************************************

// doc in super
const char *
MleFlyViewer::getDefaultTitle(void) const
{
  static const char defaultTitle[] = "Fly Viewer";
  return defaultTitle;
}

// ************************************************************************

// doc in super
const char *
MleFlyViewer::getDefaultIconTitle(void) const
{
  static const char defaultIconTitle[] = "Fly Viewer";
  return defaultIconTitle;
}

// ************************************************************************

// Documented in superclass.
SbBool
MleFlyViewer::processSoEvent(const SoEvent * const event)
{
  // FIXME: Refactor the event-handling so that it uses the same
  // strategy as in MleExaminerViewer, where the event-handler
  // only checks the state and the mode from that. 20021016 rolvs.

  // Let the end-user toggle between camera-interaction mode
  // ("viewing") and scenegraph-interaction mode with ALT key(s).
  // FIXME: Allow this to be handled by processKeyboardEvent? 
  // 20021016 rolvs.
  if (event->getTypeId().isDerivedFrom(SoKeyboardEvent::getClassTypeId())) {
    SoKeyboardEvent * ke = (SoKeyboardEvent *)event;
    switch (ke->getKey()) {
    case SoKeyboardEvent::LEFT_ALT:
    case SoKeyboardEvent::RIGHT_ALT:
      if (this->isViewing() && (ke->getState() == SoButtonEvent::DOWN)) {
        this->setViewing(FALSE);
        return TRUE;
      }
      else if (!this->isViewing() && (ke->getState() == SoButtonEvent::UP)) {
        this->setViewing(TRUE);
        return TRUE;
      }
    default:
      break;
    }
  }

  // We're in "interact" mode (ie *not* the camera modification mode),
  // so don't handle the event here. It should either be forwarded to
  // the scenegraph, or caught by SoWinViewer::processSoEvent() if
  // it's an ESC press (to switch modes).
  if (!this->isViewing()) { return inherited::processSoEvent(event); }

  // Events when in "ready-to-seek" mode are ignored, except those
  // which influence the seek mode itself -- these are handled further
  // up the inheritance hierarchy.
  if (this->isSeekMode()) { return inherited::processSoEvent(event); }
  
  // FIXME: There is more parts of the code in
  // Mle*FlyViewer::processEvent that should go in to the
  // processKeyboardEvent function; to be fixed later. 
  // 20021015 rolvs

  // Keyboard handling
  if (event->isOfType(SoKeyboardEvent::getClassTypeId())) {
    SbBool result =
      PRIVATE(this)->processKeyboardEvent( (SoKeyboardEvent*)event );

    if( result ){
      return TRUE;
    }
    // Else: Do nothing, and proceed as usual
  }

  // Mousebutton handling
  // See FIXME and comment for keyboardhandler.
  else if (event->isOfType(SoMouseButtonEvent::getClassTypeId())) {
    // FIXME: only for fly mode
    const SoMouseButtonEvent * const me = 
      (const SoMouseButtonEvent *const) event;
    SbBool result = PRIVATE( this )->processMouseButtonEvent( me );
    if( result ) 
      return TRUE;
  }

  else if (event->isOfType(SoLocation2Event::getClassTypeId())) {
    const SoLocation2Event * const le = 
      (const SoLocation2Event * const) event;
    SbBool result = PRIVATE( this )->processLocation2Event( le );
    if( result ) 
      return TRUE;
  }

  return inherited::processSoEvent(event);
}

// ************************************************************************

// doc in super
void
MleFlyViewer::setSeekMode(SbBool enable)
{
  // Note: this method is almost identical to the setSeekMode() in the
  // MleExaminerViewer, so migrate any changes.

#if SOWIN_DEBUG
  if (enable == this->isSeekMode()) {
    SoDebugError::postWarning("MleFlyViewer::setSeekMode",
                              "seek mode already %sset", enable ? "" : "un");
    return;
  }
#endif // SOWIN_DEBUG

  // FIXME: what if we're in the middle of a seek already? 20010910 mortene.
  // larsa - either stop the seek (on false) or reset timer to two new secs

  inherited::setSeekMode(enable);
  PRIVATE(this)->setMode(enable ? MleFlyViewerP::WAITING_FOR_SEEK :
                            MleFlyViewerP::FLYING);
}

// ************************************************************************

// doc in super
void
MleFlyViewer::actualRedraw(void)
{
  if (!this->isViewing()) {
    inherited::actualRedraw();
    return;
  }

  switch (PRIVATE(this)->getMode()) {
  case MleFlyViewerP::FLYING:
    {
      float dt = PRIVATE(this)->calculateChangeInTime();
      PRIVATE(this)->updateCurrentSpeed( dt );

      PRIVATE(this)->updateSpeedIndicator();

      SbTime thisrender; 
      thisrender.setToTimeOfDay();

      if (PRIVATE(this)->currentspeed != 0.0f) {
        float t = float(thisrender.getValue() -
                        PRIVATE(this)->lastrender->getValue()) * 2.0f;
        if (t > 0.0f) {
          SoCamera * camera = this->getCamera();

          if (camera){ // could be a sceneless viewer
            PRIVATE(this)->updateCameraPosition
              ( camera,
                PRIVATE(this)->currentspeed*
                PRIVATE(this)->speed_scaling_factor,
                t );
            PRIVATE(this)->updateCameraOrientation
              ( camera, 
                PRIVATE(this)->tilt_increment, 
                PRIVATE(this)->pan_increment, 
                t );
          }
        }
      }
      
      inherited::actualRedraw();
      
      PRIVATE(this)->lastrender->setValue(thisrender.getValue());
      
      if (PRIVATE(this)->currentspeed != 0.0f ||
          PRIVATE(this)->maxspeed != 0.0f)
        this->scheduleRedraw();
    }
    break;
  default:
    inherited::actualRedraw();
    break;
  }
}

// ************************************************************************

// doc in super
void
MleFlyViewer::rightWheelMotion(float value)
{
  PRIVATE(this)->dolly(value - this->getRightWheelValue());
  inherited::rightWheelMotion(value);
}

// ************************************************************************

// doc in super
void
MleFlyViewer::afterRealizeHook(void)
{
  PRIVATE(this)->updateCursorRepresentation();
  inherited::afterRealizeHook();
}

// ************************************************************************

#undef PRIVATE
#undef PUBLIC

