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
      tile0(new MarkerModel::Tile())
    {
        const QIntPair level0Sizes(18, 36);
        tesselationSizes << level0Sizes;
        for (int i = 0; i<8; ++i)
        {
            tesselationSizes << QIntPair(10, 10);
        }

        tile0->prepareForChildren(level0Sizes);
    }

    QList<QIntPair> tesselationSizes;
    MarkerModel::Tile* const tile0;
};

MarkerModel::MarkerModel()
: d(new MarkerModelPrivate())
{
}

MarkerModel::~MarkerModel()
{
    // delete all tiles
    delete d->tile0;

    delete d;
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

int MarkerModel::latLonIndexToLinearIndex(const int latIndex, const int lonIndex, const int level)
{
    WMW2_ASSERT(level<=maxLevel());

    const QIntPair tesselationSizes = d->tesselationSizes.at(level);
    const int nLon = tesselationSizes.second;
    const int linearIndex = latIndex*nLon + lonIndex;

    WMW2_ASSERT(linearIndex<(tesselationSizes.first*tesselationSizes.second));

    return linearIndex;
}

void MarkerModel::linearIndexToLatLonIndex(const int linearIndex, const int level, int* const latIndex, int* const lonIndex)
{
    WMW2_ASSERT(level<=maxLevel());

    const QIntPair tesselationSizes = d->tesselationSizes.at(level);
    const int nLon = tesselationSizes.second;
    *latIndex = linearIndex/nLon;
    *lonIndex = linearIndex%nLon;
    WMW2_ASSERT(*latIndex<tesselationSizes.first);
    WMW2_ASSERT(*lonIndex<tesselationSizes.second);
}

int MarkerModel::addMarker(const WMWMarker& newMarker)
{
    const int markerIndex = markerList.count();
    markerList<<newMarker;

    const QIntList tileIndex = coordinateToTileIndex(newMarker.coordinates, maxLevel());
    WMW2_ASSERT(tileIndex.count()==maxIndexCount());

    // add the marker to all existing tiles:
    Tile* currentTile = d->tile0;
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

    Tile* const myTile = getTile(tileIndex);
    WMW2_ASSERT(myTile!=0);

    return myTile->markerIndices.count();
}

MarkerModel::Tile* MarkerModel::getTile(const QIntList& tileIndex)
{
    WMW2_ASSERT(tileIndex.count()<=maxIndexCount());

    Tile* tile = d->tile0;
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

} /* WMW2 */

