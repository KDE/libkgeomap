/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : A model to hold the markers
 *
 * Copyright (C) 2009 by Michael G. Hansen <mike at mghansen dot de>
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

// KDE includes

#include <kdebug.h>

// local includes

#include "markermodel.h"

namespace WMW2 {

typedef QPair<int, int> QIntPair;
class MarkerModelPrivate
{
public:
    MarkerModelPrivate()
    : tesselationSizes(),
      rootTile(new MarkerModel::Tile())
    {
        const QIntPair level0Sizes(18, 36);
        tesselationSizes << level0Sizes;
        for (int i = 0; i<8; ++i)
        {
            tesselationSizes << QIntPair(10, 10);
        }

        rootTile->prepareForChildren(level0Sizes);
    }

    QList<QIntPair> tesselationSizes;
    MarkerModel::Tile* const rootTile;
};

MarkerModel::MarkerModel()
: d(new MarkerModelPrivate())
{
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

        int latIndex = int( (coordinate.lat - tileLatBL ) / dLat );
        int lonIndex = int( (coordinate.lon - tileLonBL ) / dLon );

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

void MarkerModel::addMarkers(const WMWMarker::List& newMarkers)
{
    Q_FOREACH(const WMWMarker& newMarker, newMarkers)
    {
        addMarker(newMarker);
    }
}

int MarkerModel::addMarker(const WMWMarker& newMarker)
{
    const int markerIndex = markerList.count();
    markerList<<newMarker;

    const QIntList tileIndex = coordinateToTileIndex(newMarker.coordinates, maxLevel());
    WMW2_ASSERT(tileIndex.count()==maxIndexCount());

    // add the marker to all existing tiles:
    Tile* currentTile = d->rootTile;
    for (int l = 0; l<=maxLevel(); ++l)
    {
        currentTile->markerIndices<<markerIndex;

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
        currentTile = nextTile;
    }

    return markerIndex;
}

int MarkerModel::getTileMarkerCount(const QIntList& tileIndex)
{
    WMW2_ASSERT(tileIndex.count()<=maxIndexCount());

    Tile* const myTile = getTile(tileIndex, true);

    if (!myTile)
    {
        return 0;
    }

    return myTile->markerIndices.count();
}

MarkerModel::Tile* MarkerModel::getTile(const QIntList& tileIndex, const bool stopIfEmpty)
{
    WMW2_ASSERT(tileIndex.count()<=maxIndexCount());

    Tile* tile = d->rootTile;
    for (int level = 0; level < tileIndex.size(); ++level)
    {
        const int currentIndex = tileIndex.at(level);

        Tile* childTile = 0;
        if (tile->children.isEmpty())
        {
            tile->prepareForChildren(d->tesselationSizes.at(level));

            // if there are any markers in the tile,
            // we have to sort them into the child tiles:
            if (!tile->markerIndices.isEmpty())
            {
                for (int i=0; i<tile->markerIndices.count(); ++i)
                {
                    const int currentMarkerIndex = tile->markerIndices.at(i);
                    
                    // get the tile index for this marker:
                    const QIntList markerTileIndex = coordinateToTileIndex(markerList.at(currentMarkerIndex).coordinates, level);
                    const int newTileIndex = markerTileIndex.last();

                    Tile* newTile = tile->children.at(newTileIndex);
                    if (newTile==0)
                    {
                        newTile = new Tile();
                        tile->addChild(newTileIndex, newTile);
                    }
                    newTile->markerIndices<<currentMarkerIndex;
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

MarkerModel::Tile* MarkerModel::rootTile() const
{
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

    QIntList startIndexLinear;
    QIntList endIndexLinear;
    QIntList currentIndexLinear;

    QList<QIntPair> startIndices;
    QList<QIntPair> endIndices;
    QList<QIntPair> currentIndices;

    bool atEnd;
};

MarkerModel::NonEmptyIterator::NonEmptyIterator(MarkerModel* const model, const int level)
: d(new MarkerModelNonEmptyIteratorPrivate())
{
    d->model = model;
    d->level = level;

    for (int i=0; i<=level; ++i)
    {
        d->startIndexLinear<<0;
        QIntPair currentSizes = d->model->getTesselationSizes(i);
        d->endIndexLinear<<currentSizes.first*currentSizes.second - 1;
    }
    kDebug()<<d->startIndexLinear<<d->endIndexLinear;

    initializeIterator();
}

MarkerModel::NonEmptyIterator::NonEmptyIterator(MarkerModel* const model, const int level, const QIntList& startIndex, const QIntList& endIndex)
: d(new MarkerModelNonEmptyIteratorPrivate())
{
    d->model = model;
    d->level = level;
    d->startIndexLinear = startIndex;
    d->endIndexLinear = endIndex;

    initializeIterator();
}

void MarkerModel::NonEmptyIterator::initializeIterator()
{
    WMW2_ASSERT(d->startIndexLinear.count() == d->level + 1);
    WMW2_ASSERT(d->endIndexLinear.count() == d->level + 1);

    // calculate the 'cartesian' start/end indices:
    d->startIndices = d->model->linearIndexToLatLonIndex(d->startIndexLinear);
    d->endIndices = d->model->linearIndexToLatLonIndex(d->endIndexLinear);

    d->currentIndexLinear = d->startIndexLinear;
    d->currentIndices = d->startIndices;

    if (d->model->getTileMarkerCount(d->currentIndexLinear)==0)
    {
        nextIndex();
    }
}

QIntList MarkerModel::NonEmptyIterator::nextIndex()
{
    if (d->atEnd)
    {
        return d->currentIndexLinear;
    }

    // TODO: stoppedhere
    bool doContinue = true;
    int progressLevel = d->level;
    do {
//         kDebug()<<d->currentIndexLinear<<progressLevel;
        // determine the limits in the current tile:
        int limitLatBL = 0;
        int limitLonBL = 0;
        int limitLatTR = d->model->getTesselationSizes(progressLevel).first-1;
        int limitLonTR = d->model->getTesselationSizes(progressLevel).second-1;

        // check limit on the left side:
        int compareLevel = d->currentIndices.count();
        bool onLimit = true;
        for (int i=0; onLimit&&(i<compareLevel); ++i)
        {
            onLimit = d->currentIndices.at(i).first==d->startIndices.at(i).first;
        }
        if (onLimit)
        {
            limitLatBL = d->startIndices.at(progressLevel).first;
        }

        // check limit on the bottom side:
        onLimit = true;
        for (int i=0; onLimit&&(i<compareLevel); ++i)
        {
            onLimit = d->currentIndices.at(i).first==d->startIndices.at(i).second;
        }
        if (onLimit)
        {
            limitLonBL = d->startIndices.at(progressLevel).second;
        }

        // check limit on the right side:
        onLimit = true;
        for (int i=0; onLimit&&(i<compareLevel); ++i)
        {
            onLimit = d->currentIndices.at(i).first==d->endIndices.at(i).first;
        }
        if (onLimit)
        {
            limitLatTR = d->endIndices.at(progressLevel).first;
        }

        // check limit on the top side:
        onLimit = true;
        for (int i=0; onLimit&&(i<compareLevel); ++i)
        {
            onLimit = d->currentIndices.at(i).first==d->endIndices.at(i).second;
        }
        if (onLimit)
        {
            limitLonTR = d->endIndices.at(progressLevel).second;
        }

//         kDebug()<<limitLatBL<<limitLonBL<<limitLatTR<<limitLonTR;
        
        if (d->currentIndices.count()<progressLevel+1)
        {
            d->currentIndices<<QIntPair(limitLatBL, limitLonBL);
        }
        else
        {
            int currentLat = d->currentIndices.last().first;
            int currentLon = d->currentIndices.last().second;

            currentLon++;
            if (currentLon>limitLonTR)
            {
                currentLon = limitLonBL;
                currentLat++;
                if (currentLat>limitLatTR)
                {
                    if (progressLevel == 0)
                    {
                        // we are at the end!
                        d->atEnd = true;
                        return d->currentIndexLinear;
                    }

                    kDebug()<<"up one!";
                    // we need to go one level up:
                    progressLevel--;

                    // trim the indices:
                    d->currentIndexLinear.removeLast();
                    d->currentIndices.removeLast();

                    continue;
                }
            }

            d->currentIndices.last() = QIntPair(currentLat, currentLon);
        }
        d->currentIndexLinear = d->model->latLonIndexToLinearIndex(d->currentIndices);

        // is there anything in this tile?
        if (d->model->getTileMarkerCount(d->currentIndexLinear)>0)
        {
            kDebug()<<"hit: "<<d->currentIndexLinear<<progressLevel;
            // are we at the target level?
            if (progressLevel == d->level)
            {
                // yes, return the current index:
                return d->currentIndexLinear;
            }

            // no, go one level down:
            progressLevel++;

            continue;
        }

        // no, the tile is empty. go to the next tile!
    } while (doContinue);
}

QIntList MarkerModel::NonEmptyIterator::currentIndex() const
{
    return d->currentIndexLinear;
}

bool MarkerModel::NonEmptyIterator::atEnd() const
{
    return d->atEnd;
}


} /* WMW2 */

