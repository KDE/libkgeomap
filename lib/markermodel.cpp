/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : A model to hold the markers
 *
 * Copyright (C) 2009,2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "markermodel.moc"

namespace WMW2 {

typedef QPair<int, int> QIntPair;

class MarkerModelPrivate
{
public:
    MarkerModelPrivate()
    : tesselationSizes(),
      rootTile(new MarkerModel::Tile()),
      isDirty(true),
      markerModel(0),
      coordinatesRole(0),
      selectionModel(0)
    {
        const QIntPair level0Sizes(10, 10);
        tesselationSizes << level0Sizes;
        for (int i = 0; i<8; ++i)
        {
            tesselationSizes << QIntPair(10, 10);
        }

        rootTile->prepareForChildren(level0Sizes);
    }

    QList<QIntPair> tesselationSizes;
    MarkerModel::Tile* rootTile;
    bool isDirty;
    QAbstractItemModel* markerModel;
    int coordinatesRole;
    QItemSelectionModel* selectionModel;
};

MarkerModel::MarkerModel()
: d(new MarkerModelPrivate())
{
}

void MarkerModel::setMarkerModel(QAbstractItemModel* const markerModel, const int coordinatesRole)
{
    d->isDirty = true;
    d->markerModel = markerModel;
    d->coordinatesRole = coordinatesRole;

    if (d->markerModel!=0)
    {
        // TODO: disconnect the old model if there was one
        connect(d->markerModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                this, SLOT(slotSourceModelRowsInserted(const QModelIndex&, int, int)));

        connect(d->markerModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotSourceModelRowsAboutToBeRemoved(const QModelIndex&, int, int)));

        connect(d->markerModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(slotSourceModelDataChanged(const QModelIndex&, const QModelIndex&)));
    }
}

MarkerModel::~MarkerModel()
{
    // delete all tiles
    delete d->rootTile;

    delete d;
}

QPair<int, int> MarkerModel::getTesselationSizes(const int level) const
{
    return d->tesselationSizes.at(level);
}

WMWGeoCoordinate MarkerModel::tileIndexToCoordinate(const QIntList& tileIndex)
{
    // TODO: safeguards against rounding errors!
    qreal tileLatBL = -90.0;
    qreal tileLonBL = -180.0;
    qreal tileLatHeight = 180.0;
    qreal tileLonWidth = 360.0;

    WMW2_ASSERT(tileIndex.count()<=maxLevel());

    for (int l = 0; l < qMin(tileIndex.count(), maxLevel()-1); ++l)
    {
        // how many tiles are at this level?
        const QIntPair tesselationSizes = d->tesselationSizes.at(l);
        const qreal latDivisor = tesselationSizes.first;
        const qreal lonDivisor = tesselationSizes.second;

        const qreal dLat = tileLatHeight / latDivisor;
        const qreal dLon = tileLonWidth / lonDivisor;

        const int linearIndex = tileIndex.at(l);
        int latIndex;
        int lonIndex;
        linearIndexToLatLonIndex(linearIndex, l, &latIndex, &lonIndex);

        // update the start position for the next tile:
        tileLatBL+=latIndex*dLat;
        tileLonBL+=lonIndex*dLon;
        tileLatHeight/=latDivisor;
        tileLonWidth/=lonDivisor;
    }

    return WMWGeoCoordinate(tileLatBL, tileLonBL);
}

QIntList MarkerModel::coordinateToTileIndex(const WMWGeoCoordinate& coordinate, const int level)
{
    WMW2_ASSERT(level<=maxLevel());

    if (!coordinate.hasCoordinates())
        return QIntList();

    qreal tileLatBL = -90.0;
    qreal tileLonBL = -180.0;
    qreal tileLatHeight = 180.0;
    qreal tileLonWidth = 360.0;

    QIntList indices;
    for (int l = 0; l <= level; ++l)
    {
        // how many tiles at this level?
        const QIntPair tesselationSizes = d->tesselationSizes.at(l);
        const qreal latDivisor = tesselationSizes.first;
        const qreal lonDivisor = tesselationSizes.second;

        const qreal dLat = tileLatHeight / latDivisor;
        const qreal dLon = tileLonWidth / lonDivisor;

        int latIndex = int( (coordinate.lat() - tileLatBL ) / dLat );
        int lonIndex = int( (coordinate.lon() - tileLonBL ) / dLon );

        // protect against invalid indices due to rounding errors
        bool haveRoundingErrors = false;
        if (latIndex<0)
        {
            haveRoundingErrors = true;
            latIndex = 0;
        }
        if (lonIndex<0)
        {
            haveRoundingErrors = true;
            lonIndex = 0;
        }
        if (latIndex>=latDivisor)
        {
            haveRoundingErrors = true;
            latIndex = latDivisor-1;
        }
        if (lonIndex>=lonDivisor)
        {
            haveRoundingErrors = true;
            lonIndex = lonDivisor-1;
        }
        if (haveRoundingErrors)
        {
            kDebug()<<QString("Rounding errors at level %1!").arg(l);
        }

        const int linearIndex = latLonIndexToLinearIndex(latIndex, lonIndex, l);
        WMW2_ASSERT(linearIndex<(latDivisor*lonDivisor));

        indices << linearIndex;

        // update the start position for the next tile:
        // TODO: rounding errors
        tileLatBL+=latIndex*dLat;
        tileLonBL+=lonIndex*dLon;
        tileLatHeight/=latDivisor;
        tileLonWidth/=lonDivisor;
    }

    WMW2_ASSERT(indices.count()==level+1);
    return indices;
}

int MarkerModel::latLonIndexToLinearIndex(const int latIndex, const int lonIndex, const int level) const
{
    WMW2_ASSERT(level<=maxLevel());

    const QIntPair tesselationSizes = d->tesselationSizes.at(level);
    const int nLon = tesselationSizes.second;
    const int linearIndex = latIndex*nLon + lonIndex;

    WMW2_ASSERT(linearIndex<(tesselationSizes.first*tesselationSizes.second));

    return linearIndex;
}

void MarkerModel::linearIndexToLatLonIndex(const int linearIndex, const int level, int* const latIndex, int* const lonIndex) const
{
    WMW2_ASSERT(level<=maxLevel());

    const QIntPair tesselationSizes = d->tesselationSizes.at(level);
    const int nLon = tesselationSizes.second;
    *latIndex = linearIndex/nLon;
    *lonIndex = linearIndex%nLon;
    WMW2_ASSERT(*latIndex<tesselationSizes.first);
    WMW2_ASSERT(*lonIndex<tesselationSizes.second);
}

void MarkerModel::addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex)
{
    if (d->isDirty)
    {
        regenerateTiles();
    }

    const WMWGeoCoordinate markerCoordinates = markerIndex.data(d->coordinatesRole).value<WMWGeoCoordinate>();
    if (!markerCoordinates.hasCoordinates())
        return;

    const QIntList tileIndex = coordinateToTileIndex(markerCoordinates, maxLevel());
    WMW2_ASSERT(tileIndex.count()==maxIndexCount());

    bool markerIsSelected = false;
    if (d->selectionModel)
    {
        markerIsSelected = d->selectionModel->isSelected(markerIndex);
    }

    // add the marker to all existing tiles:
    Tile* currentTile = d->rootTile;
    for (int l = 0; l<=maxLevel(); ++l)
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
        const int nextIndex = tileIndex.at(l);
        Tile* nextTile = currentTile->children.at(nextIndex);
        if (nextTile==0)
        {
            // we have to create the tile:
            nextTile = new Tile();
            currentTile->addChild(nextIndex, nextTile);
        }

        // if this is the last loop iteration, populate the next tile now:
        if (l==maxLevel())
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

int MarkerModel::getTileMarkerCount(const QIntList& tileIndex)
{
    if (d->isDirty)
    {
        regenerateTiles();
    }

    WMW2_ASSERT(tileIndex.count()<=maxIndexCount());

    Tile* const myTile = getTile(tileIndex, true);

    if (!myTile)
    {
        return 0;
    }

    return myTile->markerIndices.count();
}

int MarkerModel::getTileSelectedCount(const QIntList& tileIndex)
{
    if (d->isDirty)
    {
        regenerateTiles();
    }

    WMW2_ASSERT(tileIndex.count()<=maxIndexCount());

    Tile* const myTile = getTile(tileIndex, true);

    if (!myTile)
    {
        return 0;
    }

    return myTile->selectedCount;
}

QList<QPersistentModelIndex> MarkerModel::getTileMarkerIndices(const QIntList& tileIndex)
{
    if (d->isDirty)
    {
        regenerateTiles();
    }

    WMW2_ASSERT(tileIndex.count()<=maxIndexCount());

    Tile* const myTile = getTile(tileIndex, true);

    if (!myTile)
    {
        return QList<QPersistentModelIndex>();
    }

    return myTile->markerIndices;
}

WMWSelectionState MarkerModel::getTileSelectedState(const QIntList& tileIndex)
{
    if (d->isDirty)
    {
        regenerateTiles();
    }

    WMW2_ASSERT(tileIndex.count()<=maxIndexCount());

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


MarkerModel::Tile* MarkerModel::getTile(const QIntList& tileIndex, const bool stopIfEmpty)
{
    if (d->isDirty)
    {
        regenerateTiles();
    }

    WMW2_ASSERT(tileIndex.count()<=maxIndexCount());

    Tile* tile = d->rootTile;
    for (int level = 0; level < tileIndex.count(); ++level)
    {
        const int currentIndex = tileIndex.at(level);

        Tile* childTile = 0;
        if (tile->children.isEmpty())
        {
            WMW2_ASSERT(level<d->tesselationSizes.count());
            tile->prepareForChildren(d->tesselationSizes.at(level));

            // if there are any markers in the tile,
            // we have to sort them into the child tiles:
            if (!tile->markerIndices.isEmpty())
            {
                for (int i=0; i<tile->markerIndices.count(); ++i)
                {
                    const QPersistentModelIndex currentMarkerIndex = tile->markerIndices.at(i);
                    WMW2_ASSERT(currentMarkerIndex.isValid());
                    
                    // get the tile index for this marker:
                    const QIntList markerTileIndex = coordinateToTileIndex(currentMarkerIndex.data(d->coordinatesRole).value<WMWGeoCoordinate>(), level);
                    const int newTileIndex = markerTileIndex.last();

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

int MarkerModel::maxLevel() const
{
    return d->tesselationSizes.count()-1;
}

int MarkerModel::maxIndexCount() const
{
    return d->tesselationSizes.count();
}

MarkerModel::Tile* MarkerModel::rootTile()
{
    if (d->isDirty)
    {
        regenerateTiles();
    }

    return d->rootTile;
}

bool MarkerModel::indicesEqual(const QIntList& a, const QIntList& b, const int upToLevel) const
{
    WMW2_ASSERT(a.count()>upToLevel);
    WMW2_ASSERT(b.count()>upToLevel);

    for (int i=0; i<=upToLevel; ++i)
    {
        if (a.at(i)!=b.at(i))
            return false;
    }

    return true;
}

QList<QIntPair> MarkerModel::linearIndexToLatLonIndex(const QIntList& linearIndex) const
{
    QList<QIntPair> result;
    for (int i=0; i<linearIndex.count(); ++i)
    {
        int latIndex;
        int lonIndex;
        linearIndexToLatLonIndex(linearIndex.at(i), i, &latIndex, &lonIndex);

        result << QIntPair(latIndex, lonIndex);
    }

    return result;
}

QIntList MarkerModel::latLonIndexToLinearIndex(const QList<QIntPair>& latLonIndex) const
{
    QIntList result;
    for (int i=0; i<latLonIndex.count(); ++i)
    {
        result << latLonIndexToLinearIndex(latLonIndex.at(i).first, latLonIndex.at(i).second, i);
    }

    return result;
}

class MarkerModelNonEmptyIteratorPrivate
{
public:
    MarkerModelNonEmptyIteratorPrivate()
    : model(0),
      level(0),
      startIndexLinear(),
      endIndexLinear(),
      currentIndexLinear(),
      startIndices(),
      endIndices(),
      currentIndices(),
      atEnd(false)
    {
    }

    MarkerModel* model;
    int level;

    QList<QPair<QIntList, QIntList> > boundsList;

    QIntList startIndexLinear;
    QIntList endIndexLinear;
    QIntList currentIndexLinear;

    QList<QIntPair> startIndices;
    QList<QIntPair> endIndices;
    QList<QIntPair> currentIndices;

    bool atEnd;
    bool atStartOfLevel;
};

MarkerModel::NonEmptyIterator::~NonEmptyIterator()
{
    delete d;
}

MarkerModel::NonEmptyIterator::NonEmptyIterator(MarkerModel* const model, const int level)
: d(new MarkerModelNonEmptyIteratorPrivate())
{
    d->model = model;
    WMW2_ASSERT(level<=model->maxLevel());
    d->level = level;

    QIntList startIndexLinear;
    QIntList endIndexLinear;
    for (int i=0; i<=level; ++i)
    {
        startIndexLinear<<0;
        QIntPair currentSizes = d->model->getTesselationSizes(i);
        endIndexLinear<<currentSizes.first*currentSizes.second - 1;
    }
//     kDebug()<<d->startIndexLinear<<d->endIndexLinear;

    d->boundsList << QPair<QIntList, QIntList>(startIndexLinear, endIndexLinear);

    initializeNextBounds();
}

MarkerModel::NonEmptyIterator::NonEmptyIterator(MarkerModel* const model, const int level, const QIntList& startIndex, const QIntList& endIndex)
: d(new MarkerModelNonEmptyIteratorPrivate())
{
    d->model = model;
    WMW2_ASSERT(level<=model->maxLevel());
    d->level = level;

    WMW2_ASSERT(startIndex.count()==(level+1));
    WMW2_ASSERT(endIndex.count()==(level+1));
    d->boundsList << QPair<QIntList, QIntList>(startIndex, endIndex);

    initializeNextBounds();
}

MarkerModel::NonEmptyIterator::NonEmptyIterator(MarkerModel* const model, const int level, const WMWGeoCoordinate::PairList& normalizedMapBounds)
: d(new MarkerModelNonEmptyIteratorPrivate())
{
    d->model = model;
    WMW2_ASSERT(level<=model->maxLevel());
    d->level = level;

    // store the coordinates of the bounds as indices:
    for (int i=0; i<normalizedMapBounds.count(); ++i)
    {
        WMWGeoCoordinate::Pair currentBounds = normalizedMapBounds.at(i);
        WMW2_ASSERT(currentBounds.first.lat()<currentBounds.second.lat());
        WMW2_ASSERT(currentBounds.first.lon()<currentBounds.second.lon());

        const QIntList startIndex = d->model->coordinateToTileIndex(currentBounds.first, d->level);
        const QIntList endIndex = d->model->coordinateToTileIndex(currentBounds.second, d->level);

//         kDebug()<<currentBounds.first.geoUrl()<<startIndex<<currentBounds.second.geoUrl()<<endIndex;
        d->boundsList << QPair<QIntList, QIntList>(startIndex, endIndex);
    }

    initializeNextBounds();
}

bool MarkerModel::NonEmptyIterator::initializeNextBounds()
{
    if (d->boundsList.isEmpty())
    {
        d->atEnd = true;
        return false;
    }

    QPair<QIntList, QIntList> nextBounds = d->boundsList.takeFirst();
    d->startIndexLinear = nextBounds.first;
    d->endIndexLinear = nextBounds.second;

    WMW2_ASSERT(d->startIndexLinear.count() == d->level + 1);
    WMW2_ASSERT(d->endIndexLinear.count() == d->level + 1);

    // calculate the 'cartesian' start/end indices:
    d->startIndices = d->model->linearIndexToLatLonIndex(d->startIndexLinear);
    d->endIndices = d->model->linearIndexToLatLonIndex(d->endIndexLinear);
//     kDebug()<<d->startIndices<<d->startIndexLinear;
//     kDebug()<<d->endIndices<<d->endIndexLinear;

    d->currentIndexLinear = d->startIndexLinear.mid(0, 1);
    d->currentIndices = d->startIndices.mid(0, 1);
    d->atStartOfLevel = true;

    nextIndex();

    return d->atEnd;
}

QIntList MarkerModel::NonEmptyIterator::nextIndex()
{
    if (d->atEnd)
    {
        return d->currentIndexLinear;
    }

    Q_FOREVER
    {
        const int currentLevel = d->currentIndexLinear.count() - 1;
//         kDebug() << d->level << currentLevel << d->atStartOfLevel << d->currentIndexLinear;

        if (d->atStartOfLevel)
        {
            d->atStartOfLevel = false;
        }
        else
        {
            // go to the next tile at the current level, if that is possible:

            // determine the limits in the current tile:
            int limitLatBL = 0;
            int limitLonBL = 0;
            int limitLatTR = d->model->getTesselationSizes(currentLevel).first-1;
            int limitLonTR = d->model->getTesselationSizes(currentLevel).second-1;

            int compareLevel = currentLevel - 1;

            // check limit on the left side:
            bool onLimit = true;
            for (int i=0; onLimit&&(i<=compareLevel); ++i)
            {
                onLimit = d->currentIndices.at(i).first==d->startIndices.at(i).first;
            }
            if (onLimit)
            {
                limitLatBL = d->startIndices.at(currentLevel).first;
            }

            // check limit on the bottom side:
            onLimit = true;
            for (int i=0; onLimit&&(i<=compareLevel); ++i)
            {
                onLimit = d->currentIndices.at(i).second==d->startIndices.at(i).second;
            }
            if (onLimit)
            {
                limitLonBL = d->startIndices.at(currentLevel).second;
            }

            // check limit on the right side:
            onLimit = true;
            for (int i=0; onLimit&&(i<=compareLevel); ++i)
            {
                onLimit = d->currentIndices.at(i).first==d->endIndices.at(i).first;
            }
            if (onLimit)
            {
                limitLatTR = d->endIndices.at(currentLevel).first;
            }

            // check limit on the top side:
            onLimit = true;
            for (int i=0; onLimit&&(i<=compareLevel); ++i)
            {
                onLimit = d->currentIndices.at(i).second==d->endIndices.at(i).second;
            }
            if (onLimit)
            {
                limitLonTR = d->endIndices.at(currentLevel).second;
            }

            WMW2_ASSERT(limitLatBL<=limitLatTR);
            WMW2_ASSERT(limitLonBL<=limitLonTR);
//             kDebug() << limitLatBL << limitLonBL << limitLatTR << limitLonTR << compareLevel << currentLevel;

            int currentLat = d->currentIndices.last().first;
            int currentLon = d->currentIndices.last().second;

            currentLon++;
            if (currentLon>limitLonTR)
            {
                currentLon = limitLonBL;
                currentLat++;
                if (currentLat>limitLatTR)
                {
                    if (currentLevel == 0)
                    {
                        // we are at the end!
                        // are there other bounds to iterate over?
                        initializeNextBounds();

                        // initializeNextBounds() call nextIndex which updates d->currentIndexLinear, if possible:
                        return d->currentIndexLinear;
                    }

                    // we need to go one level up, trim the indices:
                    d->currentIndexLinear.removeLast();
                    d->currentIndices.removeLast();

                    continue;
                }
            }

            // save the new position:
            d->currentIndices.last() = QIntPair(currentLat, currentLon);
            d->currentIndexLinear = d->model->latLonIndexToLinearIndex(d->currentIndices);
        }

        // is the tile empty?
        if (d->model->getTileMarkerCount(d->currentIndexLinear)==0)
        {
            continue;
        }

        // are we at the target level?
        if (currentLevel == d->level)
        {
            // yes, return the current index:
            return d->currentIndexLinear;
        }

        // go one level down:
        int compareLevel = currentLevel;

        // determine the limits for the next level:
        int limitLatBL = 0;
        int limitLonBL = 0;
        int limitLatTR = d->model->getTesselationSizes(compareLevel).first-1;
        int limitLonTR = d->model->getTesselationSizes(compareLevel).second-1;

        // check limit on the left side:
        bool onLimit = true;
        for (int i=0; onLimit&&(i<=compareLevel); ++i)
        {
            onLimit = d->currentIndices.at(i).first==d->startIndices.at(i).first;
        }
        if (onLimit)
        {
            limitLatBL = d->startIndices.at(currentLevel+1).first;
        }

        // check limit on the bottom side:
        onLimit = true;
        for (int i=0; onLimit&&(i<=compareLevel); ++i)
        {
            onLimit = d->currentIndices.at(i).second==d->startIndices.at(i).second;
        }
        if (onLimit)
        {
            limitLonBL = d->startIndices.at(currentLevel+1).second;
        }

        // check limit on the right side:
        onLimit = true;
        for (int i=0; onLimit&&(i<=compareLevel); ++i)
        {
            onLimit = d->currentIndices.at(i).first==d->endIndices.at(i).first;
        }
        if (onLimit)
        {
            limitLatTR = d->endIndices.at(currentLevel+1).first;
        }

        // check limit on the top side:
        onLimit = true;
        for (int i=0; onLimit&&(i<=compareLevel); ++i)
        {
            onLimit = d->currentIndices.at(i).second==d->endIndices.at(i).second;
        }
        if (onLimit)
        {
            limitLonTR = d->endIndices.at(currentLevel+1).second;
        }

        WMW2_ASSERT(limitLatBL<=limitLatTR);
        WMW2_ASSERT(limitLonBL<=limitLonTR);

        // go one level down:
        d->currentIndices << QIntPair(limitLatBL, limitLonBL);
        d->currentIndexLinear = d->model->latLonIndexToLinearIndex(d->currentIndices);
        d->atStartOfLevel = true;
    }
}

QIntList MarkerModel::NonEmptyIterator::currentIndex() const
{
    return d->currentIndexLinear;
}

bool MarkerModel::NonEmptyIterator::atEnd() const
{
    return d->atEnd;
}

MarkerModel* MarkerModel::NonEmptyIterator::model() const
{
    return d->model;
}

/**
 * @brief Remove a marker from the grid
 * @param ignoreSelection Do not remove the marker from the count of selected items.
 *                        This is only used by slotSourceModelRowsAboutToBeRemoved internally,
 *                        because the selection model sends us an extra signal about the deselection.
 */
void MarkerModel::removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection)
{
    if (d->isDirty)
    {
        // if the model is dirty, there is no need to remove the marker
        // because the tiles will be regenerated on the next call
        // that requests data
        return;
    }

    WMW2_ASSERT(markerIndex.isValid());

    bool markerIsSelected = false;
    if (d->selectionModel)
    {
        markerIsSelected = d->selectionModel->isSelected(markerIndex);
    }
    
    // remove the marker from the grid:
    const WMWGeoCoordinate markerCoordinates = markerIndex.data(d->coordinatesRole).value<WMWGeoCoordinate>();
    const QIntList tileIndex = coordinateToTileIndex(markerCoordinates, maxLevel());
    QList<Tile*> tiles;
    // here l functions as the number of indices that we actually use, therefore we have to go one more up
    // in this case, l==0 returns the root tile
    for (int l=0; l<=maxLevel()+1; ++l)
    {
        Tile* const currentTile = getTile(tileIndex.mid(0, l), true);
        if (!currentTile)
            break;

        tiles << currentTile;
        currentTile->removeMarkerIndexOrInvalidIndex(markerIndex);

        if (markerIsSelected&&!ignoreSelection)
        {
            currentTile->selectedCount--;
            WMW2_ASSERT(currentTile->selectedCount>=0);
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

void MarkerModel::moveMarker(const QPersistentModelIndex& markerIndex, const WMWGeoCoordinate& newPosition)
{
    WMW2_ASSERT(markerIndex.isValid());
    // TODO: is there a way to move the marker without resetting the tiles?
    d->markerModel->setData(markerIndex, QVariant::fromValue(newPosition), d->coordinatesRole);
}

void MarkerModel::slotSourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    d->isDirty = true;
    emit(signalTilesOrSelectionChanged());

    // TODO: if only a few items were changed, try to see whether they are still in the right tiles
}

void MarkerModel::slotSourceModelRowsInserted(const QModelIndex& parentIndex, int start, int end)
{
    if (d->isDirty)
    {
        // rows will be added once the tiles are regenerated
        return;
    }

    // sort the new items into our tiles:
    for (int i=start; i<=end; ++i)
    {
        addMarkerIndexToGrid(QPersistentModelIndex(d->markerModel->index(start, 0, parentIndex)));
    }

    emit(signalTilesOrSelectionChanged());
}

void MarkerModel::slotSourceModelRowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end)
{
    // TODO: emit(signalTilesOrSelectionChanged()); in rowsWereRemoved
#if QT_VERSION < 0x040600
    // removeMarkerIndexFromGrid does not work in Qt 4.5 because the model has already deleted all
    // the data of the item, but we need the items coordinates to work efficiently
    d->isDirty = true;
    return;
#else
    if (d->isDirty)
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

void MarkerModel::setSelectionModel(QItemSelectionModel* const selectionModel)
{
    d->selectionModel = selectionModel;

    connect(d->selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // TODO: read out the selection state of existing items!!!
    d->isDirty = true;

    emit(signalTilesOrSelectionChanged());
}

void MarkerModel::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    kDebug()<<selected<<deselected;
    if (d->isDirty)
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
            const WMWGeoCoordinate coordinates = d->markerModel->data(d->markerModel->index(row, 0, selectionRange.parent()), d->coordinatesRole).value<WMWGeoCoordinate>();

            for (int l=0; l<=maxLevel(); ++l)
            {
                const QIntList tileIndex = coordinateToTileIndex(coordinates, l);
                MarkerModel::Tile* const myTile = getTile(tileIndex, true);
                if (!myTile)
                    break;

                myTile->selectedCount++;
//                 kDebug()<<l<<tileIndex<<myTile->selectedCount;
                WMW2_ASSERT(myTile->selectedCount <= myTile->markerIndices.count());

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
            const WMWGeoCoordinate coordinates = d->markerModel->data(d->markerModel->index(row, 0, selectionRange.parent()), d->coordinatesRole).value<WMWGeoCoordinate>();

            for (int l=0; l<=maxLevel(); ++l)
            {
                const QIntList tileIndex = coordinateToTileIndex(coordinates, l);
                MarkerModel::Tile* const myTile = getTile(tileIndex, true);
                if (!myTile)
                    break;

                myTile->selectedCount--;
                WMW2_ASSERT(myTile->selectedCount >= 0);

                if (myTile->children.isEmpty())
                    break;
            }
        }
    }

    emit(signalTilesOrSelectionChanged());
}

void MarkerModel::regenerateTiles()
{
    delete d->rootTile;
    d->rootTile = new Tile();
    d->rootTile->prepareForChildren(d->tesselationSizes.first());
    d->isDirty = false;

    if (!d->markerModel)
        return;

    // read out all existing markers into tiles:
    for (int row=0; row<d->markerModel->rowCount(); ++row)
    {
        const QModelIndex modelIndex = d->markerModel->index(row, 0);
        addMarkerIndexToGrid(QPersistentModelIndex(modelIndex));
    }
}

QItemSelectionModel* MarkerModel::getSelectionModel() const
{
    return d->selectionModel;
}

QVariant MarkerModel::getTileRepresentativeMarker(const QIntList& tileIndex, const int sortKey)
{
    // TODO: actually return the result of some sorting and cache it in the tile
    const QList<QPersistentModelIndex> modelIndices = getTileMarkerIndices(tileIndex);
    if (modelIndices.isEmpty())
        return QVariant();

    return QVariant::fromValue(modelIndices.first());
}

} /* WMW2 */

