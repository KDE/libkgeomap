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

// Kde includes

#include <kdebug.h>

namespace WMW2 {

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

    static WMWGeoCoordinate fromGeoUrl(const QString& url, bool* const parsedOkay)
    {
        Q_ASSERT(parsedOkay != 0);

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

} /* WMW2 */

// Q_DECLARE_METATYPE(WMW2::WMWGeoCoordinate)

#endif /* WORLDMAPWIDGET2_PRIMITIVES_H */
