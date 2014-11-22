#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(libkgeomap)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Starting CMake configuration for: ${PROJECT_NAME}")

set(CMAKE_MIN_VERSION   "2.8.12")
set(ECM_MIN_VERSION     "1.1.0")
set(KF5_VERSION         "5.0.0")
set(REQUIRED_QT_VERSION "5.2.0")

cmake_minimum_required(VERSION ${CMAKE_MIN_VERSION})

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0     (During digiKam GoSC2010)
# 2.0.0 => 1.0.0     (Included into digiKam 2.0.0 Software Collection)
# 3.0.0 => 2.0.0     (Including track manager, see bug #333622)
# 3.1.0 => 2.1.0     (Clean up API to reduce broken binary compatibility. Prepare code for KF5 port)
# 5.0.0 => 10.0.0    (Released with KDE 5.x)

# Library API version
set(KGEOMAP_LIB_MAJOR_VERSION "5")
set(KGEOMAP_LIB_MINOR_VERSION "0")
set(KGEOMAP_LIB_PATCH_VERSION "0")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
set(KGEOMAP_LIB_SO_CUR_VERSION "10")
set(KGEOMAP_LIB_SO_REV_VERSION "0")
set(KGEOMAP_LIB_SO_AGE_VERSION "0")

set(LIBKGEOMAP_LIB_VERSION "${KGEOMAP_LIB_MAJOR_VERSION}.${KGEOMAP_LIB_MINOR_VERSION}.${KGEOMAP_LIB_PATCH_VERSION}")
set(LIBKGEOMAP_SO_VERSION  "${KGEOMAP_LIB_SO_CUR_VERSION}.${KGEOMAP_LIB_SO_REV_VERSION}.${KGEOMAP_LIB_SO_AGE_VERSION}")

############## ECM setup ######################

find_package(ECM ${ECM_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${libkgeomap_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(GenerateExportHeader)
include(FeatureSummary)

############## Find Packages ###################

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS
             Core
             Widgets
             Gui
             Xml
             Concurrent
)

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS
             KDELibs4Support
             KHtml
             Parts
)

# Special handling for Marble: support both FindMarbleWidget and FindMarble,
# assuming FindMarble is the way of the future.
find_package(Marble)

if(NOT MARBLE_FOUND)
    find_package(MarbleWidget REQUIRED)
    set(MARBLE_INCLUDE_DIR ${MARBLEWIDGET_INCLUDE_DIR})
    set(MARBLE_LIBRARIES   ${LIBMARBLEWIDGET_LIBRARY})
endif()

ecm_setup_version(${LIBKGEOMAP_LIB_VERSION}
                  VARIABLE_PREFIX      KGEOMAP
                  VERSION_HEADER       "${CMAKE_CURRENT_BINARY_DIR}/libkgeomap_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KGeomapConfigVersion.cmake"
                  SOVERSION            ${LIBKGEOMAP_SO_VERSION}
)

############## Targets #########################

add_definitions(-DTRANSLATION_DOMAIN=\"libkgeomap\")
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING) # for porting

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

add_subdirectory(data)
add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(calibrator)
    add_subdirectory(demo)
endif()

############## CMake Config Files ##############

message("${CMAKECONFIG_INSTALL_DIR}")
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Kgeomap")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/KF5KGeomapConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5KGeomapConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KF5KGeomapConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KF5KGeomapConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT   Devel
)

install(EXPORT      KF5KGeomapTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE        KF5KGeomapTargets.cmake
        NAMESPACE   KF5::
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
