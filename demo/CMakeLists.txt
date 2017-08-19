#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(DETECT_LIBKEXIV2 MIN_VERSION)

    message(STATUS "libkexiv2 : search system based library")
    find_package(KF5KExiv2 ${MIN_VERSION})

    if(KF5KExiv2_FOUND)
        set(KF5KExiv2_LIBRARIES KF5::KExiv2)
    else()
        set(KF5KExiv2_FOUND FALSE)
    endif()

    message(STATUS "libkexiv2 found       : ${KF5KExiv2_FOUND}")
    message(STATUS "libkexiv2 include dir : ${KF5KExiv2_INCLUDE_DIRS}")
    message(STATUS "libkexiv2 library     : ${KF5KExiv2_LIBRARIES}")

endmacro()

DETECT_LIBKEXIV2("5.0.0")

if(KF5KExiv2_FOUND)

    message(STATUS "KExiv2 shared lib found, the demo application will be compiled.")

    include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/../)
    include_directories(${KF5KExiv2_INCLUDE_DIRS})

    set(demo_SRCS
        demo-main.cpp
        myimageitem.cpp
        mytreewidget.cpp
        dragdrophandler.cpp
        mainwindow.cpp
        ../src/kgeomap_common.cpp
        ../src/libkgeomap_debug.cpp
        ../src/backends/mapbackend.cpp
    )

    add_executable(libkgeomap_demo ${demo_SRCS})

    target_link_libraries(libkgeomap_demo
        KF5KGeoMap
        ${KF5KExiv2_LIBRARIES}

        Qt5::Gui
        Qt5::Concurrent

        KF5::ConfigCore
        KF5::CoreAddons
        KF5::I18n
    )

    install(TARGETS libkgeomap_demo ${INSTALL_TARGETS_DEFAULT_ARGS})

else()

    message(STATUS "KExiv2 shared lib NOT found, the demo application will NOT be compiled.")

endif()
