/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Primitive datatypes for WorldMapWidget2
 *
 * Copyright (C) 2009,2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include <QAbstractItemModel>
#include <QMimeData>
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

#include "worldmapwidget2_export.h"

#ifdef WMW2_HAVE_VALGRIND
#include <valgrind/valgrind.h>
#endif /* WMW2_HAVE_VALGRIND */

#define WMW2_ASSERT(cond) ((!(cond)) ? WMW2::WMW2_assert(#cond,__FILE__,__LINE__) : qt_noop())

namespace WMW2 {

inline void WMW2_assert(const char* const condition, const char* const filename, const int lineNumber)
{
    const QString debugString = QString("ASSERT: %1 - %2:%3").arg(condition).arg(filename).arg(lineNumber);
#ifdef WMW2_HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND>0)
    {
        // TODO: which encoding?
        const QByteArray dummyArray = debugString.toUtf8();
        VALGRIND_PRINTF_BACKTRACE("%s", dummyArray.constData());
    }
    else
    {
        kDebug(0)<<debugString;
    }
#else
    kDebug(0)<<debugString;
#endif /* WMW2_HAVE_VALGRIND */
    
}

class WORLDMAPWIDGET2_EXPORT  WMWGeoCoordinate
{
public:

    typedef QList<WMWGeoCoordinate> List;
    typedef QPair<WMWGeoCoordinate, WMWGeoCoordinate> Pair;
    typedef QList<WMWGeoCoordinate::Pair> PairList;
    static Pair makePair(const qreal lat1, const qreal lon1, const qreal lat2, const qreal lon2)
    {
        return Pair(WMWGeoCoordinate(lat1, lon1), WMWGeoCoordinate(lat2, lon2));
    }

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

    void clearAlt()
    {
        hasAlt = false;
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

    bool sameLonLatAs(const WMWGeoCoordinate& other) const
    {
        return (lat==other.lat)&&(lon==other.lon);
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

class WORLDMAPWIDGET2_EXPORT WMWMarker
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

enum WMWSelectionState {
    WMWSelectedNone = 0,
    WMWSelectedSome = 1,
    WMWSelectedAll = 2
};

class WORLDMAPWIDGET2_EXPORT WMWCluster
{
public:

    typedef QList<WMWCluster> List;

    WMWCluster()
    : tileIndicesList(),
      markerCount(0),
      markerSelectedCount(0),
      coordinates(),
      pixelPos(),
      selectedState(WMWSelectedNone)
    {
    }

    QList<QIntList> tileIndicesList;
    int markerCount;
    int markerSelectedCount;
    WMWGeoCoordinate coordinates;
    QPoint pixelPos;
    WMWSelectionState selectedState;
};

class MarkerModel;
class WorldMapWidget2;

class WMWSharedData : public QSharedData
{
public:
    WMWSharedData()
    : QSharedData(),
      worldMapWidget(0),
      visibleMarkers(),
      markerModel(0),
      clusterList(),
      specialMarkersModel(0),
      specialMarkersCoordinatesRole(0),
      displayMarkersModel(0),
      displayMarkersCoordinatesRole(0),
      inEditMode(false),
      haveMovingCluster(false),
      markerPixmap(),
      markerPixmaps()
    {
        QStringList markerColors;
        markerColors << "00ff00" << "00ffff" << "ff0000" << "ff7f00" << "ffff00";
        QStringList stateNames;
        stateNames << "" << "-selected" << "-someselected";
        for (QStringList::const_iterator it = markerColors.constBegin(); it!=markerColors.constEnd(); ++it)
        {
            for (QStringList::const_iterator sit = stateNames.constBegin(); sit!=stateNames.constEnd(); ++sit)
            {
                const QString pixmapName = *it + *sit;
                const KUrl markerUrl = KStandardDirs::locate("data", QString("libworldmapwidget2/marker-%1.png").arg(pixmapName));
                markerPixmaps[pixmapName] = QPixmap(markerUrl.toLocalFile());
            }
        }
        markerPixmap = markerPixmaps["00ff00"];
    }

    WorldMapWidget2* worldMapWidget;
    QIntList visibleMarkers;
    MarkerModel* markerModel;
    WMWCluster::List clusterList;
    QAbstractItemModel* specialMarkersModel;
    int specialMarkersCoordinatesRole;
    QAbstractItemModel* displayMarkersModel;
    int displayMarkersCoordinatesRole;
    bool inEditMode;
    bool haveMovingCluster;
    QPixmap markerPixmap;
    QMap<QString, QPixmap> markerPixmaps;
};

} /* WMW2 */

Q_DECLARE_OPERATORS_FOR_FLAGS(WMW2::WMWMarker::MarkerAttributes)

namespace WMW2
{

// helper functions:

WORLDMAPWIDGET2_EXPORT bool WMWHelperParseLatLonString(const QString& latLonString, WMWGeoCoordinate* const coordinates);
WORLDMAPWIDGET2_EXPORT bool WMWHelperParseXYStringToPoint(const QString& xyString, QPoint* const point);
WORLDMAPWIDGET2_EXPORT bool WMWHelperParseBoundsString(const QString& boundsString, QPair<WMWGeoCoordinate, WMWGeoCoordinate>* const boundsCoordinates);
WORLDMAPWIDGET2_EXPORT WMWGeoCoordinate::PairList WMWHelperNormalizeBounds(const WMWGeoCoordinate::Pair& boundsPair);

// primitives for altitude lookup:
class WORLDMAPWIDGET2_EXPORT WMWAltitudeLookup
{
public:
    WMWGeoCoordinate coordinates;
    QVariant data;

    typedef QList<WMWAltitudeLookup> List;
};

class WORLDMAPWIDGET2_EXPORT WMWDragData : public QMimeData
{
Q_OBJECT

public:
    WMWDragData();

    //! Total number of items in the drag, in case there are items which are not yet in the model
    int itemCount;
    QList<QPersistentModelIndex> itemIndices;
    bool haveDragPixmap;

    // TODO: find a way to retrieve the items which are not yet in the model
};

} /* WMW2 */

inline QDebug operator<<(QDebug debugOut, const WMW2::WMWGeoCoordinate& coordinate)
{
    debugOut << coordinate.geoUrl();
    return debugOut;
}

inline bool operator==(const WMW2::WMWGeoCoordinate& a, const WMW2::WMWGeoCoordinate& b)
{
    return
        ( a.lat == b.lat ) &&
        ( a.lon == b.lon ) &&
        ( a.hasAlt == b.hasAlt ) &&
        ( a.hasAlt ? ( a.alt == b.alt ) : true );
}

Q_DECLARE_METATYPE(WMW2::WMWGeoCoordinate)
Q_DECLARE_METATYPE(WMW2::WMWGeoCoordinate::Pair)
Q_DECLARE_METATYPE(WMW2::WMWGeoCoordinate::PairList)
Q_DECLARE_METATYPE(WMW2::WMWAltitudeLookup)

#endif /* WORLDMAPWIDGET2_PRIMITIVES_H */
