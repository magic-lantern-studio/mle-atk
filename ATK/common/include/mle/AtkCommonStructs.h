/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkCommonStructs.h
 * @ingroup MleATK
 *
 * This file contains common structures used by the
 * Magic Lantern Authoring Toolkit.
 *
 * @author Mark S. Millard
 * @date May 5, 2003
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

#ifndef __ATK_COMMONSTRUCTS_H_
#define __ATK_COMMONSTRUCTS_H_

#if defined(sgi)
#include <X11/X.h>

struct VisualWindow
{
    VisualID vid;
    Window wid;
};
#endif

#endif /* __ATK_COMMONSTRUCTS_H_ */
