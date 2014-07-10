# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2010-06-29
# @brief  world maps library interface for KDE
#
# @author Copyright (C) 2010-2014 by Gilles Caulier
#         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
# @author Copyright (C) 2009-2010 by Michael G. Hansen
#         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# ============================================================

PROJECT(libkgeomap)

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "Starting CMake configuration for: libkgeomap")

FIND_PACKAGE(Qt4 4.6.0 REQUIRED)
FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)
INCLUDE(MacroOptionalFindPackage)
INCLUDE(FindPackageHandleStandardArgs)

# Special handling for Marble: support both FindMarbleWidget and FindMarble,
# assuming FindMarble is the way of the future.
FIND_PACKAGE(Marble)
IF (NOT MARBLE_FOUND)
  FIND_PACKAGE(MarbleWidget REQUIRED)
  SET(MARBLE_INCLUDE_DIR ${MARBLEWIDGET_INCLUDE_DIR})
  SET(MARBLE_LIBRARIES ${LIBMARBLEWIDGET_LIBRARY})
ENDIF (NOT MARBLE_FOUND)

MACRO_OPTIONAL_FIND_PACKAGE(Kexiv2)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
INCLUDE_DIRECTORIES(${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

SET(LIBKGEOMAP_AREA_CODE_GENERAL 51006)
ADD_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=${LIBKGEOMAP_AREA_CODE_GENERAL})

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0     (During digiKam GoSC2010)
# 2.0.0 => 1.0.0     (Included into digiKam 2.0.0 Software Collection)

# Library API version
SET(KGEOMAP_LIB_MAJOR_VERSION "2")
SET(KGEOMAP_LIB_MINOR_VERSION "0")
SET(KGEOMAP_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
SET(KGEOMAP_LIB_SUFFIX_VERSION "")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
SET(KGEOMAP_LIB_SO_CUR_VERSION "1")
SET(KGEOMAP_LIB_SO_REV_VERSION "0")
SET(KGEOMAP_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

SET(KGEOMAP_LIB_VERSION_STRING "${KGEOMAP_LIB_MAJOR_VERSION}.${KGEOMAP_LIB_MINOR_VERSION}.${KGEOMAP_LIB_PATCH_VERSION}${KGEOMAP_LIB_SUFFIX_VERSION}")
SET(KGEOMAP_LIB_VERSION_ID "0x0${KGEOMAP_LIB_MAJOR_VERSION}0${KGEOMAP_LIB_MINOR_VERSION}0${KGEOMAP_LIB_PATCH_VERSION}")
SET(KGEOMAP_LIB_SO_VERSION_STRING "${KGEOMAP_LIB_SO_CUR_VERSION}.${KGEOMAP_LIB_SO_REV_VERSION}.${KGEOMAP_LIB_SO_AGE_VERSION}")

# =======================================================

SUBDIRS(libkgeomap)
SUBDIRS(data)

IF(KDE4_BUILD_TESTS)
    MACRO_OPTIONAL_ADD_SUBDIRECTORY(tests)
ENDIF(KDE4_BUILD_TESTS)

#SUBDIRS(calibrator)
# IF(KEXIV2_FOUND)
#     SUBDIRS(demo)
#     MESSAGE(STATUS "kexiv2 found, the demo application will be compiled.")
# ELSE(KEXIV2_FOUND)
#     MESSAGE(STATUS "kexiv2 NOT found, the demo application will NOT be compiled.")
# ENDIF(KEXIV2_FOUND)

IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/libkgeomap.pc.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.pc)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/libkgeomap.lsm.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.lsm)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
ENDIF(NOT WIN32)

# Install find cmake script to the system for client applications.

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/FindKGeoMap.cmake
        DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
