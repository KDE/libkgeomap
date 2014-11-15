/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Tile index used in the tiling classes
 *
 * @author Copyright (C) 2009-2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2014 by Gilles Caulier
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

#ifndef KGEOMAP_TILEINDEX_H
#define KGEOMAP_TILEINDEX_H

// Qt includes

#include <QtCore/QBitArray>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QDebug>

// local includes

#include "kgeomap_primitives.h"
#include "libkgeomap_export.h"

namespace KGeoMap
{

class KGEOMAP_EXPORT TileIndex
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

public:

    inline TileIndex()
        : m_indicesCount(0)
    {
         for (int i = 0; i < MaxIndexCount; ++i)
         {
             m_indices[i] = 0;
         }
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
        KGEOMAP_ASSERT(m_indicesCount+1<=MaxIndexCount);
        m_indices[m_indicesCount] = newIndex;
        m_indicesCount++;
    }

    inline int linearIndex(const int getLevel) const
    {
        KGEOMAP_ASSERT(getLevel<=level());
        return m_indices[getLevel];
    }

    inline int at(const int getLevel) const
    {
        KGEOMAP_ASSERT(getLevel<=level());
        return m_indices[getLevel];
    }

    inline int lastIndex() const
    {
        KGEOMAP_ASSERT(m_indicesCount>0);
        return m_indices[m_indicesCount-1];
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
        KGEOMAP_ASSERT(getLevel <= level());
        *latIndex = indexLat(getLevel);
        *lonIndex = indexLon(getLevel);
        KGEOMAP_ASSERT(*latIndex < Tiling);
        KGEOMAP_ASSERT(*lonIndex < Tiling);
    }

    inline void appendLatLonIndex(const int latIndex, const int lonIndex)
    {
        appendLinearIndex(latIndex*Tiling + lonIndex);
    }

    inline QIntList toIntList() const
    {
        QIntList result;

        for (int i = 0; i < m_indicesCount; ++i)
        {
            result << m_indices[i];
        }

        return result;
    }

    static TileIndex fromCoordinates(const KGeoMap::GeoCoordinates& coordinate, const int getLevel);

    GeoCoordinates toCoordinates()                              const;
    GeoCoordinates toCoordinates(const CornerPosition ofCorner) const;

    inline static TileIndex fromIntList(const QIntList& intList)
    {
        TileIndex result;

        for (int i = 0; i < intList.count(); ++i)
        {
            result.appendLinearIndex(intList.at(i));
        }

        return result;
    }

    inline static bool indicesEqual(const TileIndex& a, const TileIndex& b, const int upToLevel)
    {
        KGEOMAP_ASSERT(a.level() >= upToLevel);
        KGEOMAP_ASSERT(b.level() >= upToLevel);

        for (int i = 0; i <= upToLevel; ++i)
        {
            if (a.linearIndex(i)!=b.linearIndex(i))
            {
                return false;
            }
        }

        return true;
    }

    inline TileIndex mid(const int first, const int len) const
    {
        KGEOMAP_ASSERT(first+(len-1) <= m_indicesCount);
        TileIndex result;

        for (int i = first; i < first+len; ++i)
        {
            result.appendLinearIndex(m_indices[i]);
        }

        return result;
    }

    inline void oneUp()
    {
        KGEOMAP_ASSERT(m_indicesCount>0);
        m_indicesCount--;
    }

    inline static QList<QIntList> listToIntListList(const QList<TileIndex>& tileIndexList)
    {
        QList<QIntList> result;

        for (int i = 0; i < tileIndexList.count(); ++i)
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

} /* namespace KGeoMap */

KGEOMAP_EXPORT QDebug operator<<(QDebug debugOut, const KGeoMap::TileIndex& tileIndex);

Q_DECLARE_TYPEINFO(KGeoMap::TileIndex, Q_MOVABLE_TYPE);

#endif /* KGEOMAP_TILEINDEX_H */
