# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2010-06-29
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
# either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# ============================================================

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../libkmap)

IF(KDE4_BUILD_TESTS)

    INCLUDE(CTest)
    ENABLE_TESTING()

    # test the marker model
    SET(test_model_sources
        test/test-model.cpp
       )
    KDE4_ADD_EXECUTABLE( test_model ${test_model_sources} )
    TARGET_LINK_LIBRARIES( test_model kmap ${QT_QTTEST_LIBRARY} )
    ADD_TEST( test_model ${EXECUTABLE_OUTPUT_PATH}/test_model )

    # test the marker model
    SET(test_primitives_sources
        test/test-primitives.cpp
       )
    KDE4_ADD_EXECUTABLE( test_primitives ${test_primitives_sources} )
    TARGET_LINK_LIBRARIES( test_primitives kmap ${QT_QTTEST_LIBRARY} )
    ADD_TEST( test_primitives ${EXECUTABLE_OUTPUT_PATH}/test_primitives )

ENDIF(KDE4_BUILD_TESTS)

