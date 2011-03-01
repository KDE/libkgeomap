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
# either version 2, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# ============================================================

INCLUDE_DIRECTORIES(${LIBMARBLEWIDGET_INCLUDE_DIR})

INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE("valgrind/valgrind.h" HAVE_VALGRIND)
IF(${HAVE_VALGRIND})
    ADD_DEFINITIONS(-DKMAP_HAVE_VALGRIND)
ENDIF(${HAVE_VALGRIND})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

SET(widget_sources
    abstractmarkertiler.cpp
    backend_altitude.cpp
    backend_altitude_geonames.cpp
    backend_map.cpp
    dragdrophandler.cpp
    html_widget.cpp
    itemmarkertiler.cpp
    kmap_widget.cpp
    kmap_common.cpp
    modelhelper.cpp
    placeholderwidget.cpp
    tilegrouper.cpp
    tileindex.cpp
   )

SET(backend_map_marble_sources
    backend_map_marble.cpp
   )

IF (${KDE_VERSION} VERSION_GREATER "4.4.66")
    ADD_DEFINITIONS(-DKMAP_MARBLE_ADD_LAYER)
    SET( backend_map_marble_sources ${backend_map_marble_sources} backend_map_marble_layer.cpp )
ELSE (${KDE_VERSION} VERSION_GREATER "4.4.66")
    SET( backend_map_marble_sources ${backend_map_marble_sources} backend_map_marble_subwidget.cpp )
ENDIF (${KDE_VERSION} VERSION_GREATER "4.4.66")

SET(backend_map_googlemaps_sources
    backend_map_googlemaps.cpp
   )

SET(backend_map_osm_sources
    #backend_map_osm.cpp
   )

KDE4_ADD_LIBRARY(kmap SHARED ${widget_sources} 
                             ${backend_map_marble_sources}
                             ${backend_map_googlemaps_sources}
                             ${backend_map_osm_sources}
                )

TARGET_LINK_LIBRARIES(kmap ${QT_QTGUI_LIBRARY}
                           ${KDE4_KDECORE_LIBS}
                           ${KDE4_UI_LIBS}
                           ${KDE4_KPARTS_LIBS}
                           ${KDE4_KHTML_LIBS}
                           ${LIBMARBLEWIDGET_LIBRARY}
                           )

SET_TARGET_PROPERTIES(kmap PROPERTIES VERSION ${KMAP_LIB_SO_VERSION_STRING}
                           SOVERSION ${KMAP_LIB_SO_CUR_VERSION})

INSTALL(TARGETS kmap ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES libkmap_export.h
              abstractmarkertiler.h
              backend_altitude.h
              dragdrophandler.h
              geocoordinates.h
              itemmarkertiler.h
              kmap_primitives.h
              kmap_widget.h
              modelhelper.h
              tileindex.h
              ${CMAKE_CURRENT_BINARY_DIR}/version.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libkmap COMPONENT Devel
       )
