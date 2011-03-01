/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-07-17
 * @brief  A marker tiler operating on item models
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "itemmarkertiler.moc"

// local includes

#include "modelhelper.h"

namespace KMap
{

class ItemMarkerTiler::MyTile : public Tile
{
public:
    MyTile()
    : Tile(),
        markerIndices(),
        selectedCount(0)
    {
    }

    ~MyTile()
    {
    }

    QList<QPersistentModelIndex> markerIndices;
    int selectedCount;

    void removeMarkerIndexOrInvalidIndex(const QModelIndex& indexToRemove);
};

void ItemMarkerTiler::MyTile::removeMarkerIndexOrInvalidIndex(const QModelIndex& indexToRemove)
{
    int i=0;
    while (i<markerIndices.count())
    {
        const QPersistentModelIndex& currentIndex = markerIndices.at(i);

        // NOTE: this function is usually called after the model has sent
        //       an aboutToRemove-signal. It is possible that the persistent
        //       marker index became invalid before the caller received the signal.
        //       we remove any invalid indices as we find them.
        if ( !currentIndex.isValid() )
        {
            markerIndices.takeAt(i);
            continue;
        }

        if ( currentIndex == indexToRemove )
        {
            markerIndices.takeAt(i);
            return;
        }

        ++i;
    }
}

class ItemMarkerTiler::ItemMarkerTilerPrivate
{
public:
    ItemMarkerTilerPrivate()
      : modelHelper(0),
        selectionModel(0),
        markerModel(0),
        activeState(false)
    {
    }

    ModelHelper*         modelHelper;
    QItemSelectionModel* selectionModel;
    QAbstractItemModel*  markerModel;
    bool                 activeState;
};

ItemMarkerTiler::ItemMarkerTiler(ModelHelper* const modelHelper, QObject* const parent)
               : AbstractMarkerTiler(parent), d(new ItemMarkerTilerPrivate())
{
    resetRootTile();

    setMarkerModelHelper(modelHelper);
}

ItemMarkerTiler::~ItemMarkerTiler()
{
    // WARNING: we have to call clear! By the time AbstractMarkerTiler calls clear,
    // this object does not exist any more, and thus the tiles are not correctly destroyed!
    clear();

    delete d;
}

void ItemMarkerTiler::setMarkerModelHelper(ModelHelper* const modelHelper)
{
    d->modelHelper    = modelHelper;
    d->markerModel    = modelHelper->model();
    d->selectionModel = modelHelper->selectionModel();

    if (d->markerModel!=0)
    {
        // TODO: disconnect the old model if there was one
        connect(d->markerModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                this, SLOT(slotSourceModelRowsInserted(const QModelIndex&, int, int)));

        connect(d->markerModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotSourceModelRowsAboutToBeRemoved(const QModelIndex&, int, int)));

        // TODO: this signal now has to be monitored in the model helper
//         connect(d->markerModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
//                 this, SLOT(slotSourceModelDataChanged(const QModelIndex&, const QModelIndex&)));

        connect(d->modelHelper, SIGNAL(signalModelChangedDrastically()),
                this, SLOT(slotSourceModelReset()));

        connect(d->markerModel, SIGNAL(modelReset()),
                this, SLOT(slotSourceModelReset()));

        connect(d->markerModel, SIGNAL(layoutChanged()),
                this, SLOT(slotSourceModelLayoutChanged()));

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

QVariant ItemMarkerTiler::getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey)
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
            GeoCoordinates coordinates;
            if (!d->modelHelper->itemCoordinates(d->markerModel->index(row, 0, selectionRange.parent()), &coordinates))
                continue;

            for (int l=0; l<=TileIndex::MaxLevel; ++l)
            {
                const TileIndex tileIndex = TileIndex::fromCoordinates(coordinates, l);
                MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));
                if (!myTile)
                    break;

                myTile->selectedCount++;
//                 kDebug()<<l<<tileIndex<<myTile->selectedCount;
                KMAP_ASSERT(myTile->selectedCount <= myTile->markerIndices.count());

                if (myTile->childrenEmpty())
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
            GeoCoordinates coordinates;
            if (!d->modelHelper->itemCoordinates(d->markerModel->index(row, 0, selectionRange.parent()), &coordinates))
                continue;

            for (int l=0; l<=TileIndex::MaxLevel; ++l)
            {
                const TileIndex tileIndex = TileIndex::fromCoordinates(coordinates, l);
                MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));
                if (!myTile)
                    break;

                myTile->selectedCount--;
                KMAP_ASSERT(myTile->selectedCount >= 0);

                if (myTile->childrenEmpty())
                    break;
            }
        }
    }

    emit(signalTilesOrSelectionChanged());
}

void ItemMarkerTiler::slotSourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    kDebug()<<topLeft<<bottomRight;
    setDirty();
    if (d->activeState)
        emit signalTilesOrSelectionChanged();

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
    kDebug()<<"----";
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
    GeoCoordinates markerCoordinates;
    if (!d->modelHelper->itemCoordinates(markerIndex, &markerCoordinates))
        return;

    const TileIndex tileIndex = TileIndex::fromCoordinates(markerCoordinates, TileIndex::MaxLevel);
    QList<MyTile*> tiles;
    // here l functions as the number of indices that we actually use, therefore we have to go one more up
    // in this case, l==0 returns the root tile
    for (int l=0; l<=TileIndex::MaxLevel+1; ++l)
    {
        MyTile* const currentTile = static_cast<MyTile*>(getTile(tileIndex.mid(0, l), true));
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
        MyTile* const currentTile = tiles.at(l);

        if (!currentTile->markerIndices.isEmpty())
            break;

        MyTile* const parentTile = tiles.at(l-1);
        tileDeleteChild(parentTile, currentTile);
    }
}

int ItemMarkerTiler::getTileMarkerCount(const TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return 0;
    }

    return myTile->markerIndices.count();
}

int ItemMarkerTiler::getTileSelectedCount(const TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return 0;
    }

    return myTile->selectedCount;
}

KMapGroupState ItemMarkerTiler::getTileGroupState(const TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return KMapSelectedNone;
    }

    const int selectedCount = myTile->selectedCount;
    if (selectedCount==0)
    {
        return KMapSelectedNone;
    }
    else if (selectedCount==myTile->markerIndices.count())
    {
        return KMapSelectedAll;
    }

    return KMapSelectedSome;
}

AbstractMarkerTiler::Tile* ItemMarkerTiler::getTile(const TileIndex& tileIndex, const bool stopIfEmpty)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    MyTile* tile = static_cast<MyTile*>(rootTile());
    for (int level = 0; level < tileIndex.indexCount(); ++level)
    {
        const int currentIndex = tileIndex.linearIndex(level);

        MyTile* childTile = 0;
        if (tile->childrenEmpty())
        {
            // if there are any markers in the tile,
            // we have to sort them into the child tiles:
            if (!tile->markerIndices.isEmpty())
            {
                for (int i=0; i<tile->markerIndices.count(); ++i)
                {
                    const QPersistentModelIndex currentMarkerIndex = tile->markerIndices.at(i);
                    KMAP_ASSERT(currentMarkerIndex.isValid());

                    // get the tile index for this marker:
                    GeoCoordinates currentMarkerCoordinates;
                    if (!d->modelHelper->itemCoordinates(currentMarkerIndex, &currentMarkerCoordinates))
                        continue;

                    const TileIndex markerTileIndex = TileIndex::fromCoordinates(currentMarkerCoordinates, level);
                    const int newTileIndex = markerTileIndex.toIntList().last();

                    MyTile* newTile = static_cast<MyTile*>(tile->getChild(newTileIndex));
                    if (newTile==0)
                    {
                        newTile = static_cast<MyTile*>(tileNew());
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
        childTile = static_cast<MyTile*>(tile->getChild(currentIndex));

        if (childTile==0)
        {
            if (stopIfEmpty)
            {
                // there will be no markers in this tile, therefore stop
                return 0;
            }

            childTile = static_cast<MyTile*>(tileNew());
            tile->addChild(currentIndex, childTile);
        }
        tile = childTile;
    }

    return tile;
}

QList<QPersistentModelIndex> ItemMarkerTiler::getTileMarkerIndices(const TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    KMAP_ASSERT(tileIndex.level()<=TileIndex::MaxLevel);

    MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

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
    GeoCoordinates markerCoordinates;
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
    MyTile* currentTile = static_cast<MyTile*>(rootTile());
    for (int l = 0; l<=TileIndex::MaxLevel; ++l)
    {
        currentTile->markerIndices<<markerIndex;
        if (markerIsSelected)
        {
            currentTile->selectedCount++;
        }

        // does the tile have any children?
        if (currentTile->childrenEmpty())
            break;

        // the tile has children. make sure the tile for our marker exists:
        const int nextIndex = tileIndex.linearIndex(l);
        MyTile* nextTile = static_cast<MyTile*>(currentTile->getChild(nextIndex));
        if (nextTile==0)
        {
            // we have to create the tile:
            nextTile = static_cast<MyTile*>(tileNew());
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

void ItemMarkerTiler::prepareTiles(const GeoCoordinates& /*upperLeft*/, const GeoCoordinates&, int /*level*/)
{
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

void ItemMarkerTiler::onIndicesClicked(const TileIndex::List& tileIndicesList, const QVariant& representativeIndex,
                                       const KMapGroupState& groupSelectionState, const MouseModes currentMouseMode)
{
    QList<QPersistentModelIndex> clickedMarkers;
    for (int i=0; i<tileIndicesList.count(); ++i)
    {
        const TileIndex tileIndex = tileIndicesList.at(i);

        clickedMarkers << getTileMarkerIndices(tileIndex);
    }

    const QPersistentModelIndex representativeModelIndex = representativeIndex.value<QPersistentModelIndex>();

    if (currentMouseMode == MouseModeSelectThumbnail && d->selectionModel)
    {
        const bool doSelect = (groupSelectionState & KMapSelectedMask) != KMapSelectedAll;

        const QItemSelectionModel::SelectionFlags selectionFlags =
                  (doSelect ? QItemSelectionModel::Select : QItemSelectionModel::Deselect)
                | QItemSelectionModel::Rows;

        for (int i=0; i<clickedMarkers.count(); ++i)
        {
            if (d->selectionModel->isSelected(clickedMarkers.at(i))!=doSelect)
            {
                d->selectionModel->select(clickedMarkers.at(i), selectionFlags);
            }
        }

        if (representativeModelIndex.isValid())
        {
            d->selectionModel->setCurrentIndex(representativeModelIndex, selectionFlags);
        }

        /**
         * @todo When do we report the clicks to the modelHelper?
         *       Or do we only report selection changes to the selection model?
         */
        //d->modelHelper->onIndicesClicked(clickedMarkers);
    }
    else if (currentMouseMode == MouseModeFilter)
    {
        /// @todo Also forward the representative index in this call
        d->modelHelper->onIndicesClicked(clickedMarkers);
    }
}

void ItemMarkerTiler::onIndicesMoved(const TileIndex::List& tileIndicesList, const GeoCoordinates& targetCoordinates,
                                     const QPersistentModelIndex& targetSnapIndex)
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
            const TileIndex tileIndex = tileIndicesList.at(i);

            movedMarkers << getTileMarkerIndices(tileIndex);
        }
    }

    d->modelHelper->onIndicesMoved(movedMarkers, targetCoordinates, targetSnapIndex);
}

void ItemMarkerTiler::slotSourceModelLayoutChanged()
{
    setDirty();
}

void ItemMarkerTiler::setActive(const bool state)
{
    d->activeState = state;
}

AbstractMarkerTiler::Tile* ItemMarkerTiler::tileNew()
{
    return new MyTile();
}

void ItemMarkerTiler::tileDeleteInternal(AbstractMarkerTiler::Tile* const tile)
{
    delete static_cast<MyTile*>(tile);
}

AbstractMarkerTiler::Flags ItemMarkerTiler::tilerFlags() const
{
    Flags resultFlags = FlagNull;
    if (d->modelHelper->modelFlags().testFlag(ModelHelper::FlagMovable))
    {
        resultFlags|=FlagMovable;
    }

    return resultFlags;
}

KMapGroupState ItemMarkerTiler::getGlobalGroupState()
{
    if (d->selectionModel)
    {
        if (d->selectionModel->hasSelection())
        {
            return KMapSelectedMask;
        }
    }

    return KMapSelectedNone;
}

} // namespace KMap
