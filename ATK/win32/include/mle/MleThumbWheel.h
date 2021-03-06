#ifndef MLE_THUMBWHEEL_H
#define SMLE_THUMBWHEEL_H

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

#include <windows.h>

#include <Inventor/SbBasic.h>
#include <Inventor/Win/SoWinBasic.h>

class MleAnyThumbWheel;
class SbDict;

// *************************************************************************

class MleThumbWheel {

public:
  enum Interaction { START, MOVE, END };
  typedef void ThumbWheelCB(Interaction type, float val, void * userdata);

  enum Orientation {
    Horizontal,
    Vertical
  };
  enum boundaryHandling {
    CLAMP,
    MODULATE,
    ACCUMULATE
  };
  enum State {
    Idle,
    Dragging,
    Disabled
  };

  MleThumbWheel(HWND parent = 0, long id = -1, int x = 0, int y = 0, const char * name = 0);
  MleThumbWheel(Orientation, HWND parent = 0, long id = -1, int x = 0, int y = 0, const char * name = 0);
  ~MleThumbWheel(void);

  void setId(long id);
  long id(void) const;
  
  void setOrientation(Orientation);
  Orientation orientation(void) const;

  void setValue(float value);
  float value(void) const;

  void setEnabled(SbBool enable);
  SbBool isEnabled(void) const;

  void setLabelText(const char * text);
  void setLabelOffset(int x, int y);
  SIZE getLabelSize(void);

  void setRangeBoundaryHandling(boundaryHandling handling);
  boundaryHandling getRangeBoundaryHandling(void) const;

  HWND getWidget(void);
  SIZE sizeHint(void) const;
  int width(void);
  int height(void);
  void move(int x, int y);
  void size(int width, int height);
  void move(int x, int y, int width, int height);
  void setCallback(ThumbWheelCB * func, void * userdata);

  void show(void);
  void hide(void);

  static MleThumbWheel * getWheelFromHWND(HWND h);

protected:

  LRESULT CALLBACK onCreate(HWND window,
                            UINT message,
                            WPARAM wparam,
                            LPARAM lparam);

  LRESULT CALLBACK onSize(HWND window,
                          UINT message,
                          WPARAM wparam,
                          LPARAM lparam);

  LRESULT CALLBACK onPaint(HWND window,
                           UINT message,
                           WPARAM wparam,
                           LPARAM lparam);

  LRESULT CALLBACK onLButtonDown(HWND window,
                                 UINT message,
                                 WPARAM wparam,
                                 LPARAM lparam);

  LRESULT CALLBACK onLButtonUp(HWND window,
                               UINT message,
                               WPARAM wparam,
                               LPARAM lparam);

  LRESULT CALLBACK onMouseMove(HWND window,
                               UINT message,
                               WPARAM wparam,
                               LPARAM lparam);

  LRESULT CALLBACK onDestroy(HWND window,
                             UINT message,
                             WPARAM wparam,
                             LPARAM lparam);

  static LRESULT CALLBACK windowProc(HWND window,
                                     UINT message,
                                     WPARAM wparam,
                                     LPARAM lparam);

private:
  void constructor(Orientation, HWND parent, long id, int x, int y,
                   const char * name);

  HWND buildWidget(HWND parent, RECT rect, const char * name);
  void initWheel(int diameter, int width);
  HWND createLabel(HWND parent, int x, int y, const char * text);
  HBITMAP createDIB(int width, int height, int bpp, void ** bits);
  void blitBitmap(HBITMAP bitmap, HDC dc, int x,int y, int width, int height) const;
  SIZE getTextSize(HWND window, const char * text);

  Orientation orient;
  State state;
  float wheelValue, tempWheelValue;
  int mouseDownPos, mouseLastPos;

  MleAnyThumbWheel * wheel;
  HBITMAP * pixmaps;
  int numPixmaps;
  int currentPixmap;

  HWND wheelWindow;
  HWND labelWindow;

  POINT labelOffset;

  ThumbWheelCB * viewerCB;
  void * userdataCB;

  static ATOM wheelWndClassAtom;
  static int wheelWidgetCounter;
  static SbDict * hwnddict;
};

// *************************************************************************

#endif // ! MLE_THUMBWHEEL_H
