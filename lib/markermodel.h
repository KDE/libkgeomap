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
#include "worldmapwidget2_export.h"

namespace WMW2 {

class MarkerModelPrivate;
class MarkerModelNonEmptyIteratorPrivate;

class WORLDMAPWIDGET2_EXPORT MarkerModel : public QObject/* : public QAbstractItemModel*/
{
Q_OBJECT

public:

    class TileIndex
    {
    public:
        enum Constants {
            MaxLevel = 9,
            MaxIndexCount = MaxLevel+1,
            Tiling = 10,
            MaxLinearIndex = Tiling*Tiling
        };

        inline TileIndex()
        : m_indicesCount(0),
          m_indices()
        {
        }

        inline int indexCount() const
        {
            return m_indicesCount;
        }

        inline int level() const
        {
            return m_indicesCount > 0 ? m_indicesCount - 1 : 0;
        }

        inline void clear()
        {
            m_indicesCount = 0;
        }

        inline void appendLinearIndex(const int newIndex)
        {
            WMW2_ASSERT(m_indicesCount+1<=MaxIndexCount);
            m_indices[m_indicesCount] = newIndex;
            m_indicesCount++;
        }

        inline int linearIndex(const int getLevel) const
        {
            WMW2_ASSERT(getLevel<=level());
            return m_indices[getLevel];
        }

        inline int indexLat(const int getLevel) const
        {
            return linearIndex(getLevel) / Tiling;
        }

        inline int indexLon(const int getLevel) const
        {
            return linearIndex(getLevel) % Tiling;
        }

        inline QPoint latLonIndex(const int getLevel) const
        {
            return QPoint(indexLon(getLevel), indexLat(getLevel));
        }

        inline void latLonIndex(const int getLevel, int* const latIndex, int* const lonIndex) const
        {
            WMW2_ASSERT(getLevel<=level());
            *latIndex = indexLat(getLevel);
            *lonIndex = indexLon(getLevel);
            WMW2_ASSERT(*latIndex<Tiling);
            WMW2_ASSERT(*lonIndex<Tiling);
        }

        inline void appendLatLonIndex(const int latIndex, const int lonIndex)
        {
            appendLinearIndex(latIndex*Tiling + lonIndex);
        }

        inline QIntList toIntList() const
        {
            QIntList result;
            for (int i=0; i<m_indicesCount; ++i)
            {
                result << m_indices[i];
            }
            return result;
        }

        static TileIndex fromCoordinates(const WMW2::WMWGeoCoordinate& coordinate, const int getLevel);

        WMWGeoCoordinate toCoordinates() const;

        inline static TileIndex fromIntList(const QIntList& intList)
        {
            TileIndex result;
            for (int i=0; i<intList.count(); ++i)
                result.appendLinearIndex(intList.at(i));

            return result;
        }

        inline static bool indicesEqual(const TileIndex& a, const TileIndex& b, const int upToLevel)
        {
            WMW2_ASSERT(a.level()>=upToLevel);
            WMW2_ASSERT(b.level()>=upToLevel);

            for (int i=0; i<=upToLevel; ++i)
            {
                if (a.linearIndex(i)!=b.linearIndex(i))
                    return false;
            }

            return true;
        }

        inline TileIndex mid(const int first, const int len) const
        {
            WMW2_ASSERT(first+(len-1)<=m_indicesCount);
            TileIndex result;
            for (int i = first; i<first+len; ++i)
            {
                result.appendLinearIndex(m_indices[i]);
            }

            return result;
        }

        inline void oneUp()
        {
            WMW2_ASSERT(m_indicesCount>0);
            m_indicesCount--;
        }

        inline static QList<QIntList> listToIntListList(const QList<TileIndex>& tileIndexList)
        {
            QList<QIntList> result;
            for (int i=0; i<tileIndexList.count(); ++i)
            {
                result << tileIndexList.at(i).toIntList();
            }
            return result;
        }

    private:
        int m_indicesCount;
        int m_indices[MaxIndexCount];
    };

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

    void moveMarker(const QPersistentModelIndex& markerIndex, const WMWGeoCoordinate& newPosition);

    int getTileMarkerCount(const TileIndex& tileIndex);

    int getTileSelectedCount(const TileIndex& tileIndex);

    QList<QPersistentModelIndex> getTileMarkerIndices(const TileIndex& tileIndex);

    QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey);
    WMWSelectionState getTileSelectedState(const TileIndex& tileIndex);

    // to be made protected:
// protected:
    void removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection = false);
    void addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex);
    void regenerateTiles();

    Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty = false);

    Tile* rootTile();
    bool indicesEqual(const QIntList& a, const QIntList& b, const int upToLevel) const;

public:

    class NonEmptyIterator
    {
    public:
        NonEmptyIterator(MarkerModel* const model, const int level);
        NonEmptyIterator(MarkerModel* const model, const int level, const TileIndex& startIndex, const TileIndex& endIndex);
        NonEmptyIterator(MarkerModel* const model, const int level, const WMWGeoCoordinate::PairList& normalizedMapBounds);
        ~NonEmptyIterator();

        bool atEnd() const;
        TileIndex nextIndex();
        TileIndex currentIndex() const;
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

inline QDebug operator<<(QDebug debugOut, const WMW2::MarkerModel::TileIndex& tileIndex)
{
    debugOut << tileIndex.toIntList();
    return debugOut;
}

#endif /* MARKERMODEL_H */

