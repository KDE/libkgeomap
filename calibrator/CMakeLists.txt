#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/../)

set(calibrator_sources
        calibrator.cpp
   )

add_executable(libkgeomap_calibrator ${calibrator_sources})

target_link_libraries(libkgeomap_calibrator
                       KF5::KGeoMap
                       Qt5::Gui
                       KF5::Parts
                       KF5::KHtml
                     )
