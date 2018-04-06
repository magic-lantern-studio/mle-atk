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

#ifndef MLE_WIN32API
#define MLE_WIN32API

// This is an internal class which wraps most of the Win32 API
// functions we use in SoWin -- to do robust checking of return values
// etc.
//
// This way, we should be able to early detect programming errors on
// our behalf, while still keeping the sourcecode nice and clean.

#include <windows.h>
#include <winuser.h>
#include <Inventor/SbString.h>

class Win32 {
public:
  // Must suffix these two to other names than the originals, as they
  // seem to be #define macros.
  static HWND CreateWindow_(LPCTSTR, LPCTSTR, DWORD, int, int, int, int, HWND, HMENU, HANDLE, LPVOID);
  static HWND CreateWindowEx_(DWORD, LPCTSTR, LPCTSTR, DWORD, int, int, int, int, HWND, HMENU, HANDLE, LPVOID);

  static void BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
  static void EnableWindow(HWND, BOOL);
  static void GetTextExtentPoint(HDC, LPCTSTR, int, LPSIZE);
  static void MoveWindow(HWND, int, int, int, int, BOOL);
  static HGDIOBJ SelectObject(HDC, HGDIOBJ);
  static void DeleteObject(HGDIOBJ);
  static void SwapBuffers(HDC);
  static HDC GetDC(HWND);  
  static void DeleteDC(HDC);
  static void ReleaseDC(HWND, HDC);
  static HDC BeginPaint(HWND, LPPAINTSTRUCT);
  static void EndPaint(HWND, CONST PAINTSTRUCT *);
  static ATOM RegisterClass(CONST WNDCLASS *);
  static void UnregisterClass(LPCTSTR, HINSTANCE);
  static void DestroyWindow(HWND);
  static void SetWindowText(HWND, LPCTSTR);
  static void InvalidateRect(HWND, CONST RECT *, BOOL);
  static void GetWindowRect(HWND, LPRECT);
  static void GetClientRect(HWND, LPRECT); 
  static LONG SetWindowLong(HWND, int, LONG);
  static LONG GetWindowLong(HWND, int);
  static void SetWindowPos(HWND, HWND, int, int, int, int, UINT);
  static HHOOK SetWindowsHookEx(int, HOOKPROC, HINSTANCE, DWORD);
  static void UnhookWindowsHookEx(HHOOK);
  static void ModifyMenu(HMENU,UINT,UINT,UINT,LPCTSTR);
  static void EnableMenuItem(HMENU, UINT, UINT);
  static void GetMenuItemInfo(HMENU, UINT, BOOL, LPMENUITEMINFO);
  static void SetMenuItemInfo(HMENU, UINT, BOOL, LPMENUITEMINFO);
  static void InsertMenuItem(HMENU, UINT, BOOL, LPMENUITEMINFO);
  static void InsertMenu(HMENU, UINT, UINT, UINT, LPCTSTR);
  static void RemoveMenu(HMENU, UINT, UINT);
  static HMENU CreatePopupMenu(VOID);
  static void DestroyMenu(HMENU);
  static DWORD CheckMenuItem(HMENU, UINT, UINT);
  static HCURSOR CreateCursor(HINSTANCE, int, int, int, int,
                              CONST VOID *, CONST VOID *);
  static HCURSOR LoadCursor(HINSTANCE, LPCTSTR);
  static void ClientToScreen(HWND, LPPOINT);
  static HWND SetFocus(HWND);
  static UINT_PTR SetTimer(HWND, UINT_PTR, UINT, TIMERPROC);
  static void KillTimer(HWND, UINT_PTR);

  // Internal and external interface to GetLastError() + FormatMessage().
  static SbString getWin32Err(DWORD & lasterr);
  
private:
  static void showLastErr(void);
};

#endif // MLE_WIN32API
