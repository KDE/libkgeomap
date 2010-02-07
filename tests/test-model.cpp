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

// local includes

#include "test-model.h"

using namespace WMW2;

/**
 * @brief Helper function: count the number of markers found by an iterator
 */
int CountMarkersInIterator(MarkerModel::NonEmptyIterator* const it)
{
    int markerCount = 0;
    while (!it->atEnd())
    {
        const QIntList currentIndex = it->currentIndex();
        markerCount += it->model()->getTileMarkerCount(currentIndex);
        it->nextIndex();
//         kDebug()<<currentIndex;
    }

    return markerCount;
}

void TestModel::testNoOp()
{
}

void TestModel::testIndices()
{
    MarkerModel mm;
    const int maxLevel = mm.maxLevel();

    const WMWGeoCoordinate coord_1_2 = WMWGeoCoordinate(1.0, 2.0);

    for (int l = 0; l<=maxLevel; ++l)
    {
        const QIntList tileIndex = mm.coordinateToTileIndex(coord_1_2, l);
        QVERIFY( tileIndex.count() == (l+1));
    }
}

void TestModel::testBasicModel()
{
    MarkerModel mm;

    const int maxLevel = mm.maxLevel();

    const WMWGeoCoordinate coord_1_2 = WMWGeoCoordinate(1.0, 2.0);

    // there should be no tiles in the model yet:
    for (int l = 0; l<=maxLevel; ++l)
    {
        QVERIFY(mm.getTile(mm.coordinateToTileIndex(coord_1_2, l), true) == 0);
    }

    mm.addMarker(WMWMarker(coord_1_2));

    // now there should be tiles with one marker:
    for (int l = 0; l<=maxLevel; ++l)
    {
        const QIntList tileIndex = mm.coordinateToTileIndex(coord_1_2, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.count()==0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }
}

void TestModel::testMoveMarkers1()
{
    MarkerModel mm;
    const int maxLevel = mm.maxLevel();

    const int fillLevel = maxLevel - 2;

    // add a marker to the model and create tiles up to a certain level:
    const WMWGeoCoordinate coord_1_2 = WMWGeoCoordinate(1.0, 2.0);
    const int markerIndex1 = mm.addMarker(WMWMarker(coord_1_2));
    for (int l = 1; l<=fillLevel; ++l)
    {
        const QIntList tileIndex = mm.coordinateToTileIndex(coord_1_2, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.count()==0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

    // now move marker 1:
    const WMWGeoCoordinate coord_50_60 = WMWGeoCoordinate(50.0, 60.0);
    mm.moveMarker(markerIndex1, coord_50_60);

    for (int l = 0; l<=fillLevel; ++l)
    {
        // make sure the marker can not be found at the old position any more
        QIntList tileIndex = mm.coordinateToTileIndex(coord_1_2, l);
        QVERIFY(mm.getTile(tileIndex, true) == 0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 0);
        QVERIFY(mm.getTile(tileIndex, true) == 0);

        // find it at the new position:
        tileIndex = mm.coordinateToTileIndex(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.isEmpty());
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

    mm.clear();
}

void TestModel::testMoveMarkers2()
{
    MarkerModel mm;
    const int maxLevel = mm.maxLevel();

    const int fillLevel = maxLevel - 2;
    const WMWGeoCoordinate coord_1_2 = WMWGeoCoordinate(1.0, 2.0);
    const WMWGeoCoordinate coord_50_60 = WMWGeoCoordinate(50.0, 60.0);

    // add markers to the model and create tiles up to a certain level:
    const int markerIndex1 = mm.addMarker(WMWMarker(coord_1_2));
    const int markerIndex2 = mm.addMarker(WMWMarker(coord_1_2));
    for (int l = 1; l<=fillLevel; ++l)
    {
        const QIntList tileIndex = mm.coordinateToTileIndex(coord_1_2, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.count()==0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 2);
    }
    const int markerIndex3 = mm.addMarker(WMWMarker(coord_50_60));
    for (int l = 1; l<=fillLevel; ++l)
    {
        const QIntList tileIndex = mm.coordinateToTileIndex(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.count()==0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

    // now move marker 1:
    mm.moveMarker(markerIndex1, coord_50_60);

    for (int l = 0; l<=fillLevel; ++l)
    {
        // make sure there is only one marker left at the old position:
        QIntList tileIndex = mm.coordinateToTileIndex(coord_1_2, l);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);

        // find it at the new position:
        tileIndex = mm.coordinateToTileIndex(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        if (l>fillLevel)
        {
            QVERIFY(myTile->children.isEmpty());
        }
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 2);
    }

    mm.clear();
}

void TestModel::testIteratorWholeWorld()
{
    MarkerModel mm;
    const int maxLevel = mm.maxLevel();

    for (int l = 0; l<=maxLevel; ++l)
    {
        MarkerModel::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 0 );
    }

    const WMWGeoCoordinate coord_1_2 = WMWGeoCoordinate(1.0, 2.0);
    const WMWGeoCoordinate coord_50_60 = WMWGeoCoordinate(50.0, 60.0);
    mm.addMarker(WMWMarker(coord_1_2));
    mm.addMarker(WMWMarker(coord_50_60));
    for (int l = 0; l<=maxLevel; ++l)
    {
        // iterate over the whole world:
        MarkerModel::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 2 );
    }
}

void TestModel::testIteratorPartial1()
{
    MarkerModel mm;
    const int maxLevel = mm.maxLevel();

    const WMWGeoCoordinate coord_1_2 = WMWGeoCoordinate(1.0, 2.0);
    const WMWGeoCoordinate coord_50_60 = WMWGeoCoordinate(50.0, 60.0);
    mm.addMarker(WMWMarker(coord_1_2));
    mm.addMarker(WMWMarker(coord_50_60));
    for (int l = 0; l<=maxLevel; ++l)
    {
        {
            // iterate over a part which should be empty:
            QList<WMWGeoCoordinate::Pair> boundsList;
            boundsList << WMWGeoCoordinate::pair(-10.0, -10.0, -5.0, -5.0);
            MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 0 );
        }

        {
            // iterate over a part which should contain one marker:
            QList<WMWGeoCoordinate::Pair> boundsList;
            boundsList << WMWGeoCoordinate::pair(-10.0, -10.0, 5.0, 5.0);
            MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 1 );

            // iterate over a part which should contain one marker:
            QList<WMWGeoCoordinate::Pair> boundsList1;
            boundsList1 << WMWGeoCoordinate::pair(1.0, 2.0, 5.0, 5.0);
            MarkerModel::NonEmptyIterator it1(&mm, l, boundsList1);
            QVERIFY( CountMarkersInIterator(&it1) == 1 );

            QList<WMWGeoCoordinate::Pair> boundsList2;
            boundsList2 << WMWGeoCoordinate::pair(-1.0, -2.0, 1.0, 2.0);
            MarkerModel::NonEmptyIterator it2(&mm, l, boundsList2);
            QVERIFY( CountMarkersInIterator(&it2) == 1 );
        }

        {
            // iterate over a part which should contain two markers:
            QList<WMWGeoCoordinate::Pair> boundsList;
            boundsList << WMWGeoCoordinate::pair(0.0, 0.0, 60.0, 60.0);
            MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 2 );
        }

        {
            // iterate over two parts which should contain two markers:
            QList<WMWGeoCoordinate::Pair> boundsList;
            boundsList << WMWGeoCoordinate::pair(0.0, 0.0, 5.0, 5.0);
            boundsList << WMWGeoCoordinate::pair(49.0, 59.0, 51.0, 61.0);
            MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 2 );
        }
    }

    const WMWGeoCoordinate coord_2_2 = WMWGeoCoordinate(2.0, 2.0);
    mm.addMarker(WMWMarker(coord_2_2));
    {
        // at level 1, the iterator should find only one marker:
        QList<WMWGeoCoordinate::Pair> boundsList;
        boundsList << WMWGeoCoordinate::pair(0.0, 0.0, 1.0, 2.0);
        MarkerModel::NonEmptyIterator it(&mm, 1, boundsList);
        QVERIFY( CountMarkersInIterator(&it) == 1 );
    }
}

void TestModel::testIteratorPartial2()
{
    MarkerModel mm;
    const int maxLevel = mm.maxLevel();

    QList<WMWGeoCoordinate::Pair> boundsList;
    boundsList << WMWGeoCoordinate::pair(0.55, 1.55, 0.56, 1.56);

    const WMWGeoCoordinate coordInBounds1 = WMWGeoCoordinate(0.556, 1.556);
    const WMWGeoCoordinate coordOutOfBounds1 = WMWGeoCoordinate(0.5, 1.5);
    const WMWGeoCoordinate coordOutOfBounds2 = WMWGeoCoordinate(0.5, 1.6);
    const WMWGeoCoordinate coordOutOfBounds3 = WMWGeoCoordinate(0.6, 1.5);
    const WMWGeoCoordinate coordOutOfBounds4 = WMWGeoCoordinate(0.6, 1.6);
    mm.addMarker(WMWMarker(coordInBounds1));
    mm.addMarker(WMWMarker(coordOutOfBounds1));
    mm.addMarker(WMWMarker(coordOutOfBounds2));
    mm.addMarker(WMWMarker(coordOutOfBounds3));
    mm.addMarker(WMWMarker(coordOutOfBounds4));

    for (int l = 3; l<=maxLevel; ++l)
    {
        MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
        QVERIFY( CountMarkersInIterator(&it) == 1 );
    }
}

QTEST_MAIN(TestModel)

