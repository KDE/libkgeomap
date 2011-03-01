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

#include "kmap_common.moc"

// local includes

#include "backend_map.h"


namespace KMap
{

class KMapGlobalObjectCreator
{
public:

    KMapGlobalObject object;
};

K_GLOBAL_STATIC(KMapGlobalObjectCreator, kmapGlobalObjectCreator)

class KMapGlobalObject::Private
{
public:
    Private()
    : internalMapWidgetsPool(),
      markerPixmaps()
    {
    }

    QList<KMapInternalWidgetInfo> internalMapWidgetsPool;

    // marker pixmaps:
    QMap<QString, QPixmap>    markerPixmaps;

    void loadMarkerPixmaps()
    {
        QStringList markerColors;
        markerColors
            << QLatin1String( "00ff00" )
            << QLatin1String( "00ffff" )
            << QLatin1String( "ff0000" )
            << QLatin1String( "ff7f00" )
            << QLatin1String( "ffff00" );

        QStringList stateNames;
        stateNames
            << QLatin1String( "" )
            << QLatin1String( "-selected" )
            << QLatin1String( "-someselected" );

        for (QStringList::const_iterator it = markerColors.constBegin(); it!=markerColors.constEnd(); ++it)
        {
            for (QStringList::const_iterator sit = stateNames.constBegin(); sit!=stateNames.constEnd(); ++sit)
            {
                const QString pixmapName = *it + *sit;
                const KUrl markerUrl = KMapGlobalObject::instance()->locateDataFile(
                        QString::fromLatin1( "marker-%1.png").arg(pixmapName));
                markerPixmaps[pixmapName] = QPixmap(markerUrl.toLocalFile());
            }
        }

        const KUrl markerIconUrl = KMapGlobalObject::instance()->locateDataFile(QLatin1String( "marker-icon-16x16.png" ));
        markerPixmaps[QLatin1String( "marker-icon-16x16" )] = QPixmap(markerIconUrl.toLocalFile());
    }
};

KMapGlobalObject::KMapGlobalObject()
    : QObject(), d(new Private())
{
}

KMapGlobalObject::~KMapGlobalObject()
{
    delete d;
}

KMapGlobalObject* KMapGlobalObject::instance()
{
    return &(kmapGlobalObjectCreator->object);
}

QPixmap KMapGlobalObject::getMarkerPixmap(const QString pixmapId)
{
    if (d->markerPixmaps.isEmpty())
    {
        d->loadMarkerPixmaps();
    }

    return d->markerPixmaps.value(pixmapId);
}

QPixmap KMapGlobalObject::getStandardMarkerPixmap()
{
    return getMarkerPixmap(QLatin1String( "00ff00" ));
}

KUrl KMapGlobalObject::locateDataFile(const QString filename)
{
    return KStandardDirs::locate(
                "data",
                QLatin1String("libkmap/")+filename
                          );
}

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

void KMapGlobalObject::removeMyInternalWidgetFromPool(const MapBackend* const mapBackend)
{
    for (int i=0; i<d->internalMapWidgetsPool.count(); ++i)
    {
        if (d->internalMapWidgetsPool.at(i).currentOwner==static_cast<const QObject* const>(mapBackend))
        {
            d->internalMapWidgetsPool.takeAt(i);
            break;
        }
    }
}

bool KMapGlobalObject::getInternalWidgetFromPool(const MapBackend* const mapBackend, KMapInternalWidgetInfo* const targetInfo)
{
    const QString requestingBackendName = mapBackend->backendName();

    // try to find an available widget:
    int bestDockedWidget   = -1;
    int bestUndockedWidget = -1;
    int bestReleasedWidget = -1;
    for (int i=0; i<d->internalMapWidgetsPool.count(); ++i)
    {
        const KMapInternalWidgetInfo& info = d->internalMapWidgetsPool.at(i);

        if (info.backendName!=requestingBackendName)
        {
            continue;
        }

        if ((info.state.testFlag(KMapInternalWidgetInfo::InternalWidgetReleased)&&(bestReleasedWidget<0)))
        {
            bestReleasedWidget = i;
            break;
        }

        if ((info.state.testFlag(KMapInternalWidgetInfo::InternalWidgetUndocked)&&(bestUndockedWidget<0)))
        {
            bestUndockedWidget = i;
        }

        if ((info.state.testFlag(KMapInternalWidgetInfo::InternalWidgetStillDocked)&&(bestDockedWidget<0)))
        {
            bestDockedWidget = i;
        }
    }

    int widgetToUse = bestReleasedWidget;
    if ((widgetToUse<0)&&(bestUndockedWidget>=0))
    {
        widgetToUse = bestUndockedWidget;
    }
    else
    {
        widgetToUse = bestDockedWidget;
    }

    if (widgetToUse<0)
    {
        return false;
    }

    *targetInfo = d->internalMapWidgetsPool.takeAt(widgetToUse);

    if (targetInfo->currentOwner)
    {
        qobject_cast<MapBackend*>(targetInfo->currentOwner.data())->releaseWidget(targetInfo);
    }

    return true;
}

void KMapGlobalObject::addMyInternalWidgetToPool(const KMapInternalWidgetInfo& info)
{
    d->internalMapWidgetsPool.append(info);
}

void KMapGlobalObject::updatePooledWidgetState(const QWidget* const widget, const KMapInternalWidgetInfo::InternalWidgetState newState)
{
    for (int i=0; i<d->internalMapWidgetsPool.count(); ++i)
    {
        if (d->internalMapWidgetsPool.at(i).widget==widget)
        {
            KMapInternalWidgetInfo& info = d->internalMapWidgetsPool[i];
            info.state = newState;

            if (newState==KMapInternalWidgetInfo::InternalWidgetReleased)
            {
                info.currentOwner = 0;
            }

            break;
        }
    }
}

void KMapGlobalObject::clearWidgetPool()
{
    while (!d->internalMapWidgetsPool.isEmpty())
    {
        KMapInternalWidgetInfo info = d->internalMapWidgetsPool.takeLast();
        kDebug()<<info.backendName<<info.deleteFunction;
        if (info.deleteFunction)
        {
            info.deleteFunction(&info);
        }
    }
}

} /* namespace KMap */
