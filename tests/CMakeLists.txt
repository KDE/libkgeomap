#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libkgeomap)

if(KDE4_BUILD_TESTS)

    include(CTest)
    enable_testing()

    # test the primitive datatypes and helper functions
    set(test_primitives_sources
        test_primitives.cpp
       )
    KDE4_ADD_EXECUTABLE(kgeomap_test_primitives ${test_primitives_sources})
    target_link_libraries(kgeomap_test_primitives kgeomap ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
    add_test(kgeomap_test_primitives ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_primitives)

    # test the GeoCoordinates class
    set(test_geocoordinates_sources
        test_geocoordinates.cpp
       )
    KDE4_ADD_EXECUTABLE(kgeomap_test_geocoordinates ${test_geocoordinates_sources})
    target_link_libraries(kgeomap_test_geocoordinates kgeomap ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
    add_test(kgeomap_test_geocoordinates ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_geocoordinates)

    # test the TileIndex class
    set(test_tileindex_sources
        test_tileindex.cpp
       )
    KDE4_ADD_EXECUTABLE(kgeomap_test_tileindex ${test_tileindex_sources})
    target_link_libraries(kgeomap_test_tileindex kgeomap ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})
    add_test(kgeomap_test_tileindex ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_tileindex)

    # test the LookupAltitudeGeonames class
    set(test_lookup_altitude_geonames_sources
        test_lookup_altitude_geonames.cpp
       )
    KDE4_ADD_EXECUTABLE(kgeomap_test_lookup_altitude_geonames ${test_lookup_altitude_geonames_sources})
    target_link_libraries(kgeomap_test_lookup_altitude_geonames kgeomap ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
    # do not add this as a test because it only works if there is an internet connection
    # add_test(kgeomap_test_lookup_altitude_geonames ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_lookup_altitude_geonames)

    if(NOT WIN32)
        # test the marker model
        set(test_itemmarkertiler_sources
            test_itemmarkertiler.cpp
           )
        KDE4_ADD_EXECUTABLE(kgeomap_test_itemmarkertiler ${test_itemmarkertiler_sources})
        target_link_libraries(kgeomap_test_itemmarkertiler kgeomap ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})
        add_test(kgeomap_test_itemmarkertiler ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_itemmarkertiler)
    endif()

    # test the track management classes
    set(test_tracks_sources
        test_tracks.cpp
       )
    KDE4_ADD_EXECUTABLE(kgeomap_test_tracks ${test_tracks_sources})
    target_link_libraries(kgeomap_test_tracks kgeomap ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})
    add_test(kgeomap_test_tracks ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_tracks)

    # track loading application for timing tests
    set(loadtrack_sources
        loadtrack.cpp
       )
    KDE4_ADD_EXECUTABLE(loadtrack ${loadtrack_sources})
    target_link_libraries(loadtrack kgeomap ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})

ENDIF()
