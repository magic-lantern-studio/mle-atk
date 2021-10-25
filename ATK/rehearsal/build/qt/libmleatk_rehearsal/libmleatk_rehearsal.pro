QT += widgets

TARGET = mleatk
TEMPLATE = lib
DEFINES += LIBMLEATK_REHEARSAL_LIBRARY

CONFIG += c++11

unix: MLE_ROOT = /opt/MagicLantern
#unix: MLE_HOME = /media/workarea/msm/Projects/WizzerWorks/MagicLanternPythonHome
unix: MLE_HOME = $$(MLE_HOME)

INCLUDEPATH += \
    $$PWD/../../../../linux/include \
    $$PWD/../../../../common/include \
    /opt/MagicLantern/include \
    /usr/local/include

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += \
    MLE_NOT_DLL \
    MLE_REHEARSAL \
    MLE_DIGITAL_WORKPRINT \
    MLE_QT

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/../../../../common/src/AtkBasicArray.cxx \
    $$PWD/../../../../common/src/AtkWire.cxx \
    $$PWD/../../../../common/src/AtkWired.cxx \
    $$PWD/../../../../common/src/AtkWireFunc.cxx \
    $$PWD/../../../../common/src/AtkWireMsg.cxx \
    $$PWD/../../../../linux/src/MlePlayer.cxx \
    $$MLE_HOME/Core/util/common/src/mlDebug.c \
    $$MLE_HOME/Core/util/common/src/mlErrno.c

HEADERS += \
    $$PWD/../../../../common/include/mle/AtkCommonStructs.h \
    $$PWD/../../../../common/include/mle/AtkWired.h \
    $$PWD/../../../../common/include/mle/AtkWireMsg.h \
    $$PWD/../../../../common/include/mle/AtkWireFunc.h \
    $$PWD/../../../../common/include/mle/AtkWire.h \
    $$PWD/../../../../common/include/mle/AtkBasicArray.h \
    $$PWD/../../../../common/include/mle/mleatk_rehearsal.h \
    $$PWD/../../../../linux/include/mle/MlePlayer.h

macx {
    # Set the LFLAGS so that dynamic libraries behave like Linux DSOs.
    QMAKE_LFLAGS += -undefined suppress -flat_namespace
}

# Default rules for deployment.
unix {
    target.path = /opt/MagicLantern/lib/mle/qt/rehearsal
    headers.path = /opt/MagicLantern/include/mle
    headers.files = $$HEADERS
    INSTALLS += target headers
}
!isEmpty(target.path): INSTALLS += target
