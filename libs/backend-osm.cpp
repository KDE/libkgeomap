/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : OpenStreetMap-backend for WorldMapWidget2
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

#include "backend-osm.moc"

// Qt includes

#include <QActionGroup>
#include <QMenu>
#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kconfiggroup.h>
#include <khtml_part.h>
#include <kstandarddirs.h>

// local includes

#include "html_widget.h"
#include "worldmapwidget2.h"
#include "markermodel.h"

namespace WMW2 {

class BackendOSMPrivate
{
public:
    BackendOSMPrivate()
    : htmlWidget(0),
      htmlWidgetWrapper(0),
      isReady(false),
      cacheZoom(1),
      cacheCenter(0.0,0.0),
      cacheBounds()
    {
    }

    QPointer<HTMLWidget> htmlWidget;
    QPointer<QWidget> htmlWidgetWrapper;
    bool isReady;

    int cacheZoom;
    WMWGeoCoordinate cacheCenter;
    QPair<WMWGeoCoordinate, WMWGeoCoordinate> cacheBounds;
};

BackendOSM::BackendOSM(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent)
: MapBackend(sharedData, parent), d(new BackendOSMPrivate())
{
    d->htmlWidgetWrapper = new QWidget();
    d->htmlWidgetWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->htmlWidget = new HTMLWidget(d->htmlWidgetWrapper);
    d->htmlWidgetWrapper->resize(400,400);

    connect(d->htmlWidget, SIGNAL(signalJavaScriptReady()),
            this, SLOT(slotHTMLInitialized()));

    connect(d->htmlWidget, SIGNAL(signalHTMLEvents(const QStringList&)),
            this, SLOT(slotHTMLEvents(const QStringList&)));

    loadInitialHTML();
}

void BackendOSM::loadInitialHTML()
{
    const KUrl htmlUrl = KStandardDirs::locate("data", "libworldmapwidget2/backend-osm.html");

    d->htmlWidget->openUrl(htmlUrl);
}

BackendOSM::~BackendOSM()
{
    if (d->htmlWidgetWrapper)
        delete d->htmlWidgetWrapper;
    
    delete d;
}

QString BackendOSM::backendName() const
{
    return "osm";
}

QString BackendOSM::backendHumanName() const
{
    return i18n("OpenStreetMap");
}

QWidget* BackendOSM::mapWidget() const
{
    return d->htmlWidgetWrapper.data();
}

WMWGeoCoordinate BackendOSM::getCenter() const
{
    return d->cacheCenter;
}

void BackendOSM::setCenter(const WMWGeoCoordinate& coordinate)
{
    kDebug()<<isReady()<<coordinate.geoUrl();
    d->cacheCenter = coordinate;

    if (isReady())
    {
        d->htmlWidget->runScript(QString("wmwSetCenter(%1, %2);").arg(d->cacheCenter.latString()).arg(d->cacheCenter.lonString()));
    }
}

bool BackendOSM::isReady() const
{
    return d->isReady;
}

void BackendOSM::slotHTMLInitialized()
{
    kDebug()<<1;
    d->isReady = true;
    d->htmlWidget->runScript(QString("document.getElementById(\"map_canvas\").style.height=\"%1px\"").arg(d->htmlWidgetWrapper->height()));

    // TODO: call javascript directly here and update action availability in one shot
    setCenter(d->cacheCenter);
    d->htmlWidget->runScript(QString("wmwSetZoom(%1);").arg(d->cacheZoom));
    emit(signalBackendReady(backendName()));
}

void BackendOSM::zoomIn()
{
    if (!d->isReady)
        return;
    
    d->htmlWidget->runScript(QString("wmwZoomIn();"));
}

void BackendOSM::zoomOut()
{
    if (!d->isReady)
        return;

    d->htmlWidget->runScript(QString("wmwZoomOut();"));
}

void BackendOSM::updateActionsEnabled()
{
    // TODO: manage state of the zoom buttons
}

void BackendOSM::addActionsToConfigurationMenu(QMenu* const configurationMenu)
{
    WMW2_ASSERT(configurationMenu!=0);

    if (!d->isReady)
        return;
}

void BackendOSM::saveSettingsToGroup(KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
    if (!group)
        return;
}

void BackendOSM::readSettingsFromGroup(const KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
    if (!group)
        return;
}

void BackendOSM::updateMarkers()
{
    WMW2_ASSERT(isReady());
    if (!isReady())
        return;
    
    // re-transfer all markers to the javascript-part:
    d->htmlWidget->runScript(QString("wmwClearMarkers();"));
    for (int row = 0; row<s->specialMarkersModel->rowCount(); ++row)
    {
        const QModelIndex currentIndex = s->specialMarkersModel->index(row, 0);

        const WMWGeoCoordinate currentCoordinates = s->specialMarkersModel->data(currentIndex, s->specialMarkersCoordinatesRole).value<WMWGeoCoordinate>();

        d->htmlWidget->runScript(QString("wmwAddMarker(%1, %2, %3, %4);")
                .arg(row)
                .arg(currentCoordinates.latString())
                .arg(currentCoordinates.lonString())
                .arg(/*currentMarker.isDraggable()?*/"true"/*:"false"*/)
            );
    }
    
}

void BackendOSM::slotHTMLEvents(const QStringList& events)
{
    // for some events, we just note that they appeared and then process them later on:
    bool centerProbablyChanged = false;
    bool mapTypeChanged = false;
    bool zoomProbablyChanged = false;
    bool mapBoundsProbablyChanged = false;
    QIntList movedClusters;
    QList<QPersistentModelIndex> movedMarkers;
    for (QStringList::const_iterator it = events.constBegin(); it!=events.constEnd(); ++it)
    {
        const QString eventCode = it->left(2);
        const QString eventParameter = it->mid(2);
        const QStringList eventParameters = eventParameter.split('/');

        if (eventCode=="MB")
        {   // NOTE: event currently disabled in javascript part
            // map bounds changed
            centerProbablyChanged = true;
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode == "ZC")
        {   // NOTE: event currently disabled in javascript part
            // zoom changed
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode == "id")
        {
            // idle after drastic map changes
            centerProbablyChanged = true;
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode=="cm")
        {
            // TODO: buffer this event type!
            // cluster moved
            bool okay = false;
            const int clusterIndex = eventParameter.toInt(&okay);
            if (!okay)
                continue;

            if ((clusterIndex<0)||(clusterIndex>s->clusterList.size()))
                continue;

            // re-read the marker position:
            WMWGeoCoordinate clusterCoordinates;
            const bool isValid = d->htmlWidget->runScript2Coordinates(
                    QString("wmwGetClusterPosition(%1);").arg(clusterIndex),
                    &clusterCoordinates);

            if (!isValid)
                continue;

            // TODO: this discards the altitude!
            s->clusterList[clusterIndex].coordinates = clusterCoordinates;

            movedClusters << clusterIndex;
        }
        else if (eventCode=="mm")
        {
            // TODO: buffer this event type!
            // marker moved
            bool okay = false;
            const int markerRow= eventParameter.toInt(&okay);
            if (!okay)
                continue;

            if ((markerRow<0)||(markerRow>s->specialMarkersModel->rowCount()))
                continue;

            // re-read the marker position:
            WMWGeoCoordinate markerCoordinates;
            const bool isValid = d->htmlWidget->runScript2Coordinates(
                    QString("wmwGetMarkerPosition(%1);").arg(markerRow),
                    &markerCoordinates
                );

            if (!isValid)
                continue;

            // TODO: this discards the altitude!
            const QModelIndex markerIndex = s->specialMarkersModel->index(markerRow, 0);
            s->specialMarkersModel->setData(markerIndex, QVariant::fromValue(markerCoordinates), s->specialMarkersCoordinatesRole);

            movedMarkers << QPersistentModelIndex(markerIndex);
        }
        else if (eventCode=="do")
        {
            // debug output:
            kDebug()<<QString("javascript:%1").arg(eventParameter);
        }
    }

    if (!movedClusters.isEmpty())
    {
        kDebug()<<movedClusters;
        emit(signalClustersMoved(movedClusters));
    }

    if (!movedMarkers.isEmpty())
    {
        kDebug()<<movedMarkers;
        emit(signalSpecialMarkersMoved(movedMarkers));
    }

    // now process the buffered events:
    if (zoomProbablyChanged)
    {
        d->cacheZoom = d->htmlWidget->runScript(QString("wmwGetZoom();")).toInt();
        emit(signalZoomChanged(QString("googlemaps:%1").arg(d->cacheZoom)));
    }
    if (centerProbablyChanged)
    {
        // there is nothing we can do if the coordinates are invalid
        /*const bool isValid = */d->htmlWidget->runScript2Coordinates("wmwGetCenter();", &(d->cacheCenter));
    }

    // update the actions if necessary:
    if (zoomProbablyChanged||mapTypeChanged||centerProbablyChanged)
    {
        updateActionsEnabled();
    }

    if (mapBoundsProbablyChanged)
    {
        const QString mapBoundsString = d->htmlWidget->runScript("wmwGetBounds();").toString();
        WMWHelperParseBoundsString(mapBoundsString, &d->cacheBounds);
    }

    if (mapBoundsProbablyChanged||!movedClusters.isEmpty())
    {
        s->worldMapWidget->updateClusters();
    }
}

void BackendOSM::updateClusters()
{
    kDebug()<<"start updateclusters";
    // re-transfer the clusters to the map:
    WMW2_ASSERT(isReady());
    if (!isReady())
        return;

    // TODO: only update clusters that have actually changed!

    // re-transfer all markers to the javascript-part:
    d->htmlWidget->runScript(QString("wmwClearClusters();"));
    for (int currentIndex = 0; currentIndex<s->clusterList.size(); ++currentIndex)
    {
        const WMWCluster& currentCluster = s->clusterList.at(currentIndex);

        // determine the colors:
        QColor       fillColor;
        QColor       strokeColor;
        Qt::PenStyle strokeStyle;
        QColor       labelColor;
        QString      labelText;
        s->worldMapWidget->getColorInfos(currentIndex, &fillColor, &strokeColor,
                              &strokeStyle, &labelText, &labelColor);

        const QString fillColorName = fillColor.name();

        d->htmlWidget->runScript(QString("wmwAddCluster(%1, %2, %3, %4, '%5', '%6');")
                .arg(currentIndex)
                .arg(currentCluster.coordinates.latString())
                .arg(currentCluster.coordinates.lonString())
                .arg(true?"true":"false")
                .arg(fillColorName.mid(1))
                .arg(labelText)
            );
    }
    kDebug()<<"end updateclusters";
}

bool BackendOSM::screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point)
{
    if (!d->isReady)
        return false;

    const bool isValid = WMWHelperParseXYStringToPoint(
            d->htmlWidget->runScript(
                QString("wmwLatLngToPixel(%1, %2);")
                    .arg(coordinates.latString())
                    .arg(coordinates.lonString())
                    ).toString(),
            point);

    // TODO: apparently, even points outside the visible area are returned as valid
    // check whether they are actually visible
    return isValid;
}

bool BackendOSM::geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const
{
    if (!d->isReady)
        return false;

    const bool isValid = d->htmlWidget->runScript2Coordinates(
            QString("wmwPixelToLatLng(%1, %2);")
                .arg(point.x())
                .arg(point.y()),
            coordinates);

    return isValid;
}

QSize BackendOSM::mapSize() const
{
    WMW2_ASSERT(d->htmlWidgetWrapper!=0);

    return d->htmlWidgetWrapper->size();
}

void BackendOSM::slotClustersNeedUpdating()
{
    s->worldMapWidget->updateClusters();
}

void BackendOSM::setZoom(const QString& newZoom)
{
    // zoom settings for OSM are basically the same as for Google Maps, so just re-use the prefix
    const QString myZoomString = s->worldMapWidget->convertZoomToBackendZoom(newZoom, "googlemaps");
    WMW2_ASSERT(myZoomString.startsWith("googlemaps:"));

    const int myZoom = myZoomString.mid(QString("googlemaps:").length()).toInt();
    kDebug()<<myZoom;

    d->cacheZoom = myZoom;

    if (isReady())
    {
        d->htmlWidget->runScript(QString("wmwSetZoom(%1);").arg(d->cacheZoom));
    }
}

QString BackendOSM::getZoom() const
{
    // zoom settings for OSM are basically the same as for Google Maps, so just re-use the prefix
    return QString("googlemaps:%1").arg(d->cacheZoom);
}

int BackendOSM::getMarkerModelLevel()
{
    WMW2_ASSERT(isReady());
    if (!isReady())
    {
        return 0;
    }

    // get the current zoom level:
    const int currentZoom = d->cacheZoom;

    int tileLevel = 0;
         if (currentZoom== 0) { tileLevel = 1; }
    else if (currentZoom== 1) { tileLevel = 1; }
    else if (currentZoom== 2) { tileLevel = 1; }
    else if (currentZoom== 3) { tileLevel = 2; }
    else if (currentZoom== 4) { tileLevel = 2; }
    else if (currentZoom== 5) { tileLevel = 3; }
    else if (currentZoom== 6) { tileLevel = 3; }
    else if (currentZoom== 7) { tileLevel = 3; }
    else if (currentZoom== 8) { tileLevel = 4; }
    else if (currentZoom== 9) { tileLevel = 4; }
    else if (currentZoom==10) { tileLevel = 4; }
    else if (currentZoom==11) { tileLevel = 4; }
    else if (currentZoom==12) { tileLevel = 4; }
    else if (currentZoom==13) { tileLevel = 4; }
    else if (currentZoom==14) { tileLevel = 5; }
    else if (currentZoom==15) { tileLevel = 5; }
    else if (currentZoom==16) { tileLevel = 5; }
    else if (currentZoom==17) { tileLevel = 5; }
    else if (currentZoom==18) { tileLevel = 6; }
    else if (currentZoom==19) { tileLevel = 6; }
    else if (currentZoom==20) { tileLevel = 6; }
    else if (currentZoom==21) { tileLevel = 7; }
    else if (currentZoom==22) { tileLevel = 7; }
    else { tileLevel = s->markerModel->maxLevel()-1; }

    WMW2_ASSERT(tileLevel<=s->markerModel->maxLevel()-1);
    
    return tileLevel;
}

WMWGeoCoordinate::PairList BackendOSM::getNormalizedBounds()
{
    return WMWHelperNormalizeBounds(d->cacheBounds);
}

} /* WMW2 */
