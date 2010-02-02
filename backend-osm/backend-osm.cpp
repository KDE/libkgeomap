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

#include "backend-osm.h"
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

BackendOSM::BackendOSM(WMWSharedData* const sharedData, QObject* const parent)
: MapBackend(sharedData, parent), d(new BackendOSMPrivate())
{
    d->htmlWidgetWrapper = new QWidget();
    d->htmlWidgetWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->htmlWidget = new HTMLWidget(d->htmlWidgetWrapper);
    d->htmlWidgetWrapper->resize(400,400);

    connect(d->htmlWidget, SIGNAL(completed()),
            this, SLOT(slotHTMLInitialized()));

    connect(d->htmlWidget, SIGNAL(signalHTMLEvents(const QStringList&)),
            this, SLOT(slotHTMLEvents(const QStringList&)));

    loadInitialHTML();
}

void BackendOSM::loadInitialHTML()
{
    const KUrl htmlUrl = KStandardDirs::locate("data", "worldmapwidget2/backend-osm.html");

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

bool BackendOSM::googleVariantToCoordinates(const QVariant& googleVariant, WMWGeoCoordinate* const coordinates) const
{
    bool valid = ( googleVariant.type()==QVariant::String );
    if (valid)
    {
        QStringList coordinateStrings = googleVariant.toString().split(',');
        valid = ( coordinateStrings.size() == 2 );
        if (valid)
        {
            double    ptLongitude = 0.0;
            double    ptLatitude  = 0.0;

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
    }

    return false;
}

bool BackendOSM::googleVariantToPoint(const QVariant& googleVariant, QPoint* const point) const
{
    // a point is returned as (x, y)
    bool valid = ( googleVariant.type()==QVariant::String );
    if (valid)
    {
        QString pointString = googleVariant.toString();
        valid = pointString.startsWith('(')&&pointString.endsWith(')');
        QStringList pointStrings;
        if (valid)
        {
            pointStrings = pointString.mid(1, pointString.length()-2).split(',');
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
    }

    return false;
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

QString BackendOSM::getMapType() const
{
    return d->cacheMapType;
}

void BackendOSM::updateActionsEnabled()
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

void BackendOSM::slotMapTypeActionTriggered(QAction* action)
{
    const QString newMapType = action->data().toString();
}

void BackendOSM::addActionsToConfigurationMenu(QMenu* const configurationMenu)
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

void BackendOSM::saveSettingsToGroup(KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
    if (!group)
        return;

    group->writeEntry("OSM Map Type", getMapType());
    group->writeEntry("OSM Show Map Type Control", d->cacheShowMapTypeControl);
    group->writeEntry("OSM Show Navigation Control", d->cacheShowNavigationControl);
    group->writeEntry("OSM Show Scale Control", d->cacheShowScaleControl);
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

void BackendOSM::slotHTMLEvents(const QStringList& events)
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
            const bool isValid = googleVariantToCoordinates(
                d->htmlWidget->runScript(QString("wmwGetClusterPosition(%1);").arg(clusterIndex)),
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
            const int markerIndex= eventParameter.toInt(&okay);
            if (!okay)
                continue;

            if ((markerIndex<0)||(markerIndex>s->markerList.count()))
                continue;

            // re-read the marker position:
            WMWGeoCoordinate markerCoordinates;
            const bool isValid = googleVariantToCoordinates(
                d->htmlWidget->runScript(QString("wmwGetMarkerPosition(%1);").arg(markerIndex)),
                                                            &markerCoordinates);

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
        /*const bool isValid = */googleVariantToCoordinates(d->htmlWidget->runScript("wmwGetCenter();"), &(d->cacheCenter));
    }

    // update the actions if necessary:
    if (zoomProbablyChanged||mapTypeChanged||centerProbablyChanged)
    {
        updateActionsEnabled();
    }

    if (mapBoundsProbablyChanged)
    {
        kDebug()<<"getbounds";
        // TODO: parse this better!!!
        const QString mapBoundsString = "((-180,-90),(180,90))";//d->htmlWidget->runScript("map.getBounds().toString();").toString();
//         kDebug()<<mapBoundsString;
        const QString string1 = mapBoundsString.mid(1, mapBoundsString.length()-2);
        int dumpComma = string1.indexOf(",", 0);
        int splitComma = string1.indexOf(",", dumpComma+1);
        QString coord1String = string1.mid(0, splitComma);
        QString coord2String = string1.mid(splitComma+2);
        WMWGeoCoordinate coord1, coord2;
        googleVariantToCoordinates(QVariant(coord1String.mid(1, coord1String.length()-2)), &coord1);
        googleVariantToCoordinates(QVariant(coord2String.mid(1, coord2String.length()-2)), &coord2);
        d->cacheBounds = QPair<WMWGeoCoordinate, WMWGeoCoordinate>(coord1, coord2);
//         kDebug()<<coord1String<<coord2String;
//         QStringList mapBoundsStrings = mapBoundsString.split
        kDebug()<<"gotbounds";
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

        d->htmlWidget->runScript(QString("wmwAddCluster(%1, %2, %3, %4);")
                .arg(currentIndex)
                .arg(currentCluster.coordinates.latString())
                .arg(currentCluster.coordinates.lonString())
                .arg(true?"true":"false") // TODO: just for now, for testing
            );
    }
    kDebug()<<"end updateclusters";
}

bool BackendOSM::screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point)
{
    if (!d->isReady)
        return false;

    const bool isValid = googleVariantToPoint(d->htmlWidget->runScript(
            QString("wmwLatLngToPixel(%1, %2);")
                .arg(coordinates.latString())
                .arg(coordinates.lonString())
                ),
            point);

    // TODO: apparently, even points outside the visible area are returned as valid
    // check whether they are actually visible
    return isValid;
}

bool BackendOSM::geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const
{
    if (!d->isReady)
        return false;

    const QVariant coordinatesVariant = d->htmlWidget->runScript(
            QString("wmwPixelToLatLng(%1, %2);")
                .arg(point.x())
                .arg(point.y()));
    const bool isValid = googleVariantToCoordinates(coordinatesVariant, coordinates);

    return isValid;
}

QSize BackendOSM::mapSize() const
{
    WMW2_ASSERT(d->htmlWidgetWrapper!=0);

    return d->htmlWidgetWrapper->size();
}

void BackendOSM::slotFloatSettingsTriggered(QAction* action)
{
    const QString actionIdString = action->data().toString();
    const bool actionState = action->isChecked();
}

void BackendOSM::slotClustersNeedUpdating()
{
    s->worldMapWidget->updateClusters();
}

void BackendOSM::setZoom(const QString& newZoom)
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

QString BackendOSM::getZoom() const
{
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

QList<QPair<WMWGeoCoordinate, WMWGeoCoordinate> > BackendOSM::getNormalizedBounds()
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

