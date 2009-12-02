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

namespace WMW2 {

class WMWGeoCoordinate
{
public:
    WMWGeoCoordinate()
    : lat(0.0),
      lon(0.0),
      alt(0.0),
      hasAlt(0.0)
    {
    }

    WMWGeoCoordinate(const double inLat, const double inLon)
    : lat(inLat),
      lon(inLon),
      alt(0.0),
      hasAlt(0.0)
    {
    }
    
    double lat;
    double lon;
    double alt;
    bool hasAlt;

    QString altString() const { return QString::number(alt, 'g', 12); }
    QString latString() const { return QString::number(lat, 'g', 12); }
    QString lonString() const { return QString::number(lon, 'g', 12); }
};

} /* WMW2 */

// Q_DECLARE_METATYPE(WMW2::WMWGeoCoordinate)

#endif /* WORLDMAPWIDGET2_PRIMITIVES_H */
