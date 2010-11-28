/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-07-14
 * @brief  Common internal data structures for libkmap
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

#include "kmap_common.h"

namespace KMap
{

/**
 * @brief Parse a 'lat,lon' string a returned by the JavaScript parts
 * @return true if the string could be parsed successfully
 */
bool KMapHelperParseLatLonString(const QString& latLonString, GeoCoordinates* const coordinates)
{
    // parse a 'lat,lon' string:
    const QStringList coordinateStrings = latLonString.trimmed().split(QLatin1Char( ',' ));
    bool valid = ( coordinateStrings.size() == 2 );
    if (valid)
    {
        double    ptLatitude  = 0.0;
        double    ptLongitude = 0.0;

        ptLatitude = coordinateStrings.at(0).toDouble(&valid);
        if (valid)
            ptLongitude = coordinateStrings.at(1).toDouble(&valid);

        if (valid)
        {
            if (coordinates)
            {
                *coordinates = GeoCoordinates(ptLatitude, ptLongitude);
            }

            return true;
        }
    }

    return false;
}

/**
 * @brief Parse a '(x,y)' string as returned by the JavaScript parts
 */
bool KMapHelperParseXYStringToPoint(const QString& xyString, QPoint* const point)
{
    // a point is returned as (x, y)

    const QString myXYString = xyString.trimmed();
    bool valid = myXYString.startsWith(QLatin1Char( '(' )) && myXYString.endsWith(QLatin1Char( ')' ));
    QStringList pointStrings;
    if (valid)
    {
        pointStrings = myXYString.mid(1, myXYString.length()-2).split(QLatin1Char( ',' ));
        valid = ( pointStrings.size() == 2 );
    }
    if (valid)
    {
        int ptX = 0;
        int ptY = 0;

        ptX = pointStrings.at(0).toInt(&valid);
        if (valid)
            ptY = pointStrings.at(1).toInt(&valid);

        if (valid)
        {
            if (point)
            {
                *point = QPoint(ptX, ptY);
            }

            return true;
        }
    }

    return false;
}

/**
 * @brief Parses a '((lat1, lon1), (lat2, lon2))' bounds string as returned by the JavaScript parts
 */
bool KMapHelperParseBoundsString(const QString& boundsString, QPair<GeoCoordinates, GeoCoordinates>* const boundsCoordinates)
{
    // bounds are returned as ((lat1, lon1), (lat2, lon2))

    const QString myBoundsString = boundsString.trimmed();

    // check for minimum length
    bool valid =  myBoundsString.size() >= 13;
    valid      &= myBoundsString.startsWith(QLatin1Char( '(' )) && myBoundsString.endsWith(QLatin1Char( ')' ));

    if (valid)
    {
        // remove outer parentheses:
        const QString string1 = myBoundsString.mid(1, myBoundsString.length()-2).trimmed();

        // split the string at the middle comma:
        const int dumpComma  = string1.indexOf(QLatin1String("," ), 0);
        const int splitComma = string1.indexOf(QLatin1String("," ), dumpComma+1);
        valid                = (dumpComma>=0) && (splitComma>=0);

        if (valid)
        {
            const QString coord1String =  string1.mid(0, splitComma).trimmed();
            const QString coord2String =  string1.mid(splitComma+1).trimmed();
            valid                      &= coord1String.startsWith(QLatin1Char( '(' )) && coord1String.endsWith(QLatin1Char( ')' ));
            valid                      &= coord2String.startsWith(QLatin1Char( '(' )) && coord2String.endsWith(QLatin1Char( ')' ));

            GeoCoordinates coord1, coord2;
            if (valid)
            {
                valid = KMapHelperParseLatLonString(coord1String.mid(1, coord1String.length()-2), &coord1);
            }
            if (valid)
            {
                valid = KMapHelperParseLatLonString(coord2String.mid(1, coord2String.length()-2), &coord2);
            }

            if (valid && boundsCoordinates)
            {
                *boundsCoordinates = QPair<GeoCoordinates, GeoCoordinates>(coord1, coord2);
            }

            return valid;
        }
    }

    return false;
}

/**
 * @brief Split bounds crossing the dateline into parts which do not cross the dateline
 */
GeoCoordinates::PairList KMapHelperNormalizeBounds(const GeoCoordinates::Pair& boundsPair)
{
    GeoCoordinates::PairList boundsList;

    const qreal bWest  = boundsPair.first.lon();
    const qreal bEast  = boundsPair.second.lon();
    const qreal bNorth = boundsPair.second.lat();
    const qreal bSouth = boundsPair.first.lat();
//     kDebug() << bWest << bEast << bNorth << bSouth;

    if (bEast<bWest)
    {
        boundsList << GeoCoordinates::makePair(bSouth, -180, bNorth, bEast);
        boundsList << GeoCoordinates::makePair(bSouth, bWest, bNorth, 180);
    }
    else
    {
        boundsList << GeoCoordinates::makePair(bSouth, bWest, bNorth, bEast);
    }
//     kDebug()<<boundsList;
    return boundsList;
}

} /* namespace KMap */
