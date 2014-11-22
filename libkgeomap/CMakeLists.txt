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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

set(widget_sources
    abstractmarkertiler.cpp
    backend_map.cpp
    dragdrophandler.cpp
    geocoordinates.cpp
    html_widget.cpp
    itemmarkertiler.cpp
    kgeomap_widget.cpp
    kgeomap_common.cpp
    kgeomap_primitives.cpp
    lookup_altitude.cpp
    lookup_altitude_geonames.cpp
    lookup_factory.cpp
    modelhelper.cpp
    placeholderwidget.cpp
    tilegrouper.cpp
    tileindex.cpp
    tracks.cpp
    track_reader.cpp
   )

set(backend_map_marble_sources
    backend_map_marble.cpp
   )

if(${KDE_VERSION} VERSION_GREATER "4.4.66")
    add_definitions(-DKGEOMAP_MARBLE_ADD_LAYER)
    set(backend_map_marble_sources ${backend_map_marble_sources} backend_map_marble_layer.cpp)
else()
    set(backend_map_marble_sources ${backend_map_marble_sources} backend_map_marble_subwidget.cpp)
endif()

set(backend_map_googlemaps_sources
    backend_map_googlemaps.cpp
   )

set(backend_map_osm_sources
    #backend_map_osm.cpp
   )

KDE4_ADD_LIBRARY(kgeomap SHARED ${widget_sources} 
                             ${backend_map_marble_sources}
                             ${backend_map_googlemaps_sources}
                             ${backend_map_osm_sources}
                )

target_link_libraries(kgeomap ${QT_QTGUI_LIBRARY}
                           ${KDE4_KDECORE_LIBS}
                           ${KDE4_UI_LIBS}
                           ${KDE4_KPARTS_LIBS}
                           ${KDE4_KHTML_LIBS}
                           ${MARBLE_LIBRARIES}
                           )

set_target_properties(kgeomap PROPERTIES VERSION ${KGEOMAP_LIB_SO_VERSION_STRING}
                      SOVERSION ${KGEOMAP_LIB_SO_CUR_VERSION})

install(TARGETS kgeomap ${INSTALL_TARGETS_DEFAULT_ARGS})

# Check if compilation is done through digiKam SC or not. If yes, do not install header files.
if(NOT DIGIKAMSC_COMPILE_LIBKGEOMAP)

    install(FILES libkgeomap_export.h
                  abstractmarkertiler.h
                  dragdrophandler.h
                  geocoordinates.h
                  itemmarkertiler.h
                  kgeomap_primitives.h
                  kgeomap_widget.h
                  lookup_altitude.h
                  lookup_factory.h
                  modelhelper.h
                  tileindex.h
                  tracks.h
                  track_reader.h
                  ${CMAKE_CURRENT_BINARY_DIR}/version.h
            DESTINATION ${INCLUDE_INSTALL_DIR}/libkgeomap COMPONENT Devel
           )

    if(NOT WIN32)
        configure_file(${CMAKE_SOURCE_DIR}/cmake/templates/libkgeomap.pc.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.pc)
        configure_file(${CMAKE_SOURCE_DIR}/cmake/templates/libkgeomap.lsm.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.lsm)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libkgeomap.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
    endif()

    # Install find cmake script to the system for client applications.

    install(FILES ${CMAKE_SOURCE_DIR}/cmake/modules/FindKGeoMap.cmake
            DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

endif()
