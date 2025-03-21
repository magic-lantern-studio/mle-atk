/** @defgroup MleDWPAccess Magic Lantern Digital Workprint Library API */
/**
 *  @file DwpPlatformDefs.h
 *  @ingroup MleDWPAccess
 *
 *  Magic Lantern Digital Workprint - Win32 specific definitions.
 */

 // COPYRIGHT_BEGIN
 //
 // The MIT License (MIT)
 //
 // Copyright (c) 2000-2025 Wizzer Works
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
