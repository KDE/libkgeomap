# - Try to find the KMap library
#
# If you have put a local version of libkmap into your source tree,
# set KMAP_LOCAL_DIR to the relative path to the local directory.
#
# Once done this will define
#
#  KMAP_FOUND - system has libkmap
#  KMAP_INCLUDE_DIR - the libkmap include directory
#  KMAP_LIBRARIES - Link these to use libkmap
#  KMAP_DEFINITIONS - Compiler switches required for using libkmap
#

# Copyright (c) 2010, Gilles Caulier, <caulier.gilles@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES AND KMAP_DEFINITIONS)

  MESSAGE(STATUS "Found Kmap library in cache: ${KMAP_LIBRARIES}")

  # in cache already
  SET(KMAP_FOUND TRUE)

ELSE (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES AND KMAP_DEFINITIONS)

  MESSAGE(STATUS "Check Kmap library in local sub-folder...")

  # Check if library is not in local sub-folder

  IF (KMAP_LOCAL_DIR)
   SET(KMAP_LOCAL_FOUND TRUE)
  ELSE (KMAP_LOCAL_DIR)
    FIND_FILE(KMAP_LOCAL_FOUND libkmap/version.h.cmake ${CMAKE_SOURCE_DIR}/libkmap ${CMAKE_SOURCE_DIR}/libs/libkmap NO_DEFAULT_PATH)

    IF (KMAP_LOCAL_FOUND)
      FIND_FILE(KMAP_LOCAL_FOUND_IN_LIBS libkmap/version.h.cmake ${CMAKE_SOURCE_DIR}/libs/libkmap NO_DEFAULT_PATH)
        IF (KMAP_LOCAL_FOUND_IN_LIBS)
          SET(KMAP_LOCAL_DIR libs/libkmap)
        ELSE (KMAP_LOCAL_FOUND_IN_LIBS)
          SET(KMAP_LOCAL_DIR libkmap)
        ENDIF (KMAP_LOCAL_DIR)
    ENDIF (KMAP_LOCAL_FOUND)
  ENDIF (KMAP_LOCAL_DIR)

  IF (KMAP_LOCAL_FOUND)

    # we need two include directories: because the version.h file is put into the build directory
    # TODO KMAP_INCLUDE_DIR sounds like it should contain only one directory...
    SET(KMAP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${KMAP_LOCAL_DIR} ${CMAKE_BINARY_DIR}/${KMAP_LOCAL_DIR})
    SET(KMAP_DEFINITIONS "-I${CMAKE_SOURCE_DIR}/${KMAP_LOCAL_DIR}" "-I${CMAKE_BINARY_DIR}/${KMAP_LOCAL_DIR}")
    SET(KMAP_LIBRARIES kmap)
    MESSAGE(STATUS "Found Kmap library in local sub-folder: ${CMAKE_SOURCE_DIR}/${KMAP_LOCAL_DIR}")
    SET(KMAP_FOUND TRUE)
    MARK_AS_ADVANCED(KMAP_INCLUDE_DIR KMAP_LIBRARIES KMAP_DEFINITIONS)

  ELSE(KMAP_LOCAL_FOUND)
    IF(NOT WIN32) 
      MESSAGE(STATUS "Check Kmap library using pkg-config...")

      # use pkg-config to get the directories and then use these values
      # in the FIND_PATH() and FIND_LIBRARY() calls
      INCLUDE(UsePkgConfig)

      PKGCONFIG(libkmap _KMAPIncDir _KMAPLinkDir _KMAPLinkFlags _KMAPCflags)

      IF(_KMAPLinkFlags)
        # query pkg-config asking for a libkmap >= 0.1.0
        EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=0.1.0 libkmap RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
        IF(_return_VALUE STREQUAL "0")
            MESSAGE(STATUS "Found libkmap release >= 0.1.0")
            SET(KMAP_VERSION_GOOD_FOUND TRUE)
        ELSE(_return_VALUE STREQUAL "0")
            MESSAGE(STATUS "Found libkmap release < 0.1.0, too old")
            SET(KMAP_VERSION_GOOD_FOUND FALSE)
            SET(KMAP_FOUND FALSE)
        ENDIF(_return_VALUE STREQUAL "0")
      ELSE(_KMAPLinkFlags)
        SET(KMAP_VERSION_GOOD_FOUND FALSE)
        SET(KMAP_FOUND FALSE)
      ENDIF(_KMAPLinkFlags)
    ELSE(NOT WIN32)
      SET(KMAP_VERSION_GOOD_FOUND TRUE)
    ENDIF(NOT WIN32)

    IF(KMAP_VERSION_GOOD_FOUND)
        SET(KMAP_DEFINITIONS "${_KMAPCflags}")

        FIND_PATH(KMAP_INCLUDE_DIR libkmap/version.h
        ${_KMAPIncDir}
        )

        FIND_LIBRARY(KMAP_LIBRARIES NAMES kmap
        PATHS
        ${_KMAPLinkDir}
        )

        IF (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES)
            SET(KMAP_FOUND TRUE)
        ENDIF (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES)
      ENDIF(KMAP_VERSION_GOOD_FOUND) 
      IF (KMAP_FOUND)
          IF (NOT Kmap_FIND_QUIETLY)
              MESSAGE(STATUS "Found libkmap: ${KMAP_LIBRARIES}")
          ENDIF (NOT Kmap_FIND_QUIETLY)
      ELSE (KMAP_FOUND)
          IF (Kmap_FIND_REQUIRED)
              IF (NOT KMAP_INCLUDE_DIR)
                  MESSAGE(FATAL_ERROR "Could NOT find libkmap header files")
              ENDIF (NOT KMAP_INCLUDE_DIR)
              IF (NOT KMAP_LIBRARIES)
                  MESSAGE(FATAL_ERROR "Could NOT find libkmap library")
              ENDIF (NOT KMAP_LIBRARIES)
          ENDIF (Kmap_FIND_REQUIRED)
      ENDIF (KMAP_FOUND)

    MARK_AS_ADVANCED(KMAP_INCLUDE_DIR KMAP_LIBRARIES KMAP_DEFINITIONS)

  ENDIF(KMAP_LOCAL_FOUND)

ENDIF (KMAP_INCLUDE_DIR AND KMAP_LIBRARIES AND KMAP_DEFINITIONS)
