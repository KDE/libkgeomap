/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  A model to hold the markers
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ABSTRACTMARKERTILER_H
#define ABSTRACTMARKERTILER_H

// Qt includes

#include <QBitArray>
#include <QItemSelectionModel>
#include <QObject>

// local includes

#include "kmap_primitives.h"
#include "libkmap_export.h"

namespace KMapIface
{

class AbstractMarkerTilerPrivate;
class AbstractMarkerTilerNonEmptyIteratorPrivate;

class KMAP_EXPORT AbstractMarkerTiler : public QObject
{
Q_OBJECT

public:

    class TileIndex
    {
    public:
        enum Constants
        {
            MaxLevel       = 9,
            MaxIndexCount  = MaxLevel+1,
            Tiling         = 10,
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
            KMAP_ASSERT(m_indicesCount+1<=MaxIndexCount);
            m_indices[m_indicesCount] = newIndex;
            m_indicesCount++;
        }

        inline int linearIndex(const int getLevel) const
        {
            KMAP_ASSERT(getLevel<=level());
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
            KMAP_ASSERT(getLevel<=level());
            *latIndex = indexLat(getLevel);
            *lonIndex = indexLon(getLevel);
            KMAP_ASSERT(*latIndex<Tiling);
            KMAP_ASSERT(*lonIndex<Tiling);
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

        static TileIndex fromCoordinates(const KMapIface::WMWGeoCoordinate& coordinate, const int getLevel);

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
            KMAP_ASSERT(a.level()>=upToLevel);
            KMAP_ASSERT(b.level()>=upToLevel);

            for (int i=0; i<=upToLevel; ++i)
            {
                if (a.linearIndex(i)!=b.linearIndex(i))
                    return false;
            }

            return true;
        }

        inline TileIndex mid(const int first, const int len) const
        {
            KMAP_ASSERT(first+(len-1)<=m_indicesCount);
            TileIndex result;
            for (int i = first; i<first+len; ++i)
            {
                result.appendLinearIndex(m_indices[i]);
            }

            return result;
        }

        inline void oneUp()
        {
            KMAP_ASSERT(m_indicesCount>0);
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

        QVector<Tile*>               children;
        QBitArray                    childrenMask;
        QList<QPersistentModelIndex> markerIndices;
        int                          selectedCount;
    };

    AbstractMarkerTiler(QObject* const parent = 0);
    ~AbstractMarkerTiler();

    void setMarkerModelHelper(WMWModelHelper* const modelHelper);
    QItemSelectionModel* getSelectionModel() const;

    int getTileMarkerCount(const TileIndex& tileIndex);

    int getTileSelectedCount(const TileIndex& tileIndex);

    QList<QPersistentModelIndex> getTileMarkerIndices(const TileIndex& tileIndex);

    QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey);
    QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey);
    QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size);
    bool indicesEqual(const QVariant& a, const QVariant& b) const;

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

        NonEmptyIterator(AbstractMarkerTiler* const model, const int level);
        NonEmptyIterator(AbstractMarkerTiler* const model, const int level, const TileIndex& startIndex, const TileIndex& endIndex);
        NonEmptyIterator(AbstractMarkerTiler* const model, const int level, const WMWGeoCoordinate::PairList& normalizedMapBounds);
        ~NonEmptyIterator();

        bool atEnd() const;
        TileIndex nextIndex();
        TileIndex currentIndex() const;
        AbstractMarkerTiler* model() const;

    private:

        bool initializeNextBounds();
        AbstractMarkerTilerNonEmptyIteratorPrivate* const d;
    };

private Q_SLOTS:

    void slotSourceModelRowsInserted(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelRowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotSourceModelReset();
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void slotThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap);

Q_SIGNALS:

    void signalTilesOrSelectionChanged();
    void signalThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);

private:

    AbstractMarkerTilerPrivate* const d;
};

} /* namespace KMapIface */

inline QDebug operator<<(QDebug debugOut, const KMapIface::AbstractMarkerTiler::TileIndex& tileIndex)
{
    debugOut << tileIndex.toIntList();
    return debugOut;
}

#endif /* ABSTRACTMARKERTILER_H */
