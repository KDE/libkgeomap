#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(libkgeomap)

cmake_minimum_required(VERSION 2.8.12)

# ECM setup
find_package(ECM 1.1.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${libkgeomap_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)

set(KF5_VERSION "5.1.0")

############## Find Packages ##############
set(REQUIRED_QT_VERSION "5.2.0")


if (false)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Starting CMake configuration for: libkgeomap")

find_package(Qt4 4.6.0 REQUIRED)
find_package(KDE4 REQUIRED)

include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)
include(MacroOptionalFindPackage)
include(FindPackageHandleStandardArgs)

# Special handling for Marble: support both FindMarbleWidget and FindMarble,
# assuming FindMarble is the way of the future.
find_package(Marble)

if(NOT MARBLE_FOUND)
    find_package(MarbleWidget REQUIRED)
    set(MARBLE_INCLUDE_DIR ${MARBLEWIDGET_INCLUDE_DIR})
    set(MARBLE_LIBRARIES ${LIBMARBLEWIDGET_LIBRARY})
endif()

add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(LIBKGEOMAP_AREA_CODE_GENERAL 51006)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=${LIBKGEOMAP_AREA_CODE_GENERAL})

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0     (During digiKam GoSC2010)
# 2.0.0 => 1.0.0     (Included into digiKam 2.0.0 Software Collection)
# 3.0.0 => 2.0.0     (Including track manager, see bug #333622)
# 3.1.0 => 2.1.0     (Clean up API to reduce broken binary compatibility. Prepare code for KF5 port) 

# Library API version
set(KGEOMAP_LIB_MAJOR_VERSION "3")
set(KGEOMAP_LIB_MINOR_VERSION "1")
set(KGEOMAP_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
set(KGEOMAP_LIB_SUFFIX_VERSION "")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
set(KGEOMAP_LIB_SO_CUR_VERSION "2")
set(KGEOMAP_LIB_SO_REV_VERSION "1")
set(KGEOMAP_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

set(KGEOMAP_LIB_VERSION_STRING    "${KGEOMAP_LIB_MAJOR_VERSION}.${KGEOMAP_LIB_MINOR_VERSION}.${KGEOMAP_LIB_PATCH_VERSION}${KGEOMAP_LIB_SUFFIX_VERSION}")
set(KGEOMAP_LIB_VERSION_ID        "0x0${KGEOMAP_LIB_MAJOR_VERSION}0${KGEOMAP_LIB_MINOR_VERSION}0${KGEOMAP_LIB_PATCH_VERSION}")
set(KGEOMAP_LIB_SO_VERSION_STRING "${KGEOMAP_LIB_SO_CUR_VERSION}.${KGEOMAP_LIB_SO_REV_VERSION}.${KGEOMAP_LIB_SO_AGE_VERSION}")

# =======================================================

subdirs(libkgeomap)
subdirs(data)

if(KDE4_BUILD_TESTS)
    MACRO_OPTIONAL_ADD_SUBDIRECTORY(tests)
    MACRO_OPTIONAL_ADD_SUBDIRECTORY(calibrator)
    MACRO_OPTIONAL_ADD_SUBDIRECTORY(demo)
endif()

if(NOT WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/libkgeomap.pc.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.pc)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/libkgeomap.lsm.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.lsm)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
endif()

# Install find cmake script to the system for client applications.

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/FindKGeoMap.cmake
        DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
endif()
