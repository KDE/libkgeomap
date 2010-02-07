/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Google-Maps-backend for WorldMapWidget2
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

#include "backend-googlemaps.h"
#include "html_widget.h"
#include "worldmapwidget2.h"
#include "markermodel.h"

namespace WMW2 {

class BackendGoogleMapsPrivate
{
public:
    BackendGoogleMapsPrivate()
    : htmlWidget(0),
      htmlWidgetWrapper(0),
      isReady(false),
      mapTypeActionGroup(0),
      floatItemsActionGroup(0),
      showMapTypeControlAction(0),
      showNavigationControlAction(0),
      showScaleControlAction(0),
      cacheMapType("ROADMAP"),
      cacheShowMapTypeControl(true),
      cacheShowNavigationControl(true),
      cacheShowScaleControl(true),
      cacheZoom(1),
      cacheCenter(0.0,0.0),
      cacheBounds()
    {
    }

    QPointer<HTMLWidget> htmlWidget;
    QPointer<QWidget> htmlWidgetWrapper;
    bool isReady;
    QPointer<QActionGroup> mapTypeActionGroup;
    QPointer<QActionGroup> floatItemsActionGroup;
    QPointer<KAction> showMapTypeControlAction;
    QPointer<KAction> showNavigationControlAction;
    QPointer<KAction> showScaleControlAction;

    QString cacheMapType;
    bool cacheShowMapTypeControl;
    bool cacheShowNavigationControl;
    bool cacheShowScaleControl;
    int cacheZoom;
    WMWGeoCoordinate cacheCenter;
    QPair<WMWGeoCoordinate, WMWGeoCoordinate> cacheBounds;
};

BackendGoogleMaps::BackendGoogleMaps(WMWSharedData* const sharedData, QObject* const parent)
: MapBackend(sharedData, parent), d(new BackendGoogleMapsPrivate())
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

void BackendGoogleMaps::loadInitialHTML()
{
    const KUrl htmlUrl = KStandardDirs::locate("data", "libworldmapwidget2/backend-googlemaps.html");

    d->htmlWidget->openUrl(htmlUrl);
}

BackendGoogleMaps::~BackendGoogleMaps()
{
    if (d->htmlWidgetWrapper)
        delete d->htmlWidgetWrapper;
    
    delete d;
}

QString BackendGoogleMaps::backendName() const
{
    return "googlemaps";
}

QString BackendGoogleMaps::backendHumanName() const
{
    return i18n("Google Maps");
}

QWidget* BackendGoogleMaps::mapWidget() const
{
    return d->htmlWidgetWrapper.data();
}



WMWGeoCoordinate BackendGoogleMaps::getCenter() const
{
    return d->cacheCenter;
}

void BackendGoogleMaps::setCenter(const WMWGeoCoordinate& coordinate)
{
    d->cacheCenter = coordinate;

    if (isReady())
    {
        d->htmlWidget->runScript(QString("wmwSetCenter(%1, %2);").arg(d->cacheCenter.latString()).arg(d->cacheCenter.lonString()));
    }
}

bool BackendGoogleMaps::isReady() const
{
    return d->isReady;
}

void BackendGoogleMaps::slotHTMLInitialized()
{
    kDebug()<<1;
    d->isReady = true;
    d->htmlWidget->runScript(QString("document.getElementById(\"map_canvas\").style.height=\"%1px\"").arg(d->htmlWidgetWrapper->height()));

    // TODO: call javascript directly here and update action availability in one shot
    setMapType(d->cacheMapType);
    setShowMapTypeControl(d->cacheShowMapTypeControl);
    setShowNavigationControl(d->cacheShowNavigationControl);
    setShowScaleControl(d->cacheShowNavigationControl);
    setCenter(d->cacheCenter);
    d->htmlWidget->runScript(QString("wmwSetZoom(%1);").arg(d->cacheZoom));
    emit(signalBackendReady(backendName()));
}

void BackendGoogleMaps::zoomIn()
{
    if (!d->isReady)
        return;
    
    d->htmlWidget->runScript(QString("wmwZoomIn();"));
}

void BackendGoogleMaps::zoomOut()
{
    if (!d->isReady)
        return;

    d->htmlWidget->runScript(QString("wmwZoomOut();"));
}

QString BackendGoogleMaps::getMapType() const
{
    return d->cacheMapType;
}

void BackendGoogleMaps::setMapType(const QString& newMapType)
{
    d->cacheMapType = newMapType;
    kDebug()<<newMapType;

    if (isReady())
    {
        d->htmlWidget->runScript(QString("wmwSetMapType(\"%1\");").arg(newMapType));
        updateActionsEnabled();
    }
}

void BackendGoogleMaps::updateActionsEnabled()
{
    if (d->mapTypeActionGroup&&isReady())
    {
        const QString currentMapType = getMapType();
        const QList<QAction*> mapTypeActions = d->mapTypeActionGroup->actions();
        for (int i=0; i<mapTypeActions.size(); ++i)
        {
            mapTypeActions.at(i)->setChecked(mapTypeActions.at(i)->data().toString()==currentMapType);
        }
    }

    // TODO: manage state of the zoom buttons
}

void BackendGoogleMaps::slotMapTypeActionTriggered(QAction* action)
{
    const QString newMapType = action->data().toString();
    setMapType(newMapType);
}

void BackendGoogleMaps::addActionsToConfigurationMenu(QMenu* const configurationMenu)
{
    WMW2_ASSERT(configurationMenu!=0);

    if (!d->isReady)
        return;

    configurationMenu->addSeparator();
    
    // actions for selecting the map type:
    QStringList mapTypes, mapTypesHumanNames;
    mapTypes           << "ROADMAP"       << "SATELLITE"        << "HYBRID"       << "TERRAIN";
    mapTypesHumanNames << i18n("Roadmap") << i18n("Satellite")  << i18n("Hybrid") << i18n("Terrain");

    const QString currentMapType = getMapType();

    if (d->mapTypeActionGroup)
    {
        delete d->mapTypeActionGroup;
    }
    d->mapTypeActionGroup = new QActionGroup(configurationMenu);
    d->mapTypeActionGroup->setExclusive(true);
    connect(d->mapTypeActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotMapTypeActionTriggered(QAction*)));

    for (int i = 0; i<mapTypes.count(); ++i)
    {
        KAction* const mapTypeAction = new KAction(d->mapTypeActionGroup);
        mapTypeAction->setData(mapTypes.at(i));
        mapTypeAction->setText(mapTypesHumanNames.at(i));
        mapTypeAction->setCheckable(true);

        
        if (currentMapType == mapTypes.at(i))
        {
            mapTypeAction->setChecked(true);
        }

        configurationMenu->addAction(mapTypeAction);
    }

    configurationMenu->addSeparator();

    if (d->floatItemsActionGroup)
    {
        delete d->floatItemsActionGroup;
    }
    d->floatItemsActionGroup = new QActionGroup(configurationMenu);
    d->floatItemsActionGroup->setExclusive(false);
    connect(d->floatItemsActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotFloatSettingsTriggered(QAction*)));

    // TODO: we need a parent for this guy!
    QMenu* const floatItemsSubMenu = new QMenu(i18n("Float items"), configurationMenu);
    configurationMenu->addMenu(floatItemsSubMenu);

    d->showMapTypeControlAction = new KAction(i18n("Show Map Type Control"), d->floatItemsActionGroup);
    d->showMapTypeControlAction->setCheckable(true);
    d->showMapTypeControlAction->setChecked(d->cacheShowMapTypeControl);
    d->showMapTypeControlAction->setData("showmaptypecontrol");
    floatItemsSubMenu->addAction(d->showMapTypeControlAction);

    d->showNavigationControlAction = new KAction(i18n("Show Navigation Control"), d->floatItemsActionGroup);
    d->showNavigationControlAction->setCheckable(true);
    d->showNavigationControlAction->setChecked(d->cacheShowNavigationControl);
    d->showNavigationControlAction->setData("shownavigationcontrol");
    floatItemsSubMenu->addAction(d->showNavigationControlAction);

    d->showScaleControlAction = new KAction(i18n("Show Scale Control"), d->floatItemsActionGroup);
    d->showScaleControlAction->setCheckable(true);
    d->showScaleControlAction->setChecked(d->cacheShowScaleControl);
    d->showScaleControlAction->setData("showscalecontrol");
    floatItemsSubMenu->addAction(d->showScaleControlAction);
}

void BackendGoogleMaps::saveSettingsToGroup(KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
    if (!group)
        return;

    group->writeEntry("GoogleMaps Map Type", getMapType());
    group->writeEntry("GoogleMaps Show Map Type Control", d->cacheShowMapTypeControl);
    group->writeEntry("GoogleMaps Show Navigation Control", d->cacheShowNavigationControl);
    group->writeEntry("GoogleMaps Show Scale Control", d->cacheShowScaleControl);
}

void BackendGoogleMaps::readSettingsFromGroup(const KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
    if (!group)
        return;

    const QString mapType = group->readEntry("GoogleMaps Map Type", "ROADMAP");
    setMapType(mapType);
    setShowMapTypeControl(group->readEntry("GoogleMaps Show Map Type Control", true));
    setShowNavigationControl(group->readEntry("GoogleMaps Show Navigation Control", true));
    setShowScaleControl(group->readEntry("GoogleMaps Show Scale Control", true));
}

void BackendGoogleMaps::updateMarkers()
{
    WMW2_ASSERT(isReady());
    if (!isReady())
        return;
    
    // re-transfer all markers to the javascript-part:
    d->htmlWidget->runScript(QString("wmwClearMarkers();"));
    for (QIntList::const_iterator it = s->visibleMarkers.constBegin(); it!=s->visibleMarkers.constEnd(); ++it)
    {
        const int currentIndex = *it;
        const WMWMarker& currentMarker = s->markerList.at(currentIndex);

        d->htmlWidget->runScript(QString("wmwAddMarker(%1, %2, %3, %4);")
                .arg(currentIndex)
                .arg(currentMarker.coordinates.latString())
                .arg(currentMarker.coordinates.lonString())
                .arg(currentMarker.isDraggable()?"true":"false")
            );
    }
    
}

void BackendGoogleMaps::slotHTMLEvents(const QStringList& events)
{
    // for some events, we just note that they appeared and then process them later on:
    bool centerProbablyChanged = false;
    bool mapTypeChanged = false;
    bool zoomProbablyChanged = false;
    bool mapBoundsProbablyChanged = false;
    QIntList movedClusters;
    QIntList movedMarkers;
    for (QStringList::const_iterator it = events.constBegin(); it!=events.constEnd(); ++it)
    {
        const QString eventCode = it->left(2);
        const QString eventParameter = it->mid(2);
        const QStringList eventParameters = eventParameter.split('/');

        if (eventCode=="MT")
        {
            // map type changed
            mapTypeChanged = true;
            d->cacheMapType = eventParameter;
        }
        else if (eventCode=="MB")
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
                    &clusterCoordinates
                );

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
            const int markerIndex= eventParameter.toInt(&okay);
            if (!okay)
                continue;

            if ((markerIndex<0)||(markerIndex>s->markerList.count()))
                continue;

            // re-read the marker position:
            WMWGeoCoordinate markerCoordinates;
            const bool isValid = d->htmlWidget->runScript2Coordinates(
                    QString("wmwGetMarkerPosition(%1);").arg(markerIndex),
                    &markerCoordinates
                );

            if (!isValid)
                continue;

            // TODO: this discards the altitude!
            s->markerList[markerIndex].coordinates = markerCoordinates;

            movedMarkers << markerIndex;
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
        emit(signalMarkersMoved(movedMarkers));
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
        const QString mapBoundsString = d->htmlWidget->runScript("map.getBounds().toString();").toString();
        QPair<WMWGeoCoordinate, WMWGeoCoordinate> boundsCoordinates;
        WMWHelperParseBoundsString(mapBoundsString, &d->cacheBounds);
    }

    if (mapBoundsProbablyChanged||!movedClusters.isEmpty())
    {
        s->worldMapWidget->updateClusters();
    }
}

void BackendGoogleMaps::updateClusters()
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

bool BackendGoogleMaps::screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point)
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

bool BackendGoogleMaps::geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const
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

QSize BackendGoogleMaps::mapSize() const
{
    WMW2_ASSERT(d->htmlWidgetWrapper!=0);

    return d->htmlWidgetWrapper->size();
}

void BackendGoogleMaps::slotFloatSettingsTriggered(QAction* action)
{
    const QString actionIdString = action->data().toString();
    const bool actionState = action->isChecked();

    if (actionIdString=="showmaptypecontrol")
    {
        setShowMapTypeControl(actionState);
    }
    else if (actionIdString=="shownavigationcontrol")
    {
        setShowNavigationControl(actionState);
    }
    else if (actionIdString=="showscalecontrol")
    {
        setShowScaleControl(actionState);
    }
}

void BackendGoogleMaps::setShowScaleControl(const bool state)
{
    d->cacheShowScaleControl = state;

    if (d->showScaleControlAction)
        d->showScaleControlAction->setChecked(state);

    if (!isReady())
        return;

    d->htmlWidget->runScript(QString("wmwSetShowScaleControl(%1);").arg(state?"true":"false"));
}

void BackendGoogleMaps::setShowNavigationControl(const bool state)
{
    d->cacheShowNavigationControl = state;

    if (d->showNavigationControlAction)
        d->showNavigationControlAction->setChecked(state);

    if (!isReady())
        return;

    d->htmlWidget->runScript(QString("wmwSetShowNavigationControl(%1);").arg(state?"true":"false"));
}

void BackendGoogleMaps::setShowMapTypeControl(const bool state)
{
    d->cacheShowMapTypeControl = state;

    if (d->showMapTypeControlAction)
        d->showMapTypeControlAction->setChecked(state);

    if (!isReady())
        return;

    d->htmlWidget->runScript(QString("wmwSetShowMapTypeControl(%1);").arg(state?"true":"false"));
}

void BackendGoogleMaps::slotClustersNeedUpdating()
{
    s->worldMapWidget->updateClusters();
}

void BackendGoogleMaps::setZoom(const QString& newZoom)
{
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

QString BackendGoogleMaps::getZoom() const
{
    return QString("googlemaps:%1").arg(d->cacheZoom);
}

int BackendGoogleMaps::getMarkerModelLevel()
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

QList<QPair<WMWGeoCoordinate, WMWGeoCoordinate> > BackendGoogleMaps::getNormalizedBounds()
{
    QList<QPair<WMWGeoCoordinate, WMWGeoCoordinate> > boundsList;

    const qreal bWest = d->cacheBounds.first.lon;
    const qreal bEast = d->cacheBounds.second.lon;
    const qreal bNorth = d->cacheBounds.second.lat;
    const qreal bSouth = d->cacheBounds.first.lat;
    kDebug()<<bWest<<bEast<<bNorth<<bSouth;

    if (bEast<bWest)
    {
        boundsList << QPair<WMWGeoCoordinate, WMWGeoCoordinate>(WMWGeoCoordinate(bSouth, bEast), WMWGeoCoordinate(bNorth, 0));
        boundsList << QPair<WMWGeoCoordinate, WMWGeoCoordinate>(WMWGeoCoordinate(bSouth, 0), WMWGeoCoordinate(bNorth, bWest));
    }
    else
    {
        boundsList << QPair<WMWGeoCoordinate, WMWGeoCoordinate>(WMWGeoCoordinate(bSouth, bWest), WMWGeoCoordinate(bNorth, bEast));
    }
    kDebug()<<boundsList;
    return boundsList;
}

} /* WMW2 */

