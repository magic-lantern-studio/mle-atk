# -*- Makefile -*-
#
#  Copyright (C) 2000-2007  Wizzer Works
#
#  Wizzer Works makes available all content in this file ("Content").
#  Unless otherwise indicated below, the Content is provided to you
#  under the terms and conditions of the Common Public License Version 1.0
#  ("CPL"). A copy of the CPL is available at
#
#      http://opensource.org/licenses/cpl1.0.php
#
#  For purposes of the CPL, "Program" will mean the Content.
#
#  For information concerning this Makefile, contact Mark S. Millard,
#  of Wizzer Works at msm@wizzerworks.com.
#
#*************************************************************************
#
# Makefile for installing header files.
#
# $Id$
#
###########################################################################

ISM_DEPTH = ..
include $(ISM_DEPTH)/ismdefs

VPATH = mle:$(ISM_DEPTH)/../common/include/mle

COMMON_HEADERS = \
        AtkBasicArray.h \
        AtkCommonStructs.h \
        AtkWire.h \
        AtkWired.h \
        AtkWireMsg.h \
	$(NULL)

WIN32_HEADERS = \
        MlePlayer.h \
	$(NULL)

HEADERS = \
	$(COMMON_HEADERS) \
	$(WIN32_HEADERS) \
	$(NULL)

install: $(HEADERS)
	$(INSTALL) -d $(MLE_ROOT)/include/mle
	$(INSTALL) $^ $(MLE_ROOT)/include/mle

include $(ISMRULES)

collect:
	$(IBCOLLECT) -baseDir $(ISM_DEPTH)/.. \
                     -log ../build/collect.raw \
                     -srcDir $(ISM_DEPTH)/../common/include/mle \
                     -destDir "include/mle" \
                     -tags "INCLUDE" \
                     $(COMMON_HEADERS)
	$(IBCOLLECT) -baseDir $(ISM_DEPTH)/.. \
                     -log ../build/collect.raw \
                     -srcDir mle \
                     -destDir "include/mle" \
                     -tags "INCLUDE" \
                     $(WIN32_HEADERS)
