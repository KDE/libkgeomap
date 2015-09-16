#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

include(CheckIncludeFile)
CHECK_INCLUDE_FILE("valgrind/valgrind.h" HAVE_VALGRIND)
if(${HAVE_VALGRIND})
    add_definitions(-DKGEOMAP_HAVE_VALGRIND)
endif()

add_subdirectory(backends)

set(widget_LIB_SRCS abstractmarkertiler.cpp
                    dragdrophandler.cpp
                    geocoordinates.cpp
                    itemmarkertiler.cpp
                    kgeomap_widget.cpp
                    kgeomap_common.cpp
                    kgeomap_primitives.cpp
                    libkgeomap_debug.cpp
                    lookup_altitude.cpp
                    lookup_altitude_geonames.cpp
                    lookup_factory.cpp
                    modelhelper.cpp
                    placeholderwidget.cpp
                    tilegrouper.cpp
                    tileindex.cpp
                    track_reader.cpp
                    tracks.cpp
)

ecm_generate_headers(kgeomap_CamelCase_HEADERS

                     HEADER_NAMES
                     AbstractMarkerTiler
                     DragDropHandler
                     GeoCoordinates
                     ItemMarkerTiler
                     ModelHelper
                     Lookup_Altitude
                     Lookup_Factory
                     TileIndex
                     Tracks
                     Track_Reader
                     KGeoMap_Widget
                     KGeoMap_Primitives

                     PREFIX           KGeoMap
                     REQUIRED_HEADERS kgeomap_HEADERS
)

add_library(KF5KGeoMap SHARED ${widget_LIB_SRCS})

add_library(KF5::KGeoMap ALIAS KF5KGeoMap)

generate_export_header(KF5KGeoMap BASE_NAME libkgeomap EXPORT_MACRO_NAME KGEOMAP_EXPORT)

target_include_directories(KF5KGeoMap INTERFACE
                           "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KGeoMap;${KF5_INCLUDE_INSTALL_DIR}/KGeoMap/kgeomap>"
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>"
)

target_include_directories(KF5KGeoMap PUBLIC "$<BUILD_INTERFACE:${libkgeomap_SOURCE_DIR}/src;${libkgeomap_BINARY_DIR}/src>")

target_link_libraries(KF5KGeoMap
    PUBLIC
        ${MARBLE_LIBRARIES}

        Qt5::Core
        Qt5::Gui
        Qt5::Xml
        Qt5::Widgets
    PRIVATE
        mapbackends

        KF5::I18n
        KF5::ConfigCore
        KF5::KIOCore

        Qt5::Concurrent
)

set_target_properties(KF5KGeoMap  PROPERTIES
                      VERSION     ${KGEOMAP_VERSION_STRING}
                      SOVERSION   ${KGEOMAP_SOVERSION}
                      EXPORT_NAME KGeoMap
)

install(TARGETS KF5KGeoMap
        EXPORT  KF5KGeoMapTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

# Check if compilation is done through digiKam SC or not. If yes, do not install header files.
if (NOT DIGIKAMSC_COMPILE_LIBKGEOMAP)

    install(FILES       ${kgeomap_CamelCase_HEADERS}
            DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KGeoMap/KGeoMap
            COMPONENT   Devel
    )

    install(FILES       ${kgeomap_HEADERS}
            DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KGeoMap/kgeomap
            COMPONENT   Devel
    )

    install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap_export.h
            DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KGeoMap/kgeomap
            COMPONENT   Devel
    )

    install(FILES       "${CMAKE_BINARY_DIR}/libkgeomap_version.h"
            DESTINATION "${KF5_INCLUDE_INSTALL_DIR}"
            COMPONENT   Devel
    )

endif()
