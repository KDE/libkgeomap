# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2010-06-29
# @brief  world maps library interface for KDE
#
# @author Copyright (C) 2010 by Gilles Caulier
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

PROJECT(libkmap)

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "Starting CMake configuration for: libkmap")

FIND_PACKAGE(Qt4 4.6.0 REQUIRED)
FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)
INCLUDE(MacroOptionalFindPackage)
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE(MarbleWidget REQUIRED)
MACRO_OPTIONAL_FIND_PACKAGE(Kexiv2)

ADD_DEFINITIONS (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
INCLUDE_DIRECTORIES (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

SET(LIBKMAP_AREA_CODE_GENERAL 51006)
ADD_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=${LIBKMAP_AREA_CODE_GENERAL})

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0     (During digiKam GoSC2010)
# 2.0.0 => 1.0.0     (Included into digiKam 2.0.0 Software Collection)

# Library API version
SET(KMAP_LIB_MAJOR_VERSION "2")
SET(KMAP_LIB_MINOR_VERSION "0")
SET(KMAP_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-svn"   : alpha code unstable from svn. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
SET(KMAP_LIB_SUFFIX_VERSION "")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
SET(KMAP_LIB_SO_CUR_VERSION "1")
SET(KMAP_LIB_SO_REV_VERSION "0")
SET(KMAP_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

SET(KMAP_LIB_VERSION_STRING "${KMAP_LIB_MAJOR_VERSION}.${KMAP_LIB_MINOR_VERSION}.${KMAP_LIB_PATCH_VERSION}${KMAP_LIB_SUFFIX_VERSION}")
SET(KMAP_LIB_VERSION_ID "0x0${KMAP_LIB_MAJOR_VERSION}0${KMAP_LIB_MINOR_VERSION}0${KMAP_LIB_PATCH_VERSION}")
SET(KMAP_LIB_SO_VERSION_STRING "${KMAP_LIB_SO_CUR_VERSION}.${KMAP_LIB_SO_REV_VERSION}.${KMAP_LIB_SO_AGE_VERSION}")

# =======================================================

SUBDIRS(libkmap)
SUBDIRS(test)
SUBDIRS(calibrator)
IF(KEXIV2_FOUND)
    SUBDIRS(demo)
    MESSAGE(STATUS "kexiv2 found, the demo application will be compiled.")
ELSE(KEXIV2_FOUND)
    MESSAGE(STATUS "kexiv2 NOT found, the demo application will NOT be compiled.")
ENDIF(KEXIV2_FOUND)
SUBDIRS(data)

IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libkmap.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkmap.pc)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libkmap.lsm.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkmap.lsm)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libkmap.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
ENDIF(NOT WIN32)

# Install find cmake script to the system for client applications.

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/FindKmap.cmake
        DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
