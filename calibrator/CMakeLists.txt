# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2010-09-18
# @brief  world maps library interface for KDE
#
# @author Copyright (C) 2010 by Gilles Caulier
#         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
# @author Copyright (C) 2009-2010 by Michael G. Hansen
#         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# ============================================================

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/../)

SET(calibrator_sources
        calibrator.cpp
    )

KDE4_ADD_EXECUTABLE(libkmap_calibrator ${calibrator_sources})
TARGET_LINK_LIBRARIES(libkmap_calibrator
                        kmap
                        ${QT_QTGUI_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        ${KDE4_UI_LIBS}
                        ${KDE4_KPARTS_LIBS}
                        ${KDE4_KHTML_LIBS}
                    )
