/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  An abstract base class for tiling of markers
 *
 * @author Copyright (C) 2009-2011 by Michael G. Hansen
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

#include "abstractmarkertiler.moc"

namespace KMap
{

typedef QPair<int, int> QIntPair;

class AbstractMarkerTiler::AbstractMarkerTilerPrivate
{
public:

    AbstractMarkerTilerPrivate()
        : rootTile(0),
          isDirty(true)
    {
    }

    AbstractMarkerTiler::Tile* rootTile;
    bool                       isDirty;
};

AbstractMarkerTiler::AbstractMarkerTiler(QObject* const parent)
                   : QObject(parent), d(new AbstractMarkerTilerPrivate())
{
}

AbstractMarkerTiler::~AbstractMarkerTiler()
{
    // delete all tiles
    clear();

    delete d;
}

AbstractMarkerTiler::Tile* AbstractMarkerTiler::rootTile()
{
    if (isDirty())
    {
        regenerateTiles();
    }

    return d->rootTile;
}

class AbstractMarkerTiler::NonEmptyIterator::AbstractMarkerTilerNonEmptyIteratorPrivate
{
public:

    AbstractMarkerTilerNonEmptyIteratorPrivate()
        : model(0),
          level(0),
          startIndex(),
          endIndex(),
          currentIndex(),
          atEnd(false)
    {
    }

    AbstractMarkerTiler*                                                          model;
    int                                                                           level;

    QList<QPair<TileIndex, TileIndex> > boundsList;

    TileIndex                                                startIndex;
    TileIndex                                                endIndex;
    TileIndex                                                currentIndex;

    bool                                                                          atEnd;
    bool                                                                          atStartOfLevel;
};

AbstractMarkerTiler::NonEmptyIterator::~NonEmptyIterator()
{
    delete d;
}

AbstractMarkerTiler::NonEmptyIterator::NonEmptyIterator(AbstractMarkerTiler* const model, const int level)
                   : d(new AbstractMarkerTilerNonEmptyIteratorPrivate())
{
    d->model = model;
    KMAP_ASSERT(level <= TileIndex::MaxLevel);
    d->level = level;

    TileIndex startIndex;
    TileIndex endIndex;
    for (int i=0; i <= level; ++i)
    {
        startIndex.appendLinearIndex(0);
        endIndex.appendLinearIndex(TileIndex::Tiling*TileIndex::Tiling-1);
    }
//     kDebug()<<d->startIndexLinear<<d->endIndexLinear;

    d->boundsList << QPair<TileIndex, TileIndex>(startIndex, endIndex);

    initializeNextBounds();
}

AbstractMarkerTiler::NonEmptyIterator::NonEmptyIterator(AbstractMarkerTiler* const model, const int level, 
                                                        const TileIndex& startIndex, const TileIndex& endIndex)
                                     : d(new AbstractMarkerTilerNonEmptyIteratorPrivate())
{
    d->model = model;
    KMAP_ASSERT(level <= TileIndex::MaxLevel);
    d->level = level;

    KMAP_ASSERT(startIndex.level() == level);
    KMAP_ASSERT(endIndex.level() == level);
    d->boundsList << QPair<TileIndex, TileIndex>(startIndex, endIndex);

    initializeNextBounds();
}

AbstractMarkerTiler::NonEmptyIterator::NonEmptyIterator(AbstractMarkerTiler* const model, const int level, 
                                                        const GeoCoordinates::PairList& normalizedMapBounds)
                                     : d(new AbstractMarkerTilerNonEmptyIteratorPrivate())
{
    d->model = model;
    KMAP_ASSERT(level <= TileIndex::MaxLevel);
    d->level = level;

    // store the coordinates of the bounds as indices:
    for (int i=0; i < normalizedMapBounds.count(); ++i)
    {
        GeoCoordinates::Pair currentBounds = normalizedMapBounds.at(i);
        KMAP_ASSERT(currentBounds.first.lat() < currentBounds.second.lat());
        KMAP_ASSERT(currentBounds.first.lon() < currentBounds.second.lon());

        const TileIndex startIndex = TileIndex::fromCoordinates(currentBounds.first, d->level);
        const TileIndex endIndex   = TileIndex::fromCoordinates(currentBounds.second, d->level);

//         kDebug()<<currentBounds.first.geoUrl()<<startIndex<<currentBounds.second.geoUrl()<<endIndex;
        d->boundsList << QPair<TileIndex, TileIndex>(startIndex, endIndex);
    }

    initializeNextBounds();
}

bool AbstractMarkerTiler::NonEmptyIterator::initializeNextBounds()
{
    if (d->boundsList.isEmpty())
    {
        d->atEnd = true;
        return false;
    }

    QPair<TileIndex, TileIndex> nextBounds = d->boundsList.takeFirst();
    d->startIndex = nextBounds.first;
    d->endIndex   = nextBounds.second;

    KMAP_ASSERT(d->startIndex.level() == d->level);
    KMAP_ASSERT(d->endIndex.level() == d->level);

    d->currentIndex = d->startIndex.mid(0, 1);
    d->atStartOfLevel = true;

    nextIndex();

    return d->atEnd;
}

TileIndex AbstractMarkerTiler::NonEmptyIterator::nextIndex()
{
    if (d->atEnd)
    {
        return d->currentIndex;
    }

    Q_FOREVER
    {
        const int currentLevel = d->currentIndex.level();
//         kDebug() << d->level << currentLevel << d->atStartOfLevel << d->currentIndex;

        if (d->atStartOfLevel)
        {
            d->atStartOfLevel = false;
        }
        else
        {
            // go to the next tile at the current level, if that is possible:

            // determine the limits in the current tile:
            int limitLatBL   = 0;
            int limitLonBL   = 0;
            int limitLatTR   = TileIndex::Tiling-1;
            int limitLonTR   = TileIndex::Tiling-1;
            int compareLevel = currentLevel - 1;

            // check limit on the left side:
            bool onLimit = true;
            for (int i=0; onLimit && (i <= compareLevel); ++i)
            {
                onLimit = d->currentIndex.indexLat(i) == d->startIndex.indexLat(i);
            }
            if (onLimit)
            {
                limitLatBL = d->startIndex.indexLat(currentLevel);
            }

            // check limit on the bottom side:
            onLimit = true;
            for (int i=0; onLimit && (i <= compareLevel); ++i)
            {
                onLimit = d->currentIndex.indexLon(i) == d->startIndex.indexLon(i);
            }
            if (onLimit)
            {
                limitLonBL = d->startIndex.indexLon(currentLevel);
            }

            // check limit on the right side:
            onLimit = true;
            for (int i=0; onLimit && (i <= compareLevel); ++i)
            {
                onLimit = d->currentIndex.indexLat(i) == d->endIndex.indexLat(i);
            }
            if (onLimit)
            {
                limitLatTR = d->endIndex.indexLat(currentLevel);
            }

            // check limit on the top side:
            onLimit = true;
            for (int i=0; onLimit && (i <= compareLevel); ++i)
            {
                onLimit = d->currentIndex.indexLon(i) == d->endIndex.indexLon(i);
            }
            if (onLimit)
            {
                limitLonTR = d->endIndex.indexLon(currentLevel);
            }

            KMAP_ASSERT(limitLatBL <= limitLatTR);
            KMAP_ASSERT(limitLonBL <= limitLonTR);
//             kDebug() << limitLatBL << limitLonBL << limitLatTR << limitLonTR << compareLevel << currentLevel;

            int currentLat = d->currentIndex.indexLat(d->currentIndex.level());
            int currentLon = d->currentIndex.indexLon(d->currentIndex.level());

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
                        return d->currentIndex;
                    }

                    // we need to go one level up, trim the indices:
                    d->currentIndex.oneUp();

                    continue;
                }
            }

            // save the new position:
            d->currentIndex.oneUp();
            d->currentIndex.appendLatLonIndex(currentLat, currentLon);
        }

        // is the tile empty?
        if (d->model->getTileMarkerCount(d->currentIndex)==0)
        {
            continue;
        }

        // are we at the target level?
        if (currentLevel == d->level)
        {
            // yes, return the current index:
            return d->currentIndex;
        }

        // go one level down:
        int compareLevel = currentLevel;

        // determine the limits for the next level:
        int limitLatBL = 0;
        int limitLonBL = 0;
        int limitLatTR = TileIndex::Tiling-1;
        int limitLonTR = TileIndex::Tiling-1;

        // check limit on the left side:
        bool onLimit = true;
        for (int i=0; onLimit&&(i<=compareLevel); ++i)
        {
            onLimit = d->currentIndex.indexLat(i)==d->startIndex.indexLat(i);
        }
        if (onLimit)
        {
            limitLatBL = d->startIndex.indexLat(currentLevel+1);
        }

        // check limit on the bottom side:
        onLimit = true;
        for (int i=0; onLimit && (i <= compareLevel); ++i)
        {
            onLimit = d->currentIndex.indexLon(i)==d->startIndex.indexLon(i);
        }
        if (onLimit)
        {
            limitLonBL = d->startIndex.indexLon(currentLevel+1);
        }

        // check limit on the right side:
        onLimit = true;
        for (int i=0; onLimit && (i <= compareLevel); ++i)
        {
            onLimit = d->currentIndex.indexLat(i) == d->endIndex.indexLat(i);
        }
        if (onLimit)
        {
            limitLatTR = d->endIndex.indexLat(currentLevel+1);
        }

        // check limit on the top side:
        onLimit = true;
        for (int i=0; onLimit && (i <= compareLevel); ++i)
        {
            onLimit = d->currentIndex.indexLon(i) == d->endIndex.indexLon(i);
        }
        if (onLimit)
        {
            limitLonTR = d->endIndex.indexLon(currentLevel+1);
        }

        KMAP_ASSERT(limitLatBL <= limitLatTR);
        KMAP_ASSERT(limitLonBL <= limitLonTR);

        // go one level down:
        d->currentIndex.appendLatLonIndex(limitLatBL, limitLonBL);
        d->atStartOfLevel = true;
    }
}

TileIndex AbstractMarkerTiler::NonEmptyIterator::currentIndex() const
{
    return d->currentIndex;
}

bool AbstractMarkerTiler::NonEmptyIterator::atEnd() const
{
    return d->atEnd;
}

AbstractMarkerTiler* AbstractMarkerTiler::NonEmptyIterator::model() const
{
    return d->model;
}

bool AbstractMarkerTiler::isDirty() const
{
    return d->isDirty;
}

void AbstractMarkerTiler::setDirty(const bool state)
{
    if (state && !d->isDirty)
    {
        d->isDirty = true;
        emit(signalTilesOrSelectionChanged());
    }
    else
    {
        d->isDirty = state;
    }
}

AbstractMarkerTiler::Tile* AbstractMarkerTiler::resetRootTile()
{
    tileDelete(d->rootTile);
    d->rootTile = tileNew();

    return d->rootTile;
}

void AbstractMarkerTiler::onIndicesClicked(const TileIndex::List& tileIndicesList, const QVariant& representativeIndex,
                                           const KMapGroupState& groupSelectionState, const MouseModes currentMouseMode)
{
    Q_UNUSED(tileIndicesList);
    Q_UNUSED(groupSelectionState);
    Q_UNUSED(currentMouseMode);
}

void AbstractMarkerTiler::onIndicesMoved(const TileIndex::List& tileIndicesList, const GeoCoordinates& targetCoordinates,
                                         const QPersistentModelIndex& targetSnapIndex)
{
    Q_UNUSED(tileIndicesList);
    Q_UNUSED(targetCoordinates);
    Q_UNUSED(targetSnapIndex);
}

AbstractMarkerTiler::Tile* AbstractMarkerTiler::tileNew()
{
    return new Tile();
}

void AbstractMarkerTiler::tileDelete(AbstractMarkerTiler::Tile* const tile)
{
    tileDeleteChildren(tile);

    tileDeleteInternal(tile);
}

void AbstractMarkerTiler::tileDeleteInternal(AbstractMarkerTiler::Tile* const tile)
{
    delete tile;
}

void AbstractMarkerTiler::tileDeleteChildren(AbstractMarkerTiler::Tile* const tile)
{
    if (!tile)
        return;

    QVector<Tile*> tileChildren = tile->takeChildren();
    foreach(Tile* tilec, tileChildren)
    {
        tileDelete(tilec);
    }
}

void AbstractMarkerTiler::tileDeleteChild(AbstractMarkerTiler::Tile* const parentTile, AbstractMarkerTiler::Tile* const childTile, const int knownLinearIndex)
{
    int tileIndex = knownLinearIndex;
    if (tileIndex < 0)
    {
        tileIndex = parentTile->indexOfChildTile(childTile);
    }
    parentTile->clearChild(tileIndex);

    tileDelete(childTile);
}

AbstractMarkerTiler::Flags AbstractMarkerTiler::tilerFlags() const
{
    return FlagNull;
}

void AbstractMarkerTiler::clear()
{
    tileDelete(d->rootTile);
    d->rootTile = 0;
}

} /* namespace KMap */
