# Microsoft Developer Studio Project File - Name="mleatk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mleatk - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mleatk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mleatk.mak" CFG="mleatk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mleatk - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "mleatk - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mleatk - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\..\include" /I "$(MLE_ROOT)\Core\mlert\common\src\foundation" /I "$(MLE_ROOT)\Parts\sets\inventor" /I "$(MLE_ROOT)\Parts\roles\common\include" /I "$(MLE_ROOT)\DigitalWorkprint\lib\common\include" /I "$(MLE_ROOT)\Core\util\common\include" /I "$(MLE_ROOT)\Core\util\win32\include" /I "$(MLE_ROOT)\Core\math\common\include" /I "$(COINDIR)\include" /D "MLE_REHEARSAL" /D "MLE_DLL" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D ML_MATH_DEBUG=0 /D ML_FIXED_POINT=0 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Install Library
PostBuild_Cmds=mkdir $(MLE_HOME)\lib\rehearsal	copy Release\mleatk.lib $(MLE_HOME)\lib\rehearsal	mkdir $(MLE_HOME)\include\mle	xcopy /y ..\..\include\mle\*.h $(MLE_HOME)\include\mle	xcopy /y ..\..\..\common\include\mle\*.h $(MLE_HOME)\include\mle
# End Special Build Tool

!ELSEIF  "$(CFG)" == "mleatk - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\common\include" /I "..\..\include" /I "$(MLE_ROOT)\Core\mlert\common\src\foundation" /I "$(MLE_ROOT)\Parts\sets\inventor" /I "$(MLE_ROOT)\Parts\roles\common\include" /I "$(MLE_ROOT)\DigitalWorkprint\lib\common\include" /I "$(MLE_ROOT)\Core\util\common\include" /I "$(MLE_ROOT)\Core\util\win32\include" /I "$(MLE_ROOT)\Core\math\common\include" /I "$(COINDIR)\include" /D "MLE_REHEARSAL" /D "MLE_DEBUG" /D "MLE_DLL" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D ML_MATH_DEBUG=1 /D ML_FIXED_POINT=0 /Fp"Debug/mleatkd.pch" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\mleatkd.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Install Library
PostBuild_Cmds=mkdir $(MLE_HOME)\lib\rehearsal	copy Debug\mleatkd.lib $(MLE_HOME)\lib\rehearsal	mkdir $(MLE_HOME)\include\mle	xcopy /y ..\..\include\mle\*.h $(MLE_HOME)\include\mle	xcopy /y ..\..\..\common\include\mle\*.h $(MLE_HOME)\include\mle
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "mleatk - Win32 Release"
# Name "mleatk - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\common\src\AtkBasicArray.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\AtkWire.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\AtkWired.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\AtkWireFunc.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\AtkWireMsg.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Core\util\common\src\mlDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MlePlayer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Core\util\common\src\mlErrno.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\common\include\mle\AtkBasicArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\mle\AtkCommonStructs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\mle\AtkWire.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\mle\AtkWired.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\mle\AtkWireFunc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\mle\AtkWireMsg.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mle\DwpPlatformDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mle\DwpPlatformTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Core\util\common\include\mle\mlDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mle\MlePlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Core\util\common\include\mle\mlErrno.h
# End Source File
# End Group
# End Target
# End Project
