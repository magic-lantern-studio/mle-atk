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

#include <math.h>
#include <assert.h>
#include <stdio.h>

#include <Inventor/SbDict.h>

#include <Inventor/Win/SoWin.h>
#include <mle/Win32API.h>
#include <mle/MleAnyThumbWheel.h>
#include <mle/MleThumbWheel.h>
#include <sowindefs.h>

// *************************************************************************

ATOM MleThumbWheel::wheelWndClassAtom = NULL;
int MleThumbWheel::wheelWidgetCounter = 0;
SbDict * MleThumbWheel::hwnddict = NULL;

MleThumbWheel::MleThumbWheel(HWND parent,
                                 long id,
                                 int x,
                                 int y,
                                 const char * name)
{
  this->constructor(MleThumbWheel::Vertical, parent, id, x, y, name);
}

MleThumbWheel::MleThumbWheel(Orientation orientation,
                                 HWND parent,
                                 long id,
                                 int x,
                                 int y,
                                 const char * name)
{
  this->constructor(orientation, parent, id, x, y, name);
}

void
MleThumbWheel::constructor(Orientation orientation,
                             HWND parent, long id, int x, int y,
                             const char * name)
{
  MleThumbWheel::wheelWidgetCounter++;
  this->orient = orientation;
  this->state = MleThumbWheel::Idle;
  this->wheelValue = this->tempWheelValue = 0.0f;
  this->wheel = new MleAnyThumbWheel;
  this->wheel->setMovement(MleAnyThumbWheel::UNIFORM);
  this->wheel->setGraphicsByteOrder(MleAnyThumbWheel::ARGB);
  this->pixmaps = NULL;
  this->numPixmaps = 0;
  this->currentPixmap = -1;
  this->viewerCB = NULL;
  this->labelWindow = NULL;

  RECT rect = { x, y, this->sizeHint().cx, this->sizeHint().cy };
  this->buildWidget(parent, rect, name);
  this->setId(id);

  if (MleThumbWheel::hwnddict == NULL) {
    MleThumbWheel::hwnddict = new SbDict;
  }

  const unsigned long key = (unsigned long)this->getWidget();
  SbBool isnewentry = MleThumbWheel::hwnddict->enter(key, this);
  assert(isnewentry);
}

MleThumbWheel::~MleThumbWheel(void)
{
  delete this->wheel;

  const unsigned long key = (unsigned long)this->getWidget();
  SbBool found = MleThumbWheel::hwnddict->remove(key);
  assert(found);

  if (this->pixmaps) {
    for (int i = 0; i < this->numPixmaps; i++)
      Win32::DeleteObject(this->pixmaps[i]);
    delete [] this->pixmaps;
  }
  if (IsWindow(this->wheelWindow))
    Win32::DestroyWindow(this->wheelWindow);
  if (IsWindow(this->labelWindow))
    Win32::DestroyWindow(this->labelWindow);

  MleThumbWheel::wheelWidgetCounter--;
  assert(MleThumbWheel::wheelWidgetCounter >= 0);
  if (MleThumbWheel::wheelWidgetCounter == 0) {
    if (MleThumbWheel::wheelWndClassAtom)
      Win32::UnregisterClass("ThumbWheel Widget", NULL);
    MleThumbWheel::wheelWndClassAtom = NULL;
    delete MleThumbWheel::hwnddict;
    MleThumbWheel::hwnddict = NULL;
  }
}

MleThumbWheel *
MleThumbWheel::getWheelFromHWND(HWND h)
{
  if (MleThumbWheel::hwnddict == NULL) { return NULL; }

  void * w;
  SbBool found = MleThumbWheel::hwnddict->find((unsigned long)h, w);
  return found ? ((MleThumbWheel *)w) : NULL;
}

SIZE
MleThumbWheel::sizeHint(void) const
{
  const int length = 118;
  int thick = 14;
  SIZE size;

  if (this->orient == MleThumbWheel::Horizontal) {
    size.cx = length;
    size.cy = thick;
    return size;
  }
  else {
    size.cx = thick;
    size.cy = length;
    return size;
  }
}

HWND
MleThumbWheel::getWidget(void)
{
  return this->wheelWindow;
}

void
MleThumbWheel::setId(long id)
{
  (void)Win32::SetWindowLong(this->wheelWindow, GWL_ID, id);
}

long
MleThumbWheel::id(void) const
{
  return Win32::GetWindowLong(this->wheelWindow, GWL_ID);
}

void
MleThumbWheel::setOrientation(Orientation orientation)
{
  this->orient = orientation;
}

MleThumbWheel::Orientation
MleThumbWheel::orientation(void) const
{
  return this->orient;
}

LRESULT CALLBACK
MleThumbWheel::onCreate(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  return 0;
}

LRESULT CALLBACK
MleThumbWheel::onSize(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  return 0;
}

LRESULT CALLBACK
MleThumbWheel::onPaint(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  PAINTSTRUCT ps;
  HDC hdc = Win32::BeginPaint(window, & ps);

  int w, d;
  if (this->orient == MleThumbWheel::Vertical) {
    w = this->width() - 2;
    d = this->height() - 2;
  } else {
    w = this->height() - 2;
    d = this->width() - 2;
  }

  // Handle resizing to too small dimensions gracefully.
  if ((d <= 0) || (w <= 0)) return 0;

  this->initWheel(d, w);

  int pixmap = this->wheel->getBitmapForValue(this->tempWheelValue,
                                              (this->state == MleThumbWheel::Disabled) ?
                                              MleAnyThumbWheel::DISABLED : MleAnyThumbWheel::ENABLED);

  this->blitBitmap(this->pixmaps[pixmap], hdc, 0, 0, this->width() - 2, this->height() - 2);

  this->currentPixmap = pixmap;

  Win32::EndPaint(window, & ps);
  return 0;
}

LRESULT CALLBACK
MleThumbWheel::onLButtonDown(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  if (this->state != MleThumbWheel::Idle)
    return 0;

  short x = LOWORD(lparam);
  short y = HIWORD(lparam);

  SetCapture(window);

  this->state = MleThumbWheel::Dragging;
  if (this->orient == MleThumbWheel::Vertical)
    this->mouseDownPos = y;
  else
    this->mouseDownPos = x;

  this->mouseLastPos = this->mouseDownPos;

  if (this->viewerCB) {
    this->viewerCB(START, this->wheelValue, this->userdataCB);
  }

  return 0;
}

LRESULT CALLBACK
MleThumbWheel::onMouseMove(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  if (this->state != MleThumbWheel::Dragging)
    return 0;

  short x = LOWORD(lparam);
  short y = HIWORD(lparam);

  if (this->orient == MleThumbWheel::Vertical)
    this->mouseLastPos = y;
  else
    this->mouseLastPos = x;

  this->tempWheelValue =
    this->wheel->calculateValue(this->wheelValue,
                                this->mouseDownPos,
                                this->mouseLastPos - this->mouseDownPos);

#if SOWIN_DEBUG && 0 // debug
  SoDebugError::postInfo("onMouseMove",
                         "delta==%d, wheelValue==%f, tmpWheelValue==%f",
                         this->mouseLastPos - this->mouseDownPos,
                         this->wheelValue, this->tempWheelValue);
#endif // debug

  Win32::InvalidateRect(this->wheelWindow, NULL, FALSE);

  if (this->viewerCB) {
    this->viewerCB(MOVE, this->tempWheelValue, this->userdataCB);
  }

  this->setValue(this->tempWheelValue);

  return 0;
}

LRESULT CALLBACK
MleThumbWheel::onLButtonUp(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  ReleaseCapture();
  if (this->state != MleThumbWheel::Dragging)
    return 0;

  this->wheelValue = this->tempWheelValue;
  this->mouseLastPos = this->mouseDownPos;
  this->state = MleThumbWheel::Idle;

  if (this->viewerCB) {
    this->viewerCB(END, this->wheelValue, this->userdataCB);
  }

  return 0;
}

LRESULT CALLBACK
MleThumbWheel::onDestroy(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  return 0;
}

LRESULT CALLBACK
MleThumbWheel::windowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  if (message == WM_CREATE) {
    CREATESTRUCT * createstruct;
    createstruct = (CREATESTRUCT *) lparam;

    (void)Win32::SetWindowLong(window, 0, (LONG) (createstruct->lpCreateParams));

    MleThumbWheel * object = (MleThumbWheel *)(createstruct->lpCreateParams);
    return object->onCreate(window, message, wparam, lparam);
  }

  MleThumbWheel * object = (MleThumbWheel *) Win32::GetWindowLong(window, 0);

  if (object && object->getWidget()) {

    switch (message)
      {
      case WM_SIZE:
        return object->onSize(window, message, wparam, lparam);

      case WM_PAINT:
        return object->onPaint(window, message, wparam, lparam);

      case WM_LBUTTONDOWN:
        return object->onLButtonDown(window, message, wparam, lparam);

      case WM_LBUTTONUP:
        return object->onLButtonUp(window, message, wparam, lparam);

      case WM_MOUSEMOVE:
        return object->onMouseMove(window, message, wparam, lparam);

      case WM_DESTROY:
        return object->onDestroy(window, message, wparam, lparam);
      }
  }
  return DefWindowProc(window, message, wparam, lparam);
}

int
MleThumbWheel::width(void)
{
  RECT rect;
  Win32::GetWindowRect(this->wheelWindow, & rect);
  return (rect.right - rect.left);

  //return this->sizeHint().cx;
}

int
MleThumbWheel::height(void)
{
  RECT rect;
  Win32::GetWindowRect(this->wheelWindow, & rect);
  return (rect.bottom - rect.top);

  //return this->sizeHint().cy;
}


void
MleThumbWheel::move(int x, int y, int width, int height)
{
  this->size(width, height);
  this->move(x, y);
}

void
MleThumbWheel::move(int x, int y)
{
  UINT flags = SWP_NOSIZE | SWP_NOZORDER;

  Win32::SetWindowPos(this->wheelWindow, NULL, x, y, 0, 0, flags);

  if (IsWindow(this->labelWindow)) {

    RECT rect;
    Win32::GetClientRect(this->labelWindow, & rect);

    if (this->orient == MleThumbWheel::Vertical) {
      Win32::SetWindowPos(this->labelWindow, NULL,
                          x + this->labelOffset.x,
                          y + this->labelOffset.y + this->height(),
                          0, 0, flags);
    }
    else {
      Win32::SetWindowPos(this->labelWindow, NULL,
                          x + this->labelOffset.x - rect.right,
                          y + this->labelOffset.y,
                          0, 0, flags);
    }
  }
}

void
MleThumbWheel::size(int width, int height)
{
  UINT flags = SWP_NOMOVE | SWP_NOZORDER;
  Win32::SetWindowPos(this->wheelWindow, NULL, 0, 0, width, height, flags);
  Win32::InvalidateRect(this->wheelWindow, NULL, FALSE);
  if (IsWindow(this->labelWindow))
    Win32::InvalidateRect(this->labelWindow, NULL, FALSE);
}

void
MleThumbWheel::show(void)
{
  (void)ShowWindow(this->wheelWindow, SW_SHOW);
  (void)ShowWindow(this->labelWindow, SW_SHOW);
}

void
MleThumbWheel::hide(void)
{
  (void)ShowWindow(this->wheelWindow, SW_HIDE);
  (void)ShowWindow(this->labelWindow, SW_HIDE);
}

void
MleThumbWheel::setCallback(ThumbWheelCB * func, void * userdata)
{
  this->viewerCB = func;
  this->userdataCB = userdata;
}

HWND
MleThumbWheel::buildWidget(HWND parent, RECT rect, const char * name)
{

  HMENU menu = NULL;
  LPSTR wndclassname = "ThumbWheel Widget";

  if (! MleThumbWheel::wheelWndClassAtom) {

    WNDCLASS windowclass;

    windowclass.lpszClassName = wndclassname;
    windowclass.hInstance = NULL;
    windowclass.lpfnWndProc = MleThumbWheel::windowProc;
    windowclass.style = CS_HREDRAW | CS_VREDRAW;
    windowclass.lpszMenuName = NULL;
    windowclass.hIcon = NULL;
    windowclass.hCursor = Win32::LoadCursor(NULL, IDC_ARROW);
    windowclass.hbrBackground = NULL;
    windowclass.cbClsExtra = 0;
    windowclass.cbWndExtra = 4;

    MleThumbWheel::wheelWndClassAtom = Win32::RegisterClass(& windowclass);

  }

  this->wheelWindow = Win32::CreateWindow_(wndclassname,
                                           wndclassname,
                                           WS_VISIBLE |
                                           WS_CLIPCHILDREN |
                                           WS_CLIPSIBLINGS |
                                           WS_CHILD |
                                           WS_BORDER,
                                           rect.left,
                                           rect.top,
                                           rect.right,
                                           rect.bottom,
                                           parent,
                                           menu,
                                           NULL,
                                           this);

  if (name) {
    this->labelWindow = this->createLabel(parent, rect.right, rect.bottom, name);
  }
  this->setLabelOffset(0, 0);

  return this->wheelWindow;
}

void
MleThumbWheel::initWheel(int diameter, int width)
{
  int d, w;
  this->wheel->getSize(d, w);
  if (d == diameter && w == width) return;

  this->wheel->setSize(diameter, width);

  int pwidth = width;
  int pheight = diameter;
  if (this->orient == Horizontal) {
    pwidth = diameter;
    pheight = width;
  }

  if (this->pixmaps != NULL) {
    for (int i = 0; i < this->numPixmaps; i++) {
      Win32::DeleteObject(this->pixmaps[i]);
      this->pixmaps[i] = NULL;
    }
    delete [] this->pixmaps;
  }

  this->numPixmaps = this->wheel->getNumBitmaps();
  void * bits = NULL;

  this->pixmaps = new HBITMAP[numPixmaps];

  for (int i = 0; i < this->numPixmaps; i++) {
    this->pixmaps[i] = this->createDIB(pwidth, pheight, 32, &bits);
    this->wheel->drawBitmap(i, bits, (this->orient == Vertical) ?
                            MleAnyThumbWheel::VERTICAL : MleAnyThumbWheel::HORIZONTAL);
  }
}

// *************************************************************************

void
MleThumbWheel::setEnabled(SbBool enable)
{
  if (enable)
    this->state = MleThumbWheel::Idle;
  else
    this->state = MleThumbWheel::Disabled;
  Win32::InvalidateRect(this->wheelWindow, NULL, FALSE);
  if (IsWindow(this->labelWindow))
    Win32::EnableWindow(this->labelWindow, enable);
}

SbBool
MleThumbWheel::isEnabled(void) const
{
  return (this->state != MleThumbWheel::Disabled);
}

void
MleThumbWheel::setValue(float value)
{
  this->wheelValue = this->tempWheelValue = value;
  this->mouseDownPos = this->mouseLastPos;
  Win32::InvalidateRect(this->wheelWindow, NULL, FALSE);
}

float
MleThumbWheel::value(void) const
{
  return this->wheelValue;
}

void
MleThumbWheel::setLabelText(const char * text)
{
  assert(IsWindow(this->wheelWindow));

  if (IsWindow(this->labelWindow)) {
    Win32::SetWindowText(this->labelWindow, text);
  }
  else {
    RECT rect;
    HWND parent = GetParent(this->wheelWindow);
    Win32::GetWindowRect(this->wheelWindow, & rect);
    this->labelWindow = this->createLabel(parent, rect.right + this->labelOffset.x,
                                          rect.bottom + labelOffset.y, text);
  }

  int len = strlen(text);
  HDC hdc = Win32::GetDC(this->labelWindow);
  SIZE textSize;
  Win32::GetTextExtentPoint(hdc, text, len, & textSize);

  UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW;
  Win32::SetWindowPos(this->labelWindow, NULL, 0, 0,
                      textSize.cx + 2, textSize.cy, flags);
}

void
MleThumbWheel::setLabelOffset(int x, int y)
{
  this->labelOffset.x = x;
  this->labelOffset.y = y;
}

SIZE
MleThumbWheel::getLabelSize(void)
{
  RECT rect;
  Win32::GetWindowRect(this->labelWindow, & rect);
  SIZE size = { rect.right - rect.left, rect.bottom - rect.top };
  return (size);
}

// *************************************************************************

void
MleThumbWheel::setRangeBoundaryHandling(boundaryHandling handling)
{
  switch (handling) {
  case CLAMP:
    this->wheel->setBoundaryHandling(MleAnyThumbWheel::CLAMP);
    break;
  case MODULATE:
    this->wheel->setBoundaryHandling(MleAnyThumbWheel::MODULATE);
    break;
  case ACCUMULATE:
    this->wheel->setBoundaryHandling(MleAnyThumbWheel::ACCUMULATE);
    break;
  default:
    assert(0 && "impossible");
  }
}

// *************************************************************************

MleThumbWheel::boundaryHandling
MleThumbWheel::getRangeBoundaryHandling(void) const
{
  switch (this->wheel->getBoundaryHandling()) {
  case MleAnyThumbWheel::CLAMP:
    return CLAMP;
  case MleAnyThumbWheel::MODULATE:
    return MODULATE;
  case MleAnyThumbWheel::ACCUMULATE:
    return ACCUMULATE;
  default:
    assert(0 && "impossible");
  }
  return CLAMP; // never reached
}

HWND
MleThumbWheel::createLabel(HWND parent, int x, int y, const char * text)
{
  assert(IsWindow(parent));
  // FIXME: assumes the same font as parent
  SIZE textSize = this->getTextSize(parent, text);
  HWND hwnd = Win32::CreateWindow_("STATIC",
                                   (text ? text : " "),
                                   WS_VISIBLE | WS_CHILD | SS_CENTER,
                                   x, y,
                                   textSize.cx + 2, textSize.cy, // SIZE
                                   parent,
                                   NULL,
                                   NULL,
                                   NULL);
  return hwnd;
}

HBITMAP
MleThumbWheel::createDIB(int width, int height, int bpp, void ** bits) // 16||24||32 bpp
{
  assert(bpp > 8);

  HBITMAP bitmap = NULL;
  HDC hdc = CreateCompatibleDC(NULL);
  assert(hdc!=NULL && "CreateCompatibleDC() failed -- investigate");
  int heapspace = sizeof(BITMAPINFOHEADER);

  HANDLE heap = GetProcessHeap();
  BITMAPINFO * format = (BITMAPINFO *) HeapAlloc(heap, 0, heapspace);

  BITMAPINFOHEADER * header = (BITMAPINFOHEADER *) format;
  header->biSize = sizeof(BITMAPINFOHEADER);
  header->biWidth = width;
  header->biHeight = -height;
  header->biPlanes = 1;
  header->biBitCount = bpp;
  header->biCompression = BI_RGB;
  header->biSizeImage = 0;
  header->biXPelsPerMeter = 0;
  header->biYPelsPerMeter = 0;
  header->biClrUsed = 0;
  header->biClrImportant = 0;

  UINT flag = DIB_RGB_COLORS;
  bitmap = CreateDIBSection(hdc, format, flag, (void **) bits, NULL, 0);
  assert(* bits);

  HeapFree(heap, 0, format);
  DeleteDC(hdc);

  return bitmap;
}

void
MleThumbWheel::blitBitmap(HBITMAP bitmap, HDC dc, int x,int y, int width, int height) const
{
  HDC memorydc = CreateCompatibleDC(dc);
  assert(memorydc!=NULL && "CreateCompatibleDC() failed -- investigate");
  HBITMAP oldBitmap = (HBITMAP) SelectObject(memorydc, bitmap);
  Win32::BitBlt(dc, x, y, width, height, memorydc, 0, 0, SRCCOPY);
  Win32::SelectObject(memorydc, oldBitmap);
  Win32::DeleteDC(memorydc);
}

SIZE
MleThumbWheel::getTextSize(HWND window, const char * text)
{
  assert(IsWindow(window));

  int len = strlen(text);
  HDC hdc = Win32::GetDC(window);

  SIZE size;
  Win32::GetTextExtentPoint(hdc, text, len, & size);
  return size;
}
