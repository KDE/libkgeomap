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

        enum SelectionState {
            SelectedNone = 0,
            SelectedSome = 1,
            SelectedAll = 2
        };
        
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

        SelectionState getSelectionState() const
        {
            if (selectedCount==0)
            {
                return SelectedNone;
            }
            else if (selectedCount==markerIndices.count())
            {
                return SelectedAll;
            }

            return SelectedSome;
        }

        QVector<Tile*> children;
        QBitArray childrenMask;
        QList<QPersistentModelIndex> markerIndices;
        int selectedCount;
    };

    MarkerModel();
    ~MarkerModel();

    void setMarkerModel(QAbstractItemModel* const markerModel, const int coordinatesRole);

//     Qt::ItemFlags flags(const QModelIndex& index) const;
//     QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const;
//     QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
//     int rowCount(const QModelIndex& parent = QModelIndex()) const;
//     int columnCount(const QModelIndex& parent = QModelIndex()) const;
//     bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

    WMWGeoCoordinate tileIndexToCoordinate(const QIntList& tileIndex);
    QIntList coordinateToTileIndex(const WMWGeoCoordinate& coordinate, const int level);

    int latLonIndexToLinearIndex(const int latIndex, const int lonIndex, const int level) const;
    void linearIndexToLatLonIndex(const int linearIndex, const int level, int* const latIndex, int* const lonIndex) const;
    bool indicesEqual(const QIntList& a, const QIntList& b, const int upToLevel) const;
    QList<QIntPair> linearIndexToLatLonIndex(const QIntList& linearIndex) const;
    QIntList latLonIndexToLinearIndex(const QList<QIntPair>& latLonIndex) const;

    void moveMarker(const QPersistentModelIndex& markerIndex, const WMWGeoCoordinate& newPosition);
    int getTileMarkerCount(const QIntList& tileIndex);
    Tile* getTile(const QIntList& tileIndex, const bool stopIfEmpty = false);
    int maxLevel() const;
    int maxIndexCount() const;
    Tile* rootTile() const;
    QPair<int, int> getTesselationSizes(const int level) const;

    void setSelectionModel(QItemSelectionModel* const selectionModel);

private:
    void removeMarkerIndexFromGrid(const QPersistentModelIndex& markerIndex);
    void addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex);

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
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    MarkerModelPrivate* const d;
};

} /* WMW2 */

#endif /* MARKERMODEL_H */

