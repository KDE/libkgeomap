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

#include "test-model.moc"

// Qt includes

#include <QStandardItemModel>

using namespace WMW2;

const int CoordinatesRole = Qt::UserRole + 0;

const WMWGeoCoordinate coord_1_2 = WMWGeoCoordinate::fromGeoUrl("geo:1,2");
const WMWGeoCoordinate coord_50_60 = WMWGeoCoordinate::fromGeoUrl("geo:50,60");
const WMWGeoCoordinate coord_m50_m60 = WMWGeoCoordinate::fromGeoUrl("geo:-50,-60");

QStandardItem* MakeItemAt(const WMWGeoCoordinate& coordinates)
{
    QStandardItem* const newItem = new QStandardItem(coordinates.geoUrl());
    newItem->setData(QVariant::fromValue(coordinates), CoordinatesRole);

    return newItem;
}

/**
 * @brief Helper function: count the number of markers found by an iterator
 */
int CountMarkersInIterator(MarkerModel::NonEmptyIterator* const it)
{
    int markerCount = 0;
    while (!it->atEnd())
    {
        const MarkerModel::TileIndex currentIndex = it->currentIndex();
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
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_1_2, l);
        QVERIFY(tileIndex.level() == l);
    }
}

void TestModel::testAddMarkers1()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);

    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    // there should be no tiles in the model yet:
    for (int l = 0; l<=maxLevel; ++l)
    {
        QVERIFY(mm.getTile(MarkerModel::TileIndex::fromCoordinates(coord_50_60, l), true) == 0);
    }

    itemModel->appendRow(MakeItemAt(coord_50_60));

    // now there should be tiles with one marker:
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.count()==0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

    itemModel->appendRow(MakeItemAt(coord_50_60));

    // now there should be tiles with two markers:
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
    }
}

void TestModel::testRemoveMarkers2()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);

    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    itemModel->appendRow(MakeItemAt(coord_50_60));
    QStandardItem* const item2 = MakeItemAt(coord_50_60);
    itemModel->appendRow(item2);

    // now there should be tiles with two markers:
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
    }

    // remove one item:
    qDeleteAll(itemModel->takeRow(itemModel->indexFromItem(item2).row()));
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
    }
}

void TestModel::testMoveMarkers1()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    const int fillLevel = maxLevel - 2;

    // add a marker to the model and create tiles up to a certain level:
    QStandardItem* const item1 = MakeItemAt(coord_1_2);
    itemModel->appendRow(item1);
    const QModelIndex markerIndex1 = itemModel->indexFromItem(item1);
    
    WMW2_ASSERT(markerIndex1.isValid());
    for (int l = 1; l<=fillLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_1_2, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(myTile->children.count(), 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
    }

    // now move marker 1:
    mm.moveMarker(markerIndex1, coord_50_60);

    for (int l = 0; l<=fillLevel; ++l)
    {
        // make sure the marker can not be found at the old position any more
        MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_1_2, l);
        QVERIFY(mm.getTile(tileIndex, true) == 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 0);
        QVERIFY(mm.getTile(tileIndex, true) == 0);

        // find it at the new position:
        tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.isEmpty());
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

//     mm.clear();
}

void TestModel::testMoveMarkers2()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    const int fillLevel = maxLevel - 2;

    // add markers to the model and create tiles up to a certain level:
    QStandardItem* const item1 = MakeItemAt(coord_1_2);
    itemModel->appendRow(item1);
    const QModelIndex markerIndex1 = itemModel->indexFromItem(item1);
    QStandardItem* const item2 = MakeItemAt(coord_1_2);
    itemModel->appendRow(item2);
    const QModelIndex markerIndex2 = itemModel->indexFromItem(item2);
    for (int l = 1; l<=fillLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_1_2, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.count()==0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 2);
    }
    QStandardItem* const item3 = MakeItemAt(coord_50_60);
    itemModel->appendRow(item3);
    const QModelIndex markerIndex3 = itemModel->indexFromItem(item3);
    for (int l = 1; l<=fillLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->children.count()==0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

    // now move marker 1:
    mm.moveMarker(markerIndex1, coord_50_60);

    // make sure the item model was also updated:
    QVERIFY(item1->data(CoordinatesRole).value<WMWGeoCoordinate>() == coord_50_60);

    for (int l = 0; l<=fillLevel; ++l)
    {
        // make sure there is only one marker left at the old position:
        MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_1_2, l);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);

        // find it at the new position:
        tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        if (l>fillLevel)
        {
            QVERIFY(myTile->children.isEmpty());
        }
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 2);
    }

//     mm.clear();
}

void TestModel::testIteratorWholeWorldNoBackingModel()
{
    MarkerModel mm;
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    for (int l = 0; l<=maxLevel; ++l)
    {
        MarkerModel::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 0 );
    }
}

void TestModel::testIteratorWholeWorld()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    for (int l = 0; l<=maxLevel; ++l)
    {
        MarkerModel::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 0 );
    }

    itemModel->appendRow(MakeItemAt(coord_1_2));
    itemModel->appendRow(MakeItemAt(coord_50_60));
    for (int l = 0; l<=maxLevel; ++l)
    {
        // iterate over the whole world:
        MarkerModel::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 2 );
    }
}

void TestModel::testIteratorPartial1()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    itemModel->appendRow(MakeItemAt(coord_1_2));
    itemModel->appendRow(MakeItemAt(coord_50_60));
    for (int l = 0; l<=maxLevel; ++l)
    {
        {
            // iterate over a part which should be empty:
            WMWGeoCoordinate::PairList boundsList;
            boundsList << WMWGeoCoordinate::makePair(-10.0, -10.0, -5.0, -5.0);
            MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 0 );
        }

        {
            // iterate over a part which should contain one marker:
            WMWGeoCoordinate::PairList boundsList;
            boundsList << WMWGeoCoordinate::makePair(-10.0, -10.0, 5.0, 5.0);
            MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 1 );

            // iterate over a part which should contain one marker:
            WMWGeoCoordinate::PairList boundsList1;
            boundsList1 << WMWGeoCoordinate::makePair(1.0, 2.0, 5.0, 5.0);
            MarkerModel::NonEmptyIterator it1(&mm, l, boundsList1);
            QVERIFY( CountMarkersInIterator(&it1) == 1 );

            WMWGeoCoordinate::PairList boundsList2;
            boundsList2 << WMWGeoCoordinate::makePair(-1.0, -2.0, 1.0, 2.0);
            MarkerModel::NonEmptyIterator it2(&mm, l, boundsList2);
            QVERIFY( CountMarkersInIterator(&it2) == 1 );
        }

        {
            // iterate over a part which should contain two markers:
            WMWGeoCoordinate::PairList boundsList;
            boundsList << WMWGeoCoordinate::makePair(0.0, 0.0, 60.0, 60.0);
            MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 2 );
        }

        {
            // iterate over two parts which should contain two markers:
            WMWGeoCoordinate::PairList boundsList;
            boundsList << WMWGeoCoordinate::makePair(0.0, 0.0, 5.0, 5.0);
            boundsList << WMWGeoCoordinate::makePair(49.0, 59.0, 51.0, 61.0);
            MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 2 );
        }
    }

    const WMWGeoCoordinate coord_2_2 = WMWGeoCoordinate(2.0, 2.0);
    itemModel->appendRow(MakeItemAt(coord_2_2));
    {
        // at level 1, the iterator should find only one marker:
        WMWGeoCoordinate::PairList boundsList;
        boundsList << WMWGeoCoordinate::makePair(0.0, 0.0, 1.0, 2.0);
        MarkerModel::NonEmptyIterator it(&mm, 1, boundsList);
        QVERIFY( CountMarkersInIterator(&it) == 1 );
    }
}

void TestModel::testIteratorPartial2()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    WMWGeoCoordinate::PairList boundsList;
    boundsList << WMWGeoCoordinate::makePair(0.55, 1.55, 0.56, 1.56);

    const WMWGeoCoordinate coordInBounds1 = WMWGeoCoordinate(0.556, 1.556);
    const WMWGeoCoordinate coordOutOfBounds1 = WMWGeoCoordinate(0.5, 1.5);
    const WMWGeoCoordinate coordOutOfBounds2 = WMWGeoCoordinate(0.5, 1.6);
    const WMWGeoCoordinate coordOutOfBounds3 = WMWGeoCoordinate(0.6, 1.5);
    const WMWGeoCoordinate coordOutOfBounds4 = WMWGeoCoordinate(0.6, 1.6);
    itemModel->appendRow(MakeItemAt(coordInBounds1));
    itemModel->appendRow(MakeItemAt(coordOutOfBounds1));
    itemModel->appendRow(MakeItemAt(coordOutOfBounds2));
    itemModel->appendRow(MakeItemAt(coordOutOfBounds3));
    itemModel->appendRow(MakeItemAt(coordOutOfBounds4));

    for (int l = 3; l<=maxLevel; ++l)
    {
        MarkerModel::NonEmptyIterator it(&mm, l, boundsList);
        QVERIFY( CountMarkersInIterator(&it) == 1 );
    }
}

void TestModel::testRemoveMarkers1()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    for (int l = 0; l<=maxLevel; ++l)
    {
        MarkerModel::NonEmptyIterator it(&mm, l);
        QVERIFY(CountMarkersInIterator(&it) == 0 );
    }

    QStandardItem* const item1 = MakeItemAt(coord_1_2);
    itemModel->appendRow(item1);
    itemModel->appendRow(MakeItemAt(coord_50_60));
    for (int l = 0; l<=maxLevel; ++l)
    {
        // iterate over the whole world:
        MarkerModel::NonEmptyIterator it(&mm, l);
        QCOMPARE(CountMarkersInIterator(&it), 2);
    }

    // first make sure that comparison of indices still works
    const QPersistentModelIndex index1 = itemModel->indexFromItem(item1);
    const QPersistentModelIndex index2 = itemModel->indexFromItem(item1);
    QCOMPARE(index1, index2);

    // now remove items:
    QCOMPARE(itemModel->takeRow(itemModel->indexFromItem(item1).row()).count(), 1);
    delete item1;
    QCOMPARE(itemModel->rowCount(), 1);
    for (int l = 0; l<=maxLevel; ++l)
    {
        // iterate over the whole world:
        MarkerModel::NonEmptyIterator it(&mm, l);
        QCOMPARE(CountMarkersInIterator(&it), 1);
    }
}

/**
 * @brief Make sure that items which are in the model before it is given to the tiled model are found by the tile model
 */
void TestModel::testPreExistingMarkers()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    itemModel->appendRow(MakeItemAt(coord_50_60));
    
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);
    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    for (int l = 0; l<=maxLevel; ++l)
    {
        // iterate over the whole world:
        MarkerModel::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 1 );
    }
}

void TestModel::testSelectionState1()
{
    const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    MarkerModel mm;
    mm.setMarkerModel(itemModel.data(), CoordinatesRole);

    const int maxLevel = MarkerModel::TileIndex::MaxLevel;

    QStandardItem* const item1 = MakeItemAt(coord_50_60);
    item1->setSelectable(true);
    itemModel->appendRow(item1);
    QModelIndex item1Index = itemModel->indexFromItem(item1);

    // verify the selection state of the tiles:
    // make sure we do not create tiles all the way down,
    // because we want to test whether the state is okay in newly created tiles
    const int preMaxLevel = maxLevel -2;
    for (int l = 0; l<=preMaxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedNone);
    }

    QItemSelectionModel* const selectionModel = new QItemSelectionModel(itemModel.data());
    mm.setSelectionModel(selectionModel);
    selectionModel->select(item1Index, QItemSelectionModel::Select);

    // verify the selection state of the tiles:
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedAll);
        QVERIFY(mm.getTileSelectedCount(tileIndex)==1);
    }

    // add an unselected item and make sure the tilecount is still correct
    QStandardItem* const item2 = MakeItemAt(coord_50_60);
    item2->setSelectable(true);
    itemModel->appendRow(item2);
    const QPersistentModelIndex item2Index = itemModel->indexFromItem(item2);
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedSome);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
    }

    selectionModel->select(item2Index, QItemSelectionModel::Select);
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 2);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedAll);
    }

    // now remove the selected item:
    QCOMPARE(itemModel->takeRow(item2Index.row()).count(), 1);
    delete item2;
    // verify the selection state of the tiles:
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedAll);
    }

    // add a selected item and then move it:
    QStandardItem* const item3 = MakeItemAt(coord_1_2);
    item3->setSelectable(true);
    itemModel->appendRow(item3);
    const QPersistentModelIndex item3Index = itemModel->indexFromItem(item3);
    selectionModel->select(item3Index, QItemSelectionModel::Select);
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_1_2, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedAll);
    }
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedAll);
    }
    mm.moveMarker(item3Index, coord_50_60);
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 2);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedAll);
    }
    mm.moveMarker(item3Index, coord_m50_m60);
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_50_60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedAll);
    }
    for (int l = 0; l<=maxLevel; ++l)
    {
        const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromCoordinates(coord_m50_m60, l);
        MarkerModel::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileSelectedState(tileIndex)==WMWSelectedAll);
    }

    // TODO: set a model with selected items, make sure the selections are read out
    //       this is currently implemented by simply setting the tiles as dirty
}

void TestModel::benchmarkIteratorWholeWorld()
{
    QBENCHMARK
    {
        const QSharedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
        MarkerModel mm;
        mm.setMarkerModel(itemModel.data(), CoordinatesRole);
        const int maxLevel = MarkerModel::TileIndex::MaxLevel;

        for (int l = 0; l<=maxLevel; ++l)
        {
            MarkerModel::NonEmptyIterator it(&mm, l);
            QVERIFY( CountMarkersInIterator(&it) == 0 );
        }

        itemModel->appendRow(MakeItemAt(coord_1_2));
        itemModel->appendRow(MakeItemAt(coord_50_60));
        for (int x=-50; x<50; ++x)
        {
            for (int y=-50; y<50; ++y)
            {
                itemModel->appendRow(MakeItemAt(WMW2::WMWGeoCoordinate(x,y)));
            }
        }
        for (int l = 0; l<=maxLevel; ++l)
        {
            // iterate over the whole world:
            MarkerModel::NonEmptyIterator it(&mm, l);
        }
    }
}

QTEST_MAIN(TestModel)

