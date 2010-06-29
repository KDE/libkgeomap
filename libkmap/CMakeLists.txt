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

INCLUDE_DIRECTORIES( ${KEXIV2_INCLUDE_DIR}
                     ${LIBMARBLEWIDGET_INCLUDE_DIR}
                   )

INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE("valgrind/valgrind.h" HAVE_VALGRIND)
IF(${HAVE_VALGRIND})
    ADD_DEFINITIONS(-DWMW2_HAVE_VALGRIND)
ENDIF(${HAVE_VALGRIND})

ADD_DEFINITIONS(${KEXIV2_DEFINITIONS})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

SET(widget_sources
    worldmapwidget2.cpp
    map-backend.cpp
    html_widget.cpp
    markermodel.cpp
    worldmapwidget2_primitives.cpp
    altitude-backend.cpp
    worldmapwidget2_dragdrophandler.cpp
    backend-altitude-geonames.cpp
   )

SET(backend_marble_sources
    backend-marble.cpp
   )

IF (${KDE_VERSION} VERSION_GREATER "4.4.66")
    ADD_DEFINITIONS(-DWMW2_MARBLE_ADD_LAYER)
    SET( backend_marble_sources ${backend_marble_sources} backend-marble-layer.cpp )
ELSE (${KDE_VERSION} VERSION_GREATER "4.4.66")
    SET( backend_marble_sources ${backend_marble_sources} backend-marble-subwidget.cpp )
ENDIF (${KDE_VERSION} VERSION_GREATER "4.4.66")

SET(backend_googlemaps_sources
    backend-googlemaps.cpp
   )

SET(backend_osm_sources
    #backend-osm.cpp
   )

KDE4_ADD_LIBRARY(kmap SHARED ${widget_sources} 
                             ${backend_marble_sources}
                             ${backend_googlemaps_sources}
                             ${backend_osm_sources}
                )

TARGET_LINK_LIBRARIES(kmap ${QT_QTGUI_LIBRARY}
                           ${KDE4_KDECORE_LIBS}
                           ${KDE4_UI_LIBS}
                           ${KDE4_KPARTS_LIBS}
                           ${KDE4_KHTML_LIBS}
                           ${KEXIV2_LIBRARIES}
                           ${LIBMARBLEWIDGET_LIBRARY}
                           )

SET_TARGET_PROPERTIES(kmap PROPERTIES VERSION ${KFACE_LIB_SO_VERSION_STRING}
                           SOVERSION ${KFACE_LIB_SO_CUR_VERSION})

INSTALL(TARGETS kmap ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES worldmapwidget2_export.h
              worldmapwidget2.h
              worldmapwidget2_dragdrophandler.h
              worldmapwidget2_primitives.h
              altitude-backend.h
              ${CMAKE_CURRENT_BINARY_DIR}/version.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libkmap COMPONENT Devel
       )
