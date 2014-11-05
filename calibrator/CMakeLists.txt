#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/../)

set(calibrator_sources
        calibrator.cpp
   )

KDE4_ADD_EXECUTABLE(libkgeomap_calibrator ${calibrator_sources})

target_link_libraries(libkgeomap_calibrator
                       kgeomap
                       ${QT_QTGUI_LIBRARY}
                       ${KDE4_KDECORE_LIBS}
                       ${KDE4_UI_LIBS}
                       ${KDE4_KPARTS_LIBS}
                       ${KDE4_KHTML_LIBS}
                     )
