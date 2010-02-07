/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Primitive datatypes for WorldMapWidget2
 *
 * Copyright (C) 2009 by Michael G. Hansen <mike at mghansen dot de>
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

// local includes

#include "worldmapwidget2_primitives.h"

namespace WMW2
{

/**
 * @brief Parse a 'lat,lon' string a returned by the JavaScript parts
 * @return true if the string could be parsed successfully
 */
bool WMWHelperParseLatLonString(const QString& latLonString, WMWGeoCoordinate* const coordinates)
{
    // parse a 'lat,lon' string:
    const QStringList coordinateStrings = latLonString.split(',');
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
                *coordinates = WMWGeoCoordinate(ptLatitude, ptLongitude);
            }

            return true;
        }
    }

    return false;
}

/**
 * @brief Parse a '(x,y)' string as returned by the JavaScript parts
 */
bool WMWHelperParseXYStringToPoint(const QString& xyString, QPoint* const point)
{
    // a point is returned as (x, y)
    bool valid = xyString.startsWith('(') && xyString.endsWith(')');
    QStringList pointStrings;
    if (valid)
    {
        pointStrings = xyString.mid(1, xyString.length()-2).split(',');
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
bool WMWHelperParseBoundsString(const QString& boundsString, QPair<WMWGeoCoordinate, WMWGeoCoordinate>* const boundsCoordinates)
{
    // bounds are returned as ((lat1, lon1), (lat2, lon2))

    // check for minimum length
    bool valid = boundsString.size()>=13;
    if (valid)
    {
        const QString string1 = boundsString.mid(1, boundsString.length()-2);
        const int dumpComma = string1.indexOf(",", 0);
        const int splitComma = string1.indexOf(",", dumpComma+1);
        valid = (dumpComma>=0) && (splitComma>=0);

        if (valid)
        {
            QString coord1String = string1.mid(0, splitComma);
            QString coord2String = string1.mid(splitComma+2);

            WMWGeoCoordinate coord1, coord2;
            valid = WMWHelperParseLatLonString(coord1String.mid(1, coord1String.length()-2), &coord1);
            if (valid)
            {
                valid = WMWHelperParseLatLonString(coord2String.mid(1, coord2String.length()-2), &coord2);
            }

            if (valid && boundsCoordinates)
            {
                *boundsCoordinates = QPair<WMWGeoCoordinate, WMWGeoCoordinate>(coord1, coord2);
            }

            return valid;
        }
    }

    return false;
}



} /* WMW2 */
