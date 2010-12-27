/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  An abstract base class for tiling of markers
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
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

#ifndef ABSTRACTMARKERTILER_H
#define ABSTRACTMARKERTILER_H

// Qt includes

#include <QBitArray>
#include <QObject>
#include <QPoint>

// local includes

#include "kmap_primitives.h"
#include "libkmap_export.h"

namespace KMap
{

class KMAP_EXPORT AbstractMarkerTiler : public QObject
{
    Q_OBJECT

public:

    enum Flag
    {
        FlagNull    = 0,
        FlagMovable = 1
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:

    class KMAP_EXPORT TileIndex
    {
    public:

        enum Constants
        {
            MaxLevel       = 9,
            MaxIndexCount  = MaxLevel+1,
            Tiling         = 10,
            MaxLinearIndex = Tiling*Tiling
        };

        enum CornerPosition
        {
            CornerNW = 1,
            CornerSW = 2,
            CornerNE = 3,
            CornerSE = 4
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
            KMAP_ASSERT(getLevel <= level());
            *latIndex = indexLat(getLevel);
            *lonIndex = indexLon(getLevel);
            KMAP_ASSERT(*latIndex < Tiling);
            KMAP_ASSERT(*lonIndex < Tiling);
        }

        inline void appendLatLonIndex(const int latIndex, const int lonIndex)
        {
            appendLinearIndex(latIndex*Tiling + lonIndex);
        }

        inline QIntList toIntList() const
        {
            QIntList result;
            for (int i=0; i < m_indicesCount; ++i)
            {
                result << m_indices[i];
            }
            return result;
        }

        static TileIndex fromCoordinates(const KMap::GeoCoordinates& coordinate, const int getLevel);

        GeoCoordinates toCoordinates() const;

        GeoCoordinates toCoordinates(const CornerPosition ofCorner) const;

        inline static TileIndex fromIntList(const QIntList& intList)
        {
            TileIndex result;
            for (int i=0; i < intList.count(); ++i)
                result.appendLinearIndex(intList.at(i));

            return result;
        }

        inline static bool indicesEqual(const TileIndex& a, const TileIndex& b, const int upToLevel)
        {
            KMAP_ASSERT(a.level() >= upToLevel);
            KMAP_ASSERT(b.level() >= upToLevel);

            for (int i=0; i <= upToLevel; ++i)
            {
                if (a.linearIndex(i)!=b.linearIndex(i))
                    return false;
            }

            return true;
        }

        inline TileIndex mid(const int first, const int len) const
        {
            KMAP_ASSERT(first+(len-1) <= m_indicesCount);
            TileIndex result;
            for (int i = first; i < first+len; ++i)
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
            for (int i=0; i < tileIndexList.count(); ++i)
            {
                result << tileIndexList.at(i).toIntList();
            }
            return result;
        }

        typedef QList<TileIndex> List;

    private:

        int m_indicesCount;
        int m_indices[MaxIndexCount];
    };

public:

    class Tile
    {
    public:

        Tile()
            : children(),
              childrenMask(),
              selectedCount(0),
              markerCount(0)
        {
        }

        ~Tile()
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

    public:

        QVector<Tile*> children;
        QBitArray      childrenMask;
        int            selectedCount;
        int            markerCount;
    };

public:

    class NonEmptyIterator
    {
    public:

        NonEmptyIterator(AbstractMarkerTiler* const model, const int level);
        NonEmptyIterator(AbstractMarkerTiler* const model, const int level, const TileIndex& startIndex, const TileIndex& endIndex);
        NonEmptyIterator(AbstractMarkerTiler* const model, const int level, const GeoCoordinates::PairList& normalizedMapBounds);
        ~NonEmptyIterator();

        bool atEnd() const;
        TileIndex nextIndex();
        TileIndex currentIndex() const;
        AbstractMarkerTiler* model() const;

    private:

        bool initializeNextBounds();

    private:

        class AbstractMarkerTilerNonEmptyIteratorPrivate;
        AbstractMarkerTilerNonEmptyIteratorPrivate* const d;
    };

public:

    AbstractMarkerTiler(QObject* const parent = 0);
    virtual ~AbstractMarkerTiler();

    void tileDeleteChildren(Tile* const tile);
    void tileDelete(Tile* const tile);
    void tileDeleteChild(Tile* const parentTile, Tile* const childTile, const int knownLinearIndex = -1);

    // these have to be implemented
    virtual Flags tilerFlags() const;
    virtual Tile* tileNew();
    virtual void tileDeleteInternal(Tile* const tile);
    virtual void prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level) = 0;
    virtual void regenerateTiles() = 0;
    virtual Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty = false) = 0;
    virtual int getTileMarkerCount(const TileIndex& tileIndex) = 0;
    virtual int getTileSelectedCount(const TileIndex& tileIndex) = 0;

    // these should be implemented for thumbnail handling
    virtual QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey) = 0;
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey) = 0;
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size) = 0;
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const = 0;
    virtual KMapSelectionState getTileSelectedState(const TileIndex& tileIndex) = 0;

    // these can be implemented if you want to react to actions in kmap
    /// @todo Make currentMouseMode const
    virtual void onIndicesClicked(const TileIndex::List& tileIndicesList, const KMapSelectionState& groupSelectionState, MouseMode currentMouseMode);
    virtual void onIndicesMoved(const TileIndex::List& tileIndicesList, const GeoCoordinates& targetCoordinates, const QPersistentModelIndex& targetSnapIndex);

    virtual void setActive(const bool state) = 0;
    Tile* rootTile();
    bool indicesEqual(const QIntList& a, const QIntList& b, const int upToLevel) const;
    bool isDirty() const;
    void setDirty(const bool state = true);
    Tile* resetRootTile();

Q_SIGNALS:

    void signalTilesOrSelectionChanged();
    void signalThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);

protected:

    /**
     * @brief Only used to safely delete all tiles in the desctructor
     */
    void clear();

private:

    class AbstractMarkerTilerPrivate;
    AbstractMarkerTilerPrivate* const d;
};

} /* namespace KMap */

inline QDebug operator<<(QDebug debugOut, const KMap::AbstractMarkerTiler::TileIndex& tileIndex)
{
    debugOut << tileIndex.toIntList();
    return debugOut;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KMap::AbstractMarkerTiler::Flags)

#endif /* ABSTRACTMARKERTILER_H */
