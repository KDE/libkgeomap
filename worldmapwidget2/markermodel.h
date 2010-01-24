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

#ifndef MARKERMODEL_H
#define MARKERMODEL_H

// Qt includes

#include <QBitArray>

// local includes

#include "worldmapwidget2_primitives.h"

namespace WMW2 {

class MarkerModelPrivate;
class MarkerModelNonEmptyIteratorPrivate;

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

        ~Tile()
        {
            deleteChildren();
        }

        void deleteChildren()
        {
            foreach(const Tile* tile, children)
            {
                delete tile;
            }
            children.clear();
            childrenMask.clear();
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

        void deleteChild(Tile* const childTile, const int knownLinearIndex = -1)
        {
            int tileIndex = knownLinearIndex;
            if (tileIndex<0)
            {
                tileIndex = children.indexOf(childTile);
            }
            children.replace(tileIndex, 0);
            delete childTile;
        }

        QVector<Tile*> children;
        QBitArray childrenMask;
        QIntList markerIndices;
    };

    MarkerModel();
    ~MarkerModel();

    WMWGeoCoordinate tileIndexToCoordinate(const QIntList& tileIndex);
    QIntList coordinateToTileIndex(const WMWGeoCoordinate& coordinate, const int level);

    int latLonIndexToLinearIndex(const int latIndex, const int lonIndex, const int level) const;
    void linearIndexToLatLonIndex(const int linearIndex, const int level, int* const latIndex, int* const lonIndex) const;
    bool indicesEqual(const QIntList& a, const QIntList& b, const int upToLevel) const;
    QList<QIntPair> linearIndexToLatLonIndex(const QIntList& linearIndex) const;
    QIntList latLonIndexToLinearIndex(const QList<QIntPair>& latLonIndex) const;

    int addMarker(const WMWMarker& newMarker);
    void removeMarkerIndexFromGrid(const int markerIndex);
    void addMarkers(const WMWMarker::List& newMarkers);
    void addMarkerIndexToGrid(const int markerIndex);
    void moveMarker(const int markerIndex, const WMWGeoCoordinate& newPosition);
    void clear();
    int getTileMarkerCount(const QIntList& tileIndex);
    Tile* getTile(const QIntList& tileIndex, const bool stopIfEmpty = false);
    int maxLevel() const;
    int maxIndexCount() const;
    Tile* rootTile() const;
    QPair<int, int> getTesselationSizes(const int level) const;

    WMWMarker::List markerList;

    class NonEmptyIterator
    {
    public:
        NonEmptyIterator(MarkerModel* const model, const int level);
        NonEmptyIterator(MarkerModel* const model, const int level, const QIntList& startIndex, const QIntList& endIndex);
        NonEmptyIterator(MarkerModel* const model, const int level, const QList<QPair<WMWGeoCoordinate, WMWGeoCoordinate> >& normalizedMapBounds);

        bool atEnd() const;
        QIntList nextIndex();
        QIntList currentIndex() const;
        MarkerModel* model() const;

    private:
        bool initializeNextBounds();
        MarkerModelNonEmptyIteratorPrivate* const d;
    };

private:
    MarkerModelPrivate* const d;
};

} /* WMW2 */

#endif /* MARKERMODEL_H */

