/** @defgroup MleDWPAccess Magic Lantern Digital Workprint Library API */
/**
 *  @file DwpPlatformDefs.h
 *  @ingroup MleDWPAccess
 *
 *  Magic Lantern Digital Workprint - Win32 specific definitions.
 *
 *  @author Mark S. Millard
 *  @version 0.13
 */

// COPYRIGHT_BEGIN
//
//  Copyright (C) 2000-2007  Wizzer Works
//
//  Wizzer Works makes available all content in this file ("Content").
//  Unless otherwise indicated below, the Content is provided to you
//  under the terms and conditions of the Common Public License Version 1.0
//  ("CPL"). A copy of the CPL is available at
//
//      http://opensource.org/licenses/cpl1.0.php
//
//  For purposes of the CPL, "Program" will mean the Content.
//
//  For information concerning this Makefile, contact Mark S. Millard,
//  of Wizzer Works at msm@wizzerworks.com.
//
//  More information concerning Wizzer Works may be found at
//
//      http://www.wizzerworks.com
//
// COPYRIGHT_END

#ifndef __MLE_DWP_PLATFORMDEFS_H_
#define __MLE_DWP_PLATFORMDEFS_H_


// The Win32 platform definition macro.
//
#define PLATFORM_WIN32

// Win32 uses ANSI C/C++
#define MLE_DWP_ANSI_C

// The definition of export and import macros.
#define MLE_DWP_EXPORT __declspec(dllexport)
#define MLE_DWP_IMPORT __declspec(dllimport)

// The minimal import export macros.
#define MLE_DWP_MIN_EXPORT
#define MLE_DWP_MIN_IMPORT

#define MLE_DWP_CDECL __cdecl

// MLE_DWP_STDCALL used for calling functions that have non 
// C calling conventions i.e. pascal calling conventions 
// for tessellator
#define MLE_DWP_STDCALL __stdcall

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif // !DEBUG
#endif // _DEBUG

#endif /* __MLE_DWP_PLATFORMDEFS_H_ */
