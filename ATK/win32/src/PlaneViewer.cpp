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

// *************************************************************************

// Class is documented in common/viewers/MleGuiPlaneViewer.cpp.in.

// *************************************************************************

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>

#include <sowindefs.h>
#include <Inventor/Win/widgets/SoWinThumbWheel.h>
#include <mle/MleBitmapButton.h>

#include <mle/MlePlaneViewer.h>
#include <mle/MlePlaneViewerP.h>

#include <Inventor/Win/common/pixmaps/ortho.xpm>
#include <Inventor/Win/common/pixmaps/perspective.xpm>
#include <Inventor/Win/common/pixmaps/x.xpm>
#include <Inventor/Win/common/pixmaps/y.xpm>
#include <Inventor/Win/common/pixmaps/z.xpm>

// ************************************************************************

// SoQtPlaneViewerP "private implementation" class.

#ifndef DOXYGEN_SKIP_THIS

#define PUBLIC(ptr) (ptr->pub)
#define PRIVATE(ptr) (ptr->pimpl)

MlePlaneViewerP::MlePlaneViewerP(MlePlaneViewer * publ)
  : MleGuiPlaneViewerP(publ)
{
  this->camerabutton = NULL;
}

MlePlaneViewerP::~MlePlaneViewerP()
{
}

// This contains the real constructor code (the two constructors are only
// entry points for this method).
void
MlePlaneViewerP::constructor(SbBool build)
{
  this->commonConstructor(); // init generic stuff

  PUBLIC(this)->setClassName("MlePlaneViewer");
  
  if (! build) return;

  HWND viewer = PUBLIC(this)->buildWidget(PUBLIC(this)->getParentWidget());
  PUBLIC(this)->setBaseWidget(viewer);
  
  PUBLIC(this)->setLeftWheelString("TransY");
  PUBLIC(this)->setBottomWheelString("TransX");  

  PUBLIC(this)->setSize(SbVec2s(555, 515));
}

void
MlePlaneViewerP::xButtonProc(MleBitmapButton * b, void * userdata)
{
  MlePlaneViewerP * that = (MlePlaneViewerP *)userdata;
  that->viewPlaneX();
}

void
MlePlaneViewerP::yButtonProc(MleBitmapButton * b, void * userdata)
{
  MlePlaneViewerP * that = (MlePlaneViewerP *)userdata;
  that->viewPlaneY();
}

void
MlePlaneViewerP::zButtonProc(MleBitmapButton * b, void * userdata)
{
  MlePlaneViewerP * that = (MlePlaneViewerP *)userdata;
  that->viewPlaneZ();
}

void
MlePlaneViewerP::cameraButtonProc(MleBitmapButton * b, void * userdata)
{
  MlePlaneViewerP * that = (MlePlaneViewerP *)userdata;
  if (PUBLIC(that)->getCamera()) { PUBLIC(that)->toggleCameraType(); }
}

#endif // DOXYGEN_SKIP_THIS

// ************************************************************************

// Documented in superclass.
void
MlePlaneViewer::setCamera(SoCamera * camera)
{
  if (camera) {
    SoType type = camera->getTypeId();
    SbBool orthogonal =
      type.isDerivedFrom(SoOrthographicCamera::getClassTypeId());
    this->setRightWheelString(orthogonal ? "Zoom" : "Dolly");
    
    MleBitmapButton * b = (MleBitmapButton *)PRIVATE(this)->camerabutton;
    if (b) { b->setBitmap(orthogonal ? 1 : 0); }
  }
  inherited::setCamera(camera);
}

// ************************************************************************

// Documented in superclass.
HWND
MlePlaneViewer::buildWidget(HWND parent)
{
  return inherited::buildWidget(parent);
}

// ************************************************************************

// Documented in superclass.
void
MlePlaneViewer::createViewerButtons(HWND parent, SbPList * buttonlist)
{
  inherited::createViewerButtons(parent, buttonlist);

  MleBitmapButton * b = new MleBitmapButton(parent, 24, "x", NULL);
  b->addBitmap(x_xpm);
  b->setBitmap(0);
  b->registerClickedProc(MlePlaneViewerP::xButtonProc, PRIVATE(this));
  buttonlist->append(b);
  
  b = new MleBitmapButton(parent, 24, "y", NULL);
  b->addBitmap(y_xpm);
  b->setBitmap(0);
  b->registerClickedProc(MlePlaneViewerP::yButtonProc, PRIVATE(this));
  buttonlist->append(b);

  b = new MleBitmapButton(parent, 24, "z", NULL);
  b->addBitmap(z_xpm);
  b->setBitmap(0);
  b->registerClickedProc(MlePlaneViewerP::zButtonProc, PRIVATE(this));
  buttonlist->append(b);
  
  b = new MleBitmapButton(parent, 24, "camera", NULL);
  b->addBitmap(perspective_xpm);
  b->addBitmap(ortho_xpm);
  b->setBitmap(0);
  b->registerClickedProc(MlePlaneViewerP::cameraButtonProc, PRIVATE(this));
  buttonlist->append(b);
  PRIVATE(this)->camerabutton = b;
}

// ************************************************************************
