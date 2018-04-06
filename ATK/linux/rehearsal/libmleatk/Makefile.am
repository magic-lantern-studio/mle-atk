#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libmleatk.la

#######################################
# Build information for each library

# Sources for libmleatk
libmleatk_la_SOURCES = \
	../../../common/src/AtkBasicArray.cxx \
	../../../common/src/AtkWire.cxx \
	../../../common/src/AtkWired.cxx \
	../../../common/src/AtkWireFunc.cxx \
	../../../common/src/AtkWireMsg.cxx \
	../../src/MlePlayer.cxx \
	../../../../Core/util/common/src/mlDebug.c \
	../../../../Core/util/common/src/mlErrno.c

# Linker options libTestProgram
libmleatk_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libmleatk_la_CPPFLAGS = \
	-DMLE_NOT_DLL \
	-DMLE_REHEARSAL \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../../common/include \
	-I$(top_srcdir)/../../linux/include \
	-I$(top_srcdir)/../../../DigitalWorkprint/lib/common/include \
	-I$(top_srcdir)/../../../DigitalWorkprint/lib/linux/include \
	-I$(top_srcdir)/../../../Parts/sets/inventor \
	-I$(top_srcdir)/../../../Parts/roles/common/include \
	-I$(top_srcdir)/../../../Core/mlert/common/src/foundation \
	-I$(top_srcdir)/../../../Core/util/common/include \
	-I$(top_srcdir)/../../../Core/util/linux/include \
	-I$(top_srcdir)/../../../Core/math/common/include \
	-I$(top_srcdir)/../../../Core/math/linux/include

