cmake_minimum_required(VERSION 3.0)

#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(libkgeomap)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Starting CMake configuration for: ${PROJECT_NAME}")

set(REQUIRED_KF_VERSION "5.18.0")
set(REQUIRED_QT_VERSION "5.7.0")


# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0     (During GoSC2010)
# 2.0.0 => 1.0.0     (Included into 2.0.0 Software Collection)
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

find_package(ECM ${REQUIRED_KF_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${libkgeomap_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMPoQmTools)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(FeatureSummary)

############## Find Packages ###################

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS
             Core
             WebKitWidgets
             Widgets
             Gui
             Xml
             Concurrent
)

find_package(KF5 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS
             I18n
             Config
             KIO
)

find_package(Marble CONFIG REQUIRED)

ecm_setup_version(${LIBKGEOMAP_LIB_VERSION}
                  VARIABLE_PREFIX      KGEOMAP
                  VERSION_HEADER       "libkgeomap_version.h"
                  PACKAGE_VERSION_FILE "KF5KGeoMapConfigVersion.cmake"
                  SOVERSION            ${LIBKGEOMAP_SO_VERSION}
)

############## Targets #########################

add_definitions(-DTRANSLATION_DOMAIN=\"libkgeomap\")
add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(data)
add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(calibrator)
    add_subdirectory(demo)
endif()

############## CMake Config Files ##############

message("${CMAKECONFIG_INSTALL_DIR}")
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5KGeoMap")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/KF5KGeoMapConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5KGeoMapConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KF5KGeoMapConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KF5KGeoMapConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT   Devel
)

install(EXPORT      KF5KGeoMapTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE        KF5KGeoMapTargets.cmake
        NAMESPACE   KF5::
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
