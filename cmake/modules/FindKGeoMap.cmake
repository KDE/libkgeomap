# Try to find the KGeoMap library
#
# Parameters:
#  KGEOMAP_LOCAL_DIR - If you have put a local version of libkgeomap into
#                      your source tree, set KGEOMAP_LOCAL_DIR to the
#                      relative path from the root of your source tree
#                      to the libkgeomap local directory.
#
# Once done this will define
#
#  KGEOMAP_FOUND - System has libkgeomap
#  KGEOMAP_INCLUDE_DIR - The libkgeomap include directory/directories (for #include <libkgeomap/...> style)
#  KGEOMAP_LIBRARIES - Link these to use libkgeomap
#  KGEOMAP_DEFINITIONS - Compiler switches required for using libkgeomap
#  KGEOMAP_VERSION - Version of libkgeomap which was found
#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier.gilles@gmail.com>
# Copyright (c) 2011, Michael G. Hansen, <mike@mghansen.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# KGeoMap_FIND_QUIETLY and KGeoMap_FIND_REQUIRED may be defined by CMake.

if (KGEOMAP_INCLUDE_DIR AND KGEOMAP_LIBRARIES AND KGEOMAP_DEFINITIONS AND KGEOMAP_VERSION)

  if (NOT KGeoMap_FIND_QUIETLY)
    message(STATUS "Found KGeoMap library in cache: ${KGEOMAP_LIBRARIES}")
  endif (NOT KGeoMap_FIND_QUIETLY)

  # in cache already
  set(KGEOMAP_FOUND TRUE)

else (KGEOMAP_INCLUDE_DIR AND KGEOMAP_LIBRARIES AND KGEOMAP_DEFINITIONS AND KGEOMAP_VERSION)

  if (NOT KGeoMap_FIND_QUIETLY)
    message(STATUS "Check for KGeoMap library in local sub-folder...")
  endif (NOT KGeoMap_FIND_QUIETLY)

  # Check for a local version of the library.
  if (KGEOMAP_LOCAL_DIR)
    find_file(KGEOMAP_LOCAL_FOUND libkgeomap/libkgeomap_export.h ${CMAKE_SOURCE_DIR}/${KGEOMAP_LOCAL_DIR} NO_DEFAULT_PATH)
    if (NOT KGEOMAP_LOCAL_FOUND)
      message(WARNING "KGEOMAP_LOCAL_DIR specified as \"${KGEOMAP_LOCAL_DIR}\" but libkgeomap could not be found there.")
    endif (NOT KGEOMAP_LOCAL_FOUND)
  else (KGEOMAP_LOCAL_DIR)
    find_file(KGEOMAP_LOCAL_FOUND libkgeomap/libkgeomap_export.h ${CMAKE_SOURCE_DIR}/libkgeomap NO_DEFAULT_PATH)
    if (KGEOMAP_LOCAL_FOUND)
      set(KGEOMAP_LOCAL_DIR libkgeomap)
    endif (KGEOMAP_LOCAL_FOUND)
    find_file(KGEOMAP_LOCAL_FOUND libkgeomap/libkgeomap_export.h ${CMAKE_SOURCE_DIR}/libs/libkgeomap NO_DEFAULT_PATH)
    if (KGEOMAP_LOCAL_FOUND)
      set(KGEOMAP_LOCAL_DIR libs/libkgeomap)
    endif (KGEOMAP_LOCAL_FOUND)
  endif (KGEOMAP_LOCAL_DIR)

  if (KGEOMAP_LOCAL_FOUND)
    # We need two include directories: because the version.h file is put into the build directory
    # TODO KGEOMAP_INCLUDE_DIR sounds like it should contain only one directory...
    set(KGEOMAP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${KGEOMAP_LOCAL_DIR} ${CMAKE_BINARY_DIR}/${KGEOMAP_LOCAL_DIR})
    set(KGEOMAP_DEFINITIONS "-I${CMAKE_SOURCE_DIR}/${KGEOMAP_LOCAL_DIR}" "-I${CMAKE_BINARY_DIR}/${KGEOMAP_LOCAL_DIR}")
    set(KGEOMAP_LIBRARIES kgeomap)
    if (NOT KGeoMap_FIND_QUIETLY)
      message(STATUS "Found KGeoMap library in local sub-folder: ${CMAKE_SOURCE_DIR}/${KGEOMAP_LOCAL_DIR}")
    endif (NOT KGeoMap_FIND_QUIETLY)
    set(KGEOMAP_FOUND TRUE)

    set(kgeomap_version_h_filename "${CMAKE_BINARY_DIR}/${KGEOMAP_LOCAL_DIR}/libkgeomap/version.h")

  else (KGEOMAP_LOCAL_FOUND)
    if (NOT WIN32)
      if (NOT KGeoMap_FIND_QUIETLY)
        message(STATUS "Check KGeoMap library using pkg-config...")
      endif (NOT KGeoMap_FIND_QUIETLY)

      # use FindPkgConfig to get the directories and then use these values
      # in the find_path() and find_library() calls
      include(FindPkgConfig)

      pkg_search_module(PC_KGEOMAP libkgeomap)

      if (PC_KGEOMAP_FOUND)
        # make sure the version is >= 0.1.0
        # TODO: WHY?
        if (PC_KGEOMAP_VERSION VERSION_LESS 0.1.0)
          message(STATUS "Found libkgeomap release < 0.1.0, too old")
          set(KGEOMAP_VERSION_GOOD_FOUND FALSE)
          set(KGEOMAP_FOUND FALSE)
        else (PC_KGEOMAP_VERSION VERSION_LESS 0.1.0)
          set(KGEOMAP_VERSION "${PC_KGEOMAP_VERSION}")
          if (NOT KGeoMap_FIND_QUIETLY)
            message(STATUS "Found libkgeomap release ${KGEOMAP_VERSION}")
          endif (NOT KGeoMap_FIND_QUIETLY)
          set(KGEOMAP_VERSION_GOOD_FOUND TRUE)
        endif (PC_KGEOMAP_VERSION VERSION_LESS 0.1.0)
      else (PC_KGEOMAP_FOUND)
        set(KGEOMAP_VERSION_GOOD_FOUND FALSE)
      endif (PC_KGEOMAP_FOUND)
    else (NOT WIN32)
      # TODO: Why do we just assume the version is good?
      set(KGEOMAP_VERSION_GOOD_FOUND TRUE)
    endif (NOT WIN32)

    if (KGEOMAP_VERSION_GOOD_FOUND)
      set(KGEOMAP_DEFINITIONS "${PC_KGEOMAP_CFLAGS}")

      find_path(KGEOMAP_INCLUDE_DIR libkgeomap/version.h ${PC_KGEOMAP_INCLUDEDIR})
      set(kgeomap_version_h_filename "${KGEOMAP_INCLUDE_DIR}/libkgeomap/version.h")

      find_library(KGEOMAP_LIBRARIES NAMES kgeomap PATHS ${PC_KGEOMAP_LIBDIR})

      if (KGEOMAP_INCLUDE_DIR AND KGEOMAP_LIBRARIES)
        set(KGEOMAP_FOUND TRUE)
      else (KGEOMAP_INCLUDE_DIR AND KGEOMAP_LIBRARIES)
        set(KGEOMAP_FOUND FALSE)
      endif (KGEOMAP_INCLUDE_DIR AND KGEOMAP_LIBRARIES)
    endif (KGEOMAP_VERSION_GOOD_FOUND)

    if (KGEOMAP_FOUND)
      if (NOT KGeoMap_FIND_QUIETLY)
        message(STATUS "Found libkgeomap: ${KGEOMAP_LIBRARIES}")
      endif (NOT KGeoMap_FIND_QUIETLY)
    else (KGEOMAP_FOUND)
      if (KGeoMap_FIND_REQUIRED)
        if (NOT KGEOMAP_INCLUDE_DIR)
          message(FATAL_ERROR "Could NOT find libkgeomap header files.")
        else(NOT KGEOMAP_INCLUDE_DIR)
          message(FATAL_ERROR "Could NOT find libkgeomap library.")
        endif (NOT KGEOMAP_INCLUDE_DIR)
      endif (KGeoMap_FIND_REQUIRED)
    endif (KGEOMAP_FOUND)

  endif (KGEOMAP_LOCAL_FOUND)

  if (KGEOMAP_FOUND)
    # Find the version information, unless that was reported by pkg_search_module.
    if (NOT KGEOMAP_VERSION)
      file(READ "${kgeomap_version_h_filename}" kgeomap_version_h_content)
      # This is the line we are trying to find: static const char kgeomap_version[] = "1.22.4-beta_5+dfsg";
      string(REGEX REPLACE ".*char +kgeomap_version\\[\\] += +\"([^\"]+)\".*" "\\1" KGEOMAP_VERSION "${kgeomap_version_h_content}")
      unset(kgeomap_version_h_content)

    endif (NOT KGEOMAP_VERSION)
    unset(kgeomap_version_h_filename)
  endif (KGEOMAP_FOUND)

  if (KGEOMAP_FOUND)
    mark_as_advanced(KGEOMAP_INCLUDE_DIR KGEOMAP_LIBRARIES KGEOMAP_DEFINITIONS KGEOMAP_VERSION KGEOMAP_FOUND)
  else (KGEOMAP_FOUND)
    # The library was not found, reset all related variables.
    unset(KGEOMAP_INCLUDE_DIR)
    unset(KGEOMAP_LIBRARIES)
    unset(KGEOMAP_DEFINITIONS)
    unset(KGEOMAP_VERSION)
  endif (KGEOMAP_FOUND)

endif (KGEOMAP_INCLUDE_DIR AND KGEOMAP_LIBRARIES AND KGEOMAP_DEFINITIONS AND KGEOMAP_VERSION)
