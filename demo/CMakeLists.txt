#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

MACRO_OPTIONAL_FIND_PACKAGE(Kexiv2)

if(KEXIV2_FOUND)

    message(STATUS "kexiv2 found, the demo application will be compiled.")

    include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/../)
    include_directories(${KEXIV2_INCLUDE_DIR})

    add_definitions(${KEXIV2_DEFINITIONS})

    set(demo_sources
        demo-main.cpp
        mainwindow.cpp
        myimageitem.cpp
        mytreewidget.cpp
        dragdrophandler.cpp
    )

    KDE4_ADD_EXECUTABLE(libkgeomap_demo ${demo_sources})

    target_link_libraries(libkgeomap_demo
                          kgeomap
                          ${KEXIV2_LIBRARIES}
                          ${QT_QTGUI_LIBRARY}
                          ${KDE4_KDECORE_LIBS}
                          ${KDE4_UI_LIBS}
                          ${KDE4_KPARTS_LIBS}
                          ${KDE4_KHTML_LIBS}
                         )

    install(TARGETS libkgeomap_demo ${INSTALL_TARGETS_DEFAULT_ARGS})

else()

     message(STATUS "kexiv2 NOT found, the demo application will NOT be compiled.")

endif()
