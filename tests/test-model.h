/* ============================================================
 *
 * Date        : 2010-01-16
 * Description : test for the model holding markers
 *
* Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TEST_MODEL_H
#define TEST_MODEL_H

// Qt includes

#include <QtTest/QtTest>

// KDE includes

// local includes

#include "markermodel.h"

class TestModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNoOp();
    void testIndices();
    void testAddMarkers1();
    void testRemoveMarkers1();
    void testRemoveMarkers2();
    void testMoveMarkers1();
    void testMoveMarkers2();
    void testIteratorWholeWorldNoBackingModel();
    void testIteratorWholeWorld();
    void testIteratorPartial1();
    void testIteratorPartial2();
    void testPreExistingMarkers();
    void testSelectionState1();
    void benchmarkIteratorWholeWorld();
};

#endif /* TEST_MODEL_H */

