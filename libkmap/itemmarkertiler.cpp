/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-07-17
 * @brief  A marker tiler operating on item models
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "itemmarkertiler.moc"

namespace KMapIface
{

class ItemMarkerTiler::ItemMarkerTilerPrivate
{
public:
    ItemMarkerTilerPrivate()
      : modelHelper(0),
        selectionModel(0),
        markerModel(0)
    {
    }

    WMWModelHelper*      modelHelper;
    QItemSelectionModel* selectionModel;
    QAbstractItemModel*  markerModel;
};

ItemMarkerTiler::ItemMarkerTiler(WMWModelHelper* const modelHelper, QObject* const parent)
               : AbstractMarkerTiler(parent), d(new ItemMarkerTilerPrivate())
{
    setMarkerModelHelper(modelHelper);
}

ItemMarkerTiler::~ItemMarkerTiler()
{
    delete d;
}

QItemSelectionModel* ItemMarkerTiler::getSelectionModel() const
{
    return d->selectionModel;
}

QAbstractItemModel* ItemMarkerTiler::getModel() const
{
    return d->markerModel;
}

void ItemMarkerTiler::setMarkerModelHelper(WMWModelHelper* const modelHelper)
{
    d->modelHelper = modelHelper;
    d->markerModel = modelHelper->model();
    d->selectionModel = modelHelper->selectionModel();

    if (d->markerModel!=0)
    {
        // TODO: disconnect the old model if there was one
        connect(d->markerModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                this, SLOT(slotSourceModelRowsInserted(const QModelIndex&, int, int)));

        connect(d->markerModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotSourceModelRowsAboutToBeRemoved(const QModelIndex&, int, int)));

        connect(d->markerModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(slotSourceModelDataChanged(const QModelIndex&, const QModelIndex&)));

        connect(d->markerModel, SIGNAL(modelReset()),
                this, SLOT(slotSourceModelReset()));

        connect(d->modelHelper, SIGNAL(signalThumbnailAvailableForIndex(const QPersistentModelIndex&, const QPixmap&)),
                this, SLOT(slotThumbnailAvailableForIndex(const QPersistentModelIndex&, const QPixmap&)));

        if (d->selectionModel)
        {
            connect(d->selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));
        }
    }

    setDirty();
}

QVariant ItemMarkerTiler::getTileRepresentativeMarker(const AbstractMarkerTiler::TileIndex& tileIndex, const int sortKey)
{
    const QList<QPersistentModelIndex> modelIndices = getTileMarkerIndices(tileIndex);
    if (modelIndices.isEmpty())
        return QVariant();

    return QVariant::fromValue(d->modelHelper->bestRepresentativeIndexFromList(modelIndices, sortKey));
}

QPixmap ItemMarkerTiler::pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size)
{
    return d->modelHelper->pixmapFromRepresentativeIndex(index.value<QPersistentModelIndex>(), size);
}

QVariant ItemMarkerTiler::bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey)
{
    QList<QPersistentModelIndex> indexList;
    for (int i=0; i<indices.count(); ++i)
    {
        indexList << indices.at(i).value<QPersistentModelIndex>();
    }
    return QVariant::fromValue(d->modelHelper->bestRepresentativeIndexFromList(indexList, sortKey));
}

void ItemMarkerTiler::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
//     kDebug()<<selected<<deselected;
    if (isDirty())
    {
        return;
    }
//     d->isDirty=true;
//     emit(signalTilesOrSelectionChanged());
//     return;

    for (int i=0; i<selected.count(); ++i)
    {
        const QItemSelectionRange selectionRange = selected.at(i);
        for (int row = selectionRange.top(); row<=selectionRange.bottom(); ++row)
        {
            // get the coordinates of the item
            WMWGeoCoordinate coordinates;
            if (!d->modelHelper->itemCoordinates(d->markerModel->index(row, 0, selectionRange.parent()), &coordinates))
                continue;

            for (int l=0; l<=TileIndex::MaxLevel; ++l)
            {
                const TileIndex tileIndex = TileIndex::fromCoordinates(coordinates, l);
                Tile* const myTile = getTile(tileIndex, true);
                if (!myTile)
                    break;

                myTile->selectedCount++;
//                 kDebug()<<l<<tileIndex<<myTile->selectedCount;
                KMAP_ASSERT(myTile->selectedCount <= myTile->markerIndices.count());

                if (myTile->children.isEmpty())
                    break;
            }
        }
    }

    for (int i=0; i<deselected.count(); ++i)
    {
        const QItemSelectionRange selectionRange = deselected.at(i);
        for (int row = selectionRange.top(); row<=selectionRange.bottom(); ++row)
        {
            // get the coordinates of the item
            WMWGeoCoordinate coordinates;
            if (!d->modelHelper->itemCoordinates(d->markerModel->index(row, 0, selectionRange.parent()), &coordinates))
                continue;

            for (int l=0; l<=TileIndex::MaxLevel; ++l)
            {
                const TileIndex tileIndex = TileIndex::fromCoordinates(coordinates, l);
                Tile* const myTile = getTile(tileIndex, true);
                if (!myTile)
                    break;

                myTile->selectedCount--;
                KMAP_ASSERT(myTile->selectedCount >= 0);

                if (myTile->children.isEmpty())
                    break;
            }
        }
    }

    emit(signalTilesOrSelectionChanged());
}

void ItemMarkerTiler::slotSourceModelDataChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/)
{
    setDirty();

    // TODO: if only a few items were changed, try to see whether they are still in the right tiles
}

void ItemMarkerTiler::slotSourceModelRowsInserted(const QModelIndex& parentIndex, int start, int end)
{
    if (isDirty())
    {
        // rows will be added once the tiles are regenerated
        return;
    }

    // sort the new items into our tiles:
    for (int i=start; i<=end; ++i)
    {
        addMarkerIndexToGrid(QPersistentModelIndex(d->markerModel->index(i, 0, parentIndex)));
    }
    emit(signalTilesOrSelectionChanged());
}

void ItemMarkerTiler::slotSourceModelRowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end)
{
    // TODO: emit(signalTilesOrSelectionChanged()); in rowsWereRemoved
#if QT_VERSION < 0x040600
    // removeMarkerIndexFromGrid does not work in Qt 4.5 because the model has already deleted all
    // the data of the item, but we need the items coordinates to work efficiently
    setDirty();
    return;
#else
    if (isDirty())
    {
        return;
    }

    // remove the items from their tiles:
    for (int i=start; i<=end; ++i)
    {
        const QModelIndex itemIndex = d->markerModel->index(start, 0, parentIndex);

        // remove the marker from the grid, but leave the selection count alone because the
        // selection model will send a signal about the deselection of the marker
        removeMarkerIndexFromGrid(itemIndex, true);
    }
#endif
}

void ItemMarkerTiler::slotThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap)
{
    emit(signalThumbnailAvailableForIndex(QVariant::fromValue(index), pixmap));
}

void ItemMarkerTiler::slotSourceModelReset()
{
    setDirty();
}

/**
 * @brief Remove a marker from the grid
 * @param ignoreSelection Do not remove the marker from the count of selected items.
 *                        This is only used by slotSourceModelRowsAboutToBeRemoved internally,
 *                        because the selection model sends us an extra signal about the deselection.
 */
void ItemMarkerTiler::removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection)
{
    if (isDirty())
    {
        // if the model is dirty, there is no need to remove the marker
        // because the tiles will be regenerated on the next call
        // that requests data
        return;
    }

    KMAP_ASSERT(markerIndex.isValid());

    bool markerIsSelected = false;
    if (d->selectionModel)
    {
        markerIsSelected = d->selectionModel->isSelected(markerIndex);
    }

    // remove the marker from the grid:
    WMWGeoCoordinate markerCoordinates;
    if (!d->modelHelper->itemCoordinates(markerIndex, &markerCoordinates))
        return;

    const TileIndex tileIndex = TileIndex::fromCoordinates(markerCoordinates, TileIndex::MaxLevel);
    QList<Tile*> tiles;
    // here l functions as the number of indices that we actually use, therefore we have to go one more up
    // in this case, l==0 returns the root tile
    for (int l=0; l<=TileIndex::MaxLevel+1; ++l)
    {
        Tile* const currentTile = getTile(tileIndex.mid(0, l), true);
        if (!currentTile)
            break;

        tiles << currentTile;
        currentTile->removeMarkerIndexOrInvalidIndex(markerIndex);

        if (markerIsSelected&&!ignoreSelection)
        {
            currentTile->selectedCount--;
            KMAP_ASSERT(currentTile->selectedCount>=0);
        }
    }

    // delete the tiles which are now empty!
    for (int l = tiles.count()-1; l>0; --l)
    {
        Tile* const currentTile = tiles.at(l);

        if (!currentTile->markerIndices.isEmpty())
            break;

        Tile* const parentTile = tiles.at(l-1);
        parentTile->deleteChild(currentTile);
    }
}

int ItemMarkerTiler::getTileMarkerCount(const AbstractMarkerTiler::TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    Tile* const myTile = getTile(tileIndex, true);

    if (!myTile)
    {
        return 0;
    }

    return myTile->markerIndices.count();
}

int ItemMarkerTiler::getTileSelectedCount(const AbstractMarkerTiler::TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    Tile* const myTile = getTile(tileIndex, true);

    if (!myTile)
    {
        return 0;
    }

    return myTile->selectedCount;
}

WMWSelectionState ItemMarkerTiler::getTileSelectedState(const AbstractMarkerTiler::TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    Tile* const myTile = getTile(tileIndex, true);

    if (!myTile)
    {
        return WMWSelectedNone;
    }

    const int selectedCount = myTile->selectedCount;
    if (selectedCount==0)
    {
        return WMWSelectedNone;
    }
    else if (selectedCount==myTile->markerIndices.count())
    {
        return WMWSelectedAll;
    }

    return WMWSelectedSome;
}

AbstractMarkerTiler::Tile* ItemMarkerTiler::getTile(const AbstractMarkerTiler::TileIndex& tileIndex, const bool stopIfEmpty)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    Tile* tile = rootTile();
    for (int level = 0; level < tileIndex.indexCount(); ++level)
    {
        const int currentIndex = tileIndex.linearIndex(level);

        Tile* childTile = 0;
        if (tile->children.isEmpty())
        {
            tile->prepareForChildren(QIntPair(TileIndex::Tiling, TileIndex::Tiling));

            // if there are any markers in the tile,
            // we have to sort them into the child tiles:
            if (!tile->markerIndices.isEmpty())
            {
                for (int i=0; i<tile->markerIndices.count(); ++i)
                {
                    const QPersistentModelIndex currentMarkerIndex = tile->markerIndices.at(i);
                    KMAP_ASSERT(currentMarkerIndex.isValid());

                    // get the tile index for this marker:
                    WMWGeoCoordinate currentMarkerCoordinates;
                    if (!d->modelHelper->itemCoordinates(currentMarkerIndex, &currentMarkerCoordinates))
                        continue;

                    const TileIndex markerTileIndex = TileIndex::fromCoordinates(currentMarkerCoordinates, level);
                    const int newTileIndex = markerTileIndex.toIntList().last();

                    Tile* newTile = tile->children.at(newTileIndex);
                    if (newTile==0)
                    {
                        newTile = new Tile();
                        tile->addChild(newTileIndex, newTile);
                    }
                    newTile->markerIndices<<currentMarkerIndex;
                    if (d->selectionModel)
                    {
                        if (d->selectionModel->isSelected(currentMarkerIndex))
                        {
                            newTile->selectedCount++;
                        }
                    }
                }
            }
        }
        childTile = tile->children.at(currentIndex);

        if (childTile==0)
        {
            if (stopIfEmpty)
            {
                // there will be no markers in this tile, therefore stop
                return 0;
            }

            childTile = new Tile();
            tile->addChild(currentIndex, childTile);
        }
        tile = childTile;
    }

    return tile;
}

QList<QPersistentModelIndex> ItemMarkerTiler::getTileMarkerIndices(const AbstractMarkerTiler::TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    Tile* const myTile = getTile(tileIndex, true);

    if (!myTile)
    {
        return QList<QPersistentModelIndex>();
    }

    return myTile->markerIndices;
}

void ItemMarkerTiler::addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex)
{
    if (isDirty())
    {
        // the model is dirty, so let regenerateTiles do the rest
        regenerateTiles();
        return;
    }
    WMWGeoCoordinate markerCoordinates;
    if (!d->modelHelper->itemCoordinates(markerIndex, &markerCoordinates))
        return;

    TileIndex tileIndex = TileIndex::fromCoordinates(markerCoordinates, TileIndex::MaxLevel);
    KMAP_ASSERT(tileIndex.level()==TileIndex::MaxLevel);

    bool markerIsSelected = false;
    if (d->selectionModel)
    {
        markerIsSelected = d->selectionModel->isSelected(markerIndex);
    }

    // add the marker to all existing tiles:
    Tile* currentTile = rootTile();
    for (int l = 0; l<=TileIndex::MaxLevel; ++l)
    {
        currentTile->markerIndices<<markerIndex;
        if (markerIsSelected)
        {
            currentTile->selectedCount++;
        }

        // does the tile have any children?
        if (currentTile->children.isEmpty())
            break;

        // the tile has children. make sure the tile for our marker exists:
        const int nextIndex = tileIndex.linearIndex(l);
        Tile* nextTile = currentTile->children.at(nextIndex);
        if (nextTile==0)
        {
            // we have to create the tile:
            nextTile = new Tile();
            currentTile->addChild(nextIndex, nextTile);
        }

        // if this is the last loop iteration, populate the next tile now:
        if (l==TileIndex::MaxLevel)
        {
            nextTile->markerIndices<<markerIndex;
            if (markerIsSelected)
            {
                nextTile->selectedCount++;
            }
        }

        currentTile = nextTile;
    }
}

void ItemMarkerTiler::regenerateTiles()
{
    resetRootTile();
    setDirty(false);

    if (!d->markerModel)
        return;

    // read out all existing markers into tiles:
    for (int row=0; row<d->markerModel->rowCount(); ++row)
    {
        const QModelIndex modelIndex = d->markerModel->index(row, 0);
        addMarkerIndexToGrid(QPersistentModelIndex(modelIndex));
    }
}

bool ItemMarkerTiler::indicesEqual(const QVariant& a, const QVariant& b) const
{
    return a.value<QPersistentModelIndex>()==b.value<QPersistentModelIndex>();
}

bool ItemMarkerTiler::isItemModelBased() const
{
    return true;
}

void ItemMarkerTiler::onIndicesClicked(const TileIndex::List& tileIndicesList)
{
    Q_UNUSED(tileIndicesList);
}

void ItemMarkerTiler::onIndicesMoved(const TileIndex::List& tileIndicesList, const WMWGeoCoordinate& targetCoordinates, const QPersistentModelIndex& targetSnapIndex)
{
    QList<QPersistentModelIndex> movedMarkers;
    if (tileIndicesList.isEmpty())
    {
        // complicated case: all selected markers were moved
        QModelIndexList selectedIndices = d->selectionModel->selectedIndexes();
        for (int i=0; i<selectedIndices.count(); ++i)
        {
            // TODO: correctly handle items with multiple columns
            QModelIndex movedMarker = selectedIndices.at(i);
            if (movedMarker.column() == 0)
            {
                movedMarkers << movedMarker;
            }
        }
    }
    else
    {
        // only the tiles in tileIndicesList were moved
        for (int i=0; i<tileIndicesList.count(); ++i)
        {
            const AbstractMarkerTiler::TileIndex tileIndex = tileIndicesList.at(i);

            movedMarkers << getTileMarkerIndices(tileIndex);
        }
    }

    d->modelHelper->onIndicesMoved(movedMarkers, targetCoordinates, targetSnapIndex);
}

} // namespace KMapIface
