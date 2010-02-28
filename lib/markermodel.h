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
#include <QItemSelectionModel>
#include <QObject>

// local includes

#include "worldmapwidget2_primitives.h"

namespace WMW2 {

class MarkerModelPrivate;
class MarkerModelNonEmptyIteratorPrivate;

class MarkerModel : public QObject/* : public QAbstractItemModel*/
{
Q_OBJECT

public:

    class Tile
    {
    public:
        
        Tile()
        : children(),
          childrenMask(),
          markerIndices(),
          selectedCount(0)
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

        void removeMarkerIndexOrInvalidIndex(const QModelIndex& indexToRemove)
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

        QVector<Tile*> children;
        QBitArray childrenMask;
        QList<QPersistentModelIndex> markerIndices;
        int selectedCount;
    };

    MarkerModel();
    ~MarkerModel();

    void setMarkerModel(QAbstractItemModel* const markerModel, const int coordinatesRole);
    void setSelectionModel(QItemSelectionModel* const selectionModel);
    QItemSelectionModel* getSelectionModel() const;

    WMWGeoCoordinate tileIndexToCoordinate(const QIntList& tileIndex);
    QIntList coordinateToTileIndex(const WMWGeoCoordinate& coordinate, const int level);

    void moveMarker(const QPersistentModelIndex& markerIndex, const WMWGeoCoordinate& newPosition);
    int getTileMarkerCount(const QIntList& tileIndex);
    int getTileSelectedCount(const QIntList& tileIndex);
    QList<QPersistentModelIndex> getTileMarkerIndices(const QIntList& tileIndex);
    WMWSelectionState getTileSelectedState(const QIntList& tileIndex);
    int maxLevel() const;
    int maxIndexCount() const;
    QPair<int, int> getTesselationSizes(const int level) const;

    // to be made protected:
// protected:
    void removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection = false);
    void addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex);
    void regenerateTiles();
    Tile* getTile(const QIntList& tileIndex, const bool stopIfEmpty = false);
    Tile* rootTile();
    QList<QIntPair> linearIndexToLatLonIndex(const QIntList& linearIndex) const;
    QIntList latLonIndexToLinearIndex(const QList<QIntPair>& latLonIndex) const;
    int latLonIndexToLinearIndex(const int latIndex, const int lonIndex, const int level) const;
    void linearIndexToLatLonIndex(const int linearIndex, const int level, int* const latIndex, int* const lonIndex) const;
    bool indicesEqual(const QIntList& a, const QIntList& b, const int upToLevel) const;

public:

    class NonEmptyIterator
    {
    public:
        NonEmptyIterator(MarkerModel* const model, const int level);
        NonEmptyIterator(MarkerModel* const model, const int level, const QIntList& startIndex, const QIntList& endIndex);
        NonEmptyIterator(MarkerModel* const model, const int level, const WMWGeoCoordinate::PairList& normalizedMapBounds);
        ~NonEmptyIterator();

        bool atEnd() const;
        QIntList nextIndex();
        QIntList currentIndex() const;
        MarkerModel* model() const;

    private:
        bool initializeNextBounds();
        MarkerModelNonEmptyIteratorPrivate* const d;
    };

private Q_SLOTS:
    void slotSourceModelRowsInserted(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelRowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

Q_SIGNALS:
    void signalTilesOrSelectionChanged();

private:
    MarkerModelPrivate* const d;
};

} /* WMW2 */

#endif /* MARKERMODEL_H */

