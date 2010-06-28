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

SET( MY_COMPILER_WARNINGS
    "-Wall -Wextra -pedantic"
    # -Wold-style-cast
)

find_package( KDE4 REQUIRED )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )
FIND_PACKAGE(MarbleWidget REQUIRED)
FIND_PACKAGE(Kexiv2 REQUIRED)
INCLUDE_DIRECTORIES(
    ${KEXIV2_INCLUDE_DIR}
)

INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE("valgrind/valgrind.h" HAVE_VALGRIND)
IF(${HAVE_VALGRIND})
    ADD_DEFINITIONS(-DWMW2_HAVE_VALGRIND)
ENDIF(${HAVE_VALGRIND})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/demo
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

SET( widget_sources
    libs/worldmapwidget2.cpp
    libs/map-backend.cpp
    libs/html_widget.cpp
    libs/markermodel.cpp
    libs/worldmapwidget2_primitives.cpp
    libs/altitude-backend.cpp
    libs/worldmapwidget2_dragdrophandler.cpp

    libs/backend-altitude-geonames.cpp
)

SET( backend_marble_sources
    libs/backend-marble.cpp
)
IF (${KDE_VERSION} VERSION_GREATER "4.4.66")
    ADD_DEFINITIONS(-DWMW2_MARBLE_ADD_LAYER)
    SET( backend_marble_sources ${backend_marble_sources} libs/backend-marble-layer.cpp )
ELSE (${KDE_VERSION} VERSION_GREATER "4.4.66")
    SET( backend_marble_sources ${backend_marble_sources} libs/backend-marble-subwidget.cpp )
ENDIF (${KDE_VERSION} VERSION_GREATER "4.4.66")


SET( backend_googlemaps_sources
    libs/backend-googlemaps.cpp
)

SET( backend_osm_sources
#     libs/backend-osm.cpp
)

KDE4_ADD_LIBRARY( worldmapwidget2 SHARED ${widget_sources} ${backend_marble_sources} ${backend_googlemaps_sources} ${backend_osm_sources})
# TODO: soname version numbers of the library
SET( worldmapwidget2_libs
    ${KDE4_UI_LIBS} ${KDE4_KPARTS_LIBS} ${LIBMARBLEWIDGET_LIBRARY} ${KDE4_KHTML_LIBS}
)
target_link_libraries( worldmapwidget2 ${worldmapwidget2_libs} )
SET_TARGET_PROPERTIES( worldmapwidget2 PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
SET(worldmapwidget2_libs
    worldmapwidget2 ${worldmapwidget2_libs}
)

INSTALL(TARGETS worldmapwidget2 ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES data/backend-osm.html
              data/backend-osm-js.js
              data/backend-googlemaps.html
              data/backend-googlemaps-js.js
              data/cluster-circle-00ff00.png
              data/cluster-circle-00ffff.png
              data/cluster-circle-ff0000.png
              data/cluster-circle-ff7f00.png
              data/cluster-circle-ffff00.png
              data/cluster-circle-00ff00-someselected.png
              data/cluster-circle-00ffff-someselected.png
              data/cluster-circle-ff0000-someselected.png
              data/cluster-circle-ff7f00-someselected.png
              data/cluster-circle-ffff00-someselected.png
              data/cluster-circle-00ff00-selected.png
              data/cluster-circle-00ffff-selected.png
              data/cluster-circle-ff0000-selected.png
              data/cluster-circle-ff7f00-selected.png
              data/cluster-circle-ffff00-selected.png
              data/marker-00ff00.png
              data/marker-00ffff.png
              data/marker-ff0000.png
              data/marker-ff7f00.png
              data/marker-ffff00.png
              data/marker-00ff00-someselected.png
              data/marker-00ffff-someselected.png
              data/marker-ff0000-someselected.png
              data/marker-ff7f00-someselected.png
              data/marker-ffff00-someselected.png
              data/marker-00ff00-selected.png
              data/marker-00ffff-selected.png
              data/marker-ff0000-selected.png
              data/marker-ff7f00-selected.png
              data/marker-ffff00-selected.png
        DESTINATION ${DATA_INSTALL_DIR}/libworldmapwidget2/)

SET(worldmapwidget2_headers
    libs/worldmapwidget2_export.h
    libs/worldmapwidget2.h
    libs/worldmapwidget2_dragdrophandler.h
    libs/worldmapwidget2_primitives.h
    libs/altitude-backend.h
)
INSTALL(FILES ${worldmapwidget2_headers}
        DESTINATION ${INCLUDE_INSTALL_DIR}/worldmapwidget2 COMPONENT Devel)

# # demo widget
 SET( demo_sources
     demo/demo-main.cpp
     demo/mainwindow.cpp
     demo/myimageitem.cpp
     demo/mytreewidget.cpp
     demo/dragdrophandler.cpp
 )
 kde4_add_executable( worldmapwidget2_demo  ${demo_sources} )
 target_link_libraries( worldmapwidget2_demo ${worldmapwidget2_libs} ${KEXIV2_LIBRARIES})
 SET_TARGET_PROPERTIES( worldmapwidget2_demo PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
 INSTALL(TARGETS worldmapwidget2_demo ${INSTALL_TARGETS_DEFAULT_ARGS})

IF(KDE4_BUILD_TESTS)

    INCLUDE(CTest)
    ENABLE_TESTING()

    # test the marker model
    SET( test_model_sources
        tests/test-model.cpp
    )
    kde4_add_executable( test_model ${test_model_sources} )
    target_link_libraries( test_model worldmapwidget2 ${worldmapwidget2_libs} ${QT_QTTEST_LIBRARY} )
    SET_TARGET_PROPERTIES( test_model PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
    ADD_TEST( test_model ${EXECUTABLE_OUTPUT_PATH}/test_model )

    # test the marker model
    SET( test_primitives_sources
        tests/test-primitives.cpp
    )
    kde4_add_executable( test_primitives ${test_primitives_sources} )
    target_link_libraries( test_primitives worldmapwidget2 ${worldmapwidget2_libs} ${QT_QTTEST_LIBRARY} )
    SET_TARGET_PROPERTIES( test_primitives PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
    ADD_TEST( test_primitives ${EXECUTABLE_OUTPUT_PATH}/test_primitives )

ENDIF(KDE4_BUILD_TESTS)
