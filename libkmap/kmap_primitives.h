/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Primitive datatypes for KMap
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
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

#ifndef KMAP_PRIMITIVES_H
#define KMAP_PRIMITIVES_H

// Qt includes

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QMimeData>
#include <QPersistentModelIndex>
#include <QPixmap>
#include <QPoint>
#include <QSharedData>
#include <QString>
#include <QStringList>
#include <QVariant>

// Kde includes

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kurl.h>

// local includes

#include "libkmap_export.h"

#ifdef KMAP_HAVE_VALGRIND
#include <valgrind/valgrind.h>
#endif /* KMAP_HAVE_VALGRIND */

#define KMAP_ASSERT(cond) ((!(cond)) ? KMap::KMap_assert(#cond,__FILE__,__LINE__) : qt_noop())

Q_DECLARE_METATYPE(QPersistentModelIndex)

namespace KMap
{

inline void KMap_assert(const char* const condition, const char* const filename, const int lineNumber)
{
    const QString debugString = QString("ASSERT: %1 - %2:%3").arg(condition).arg(filename).arg(lineNumber);
#ifdef KMAP_HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND>0)
    {
        // TODO: which encoding?
        const QByteArray dummyArray = debugString.toUtf8();
        VALGRIND_PRINTF_BACKTRACE("%s", dummyArray.constData());
    }
    else
    {
        kDebug(51006)<<debugString;
    }
#else
    kDebug(51006)<<debugString;
#endif /* KMAP_HAVE_VALGRIND */
}

enum MouseMode
{
    MouseModePan = 1,
    MouseModeSelection = 2,
    MouseModeZoom = 4,
    MouseModeFilter = 8,
    MouseModeSelectThumbnail = 16,
    MouseModeLast = 16
};

Q_DECLARE_FLAGS(MouseModes, MouseMode)
Q_DECLARE_OPERATORS_FOR_FLAGS(MouseModes);

enum DisplayedRectangles
{
    SelectionRectangle = 0,
    DisplayedRectangle,
    Both
};

class KMAP_EXPORT WMWGeoCoordinate
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

    typedef QList<WMWGeoCoordinate>                   List;
    typedef QPair<WMWGeoCoordinate, WMWGeoCoordinate> Pair;
    typedef QList<WMWGeoCoordinate::Pair>             PairList;

    static Pair makePair(const qreal lat1, const qreal lon1, const qreal lat2, const qreal lon2)
    {
        return Pair(WMWGeoCoordinate(lat1, lon1), WMWGeoCoordinate(lat2, lon2));
    }

    WMWGeoCoordinate()
        : m_lat(0.0),
          m_lon(0.0),
          m_alt(0.0),
          m_hasFlags(HasNothing)
    {
    }

    WMWGeoCoordinate(const double inLat, const double inLon)
        : m_lat(inLat),
          m_lon(inLon),
          m_alt(0.0),
          m_hasFlags(HasCoordinates)
    {
    }

    WMWGeoCoordinate(const double inLat, const double inLon, const double inAlt)
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

    bool sameLonLatAs(const WMWGeoCoordinate& other) const
    {
        return m_hasFlags.testFlag(HasCoordinates) &&
               other.m_hasFlags.testFlag(HasCoordinates) &&
               (m_lat==other.m_lat)&&(m_lon==other.m_lon);
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

private:

    double   m_lat;
    double   m_lon;
    double   m_alt;
    HasFlags m_hasFlags;
};

typedef QList<int> QIntList;
typedef QPair<int, int> QIntPair;

enum WMWSelectionState
{
    WMWSelectedNone = 0,
    WMWSelectedSome = 1,
    WMWSelectedAll  = 2
};

class KMAP_EXPORT WMWCluster
{
public:

    typedef QList<WMWCluster> List;

    WMWCluster()
        : tileIndicesList(),
        markerCount(0),
        markerSelectedCount(0),
        coordinates(),
        pixelPos(),
        selectedState(WMWSelectedNone),
        representativeMarkers(),
        pixmapSize(),
        pixmapOffset()
    {
    }

    QList<QIntList>     tileIndicesList;
    int                 markerCount;
    int                 markerSelectedCount;
    WMWGeoCoordinate    coordinates;
    QPoint              pixelPos;
    WMWSelectionState   selectedState;
    QMap<int, QVariant> representativeMarkers;

    enum PixmapType
    {
        PixmapMarker,
        PixmapCircle,
        PixmapImage
    } pixmapType;
    QSize pixmapSize;

    //! anchor point of the image, measured from bottom-left
    QPoint pixmapOffset;
};

class KMapWidget;

class KMAP_EXPORT WMWModelHelper : public QObject
{
    Q_OBJECT

public:

    enum Flag
    {
        FlagNull    = 0,
        FlagVisible = 1,
        FlagMovable = 2,
        FlagSnaps   = 4
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    WMWModelHelper(QObject* const parent = 0);
    virtual ~WMWModelHelper();

    void snapItemsTo(const QModelIndex& targetIndex, const QList<QPersistentModelIndex>& snappedIndices);

    // these are necessary for grouped and ungrouped models
    virtual QAbstractItemModel* model() const = 0;
    virtual QItemSelectionModel* selectionModel() const = 0;
    virtual bool itemCoordinates(const QModelIndex& index, WMWGeoCoordinate* const coordinates) const = 0;

    // these are necessary for ungrouped models
    virtual QPixmap itemIcon(const QModelIndex& index, QPoint* const offset) const;
    virtual Flags modelFlags() const;
    virtual Flags itemFlags(const QModelIndex& index) const;
    virtual void snapItemsTo(const QModelIndex& targetIndex, const QList<QModelIndex>& snappedIndices);

    // these are used by MarkerModel for grouped models
    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int sortKey);

    virtual void onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices);
    virtual void onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices, const WMWGeoCoordinate& targetCoordinates, const QPersistentModelIndex& targetSnapIndex);

Q_SIGNALS:

    void signalVisibilityChanged();
    void signalThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap);
    void signalModelChangedDrastically();
};


} /* namespace KMap */

Q_DECLARE_OPERATORS_FOR_FLAGS(KMap::WMWGeoCoordinate::HasFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(KMap::WMWModelHelper::Flags)

namespace KMap
{

// primitives for altitude lookup:
class KMAP_EXPORT WMWAltitudeLookup
{
public:

    WMWGeoCoordinate                 coordinates;
    QVariant                         data;

    typedef QList<WMWAltitudeLookup> List;
};

} /* namespace KMap */

inline QDebug operator<<(QDebug debugOut, const KMap::WMWGeoCoordinate& coordinate)
{
    debugOut << coordinate.geoUrl();
    return debugOut;
}

inline bool operator==(const KMap::WMWGeoCoordinate& a, const KMap::WMWGeoCoordinate& b)
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

Q_DECLARE_METATYPE(KMap::WMWGeoCoordinate)
Q_DECLARE_METATYPE(KMap::WMWGeoCoordinate::Pair)
Q_DECLARE_METATYPE(KMap::WMWGeoCoordinate::PairList)
Q_DECLARE_METATYPE(KMap::WMWAltitudeLookup)

#endif /* KMAP_PRIMITIVES_H */
