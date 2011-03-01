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

#include "tileindex.h"

namespace KMap
{

TileIndex TileIndex::fromCoordinates(const KMap::GeoCoordinates& coordinate, const int getLevel)
{
    KMAP_ASSERT(getLevel<=MaxLevel);

    if (!coordinate.hasCoordinates())
        return TileIndex();

    qreal tileLatBL     = -90.0;
    qreal tileLonBL     = -180.0;
    qreal tileLatHeight = 180.0;
    qreal tileLonWidth  = 360.0;

    TileIndex resultIndex;
    for (int l = 0; l <= getLevel; ++l)
    {
        // how many tiles at this level?
        const qreal latDivisor = TileIndex::Tiling;
        const qreal lonDivisor = TileIndex::Tiling;

        const qreal dLat       = tileLatHeight / latDivisor;
        const qreal dLon       = tileLonWidth / lonDivisor;

        int latIndex           = int( (coordinate.lat() - tileLatBL ) / dLat );
        int lonIndex           = int( (coordinate.lon() - tileLonBL ) / dLon );

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
//             kDebug() << QString::fromLatin1("Rounding errors at level %1!").arg(l);
        }

        resultIndex.appendLatLonIndex(latIndex, lonIndex);

        // update the start position for the next tile:
        // TODO: rounding errors
        tileLatBL     += latIndex*dLat;
        tileLonBL     += lonIndex*dLon;
        tileLatHeight /= latDivisor;
        tileLonWidth  /= lonDivisor;
    }

    return resultIndex;
}

GeoCoordinates TileIndex::toCoordinates() const
{
    // TODO: safeguards against rounding errors!
    qreal tileLatBL     = -90.0;
    qreal tileLonBL     = -180.0;
    qreal tileLatHeight = 180.0;
    qreal tileLonWidth  = 360.0;

    for (int l = 0; l < m_indicesCount; ++l)
    {
        // how many tiles are at this level?
        const qreal latDivisor = TileIndex::Tiling;
        const qreal lonDivisor = TileIndex::Tiling;

        const qreal dLat       = tileLatHeight / latDivisor;
        const qreal dLon       = tileLonWidth / lonDivisor;

        const int latIndex     = indexLat(l);
        const int lonIndex     = indexLon(l);

        // update the start position for the next tile:
        tileLatBL     += latIndex*dLat;
        tileLonBL     += lonIndex*dLon;
        tileLatHeight /= latDivisor;
        tileLonWidth  /= lonDivisor;
    }

    return GeoCoordinates(tileLatBL, tileLonBL);
}


GeoCoordinates TileIndex::toCoordinates(const CornerPosition ofCorner) const
{
    // TODO: safeguards against rounding errors!
    qreal tileLatBL     = -90.0;
    qreal tileLonBL     = -180.0;
    qreal tileLatHeight = 180.0;
    qreal tileLonWidth  = 360.0;

    for (int l = 0; l < m_indicesCount; ++l)
    {
        // how many tiles are at this level?
        const qreal latDivisor = TileIndex::Tiling;
        const qreal lonDivisor = TileIndex::Tiling;

        const qreal dLat       = tileLatHeight / latDivisor;
        const qreal dLon       = tileLonWidth / lonDivisor;

        const int latIndex     = indexLat(l);
        const int lonIndex     = indexLon(l);

        // update the start position for the next tile:
        if (l+1 >= m_indicesCount)
        {
            if (ofCorner == CornerNW)
            {
                tileLatBL += latIndex*dLat;
                tileLonBL += lonIndex*dLon;
            }
            else if (ofCorner == CornerSW)
            {
                tileLatBL += (latIndex+1)*dLat;
                tileLonBL += lonIndex*dLon;
            }
            else if (ofCorner == CornerNE)
            {
                tileLatBL += latIndex*dLat;
                tileLonBL += (lonIndex+1)*dLon;
            }
            else if (ofCorner == CornerSE)
            {
                tileLatBL += (latIndex+1)*dLat;
                tileLonBL += (lonIndex+1)*dLon;
            }
        }
        else
        {
            // update the start position for the next tile:
            tileLatBL     += latIndex*dLat;
            tileLonBL     += lonIndex*dLon;
        }

        tileLatHeight /= latDivisor;
        tileLonWidth  /= lonDivisor;
    }

    return GeoCoordinates(tileLatBL, tileLonBL);
}

} /* KMap */
