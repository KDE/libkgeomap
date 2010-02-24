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

ADD_DEFINITIONS(-Wall)
INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE("valgrind/valgrind.h" HAVE_VALGRIND)
IF(${HAVE_VALGRIND})
    ADD_DEFINITIONS(-DWMW2_HAVE_VALGRIND)
ENDIF(${HAVE_VALGRIND})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/demo
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

SET( widget_sources
    lib/worldmapwidget2.cpp
    lib/map-backend.cpp
    lib/html_widget.cpp
    lib/markermodel.cpp
    lib/worldmapwidget2_primitives.cpp
    lib/altitude-backend.cpp

    lib/backend-altitude-geonames.cpp
)

SET( backend_marble_sources
    lib/backend-marble.cpp
    lib/bm-widget.cpp
)

SET( backend_googlemaps_sources
    lib/backend-googlemaps.cpp
)

SET( backend_osm_sources
    lib/backend-osm.cpp
)

# TODO: we want to create a shared library
KDE4_ADD_LIBRARY( worldmapwidget2 STATIC ${widget_sources} ${backend_marble_sources} ${backend_googlemaps_sources} ${backend_osm_sources})
SET( worldmapwidget2_libs
    ${KDE4_UI_LIBS} ${KDE4_KPARTS_LIBS} ${LIBMARBLEWIDGET_LIBRARY} ${KDE4_KHTML_LIBS}
)
target_link_libraries( worldmapwidget2 ${KDE4_UI_LIBS} ${KDE4_KPARTS_LIBS} ${LIBMARBLEWIDGET_LIBRARY} ${KDE4_KHTML_LIBS} )
SET_TARGET_PROPERTIES( worldmapwidget2 PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
SET( worldmapwidget2_libs
    worldmapwidget2 ${worldmapwidget2_libs}
)

INSTALL(TARGETS worldmapwidget2 ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES lib-data/backend-osm.html
              lib-data/backend-osm-js.js
              lib-data/backend-googlemaps.html
              lib-data/backend-googlemaps-js.js
              lib-data/marker-green.png
              lib-data/cluster-circle-00ff00.png
              lib-data/cluster-circle-00ffff.png
              lib-data/cluster-circle-ff0000.png
              lib-data/cluster-circle-ff7f00.png
              lib-data/cluster-circle-ffff00.png
        DESTINATION ${DATA_INSTALL_DIR}/libworldmapwidget2/)

INSTALL(FILES lib/worldmapwidget2.h
              lib/worldmapwidget2_primitives.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libworldmapwidget2 COMPONENT Devel)

# demo widget
SET( demo_sources
    demo/demo-main.cpp
    demo/mainwindow.cpp
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
    target_link_libraries( test_model ${worldmapwidget2_libs} ${QT_QTTEST_LIBRARY} )
    SET_TARGET_PROPERTIES( test_model PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
    ADD_TEST( test_model ${EXECUTABLE_OUTPUT_PATH}/test_model )

        # test the marker model
    SET( test_primitives_sources
        tests/test-primitives.cpp
    )
    kde4_add_executable( test_primitives ${test_primitives_sources} )
    target_link_libraries( test_primitives ${worldmapwidget2_libs} ${QT_QTTEST_LIBRARY} )
    SET_TARGET_PROPERTIES( test_primitives PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} ${MY_COMPILER_WARNINGS}")
    ADD_TEST( test_primitives ${EXECUTABLE_OUTPUT_PATH}/test_primitives )

ENDIF(KDE4_BUILD_TESTS)
