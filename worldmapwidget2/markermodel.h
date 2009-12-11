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

#ifndef MARKERMODEL_H
#define MARKERMODEL_H

// Qt includes

#include <QBitArray>

// local includes

#include "worldmapwidget2_primitives.h"

namespace WMW2 {

class MarkerModelPrivate;

class MarkerModel
{
public:

    class Tile
    {
    public:
        Tile()
        : children(),
          childrenMask(),
          markerIndices()
        {
        }

        void prepareForChildren(const QPair<int, int>& childCount)
        {
            prepareForChildren(childCount.first*childCount.second);
        }

        void prepareForChildren(const int childCount)
        {
            childrenMask.resize(childCount);
            children = QVector<Tile*>(childCount, 0);
        }

        void addChild(const int linearIndex, Tile* const tilePointer = 0)
        {
            childrenMask.setBit(linearIndex);
            children[linearIndex] = tilePointer;
        }

        bool childValid(const int linearIndex)
        {
            return childrenMask.testBit(linearIndex);
        }

        QVector<Tile*> children;
        QBitArray childrenMask;
        QIntList markerIndices;
    };

    MarkerModel();
    ~MarkerModel();

    WMWGeoCoordinate tileIndexToCoordinate(const QIntList& tileIndex);
    QIntList coordinateToTileIndex(const WMWGeoCoordinate& coordinate, const int level);

    int latLonIndexToLinearIndex(const int latIndex, const int lonIndex, const int level);
    void linearIndexToLatLonIndex(const int linearIndex, const int level, int* const latIndex, int* const lonIndex);

    int addMarker(const WMWMarker& newMarker);
    int getTileMarkerCount(const QIntList& tileIndex);
    Tile* getTile(const QIntList& tileIndex);
    int maxLevel() const;

    WMWMarker::List markerList;

private:
    MarkerModelPrivate* const d;
};

} /* WMW2 */

#endif /* MARKERMODEL_H */

