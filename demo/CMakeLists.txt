#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(KF5KExiv2)

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
    )

    add_executable(libkgeomap_demo ${demo_SRCS})

    target_link_libraries(libkgeomap_demo
                          Qt5::Gui
                          KF5::Parts
                          KF5::KHtml
                          KF5::KGeoMap
                          KF5::KExiv2
                         )

    install(TARGETS libkgeomap_demo ${INSTALL_TARGETS_DEFAULT_ARGS})

else()

    message(STATUS "KExiv2 shared lib NOT found, the demo application will NOT be compiled.")

endif()
