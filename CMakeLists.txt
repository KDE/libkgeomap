# * ============================================================
# *
# * Copyright (C) 2009 by Michael G. Hansen http://www.mghansen.de/
# *
# * This program is free software; you can redistribute it
# * and/or modify it under the terms of the GNU General
# * Public License as published by the Free Software Foundation;
# * either version 2, or (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * ============================================================

project(worldmapwidget2)

find_package( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )
FIND_PACKAGE(MarbleWidget REQUIRED)

ADD_DEFINITIONS(-Wall)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/demo
    ${CMAKE_CURRENT_SOURCE_DIR}/worldmapwidget2
    ${CMAKE_CURRENT_SOURCE_DIR}/backend-marble
    ${CMAKE_CURRENT_SOURCE_DIR}/backend-googlemaps
)

SET( widget_sources
    worldmapwidget2/worldmapwidget2.cpp
    worldmapwidget2/map-backend.cpp
)

SET( backend_marble_sources
    backend-marble/backend-marble.cpp
)

SET( backend_googlemaps_sources
    backend-googlemaps/backend-googlemaps.cpp
    backend-googlemaps/bgm_widget.cpp
)

SET( test_sources
    demo/main.cpp
    demo/mainwindow.cpp
)


kde4_add_executable( test ${widget_sources} ${backend_marble_sources} ${backend_googlemaps_sources} ${demo_sources} )
target_link_libraries( test ${KDE4_UI_LIBS} ${KDE4_KPARTS_LIBS} ${LIBMARBLEWIDGET_LIBRARY} ${KDE4_KPARTS_LIBS} ${KDE4_KHTML_LIBS} )
SET_TARGET_PROPERTIES( test PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -Wall -Wold-style-cast -Wextra -pedantic")# -Weffc++")


