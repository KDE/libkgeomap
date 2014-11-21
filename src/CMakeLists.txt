#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(${MARBLE_INCLUDE_DIR})

include(CheckIncludeFile)
CHECK_INCLUDE_FILE("valgrind/valgrind.h" HAVE_VALGRIND)
if(${HAVE_VALGRIND})
    add_definitions(-DKGEOMAP_HAVE_VALGRIND)
endif()

set(widget_LIB_SRCS abstractmarkertiler.cpp
                    backend_map.cpp
                    dragdrophandler.cpp
                    geocoordinates.cpp
                    html_widget.cpp
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

set(backend_map_marble_LIB_SRCS
    backend_map_marble.cpp
)

add_definitions(-DKGEOMAP_MARBLE_ADD_LAYER)
set(backend_map_marble_LIB_SRCS ${backend_map_marble_sources}
                                backend_map_marble_layer.cpp
)

#obsolete with KF5
#    set(backend_map_marble_LIB_SRCS ${backend_map_marble_sources} backend_map_marble_subwidget.cpp)

set(backend_map_googlemaps_LIB_SRCS
    backend_map_googlemaps.cpp
)

set(backend_map_osm_LIB_SRCS
    #backend_map_osm.cpp
)

ecm_generate_headers(kgeomap_CamelCase_HEADERS

                     HEADER_NAMES
                     AbstractMarkerTiler
                     DragDropHandler
                     GeoCoordinates
                     ItemMarkerTiler
                     ModelHelper
                     TileIndex
                     Tracks
#                     KGeoMapWidget
#                     LookupAltitude
#                     LookupFactory
#                     TrackReader

                     PREFIX           KGEOMAP
                     REQUIRED_HEADERS kgeomap_HEADERS
)

add_library(KF5KGeomap SHARED ${widget_LIB_SRCS} 
                              ${backend_map_marble__LIB_SRCS}
                              ${backend_map_googlemaps_LIB_SRCS}
                              ${backend_map_osm_LIB_SRCS}
)

add_library(KF5::KGeomap ALIAS KF5KGeomap)

generate_export_header(KF5KGeomap BASE_NAME libkgeomap EXPORT_MACRO_NAME LIBKGEOMAP_EXPORT)

# Check if compilation is done through digiKam SC or not. If yes, do not install header files.
if (NOT DIGIKAMSC_COMPILE_LIBKGEOMAP)

    install(FILES       ${kgeomap_CamelCase_HEADERS}
            DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KGEOMAP/KGEOMAP
            COMPONENT   Devel
    )

    install(FILES       ${kgeomap_HEADERS}
            DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KGEOMAP/kgeomap
            COMPONENT   Devel
    )

    install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap_export.h
            DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KGEOMAP/kgeomap
            COMPONENT   Devel
    )

    install(FILES       "${CMAKE_BINARY_DIR}/libkgeomap_version.h"
            DESTINATION "${KF5_INCLUDE_INSTALL_DIR}"
            COMPONENT   Devel
    )

endif()

target_include_directories(KF5KGeomap INTERFACE
                           "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KGEOMAP;${KF5_INCLUDE_INSTALL_DIR}/KGEOMAP/kgeomap>"
)

target_link_libraries(KF5KGeomap
                      PRIVATE
                      ${MARBLE_LIBRARIES}
                      PUBLIC
                      KF5::KDELibs4Support
                      KF5::Parts
                      KF5::KHtml
                      Qt5::Gui
                      Qt5::Core
)

install(TARGETS KF5KGeomap
        EXPORT  KF5KGeomapTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

set_target_properties(KF5KGeomap  PROPERTIES
                      VERSION     ${KGEOMAP_VERSION_STRING}
                      SOVERSION   ${KGEOMAP_SOVERSION}
                      EXPORT_NAME KGeomap
)
