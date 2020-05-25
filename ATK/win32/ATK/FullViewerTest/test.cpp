/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file test.cpp
 * @ingroup MleATK
 *
 * This file contains a test for the Magic Lantern Examiner Viewer.
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

#include <Inventor/Win/SoWin.h>

#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoSeparator.h>

#include <mle/MleExaminerViewer.h>

int
main(int argc, char ** argv)
{
    // Initializes SoWin library (and implicitly also the Coin
    // library). Returns a top-level / shell window to use.
    HWND mainwin = SoWin::init(argc, argv, argv[0]);
  
    // Make a dead simple scene graph by using the Coin library, only
    // containing a single yellow cone under the scenegraph root.
    SoSeparator * root = new SoSeparator;
    root->ref();

    SoBaseColor * col = new SoBaseColor;
    col->rgb = SbColor(1, 1, 0);
    root->addChild(col);

    root->addChild(new SoCone);
  
    // Use one of the convenient SoWin viewer classes.
    MleExaminerViewer * eviewer = new MleExaminerViewer(mainwin);
    eviewer->setSceneGraph(root);
    eviewer->show();
  
    // Pop up the main window.
    SoWin::show(mainwin);
    // Loop until exit.
    SoWin::mainLoop();

    // Clean up resources.
    delete eviewer;
    root->unref();

    return 0;
}
