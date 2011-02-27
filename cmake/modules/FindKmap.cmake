# Try to find the KMap library
#
# Parameters:
#  KMAP_LOCAL_DIR - If you have put a local version of libkmap into
#                   your source tree, set KMAP_LOCAL_DIR to the
#                   relative path from the root of your source tree
#                   to the libkmap local directory.
#
# Once done this will define
#
#  KMAP_FOUND - System has libkmap
#  KMAP_INCLUDE_DIR - The libkmap include directory/directories (for #include <libkmap/...> style)
#  KMAP_LIBRARIES - Link these to use libkmap
#  KMAP_DEFINITIONS - Compiler switches required for using libkmap
#  KMAP_VERSION - Version of libkmap which was found
#
# Copyright (c) 2010, Gilles Caulier, <caulier.gilles@gmail.com>
# Copyright (c) 2011, Michael G. Hansen, <mike@mghansen.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Kmap_FIND_QUIETLY and Kmap_FIND_REQUIRED may be defined by CMake.

if (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES AND KMAP_DEFINITIONS AND KMAP_VERSION)

  if (NOT Kmap_FIND_QUIETLY)
    message(STATUS "Found Kmap library in cache: ${KMAP_LIBRARIES}")
  endif (NOT Kmap_FIND_QUIETLY)

  # in cache already
  set(KMAP_FOUND TRUE)

else (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES AND KMAP_DEFINITIONS AND KMAP_VERSION)

  if (NOT Kmap_FIND_QUIETLY)
    message(STATUS "Check for Kmap library in local sub-folder...")
  endif (NOT Kmap_FIND_QUIETLY)

  # Check for a local version of the library.
  if (KMAP_LOCAL_DIR)
    find_file(KMAP_LOCAL_FOUND libkmap/version.h.cmake ${CMAKE_SOURCE_DIR}/${KMAP_LOCAL_DIR} NO_DEFAULT_PATH)
    if (NOT KMAP_LOCAL_FOUND)
      message(WARNING "KMAP_LOCAL_DIR specified as \"${KMAP_LOCAL_DIR}\" but libkmap could not be found there.")
    endif (NOT KMAP_LOCAL_FOUND)
  else (KMAP_LOCAL_DIR)
    find_file(KMAP_LOCAL_FOUND libkmap/version.h.cmake ${CMAKE_SOURCE_DIR}/libkmap NO_DEFAULT_PATH)
    if (KMAP_LOCAL_FOUND)
      set(KMAP_LOCAL_DIR libkmap)
    endif (KMAP_LOCAL_FOUND)
    find_file(KMAP_LOCAL_FOUND libkmap/version.h.cmake ${CMAKE_SOURCE_DIR}/libs/libkmap NO_DEFAULT_PATH)
    if (KMAP_LOCAL_FOUND)
      set(KMAP_LOCAL_DIR libs/libkmap)
    endif (KMAP_LOCAL_FOUND)
  endif (KMAP_LOCAL_DIR)

  if (KMAP_LOCAL_FOUND)
    # We need two include directories: because the version.h file is put into the build directory
    # TODO KMAP_INCLUDE_DIR sounds like it should contain only one directory...
    set(KMAP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${KMAP_LOCAL_DIR} ${CMAKE_BINARY_DIR}/${KMAP_LOCAL_DIR})
    set(KMAP_DEFINITIONS "-I${CMAKE_SOURCE_DIR}/${KMAP_LOCAL_DIR}" "-I${CMAKE_BINARY_DIR}/${KMAP_LOCAL_DIR}")
    set(KMAP_LIBRARIES kmap)
    if (NOT Kmap_FIND_QUIETLY)
      message(STATUS "Found Kmap library in local sub-folder: ${CMAKE_SOURCE_DIR}/${KMAP_LOCAL_DIR}")
    endif (NOT Kmap_FIND_QUIETLY)
    set(KMAP_FOUND TRUE)

    set(kmap_version_h_filename "${CMAKE_BINARY_DIR}/${KMAP_LOCAL_DIR}/libkmap/version.h")

  else (KMAP_LOCAL_FOUND)
    if (NOT WIN32)
      if (NOT Kmap_FIND_QUIETLY)
        message(STATUS "Check Kmap library using pkg-config...")
      endif (NOT Kmap_FIND_QUIETLY)

      # use FindPkgConfig to get the directories and then use these values
      # in the find_path() and find_library() calls
      include(FindPkgConfig)

      pkg_search_module(KMAP libkmap)

      if (KMAP_FOUND)
        # make sure the version is >= 0.1.0
        # TODO: WHY?
        if (KMAP_VERSION VERSION_LESS 0.1.0)
          message(STATUS "Found libkmap release < 0.1.0, too old")
          set(KMAP_VERSION_GOOD_FOUND FALSE)
          set(KMAP_FOUND FALSE)
        else (KMAP_VERSION VERSION_LESS 0.1.0)
          if (NOT Kmap_FIND_QUIETLY)
            message(STATUS "Found libkmap release ${KMAP_VERSION}")
          endif (NOT Kmap_FIND_QUIETLY)
          set(KMAP_VERSION_GOOD_FOUND TRUE)
        endif (KMAP_VERSION VERSION_LESS 0.1.0)
      else (KMAP_FOUND)
        set(KMAP_VERSION_GOOD_FOUND FALSE)
      endif (KMAP_FOUND)
    else (NOT WIN32)
      # TODO: Why do we just assume the version is good?
      set(KMAP_VERSION_GOOD_FOUND TRUE)
    endif (NOT WIN32)

    if (KMAP_VERSION_GOOD_FOUND)
      set(KMAP_DEFINITIONS "${KMAP_CFLAGS}")

      find_path(KMAP_INCLUDE_DIR libkmap/version.h ${KMAP_INCLUDEDIR})
      set(kmap_version_h_filename "${KMAP_INCLUDE_DIR}/libkmap/version.h")

      find_library(KMAP_LIBRARIES NAMES kmap PATHS ${KMAP_LIBDIR})

      if (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES)
        set(KMAP_FOUND TRUE)
      else (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES)
        set(KMAP_FOUND FALSE)
      endif (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES)
    endif (KMAP_VERSION_GOOD_FOUND)

    if (KMAP_FOUND)
      if (NOT Kmap_FIND_QUIETLY)
        message(STATUS "Found libkmap: ${KMAP_LIBRARIES}")
      endif (NOT Kmap_FIND_QUIETLY)
    else (KMAP_FOUND)
      if (Kmap_FIND_REQUIRED)
        if (NOT KMAP_INCLUDE_DIR)
          message(FATAL_ERROR "Could NOT find libkmap header files.")
        else(NOT KMAP_INCLUDE_DIR)
          message(FATAL_ERROR "Could NOT find libkmap library.")
        endif (NOT KMAP_INCLUDE_DIR)
      endif (Kmap_FIND_REQUIRED)
    endif (KMAP_FOUND)

  endif (KMAP_LOCAL_FOUND)

  if (KMAP_FOUND)
    # Find the version information, unless that was reported by pkg_search_module.
    if (NOT KMAP_VERSION)
      file(READ "${kmap_version_h_filename}" kmap_version_h_content)
      # This is the line we are trying to find: static const char kmap_version[] = "1.22.4-beta_5+dfsg";
      string(REGEX REPLACE ".*char +kmap_version\\[\\] += +\"([^\"]+)\".*" "\\1" KMAP_VERSION "${kmap_version_h_content}")
      unset(kmap_version_h_content)

    endif (NOT KMAP_VERSION)
    unset(kmap_version_h_filename)
  endif (KMAP_FOUND)

  if (KMAP_FOUND)
    mark_as_advanced(KMAP_INCLUDE_DIR KMAP_LIBRARIES KMAP_DEFINITIONS KMAP_VERSION KMAP_FOUND)
  else (KMAP_FOUND)
    # The library was not found, reset all related variables.
    unset(KMAP_INCLUDE_DIR)
    unset(KMAP_LIBRARIES)
    unset(KMAP_DEFINITIONS)
    unset(KMAP_VERSION)
  endif (KMAP_FOUND)

endif (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES AND KMAP_DEFINITIONS AND KMAP_VERSION)
