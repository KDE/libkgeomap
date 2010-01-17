# * ============================================================
# *
# * Copyright (C) 2009,2010 by Michael G. Hansen <mike at mghansen dot de>
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

INCLUDE(CTest)
ENABLE_TESTING()

SET( MY_COMPILER_WARNINGS
    "-Wall -Wold-style-cast -Wextra -pedantic"
)

find_package( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )
FIND_PACKAGE(MarbleWidget REQUIRED)
FIND_PACKAGE(Kexiv2 REQUIRED)
INCLUDE_DIRECTORIES(
    ${KEXIV2_INCLUDE_DIR}
)

ADD_DEFINITIONS(-Wall)
INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE("valgrind/valgrind.h" HAVE_VALGRIND)
IF(${HAVE_VALGRIND})
    ADD_DEFINITIONS(-DWMW2_HAVE_VALGRIND)
ENDIF(${HAVE_VALGRIND})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/demo
    ${CMAKE_CURRENT_SOURCE_DIR}/worldmapwidget2
    ${CMAKE_CURRENT_SOURCE_DIR}/backend-marble
    ${CMAKE_CURRENT_SOURCE_DIR}/backend-googlemaps
)

SET( widget_sources
    worldmapwidget2/worldmapwidget2.cpp
    worldmapwidget2/map-backend.cpp
    worldmapwidget2/markermodel.cpp
)

SET( backend_marble_sources
    backend-marble/backend-marble.cpp
    backend-marble/bm-widget.cpp
)

SET( backend_googlemaps_sources
    backend-googlemaps/backend-googlemaps.cpp
    backend-googlemaps/bgm_widget.cpp
)

KDE4_ADD_LIBRARY( worldmapwidget2library STATIC ${widget_sources} ${backend_marble_sources} ${backend_googlemaps_sources})
SET( worldmapwidget2library_libs
    ${KDE4_UI_LIBS} ${KDE4_KPARTS_LIBS} ${LIBMARBLEWIDGET_LIBRARY} ${KDE4_KPARTS_LIBS} ${KDE4_KHTML_LIBS}
)
target_link_libraries( worldmapwidget2library ${KDE4_UI_LIBS} ${KDE4_KPARTS_LIBS} ${LIBMARBLEWIDGET_LIBRARY} ${KDE4_KPARTS_LIBS} ${KDE4_KHTML_LIBS} )
SET_TARGET_PROPERTIES( worldmapwidget2library PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
SET( worldmapwidget2library_libs
    worldmapwidget2library ${worldmapwidget2library_libs}
)

# demo widget
SET( demo_widget_sources
    demo-widget/demo-main.cpp
    demo-widget/mainwindow.cpp
)
kde4_add_executable( demo_widget  ${demo_widget_sources} )
target_link_libraries( demo_widget ${worldmapwidget2library_libs} ${KEXIV2_LIBRARIES})
SET_TARGET_PROPERTIES( demo_widget PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")

# test the marker model
SET( test_model_sources
    tests/test-model.cpp
)
kde4_add_executable( test_model ${test_model_sources} )
target_link_libraries( test_model ${worldmapwidget2library_libs} ${QT_QTTEST_LIBRARY} )
SET_TARGET_PROPERTIES( test_model PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
ADD_TEST( test_model ${EXECUTABLE_OUTPUT_PATH}/test_model )
