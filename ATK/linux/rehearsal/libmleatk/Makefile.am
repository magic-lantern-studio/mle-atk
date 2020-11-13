#######################################
# The directory for object files and libraries of object code
libdir = $(exec_prefix)/lib/mle/rehearsal

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
	$(MLE_HOME)/Core/util/common/src/mlDebug.c \
	$(MLE_HOME)/Core/util/common/src/mlErrno.c

# Linker options libTestProgram
libmleatk_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libmleatk_la_CPPFLAGS = \
	-DMLE_NOT_DLL \
	-DMLE_REHEARSAL \
	-DMLE_DIGITAL_WORKPRINT \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../../common/include \
	-I$(top_srcdir)/../../linux/include \
	-I$(MLE_HOME)/DigitalWorkprint/lib/common/include \
	-I$(MLE_HOME)/DigitalWorkprint/lib/linux/include \
	-I$(MLE_HOME)/Parts/sets/inventor \
	-I$(MLE_HOME)/Parts/roles/common/include \
	-I$(MLE_HOME)/Core/mlert/common/src/foundation \
	-I$(MLE_HOME)/Core/util/common/include \
	-I$(MLE_HOME)/Core/util/linux/include \
	-I$(MLE_HOME)/Core/math/common/include \
	-I$(MLE_HOME)/Core/math/linux/include
