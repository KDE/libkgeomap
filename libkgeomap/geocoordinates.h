/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-08-16
 * @brief  GeoCoordinates class
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

#ifndef KMAP_GEOCOORDINATES_H
#define KMAP_GEOCOORDINATES_H

// Qt includes

#include <QtCore/QString>
#include <QtCore/QStringList>

// Kde includes

#include <kdebug.h>
#include <kurl.h>

// local includes

#include "libkmap_export.h"

namespace KMap
{

class KMAP_EXPORT GeoCoordinates
{
public:

    enum HasFlag
    {
        HasNothing     = 0,
        HasLatitude    = 1,
        HasLongitude   = 2,
        HasCoordinates = 3,
        HasAltitude    = 4
    };

    Q_DECLARE_FLAGS(HasFlags, HasFlag)

    typedef QList<GeoCoordinates>                   List;
    typedef QPair<GeoCoordinates, GeoCoordinates>   Pair;
    typedef QList<GeoCoordinates::Pair>             PairList;

    static Pair makePair(const qreal lat1, const qreal lon1, const qreal lat2, const qreal lon2)
    {
        return Pair(GeoCoordinates(lat1, lon1), GeoCoordinates(lat2, lon2));
    }

    GeoCoordinates()
        : m_lat(0.0),
          m_lon(0.0),
          m_alt(0.0),
          m_hasFlags(HasNothing)
    {
    }

    GeoCoordinates(const double inLat, const double inLon)
        : m_lat(inLat),
          m_lon(inLon),
          m_alt(0.0),
          m_hasFlags(HasCoordinates)
    {
    }

    GeoCoordinates(const double inLat, const double inLon, const double inAlt)
        : m_lat(inLat),
          m_lon(inLon),
          m_alt(inAlt),
          m_hasFlags(HasCoordinates|HasAltitude)
    {
    }

    double lat() const { return m_lat; }
    double lon() const { return m_lon; }
    double alt() const { return m_alt; }

    bool hasCoordinates() const { return m_hasFlags.testFlag(HasCoordinates); }
    bool hasLatitude() const    { return m_hasFlags.testFlag(HasLatitude);    }
    bool hasLongitude() const   { return m_hasFlags.testFlag(HasLongitude);   }

    void setLatLon(const double inLat, const double inLon)
    {
        m_lat      = inLat;
        m_lon      = inLon;
        m_hasFlags |= HasCoordinates;
    }

    bool hasAltitude() const { return m_hasFlags.testFlag(HasAltitude); }

    HasFlags hasFlags() const { return m_hasFlags; }

    void setAlt(const double inAlt)
    {
        m_hasFlags |= HasAltitude;
        m_alt      = inAlt;
    }

    void clearAlt()
    {
        m_hasFlags &= ~HasAltitude;
    }

    void clear()
    {
        m_hasFlags = HasNothing;
    }

    QString altString() const { return m_hasFlags.testFlag(HasAltitude)  ? QString::number(m_alt, 'g', 12) : QString(); }
    QString latString() const { return m_hasFlags.testFlag(HasLatitude)  ? QString::number(m_lat, 'g', 12) : QString(); }
    QString lonString() const { return m_hasFlags.testFlag(HasLongitude) ? QString::number(m_lon, 'g', 12) : QString(); }

    QString geoUrl() const
    {
        if (!hasCoordinates())
        {
            return QString();
        }

        if (m_hasFlags.testFlag(HasAltitude))
        {
            return QString::fromLatin1("geo:%1,%2,%3").arg(latString()).arg(lonString()).arg(altString());
        }
        else
        {
            return QString::fromLatin1("geo:%1,%2").arg(latString()).arg(lonString());
        }
    }

    bool sameLonLatAs(const GeoCoordinates& other) const
    {
        return m_hasFlags.testFlag(HasCoordinates) &&
               other.m_hasFlags.testFlag(HasCoordinates) &&
               (m_lat==other.m_lat)&&(m_lon==other.m_lon);
    }

    static GeoCoordinates fromGeoUrl(const QString& url, bool* const parsedOkay = 0)
    {
        // parse geo:-uri according to (only partially implemented):
        // http://tools.ietf.org/html/draft-ietf-geopriv-geo-uri-04
        // TODO: verify that we follow the spec fully!
        if (!url.startsWith(QLatin1String( "geo:" )))
        {
            // TODO: error
            if (parsedOkay)
                *parsedOkay = false;
            return GeoCoordinates();
        }

        const QStringList parts = url.mid(4).split(QLatin1Char( ',' ));

        GeoCoordinates position;
        if ((parts.size()==3)||(parts.size()==2))
        {
            bool okay              = true;
            double ptLongitude     = 0.0;
            double ptLatitude      = 0.0;
            double ptAltitude      = 0.0;
            const bool hasAltitude = parts.size()==3;

            ptLatitude = parts[0].toDouble(&okay);
            if (okay)
                ptLongitude = parts[1].toDouble(&okay);

            if (okay&&(hasAltitude))
                ptAltitude = parts[2].toDouble(&okay);

            if (!okay)
            {
                *parsedOkay = false;
                return GeoCoordinates();
            }

            position = GeoCoordinates(ptLatitude, ptLongitude);
            if (hasAltitude)
            {
                position.setAlt(ptAltitude);
            }
        }
        else
        {
            if (parsedOkay)
                *parsedOkay = false;
            return GeoCoordinates();
        }

        if (parsedOkay)
                *parsedOkay = true;
        return position;
    }

private:

    double   m_lat;
    double   m_lon;
    double   m_alt;
    HasFlags m_hasFlags;
};

} /* namespace KMap */

Q_DECLARE_OPERATORS_FOR_FLAGS(KMap::GeoCoordinates::HasFlags)

inline QDebug operator<<(QDebug debugOut, const KMap::GeoCoordinates& coordinate)
{
    debugOut << coordinate.geoUrl();
    return debugOut;
}

inline bool operator==(const KMap::GeoCoordinates& a, const KMap::GeoCoordinates& b)
{
    return
        ( a.hasCoordinates() == b.hasCoordinates() ) &&
        ( a.hasCoordinates() ?
            ( ( a.lat() == b.lat() ) &&
              ( a.lon() == b.lon() )
            ) : true
        ) &&
        ( a.hasAltitude() == b.hasAltitude() ) &&
        ( a.hasAltitude() ? ( a.alt() == b.alt() ) : true );
}

Q_DECLARE_TYPEINFO(KMap::GeoCoordinates, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KMap::GeoCoordinates)
Q_DECLARE_METATYPE(KMap::GeoCoordinates::Pair)
Q_DECLARE_METATYPE(KMap::GeoCoordinates::PairList)


#endif /* KMAP_GEOCOORDINATES_H */
