# COPYRIGHT_BEGIN
#
# The MIT License (MIT)
#
# Copyright (c) 2025 Wizzer Works
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#  For information concerning this header file, contact Mark S. Millard,
#  of Wizzer Works at msm@wizzerworks.com.
#
#  More information concerning Wizzer Works may be found at
#
#  For information concerning this header file, contact Mark S. Millard,
#  of Wizzer Works at msm@wizzerworks.com.
#
#  More information concerning Wizzer Works may be found at
#
#      http://www.wizzerworks.com
#
# COPYRIGHT_END

# Stop if cmake version is below 3.10
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Project name and enable C and C++ support
project(libmleatk VERSION 1.0.0 LANGUAGES C CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify Debug postfix
set(CMAKE_DEBUG_POSTFIX "d")

# Check for Magic Lantern environment variables
if (DEFINED ENV{MLE_ROOT})
  message(STATUS "MLE_ROOT=" $ENV{MLE_ROOT})
else()
  message(STATUS "MLE_ROOT NOT SET")
  set(ENV{MLE_ROOT} "/opt/MagicLantern")
  message(STATUS "Using " $ENV{MLE_ROOT})
endif()

# Fail early if one of the required packages cannot be found. The ATK is
# built against the header files installed in $MLE_ROOT by mle-core-util,
# mle-core-math and mle-core-dwp.
foreach(header mle/mlTypes.h mle/DwpItem.h math/transfrm.h)
  if (NOT EXISTS $ENV{MLE_ROOT}/include/${header})
    message(FATAL_ERROR
      "Could not find $ENV{MLE_ROOT}/include/${header}. "
      "Install mle-core-util, mle-core-math and mle-core-dwp into MLE_ROOT first.")
  endif()
endforeach()
find_path(X11_INCLUDE_DIR X11/Xlib.h)
if (NOT X11_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find X11/Xlib.h. Install the X11 development files.")
endif()

# Specify the compiler macro definitions
add_compile_definitions(
  MLE_REHEARSAL
  MLE_DIGITAL_WORKPRINT
  MLE_NOT_UTIL_DLL
  MLE_NOT_MATH_DLL
  MLE_NOT_DWP_DLL
  MLE_NOT_RUNTIME_DLL
  MLE_NOT_3DCAMERACARRIER_DLL
  MLE_NOT_3DSET_DLL
  $<$<CONFIG:Debug>:MLE_DEBUG>)

# Specify the include directories for the header files
include_directories(
  include
  ../../common/include
  ../../linux/include
  $ENV{MLE_ROOT}/include
  ${X11_INCLUDE_DIR})

# The ATK library, libmleatk. Like the autotools build, it is not linked
# against the Magic Lantern libraries.
add_library(mleatk SHARED
  ../../common/src/AtkBasicArray.cxx
  ../../common/src/AtkWire.cxx
  ../../common/src/AtkWired.cxx
  ../../common/src/AtkWireFunc.cxx
  ../../common/src/AtkWireMsg.cxx
  ../src/MlePlayer.cxx)
set_target_properties(mleatk PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_compile_options(mleatk PRIVATE $<$<CONFIG:Debug>:-O0>)
install(TARGETS mleatk LIBRARY DESTINATION lib/rehearsal)

# The example program. It is built, but not installed.
add_executable(exampleProgram exampleProgram/exampleProgram.c)
target_link_libraries(exampleProgram mleatk)

# Install the header files
install(
  FILES
    ../../common/include/mle/AtkBasicArray.h
    ../../common/include/mle/AtkCommonStructs.h
    ../../common/include/mle/AtkWired.h
    ../../common/include/mle/AtkWireFunc.h
    ../../common/include/mle/AtkWire.h
    ../../common/include/mle/AtkWireMsg.h
    ../../common/include/mle/mleatk_rehearsal.h
    ../../linux/include/mle/MlePlayer.h
  DESTINATION
    include/mle)

# Uninstall libraries and header files
add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
  TARGET "uninstall"
  POST_BUILD
  COMMENT "Uninstall files with install_manifest.txt"
  COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in
          install_manifest.txt to be uninstalled!)
