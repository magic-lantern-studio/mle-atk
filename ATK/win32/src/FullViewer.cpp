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

// ************************************************************************

// Class is documented in common/viewers/MleGuiFullViewer.cpp.in.

// *************************************************************************

#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/errors/SoDebugError.h>

#include <sowindefs.h>
#include <Inventor/Win/Win32API.h>
#include <Inventor/Win/SoWin.h>
#include <mle/MleBitmapButton.h>
#include <Inventor/Win/widgets/WinNativePopupMenu.h>

#include <mle/MleFullViewer.h>
#include <mle/MleFullViewerP.h>

#include <float.h> // FLT_MAX

// Button icons.
#include <Inventor/Win/common/pixmaps/pick.xpm>
#include <Inventor/Win/common/pixmaps/view.xpm>
#include <Inventor/Win/common/pixmaps/home.xpm>
#include <Inventor/Win/common/pixmaps/set_home.xpm>
#include <Inventor/Win/common/pixmaps/view_all.xpm>
#include <Inventor/Win/common/pixmaps/seek.xpm>

static const int DECORATION_SIZE = 30;
static const int DECORATION_BUFFER = 5;

#define PRIVATE(o) (o->pimpl)
#define PUBLIC(o) (o->pub)

// *************************************************************************

SOWIN_OBJECT_ABSTRACT_SOURCE(MleFullViewer);

// *************************************************************************

MleFullViewer::MleFullViewer(HWND parent,
                             const char * name,
                             SbBool embedded,
                             BuildFlag flag,
                             SoWinViewer::Type type,
                             SbBool build) :
  inherited(parent, name, embedded, type, FALSE)
{
  this->pimpl = new MleFullViewerP(this);
  MleFullViewerP::nrinstances++;

  if (MleFullViewerP::nrinstances == 1) {
    if (MleFullViewerP::parentHWNDmappings == NULL) {
      MleFullViewerP::parentHWNDmappings = new SbDict;
    }

    MleFullViewerP::hookhandle =
      Win32::SetWindowsHookEx(WH_CALLWNDPROC,
                              (HOOKPROC)MleFullViewerP::systemEventHook,
                              NULL, GetCurrentThreadId());
  }

  PRIVATE(this)->viewerwidget = NULL;
  PRIVATE(this)->renderareawidget = NULL;

  PRIVATE(this)->menuenabled = (flag & MleFullViewer::BUILD_POPUP) ? TRUE : FALSE;
  PRIVATE(this)->decorations = (flag & MleFullViewer::BUILD_DECORATION) ? TRUE : FALSE;

  this->prefmenu = NULL;

  PRIVATE(this)->leftthumbwheel = NULL;
  PRIVATE(this)->bottomthumbwheel = NULL;
  PRIVATE(this)->rightthumbwheel = NULL;
  this->leftWheel = NULL;
  this->bottomWheel = NULL;
  this->rightWheel = NULL;

  // Let these be bogus until we actually set up the wheels.
  this->leftWheelVal = FLT_MAX;
  this->bottomWheelVal = FLT_MAX;
  this->rightWheelVal = FLT_MAX;

  if (build) {
    this->setClassName("MleFullViewer");
    this->setBaseWidget(this->buildWidget(this->getParentWidget()));
  }

  /* FIXME: We should use the actual size of the parent window instead
     of hardcoding the size here, at least if the size of the parent
     window is reasonable. See also
     MleExaminerViewerP::constructor(). 2004-01-08 thammer.  
  */
  this->setSize(SbVec2s(500, 420));

  if (! this->isViewing())
    this->setViewing(TRUE);
}

MleFullViewer::~MleFullViewer()
{
  (void)MleFullViewerP::parentHWNDmappings->remove((unsigned long)this->getParentWidget());

  MleFullViewerP::nrinstances--;

  if (MleFullViewerP::nrinstances == 0) {
    assert(MleFullViewerP::hookhandle != NULL);
    Win32::UnhookWindowsHookEx(MleFullViewerP::hookhandle);

    // Parent HWND -> MleFullViewer dict.
    delete MleFullViewerP::parentHWNDmappings;
    MleFullViewerP::parentHWNDmappings = NULL;
  }
  
  delete this->prefmenu;

  delete PRIVATE(this);
}

void
MleFullViewer::setDecoration(SbBool enable)
{
#if SOWIN_DEBUG
  if ((enable && this->isDecoration()) ||
       (! enable && ! this->isDecoration())) {
    SoDebugError::postWarning("MleFullViewer::setDecoration",
                               "decorations already turned %s",
                              enable ? "on" : "off");
    return;
  }
#endif // SOWIN_DEBUG

  PRIVATE(this)->decorations = enable;
  PRIVATE(this)->showDecorationWidgets(enable);

  // reposition all widgets
  RECT rect;
  Win32::GetClientRect(this->getParentWidget(), & rect);
  PRIVATE(this)->layoutWidgets(rect.right, rect.bottom);
  if (enable) {
    rect.right -= DECORATION_SIZE * 2;
    rect.bottom -= DECORATION_SIZE;
  }
  SoWinRenderArea::sizeChanged(SbVec2s((short)rect.right, (short)rect.bottom));
  Win32::InvalidateRect(this->getParentWidget(), NULL, TRUE);
}

SbBool
MleFullViewer::isDecoration(void) const
{
  return PRIVATE(this)->decorations;
}

void
MleFullViewer::setPopupMenuEnabled(SbBool enable)
{
#if SOWIN_DEBUG
  if ((enable && this->isPopupMenuEnabled()) ||
       (! enable && ! this->isPopupMenuEnabled())) {
    SoDebugError::postWarning("MleFullViewer::setPopupMenuEnabled",
                               "popup menu already turned %s",
                               enable ? "on" : "off");
    return;
  }
#endif // SOWIN_DEBUG
  PRIVATE(this)->menuenabled = enable;
}

SbBool
MleFullViewer::isPopupMenuEnabled(void) const
{
  return PRIVATE(this)->menuenabled;
}

HWND
MleFullViewer::getAppPushButtonParent(void) const
{
  return PRIVATE(this)->viewerwidget;
}

void
MleFullViewer::addAppPushButton(HWND newButton)
{
  PRIVATE(this)->lefttrimbuttons.append(newButton);
}

void
MleFullViewer::insertAppPushButton(HWND newButton, int index)
{
  PRIVATE(this)->lefttrimbuttons.insert(newButton, index);
}

void
MleFullViewer::removeAppPushButton(HWND oldButton)
{
  int index = PRIVATE(this)->lefttrimbuttons.find(oldButton);
  PRIVATE(this)->lefttrimbuttons.remove(index);
}

int
MleFullViewer::findAppPushButton(HWND oldButton) const
{
  return PRIVATE(this)->lefttrimbuttons.find(oldButton);
}

int
MleFullViewer::lengthAppPushButton(void) const
{
  return PRIVATE(this)->lefttrimbuttons.getLength();
}

HWND
MleFullViewer::getRenderAreaWidget(void) const
{
  return PRIVATE(this)->renderareawidget;
}

// Doc in superclass.
void
MleFullViewer::setComponentCursor(const SoWinCursor & cursor)
{
  // Overridden to apply the new cursor only for the rendering canvas
  // widget. Otherwise, the default SoWinComponent setComponentCursor()
  // method will set the cursor for the top-most parent widget, which
  // makes it affect all sub-widgets, like the decorations stuff.

  PRIVATE(this)->cursor = cursor;
  SoWinComponent::setWidgetCursor(this->getRenderAreaWidget(), cursor);
}

// Documented in superclass.
void
MleFullViewer::setViewing(SbBool enable)
{
  if ((enable && this->isViewing()) || (! enable && ! this->isViewing())) {
#if SOWIN_DEBUG
    SoDebugError::postWarning("MleFullViewer::setViewing", "view mode already %s",
                               enable ? "on" : "off");
#endif // SOWIN_DEBUG
    return;
  }

  inherited::setViewing(enable);

  if (PRIVATE(this)->viewerButton(MleFullViewerP::VIEWERBUTTON_VIEW) != NULL) {
    PRIVATE(this)->viewerButton(MleFullViewerP::VIEWERBUTTON_VIEW)->setPressedState(enable);
    PRIVATE(this)->viewerButton(MleFullViewerP::VIEWERBUTTON_PICK)->setPressedState(enable ? FALSE : TRUE);
    PRIVATE(this)->viewerButton(MleFullViewerP::VIEWERBUTTON_SEEK)->setEnabled(enable);
  }
}

///////////////////////////////////////////////////////////////////
//
//  (protected)
//
//

// Documented in superclass.
HWND
MleFullViewer::buildWidget(HWND parent)
{
  // This method will always be called with a parent.

  assert(IsWindow(parent));
  
  MleFullViewerP::parentHWNDmappings->enter((unsigned long)parent, this);

  PRIVATE(this)->viewerwidget = parent;
  PRIVATE(this)->renderareawidget = inherited::buildWidget(parent);
  assert(IsWindow(PRIVATE(this)->renderareawidget));

  // Change default cursor from pointer arrow, to *no* default
  // cursor. This must be done for the SetCursor()-call in
  // MleFullViewerP::systemEventHook() to work even when the canvas has
  // not grabbed the mouse.
  SetClassLong(this->getGLWidget(), GCL_HCURSOR, 0);
  
  if (PRIVATE(this)->menuenabled) { this->buildPopupMenu(); }
  if (PRIVATE(this)->decorations) { this->buildDecoration(parent); }

  return PRIVATE(this)->renderareawidget;
}

// doc in super
void
MleFullViewer::sizeChanged(const SbVec2s & size)
{
  if (! IsWindow(this->getBaseWidget())) return;
  
  if (PRIVATE(this)->decorations) {
    SoWinRenderArea::sizeChanged(SbVec2s(size[0] - (2 * DECORATION_SIZE),
                                         size[1] - DECORATION_SIZE));
  }
  else {
    SoWinRenderArea::sizeChanged(size);
  }
  PRIVATE(this)->layoutWidgets(size[0], size[1]);
  Win32::InvalidateRect(this->getParentWidget(), NULL, TRUE);
}

void
MleFullViewer::buildDecoration(HWND parent)
{
  this->leftWheel = PRIVATE(this)->buildLeftWheel(parent);
  this->bottomWheel = PRIVATE(this)->buildBottomWheel(parent);
  this->rightWheel = PRIVATE(this)->buildRightWheel(parent);

  (void)this->buildViewerButtons(parent);
}

HWND
MleFullViewer::buildViewerButtons(HWND parent)
{
  this->createViewerButtons(parent, & PRIVATE(this)->righttrimbuttons);

  // Now position the buttons.

  RECT rect;
  Win32::GetClientRect(parent, &rect);
  const int x = rect.right - DECORATION_SIZE;

  for (int i=0; i < PRIVATE(this)->righttrimbuttons.getLength(); i++) {
    MleBitmapButton * b = (MleBitmapButton *)
      PRIVATE(this)->righttrimbuttons[i];
    b->move(x, i * DECORATION_SIZE, DECORATION_SIZE, DECORATION_SIZE);
  }

  return parent;
}

void
MleFullViewer::buildPopupMenu(void)
{
  this->prefmenu = PRIVATE(this)->setupStandardPopupMenu();
}

void
MleFullViewer::openPopupMenu(const SbVec2s position)
{
  assert(this->prefmenu != NULL);
  PRIVATE(this)->prepareMenu(this->prefmenu);

  RECT clientrect;
  Win32::GetClientRect(PRIVATE(this)->renderareawidget, &clientrect);

  this->prefmenu->popUp(PRIVATE(this)->renderareawidget,
                        position[0], clientrect.bottom - position[1]);
}

void
MleFullViewer::setLeftWheelString(const char * const name)
{
  if (PRIVATE(this)->leftthumbwheel)
    PRIVATE(this)->leftthumbwheel->setLabelText(name);
}

void
MleFullViewer::setBottomWheelString(const char * const name)
{
  if (PRIVATE(this)->bottomthumbwheel)
    PRIVATE(this)->bottomthumbwheel->setLabelText(name);
}

void
MleFullViewer::setRightWheelString(const char * const name)
{
  if (PRIVATE(this)->rightthumbwheel)
    PRIVATE(this)->rightthumbwheel->setLabelText(name);
}

// *************************************************************************

#ifndef DOXYGEN_SKIP_THIS

HHOOK MleFullViewerP::hookhandle = NULL;
int MleFullViewerP::nrinstances = 0;
SbDict * MleFullViewerP::parentHWNDmappings = NULL;

HWND
MleFullViewerP::buildLeftWheel(HWND parent)
{
  // Create coords are not needed - the widget is moved into place
  // by layoutWidgets
  this->leftthumbwheel =
    new MleThumbWheel(MleThumbWheel::Vertical,
                        parent,
                        0,
                        0,
                        0,
                        "RotX");
  PUBLIC(this)->leftWheelVal = this->leftthumbwheel->value();
  this->leftthumbwheel->setCallback(this->leftWheelCB, this);
  this->leftthumbwheel->setRangeBoundaryHandling(MleThumbWheel::ACCUMULATE);
  this->leftthumbwheel->setLabelOffset(0,
                                          ((DECORATION_SIZE - this->leftthumbwheel->sizeHint().cx) / 2)
                                          + DECORATION_BUFFER + 1);

  return this->leftthumbwheel->getWidget();
}

HWND
MleFullViewerP::buildBottomWheel(HWND parent)
{
  // Create coords are not needed - the widget is moved into place
  // by layoutWidgets
  this->bottomthumbwheel =
    new MleThumbWheel(MleThumbWheel::Horizontal,
                        parent,
                        1,
                        0,
                        0,
                        "RotY");
  PUBLIC(this)->bottomWheelVal = this->bottomthumbwheel->value();
  this->bottomthumbwheel->setCallback(this->bottomWheelCB, this);
  this->bottomthumbwheel->setRangeBoundaryHandling(MleThumbWheel::ACCUMULATE);
  this->bottomthumbwheel->setLabelOffset(-4, 0);

  return this->bottomthumbwheel->getWidget();
}

HWND
MleFullViewerP::buildRightWheel(HWND parent)
{
  // Create coords are not needed - the widget is moved into place
  // by layoutWidgets
  this->rightthumbwheel =
    new MleThumbWheel(MleThumbWheel::Vertical,
                        parent,
                        2,
                        0,
                        0,
                        "Dolly");
  PUBLIC(this)->rightWheelVal = this->rightthumbwheel->value();
  this->rightthumbwheel->setCallback(this->rightWheelCB, this);
  this->rightthumbwheel->setRangeBoundaryHandling(MleThumbWheel::ACCUMULATE);
  this->rightthumbwheel->setLabelOffset(- (this->bottomthumbwheel->getLabelSize().cx - this->rightthumbwheel->sizeHint().cx),
                                           ((DECORATION_SIZE - this->leftthumbwheel->sizeHint().cx) / 2)
                                           + DECORATION_BUFFER + 1);

  return this->rightthumbwheel->getWidget();
}

void
MleFullViewerP::seekbuttonProc(MleBitmapButton * b, void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  that->seekbuttonClicked();
}

void
MleFullViewerP::seekbuttonClicked(void)
{
  PUBLIC(this)->setSeekMode(PUBLIC(this)->isSeekMode() ? FALSE : TRUE);
}

void
MleFullViewerP::leftWheelCB(MleThumbWheel::Interaction type, float val,
                              void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  if (type == MleThumbWheel::START) { PUBLIC(that)->leftWheelStart(); }
  else if (type == MleThumbWheel::END) { PUBLIC(that)->leftWheelFinish(); }
  else { PUBLIC(that)->leftWheelMotion(val); }
}

void
MleFullViewerP::bottomWheelCB(MleThumbWheel::Interaction type, float val,
                                void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  if (type == MleThumbWheel::START) { PUBLIC(that)->bottomWheelStart(); }
  else if (type == MleThumbWheel::END) { PUBLIC(that)->bottomWheelFinish(); }
  else { PUBLIC(that)->bottomWheelMotion(val); }
}

void
MleFullViewerP::rightWheelCB(MleThumbWheel::Interaction type, float val,
                               void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  if (type == MleThumbWheel::START) { PUBLIC(that)->rightWheelStart(); }
  else if (type == MleThumbWheel::END) { PUBLIC(that)->rightWheelFinish(); }
  else { PUBLIC(that)->rightWheelMotion(val); }
}

void
MleFullViewerP::interactbuttonProc(MleBitmapButton * b, void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  that->viewerButton(MleFullViewerP::VIEWERBUTTON_PICK)->setPressedState(TRUE);
  that->viewerButton(MleFullViewerP::VIEWERBUTTON_VIEW)->setPressedState(FALSE);
  if (PUBLIC(that)->isViewing()) { PUBLIC(that)->setViewing(FALSE); }
}

void
MleFullViewerP::examinebuttonProc(MleBitmapButton * b, void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  that->viewerButton(MleFullViewerP::VIEWERBUTTON_VIEW)->setPressedState(TRUE);
  that->viewerButton(MleFullViewerP::VIEWERBUTTON_PICK)->setPressedState(FALSE);
  if (!PUBLIC(that)->isViewing()) { PUBLIC(that)->setViewing(TRUE); }
}

void
MleFullViewerP::homebuttonProc(MleBitmapButton * b, void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  PUBLIC(that)->resetToHomePosition();
}

void
MleFullViewerP::sethomebuttonProc(MleBitmapButton * b, void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  PUBLIC(that)->saveHomePosition();
}

void
MleFullViewerP::viewallbuttonProc(MleBitmapButton * b, void * userdata)
{
  MleFullViewerP * that = (MleFullViewerP *)userdata;
  PUBLIC(that)->viewAll();
}

int
MleFullViewerP::layoutWidgets(int cx, int cy)
{
  int x, y, width, height, bottom, right, top;
  const int numViewerButtons = this->righttrimbuttons.getLength();
  const int numAppButtons = this->lefttrimbuttons.getLength();
  HWND renderArea = PUBLIC(this)->getBaseWidget();
  UINT flags = SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW;
  
  // RenderArea
  assert(IsWindow(renderArea));

  if (this->decorations) {
    Win32::SetWindowPos(renderArea, NULL, DECORATION_SIZE, 0, 0, 0, flags);
  }
  else {
    Win32::SetWindowPos(renderArea, NULL, 0, 0, 0, 0, flags);
    return 0;
  }
  
  // Viewer buttons
  int i;
  for(i = 0; i < numViewerButtons; i++)
    this->viewerButton(i)->move(cx - DECORATION_SIZE, DECORATION_SIZE * i);

  // App buttons
  for(i = 0; i < numAppButtons; i++) {
    Win32::MoveWindow(this->appButton(i),
                      0, DECORATION_SIZE * i,
                      DECORATION_SIZE, DECORATION_SIZE, TRUE);
  }

  // Wheels

  bottom = (cy - (DECORATION_SIZE + DECORATION_BUFFER));
  right = (cx - ((this->rightthumbwheel ? this->rightthumbwheel->getLabelSize().cx : 0) + 8));

  // Left wheel
  if (this->leftthumbwheel) {

    x = (DECORATION_SIZE / 2) - (this->leftthumbwheel->sizeHint().cx / 2) - 1;
    width = this->leftthumbwheel->sizeHint().cx;

    top = numAppButtons * DECORATION_SIZE + DECORATION_BUFFER;

    // if area is large enough for full height
    if ((bottom - top) > this->leftthumbwheel->sizeHint().cy) {

      height = this->leftthumbwheel->sizeHint().cy;

      y = bottom - height;

    } // else we must use all available space
    else {

      y = top;

      height = bottom - top;

    }

    this->leftthumbwheel->move(x, y, width, height);
  }

  // Bottom wheel
  if (this->bottomthumbwheel) {

    x += this->leftthumbwheel->getLabelSize().cx +
      this->bottomthumbwheel->getLabelSize().cx + 8;
    y = (cy - DECORATION_SIZE) +
      ((DECORATION_SIZE / 2) - (this->bottomthumbwheel->sizeHint().cy / 2) + 1);

    height = this->bottomthumbwheel->sizeHint().cy;

    if (right < (x + this->bottomthumbwheel->sizeHint().cx)) {

      width = right - x;

    }
    else {

      width =  this->bottomthumbwheel->sizeHint().cx;

    }

    this->bottomthumbwheel->move(x, y, width, height);

  }

  // Right wheel
  if (this->rightthumbwheel) {

    width = this->rightthumbwheel->sizeHint().cx;

    x = (cx - DECORATION_SIZE) +
      ((DECORATION_SIZE / 2) - (width / 2) + 1);

    top = numViewerButtons * DECORATION_SIZE + DECORATION_BUFFER;

    // if area is large enough for original height
    if ((bottom - top) > this->rightthumbwheel->sizeHint().cy) {

      height = this->rightthumbwheel->sizeHint().cy;

      y = bottom - height;

    } // else we must use all available space
    else {

      y = top;

      height = bottom - top;

    }

    this->rightthumbwheel->move(x, y, width, height);
  }

  return 0;
}

void
MleFullViewerP::showDecorationWidgets(SbBool enable)
{
  const int numViewerButtons = this->righttrimbuttons.getLength();
  const int numAppButtons = this->lefttrimbuttons.getLength();

  // Viewer buttons
  int i;
  for(i = 0; i < numViewerButtons; i++) {
    (void)ShowWindow(this->viewerButton(i)->getWidget(),
                     (enable ? SW_SHOW : SW_HIDE));
  }

  // App buttons
  for(i = 0; i < numAppButtons; i++) {
    (void)ShowWindow(this->appButton(i), (enable ? SW_SHOW : SW_HIDE));
  }

  // Thumbwheels
  if (enable) {
    this->leftthumbwheel->show();
    this->bottomthumbwheel->show();
    this->rightthumbwheel->show();
  }
  else {
    this->leftthumbwheel->hide();
    this->bottomthumbwheel->hide();
    this->rightthumbwheel->hide();
  }
}

LRESULT CALLBACK
MleFullViewerP::systemEventHook(int code, WPARAM wparam, LPARAM lparam)
{
  // FIXME: if I'm not mistaken, this message hook catches ~ all
  // messages in our process, for _all_ windows / widgets. This is
  // superbly inefficient, and should be cleaned up. 20030424 mortene.

  CWPSTRUCT * msg = (CWPSTRUCT *)lparam;

  void * comp;
  SbBool found =
    MleFullViewerP::parentHWNDmappings->find((unsigned long)msg->hwnd, comp);

  if (found) {
    MleFullViewer * object = (MleFullViewer *)comp;

    // According to MSVC++ Win32 API doc on CallWndProc(), if code<0
    // we should not do any processing -- just pass message along to
    // next hook.
    if (code >= 0) {
      // FIXME: if code==HC_ACTION the Win32 API doc says we _must_
      // process the message. Weird. Try to find out what that really
      // means. 20011126 mortene.

      switch (msg->message) {
      case WM_SETCURSOR:
        SoWinComponent::setWidgetCursor(object->getRenderAreaWidget(),
                                        PRIVATE(object)->cursor);
        break;

      case WM_LBUTTONDOWN:
      case WM_SETFOCUS:
	if ( object->isStealFocus() )
	  (void)Win32::SetFocus(object->getGLWidget());
        break;
      }
    }
  }

  return CallNextHookEx(MleFullViewerP::hookhandle, code, wparam, lparam);
}

// *************************************************************************

void
MleFullViewer::createViewerButtons(HWND parent, SbPList * buttonlist)
{
  MleBitmapButton * button = new MleBitmapButton(parent, 24, "pick", NULL);
  button->addBitmap(pick_xpm);
  button->setBitmap(0); // use first (and only) bitmap
  button->registerClickedProc(MleFullViewerP::interactbuttonProc, PRIVATE(this));
  buttonlist->append(button);
  button->setPressedState(this->isViewing() ? FALSE : TRUE);

  button = new MleBitmapButton(parent, 24, "view", NULL);
  button->addBitmap(view_xpm);
  button->setBitmap(0); // use first (and only) bitmap
  button->registerClickedProc(MleFullViewerP::examinebuttonProc, PRIVATE(this));
  buttonlist->append(button);
  button->setPressedState(this->isViewing());

  button = new MleBitmapButton(parent, 24, "home", NULL);
  button->addBitmap(home_xpm);
  button->setBitmap(0); // use first (and only) bitmap
  button->registerClickedProc(MleFullViewerP::homebuttonProc, PRIVATE(this));
  buttonlist->append(button);

  button = new MleBitmapButton(parent, 24, "set_home", NULL);
  button->addBitmap(set_home_xpm);
  button->setBitmap(0);
  button->registerClickedProc(MleFullViewerP::sethomebuttonProc, PRIVATE(this));
  buttonlist->append(button);

  button = new MleBitmapButton(parent, 24, "view_all", NULL);
  button->addBitmap(view_all_xpm);
  button->setBitmap(0); // use first (and only) bitmap
  button->registerClickedProc(MleFullViewerP::viewallbuttonProc, PRIVATE(this));
  buttonlist->append(button);

  button = new MleBitmapButton(parent, 24, "seek", NULL);
  button->addBitmap(seek_xpm);
  button->setBitmap(0); // use first (and only) bitmap
  button->registerClickedProc(MleFullViewerP::seekbuttonProc, PRIVATE(this));
  buttonlist->append(button);
}

// FIXME: the build*Trim() functions are present and working in the
// other So* toolkits, but are unimplemented missing in SoWin. Why?
// 20020111 mortene.

HWND
MleFullViewer::buildLeftTrim(HWND parent)
{
  return NULL;
}

HWND
MleFullViewer::buildBottomTrim(HWND parent)
{
  return NULL;
}

HWND
MleFullViewer::buildRightTrim(HWND parent)
{
  return NULL;
}

// *************************************************************************

MleFullViewerP::MleFullViewerP(MleFullViewer * publ)
  : MleGuiFullViewerP(publ)
{
}

MleFullViewerP::~MleFullViewerP()
{
  const int len = this->righttrimbuttons.getLength();
  for (int i = 0; i < len; i++) {
    delete (MleBitmapButton *)this->righttrimbuttons[i];
  }
  if (this->leftthumbwheel) 
    delete this->leftthumbwheel;
  if (this->rightthumbwheel) 
    delete this->rightthumbwheel;
  if (this->bottomthumbwheel) 
    delete this->bottomthumbwheel;
}

MleBitmapButton *
MleFullViewerP::viewerButton(int idx)
{
  if (this->righttrimbuttons.getLength() <= idx) { return NULL; }

  return (MleBitmapButton *)this->righttrimbuttons[idx];
}

HWND
MleFullViewerP::appButton(int idx)
{
  if (this->lefttrimbuttons.getLength() <= idx) { return NULL; }

  return (HWND)this->lefttrimbuttons[idx];
}

void
MleFullViewerP::setThumbWheelValue(void * wheel, float val)
{
  MleThumbWheel * twheel = MleThumbWheel::getWheelFromHWND((HWND)wheel);
  assert(twheel != NULL);
  twheel->setValue(val);
}

#endif // !DOXYGEN_SKIP_THIS

// *************************************************************************
