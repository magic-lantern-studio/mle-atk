#ifndef MLEGUIPLANEVIEWERP_H
#define MLeGUIPLANEVIEWERP_H

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

#ifndef SOWIN_INTERNAL
#error this is a private header file
#endif /* !SOWIN_INTERNAL */

#include <Inventor/SbLinear.h>

class MlePlaneViewer;

// ************************************************************************

// This class contains private data and methods used within the
// MleGuiPlaneViewer class.

class MleGuiPlaneViewerP
{
public:
  ~MleGuiPlaneViewerP();

  void commonConstructor(void);

  void pan(const SbVec2f & thispos, const SbVec2f & prevpos);
  void rotateZ(const float angle) const;

  void viewPlaneX(void) const;
  void viewPlaneY(void) const;
  void viewPlaneZ(void) const;

  void setCanvasSize(const SbVec2s size);
  void setPointerLocation(const SbVec2s location);
  int getPointerXMotion(void) const;
  int getPointerYMotion(void) const;
  float getPointerOrigoAngle(void) const;
  float getPointerOrigoMotionAngle(void) const;

  void updateAnchorScenegraph(void) const;

  enum PlaneViewerMode {
    SCENEGRAPH_INTERACT_MODE,

    IDLE_MODE,

    DOLLY_MODE,
    TRANSLATE_MODE,

    ROTZ_WAIT_MODE,
    ROTZ_MODE,

    SEEK_WAIT_MODE,
    SEEK_MODE
  } mode;

  void changeMode(PlaneViewerMode newmode);
  void setCursorRepresentation(PlaneViewerMode mode);

  struct pointerdata {
    SbVec2s now;
    SbVec2s then;
  } pointer;
  SbVec2s canvas;

  SbBool leftcontroldown;
  SbBool rightcontroldown;
  SbBool leftshiftdown;
  SbBool rightshiftdown;
  SbBool button1down;
  SbBool button3down;

  SbPlane panningplane;

  class SoNode * superimposition;
  struct superdata {
    class SoCoordinate3 * coords;
    class SoOrthographicCamera * camera;
  } super;

protected:
  MleGuiPlaneViewerP(MlePlaneViewer * publ);
  MlePlaneViewer * pub;
};

// ************************************************************************

#endif // ! MLEGUIPLANEVIEWERP_H
