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

#ifndef WORLDMAPWIDGET2_PRIMITIVES_H
#define WORLDMAPWIDGET2_PRIMITIVES_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QVariant>

// Kde includes

#include <kdebug.h>

#define WMW2_ASSERT(cond) ((!(cond)) ? WMW2::WMW2_assert(#cond,__FILE__,__LINE__) : qt_noop())

namespace WMW2 {

inline void WMW2_assert(const char* const condition, const char* const filename, const int lineNumber)
{
    kDebug()<<QString("ASSERT: %1 - %2:%3").arg(condition).arg(filename).arg(lineNumber);
}

class WMWGeoCoordinate
{
public:
    WMWGeoCoordinate()
    : lat(0.0),
      lon(0.0),
      alt(0.0),
      hasAlt(false)
    {
    }

    WMWGeoCoordinate(const double inLat, const double inLon)
    : lat(inLat),
      lon(inLon),
      alt(0.0),
      hasAlt(false)
    {
    }

    WMWGeoCoordinate(const double inLat, const double inLon, const double inAlt)
    : lat(inLat),
      lon(inLon),
      alt(inAlt),
      hasAlt(true)
    {
    }
    
    double lat;
    double lon;
    double alt;
    bool hasAlt;

    void setAlt(const double inAlt)
    {
        hasAlt = true;
        alt = inAlt;
    }
    
    QString altString() const { return QString::number(alt, 'g', 12); }
    QString latString() const { return QString::number(lat, 'g', 12); }
    QString lonString() const { return QString::number(lon, 'g', 12); }

    QString geoUrl() const
    {
        if (hasAlt)
        {
            return QString::fromLatin1("geo:%1,%2,%3").arg(latString()).arg(lonString()).arg(altString());
        }
        else
        {
            return QString::fromLatin1("geo:%1,%2").arg(latString()).arg(lonString());
        }
    }

    static WMWGeoCoordinate fromGeoUrl(const QString& url, bool* const parsedOkay = 0)
    {
        // parse geo:-uri according to (only partially implemented):
        // http://tools.ietf.org/html/draft-ietf-geopriv-geo-uri-04
        // TODO: verify that we follow the spec fully!
        if (!url.startsWith("geo:"))
        {
            // TODO: error
            if (parsedOkay)
                *parsedOkay = false;
            return WMWGeoCoordinate();
        }

        const QStringList parts = url.mid(4).split(',');

        WMWGeoCoordinate position;
        if ((parts.size()==3)||(parts.size()==2))
        {
            bool okay = true;
            double ptLongitude = 0.0;
            double ptLatitude  = 0.0;
            double ptAltitude  = 0.0;
            const bool hasAltitude = parts.size()==3;

            ptLatitude = parts[0].toDouble(&okay);
            if (okay)
                ptLongitude = parts[1].toDouble(&okay);

            if (okay&&(hasAltitude))
                ptAltitude = parts[2].toDouble(&okay);

            if (!okay)
            {
                *parsedOkay = false;
                return WMWGeoCoordinate();
            }

            position = WMWGeoCoordinate(ptLatitude, ptLongitude);
            if (hasAltitude)
            {
                position.setAlt(ptAltitude);
            }
        }
        else
        {
            if (parsedOkay)
                *parsedOkay = false;
            return WMWGeoCoordinate();
        }

        if (parsedOkay)
                *parsedOkay = true;
        return position;
    }
};

class WMWMarker
{
public:
    WMWMarker()
    : coordinates(),
      data(),
      attributes()
    {
    }

    WMWMarker(const WMWGeoCoordinate& geoCoordinate)
    : coordinates(geoCoordinate),
      data(),
      attributes()
    {
    }

    WMWGeoCoordinate coordinates;
    QVariant data;

    enum MarkerAttribute
    {
        MarkerDraggable = 1
    };
    Q_DECLARE_FLAGS(MarkerAttributes, MarkerAttribute)
    MarkerAttributes attributes;

    bool isDraggable() const { return attributes.testFlag(MarkerDraggable); }
    void setDraggable(const bool state)
    {
        if (state)
        {
            attributes|=MarkerDraggable;
        }
        else
        {
            attributes&=~MarkerDraggable;
        }
    }

    typedef QList<WMWMarker> List;

};

typedef QList<int> QIntList;
typedef QPair<int, int> QIntPair;

class WMWCluster
{
public:

    typedef QList<WMWCluster> List;

    WMWCluster()
    : tileIndicesList(),
      markerCount(0),
      coordinates()
    {
    }

    QList<QIntList> tileIndicesList;
    int markerCount;
    WMWGeoCoordinate coordinates;
};

class MarkerModel;

class WMWSharedData
{
public:
    WMWSharedData()
    : markerList(),
      visibleMarkers(),
      markerModel(0),
      clusterList()
    {
    }

    WMWMarker::List markerList;
    QIntList visibleMarkers;
    MarkerModel* markerModel;
    WMWCluster::List clusterList;
};

} /* WMW2 */

Q_DECLARE_OPERATORS_FOR_FLAGS(WMW2::WMWMarker::MarkerAttributes)

// Q_DECLARE_METATYPE(WMW2::WMWGeoCoordinate)

#endif /* WORLDMAPWIDGET2_PRIMITIVES_H */
