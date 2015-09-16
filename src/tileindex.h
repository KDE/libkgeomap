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
 * @author Copyright (C) 2010-2015 by Gilles Caulier
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

#include "geocoordinates.h"
#include "types.h"
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

    TileIndex();
    virtual ~TileIndex();

    int indexCount()                    const;
    int level()                         const;
    int linearIndex(const int getLevel) const;
    int at(const int getLevel)          const;
    int lastIndex()                     const;
    int indexLat(const int getLevel)    const;
    int indexLon(const int getLevel)    const;

    void clear();
    void appendLinearIndex(const int newIndex);
    
    QPoint latLonIndex(const int getLevel) const;

    void latLonIndex(const int getLevel, int* const latIndex, int* const lonIndex) const;
    void appendLatLonIndex(const int latIndex, const int lonIndex);

    QIntList toIntList() const;

    GeoCoordinates toCoordinates()                              const;
    GeoCoordinates toCoordinates(const CornerPosition ofCorner) const;

    TileIndex mid(const int first, const int len) const;
    void oneUp();

    static TileIndex fromCoordinates(const KGeoMap::GeoCoordinates& coordinate, const int getLevel);
    static TileIndex fromIntList(const QIntList& intList);
    static bool indicesEqual(const TileIndex& a, const TileIndex& b, const int upToLevel);
    static QList<QIntList> listToIntListList(const QList<TileIndex>& tileIndexList);

public:

    typedef QList<TileIndex> List;

private:

    int m_indicesCount;
    int m_indices[MaxIndexCount];
};

} // namespace KGeoMap

KGEOMAP_EXPORT QDebug operator<<(QDebug debugOut, const KGeoMap::TileIndex& tileIndex);

Q_DECLARE_TYPEINFO(KGeoMap::TileIndex, Q_MOVABLE_TYPE);

#endif // KGEOMAP_TILEINDEX_H
