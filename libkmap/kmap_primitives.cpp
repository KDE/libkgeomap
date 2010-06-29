/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Primitive datatypes for KMap
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

#include "kmap_primitives.moc"

namespace KMapIface
{

/**
 * @brief Parse a 'lat,lon' string a returned by the JavaScript parts
 * @return true if the string could be parsed successfully
 */
bool WMWHelperParseLatLonString(const QString& latLonString, WMWGeoCoordinate* const coordinates)
{
    // parse a 'lat,lon' string:
    const QStringList coordinateStrings = latLonString.trimmed().split(',');
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

    const QString myXYString = xyString.trimmed();
    bool valid = myXYString.startsWith('(') && myXYString.endsWith(')');
    QStringList pointStrings;
    if (valid)
    {
        pointStrings = myXYString.mid(1, myXYString.length()-2).split(',');
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

    const QString myBoundsString = boundsString.trimmed();

    // check for minimum length
    bool valid = myBoundsString.size()>=13;
    valid&= myBoundsString.startsWith('(') && myBoundsString.endsWith(')');

    if (valid)
    {
        // remove outer parentheses:
        const QString string1 = myBoundsString.mid(1, myBoundsString.length()-2).trimmed();

        // split the string at the middle comma:
        const int dumpComma = string1.indexOf(",", 0);
        const int splitComma = string1.indexOf(",", dumpComma+1);
        valid = (dumpComma>=0) && (splitComma>=0);

        if (valid)
        {
            const QString coord1String = string1.mid(0, splitComma).trimmed();
            const QString coord2String = string1.mid(splitComma+1).trimmed();
            valid&= coord1String.startsWith('(') && coord1String.endsWith(')');
            valid&= coord2String.startsWith('(') && coord2String.endsWith(')');

            WMWGeoCoordinate coord1, coord2;
            if (valid)
            {
                valid = WMWHelperParseLatLonString(coord1String.mid(1, coord1String.length()-2), &coord1);
            }
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

/**
 * @brief Split bounds crossing the dateline into parts which do not cross the dateline
 */
WMWGeoCoordinate::PairList WMWHelperNormalizeBounds(const WMWGeoCoordinate::Pair& boundsPair)
{
    WMWGeoCoordinate::PairList boundsList;

    const qreal bWest = boundsPair.first.lon();
    const qreal bEast = boundsPair.second.lon();
    const qreal bNorth = boundsPair.second.lat();
    const qreal bSouth = boundsPair.first.lat();
//     kDebug()<<bWest<<bEast<<bNorth<<bSouth;

    if (bEast<bWest)
    {
        boundsList << WMWGeoCoordinate::makePair(bSouth, -180, bNorth, bEast);
        boundsList << WMWGeoCoordinate::makePair(bSouth, bWest, bNorth, 180);
    }
    else
    {
        boundsList << WMWGeoCoordinate::makePair(bSouth, bWest, bNorth, bEast);
    }
//     kDebug()<<boundsList;
    return boundsList;
}

WMWRepresentativeChooser::WMWRepresentativeChooser(QObject* const parent)
: QObject(parent)
{
}

WMWRepresentativeChooser::~WMWRepresentativeChooser()
{
}

WMWModelHelper::WMWModelHelper(QObject* const parent)
: QObject(parent)
{
}

WMWModelHelper::~WMWModelHelper()
{
}

void WMWModelHelper::snapItemsTo(const QModelIndex& targetIndex, const QList<QPersistentModelIndex>& snappedIndices)
{
    QList<QModelIndex> result;
    for (int i=0; i<snappedIndices.count(); ++i)
    {
        result << snappedIndices.at(i);
    }
    snapItemsTo(targetIndex, result);
}

} /* KMapIface */
