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

    connect(d->htmlWidget, SIGNAL(completed()),
            this, SLOT(slotHTMLInitialized()));

    connect(d->htmlWidget, SIGNAL(signalHTMLEvents(const QStringList&)),
            this, SLOT(slotHTMLEvents(const QStringList&)));

    loadInitialHTML();
}

void BackendGoogleMaps::loadInitialHTML()
{
        QString mapHTMLCode =
"<html>\n"
"<head>\n"
"<script type=\"text/javascript\" src=\"http://maps.google.com/maps/api/js?sensor=false\"></script>\n"
"<script type=\"text/javascript\">\n"
"   var mapDiv;\n"
"   var map;\n"
"   var eventBuffer = new Array();\n"
"   var markerList = new Object();\n"
"   var clusterList = new Object();\n"
// ProjectionHelp: http://taapps-javalibs.blogspot.com/2009/10/google-map-v3how-to-use-overlayviews.html
"   function ProjectionHelper(overlayMap) {\n"
"       google.maps.OverlayView.call(this);\n"
"       this.setMap(overlayMap);\n"
"   }\n"
"   ProjectionHelper.prototype = new google.maps.OverlayView();\n"
"   ProjectionHelper.prototype.draw = function() {\n"
"       \n"
"   }\n"
"   var projectionHelper = null;\n"
"   function wmwPostEventString(eventString) {\n"
"       eventBuffer.push(eventString);\n"
"       window.status = '(event)';\n"
"   }\n"
"   function wmwReadEventStrings() {\n"
"       var eventBufferString = eventBuffer.join('|');\n"
"       eventBuffer = new Array();\n"
// let the application know that there are no more events waiting:
"       window.status = '()';\n"
"       return eventBufferString;\n"
"   }\n"
"   function wmwDebugOut(someString) {\n"
"       wmwPostEventString('do'+someString);\n"
"   }\n"
"   function wmwSetZoom(zoomvalue) {\n"
"       map.setZoom(zoomvalue);\n"
"   }\n"
"   function wmwGetZoom() {\n"
"       return map.getZoom();\n"
"   }\n"
"   function wmwZoomIn() {\n"
"       map.setZoom(map.getZoom()+1);\n"
"   }\n"
"   function wmwZoomOut() {\n"
"       map.setZoom(map.getZoom()-1);\n"
"   }\n"
"   function wmwSetCenter(lat, lon) {\n"
"       var latlng = new google.maps.LatLng(lat, lon);\n"
"       map.setCenter(latlng);\n"
"   }\n"
"   function wmwGetCenter() {\n"
"       var latlngString = map.getCenter().toUrlValue(12);\n"
"       return latlngString;\n"
"   }\n"
"   function wmwLatLngToPixel(lat, lon) {\n"
//      There is an offset in fromLatLngToDivPixel once the map has been panned
"       var latlng = new google.maps.LatLng(lat, lon);\n"
"       var myPoint = projectionHelper.getProjection().fromLatLngToDivPixel(latlng);\n"
"       var centerPoint = projectionHelper.getProjection().fromLatLngToDivPixel(map.getCenter());\n"
"       var centerOffsetX = Math.floor(mapDiv.offsetWidth / 2);\n"
"       var centerOffsetY = Math.floor(mapDiv.offsetHeight / 2);\n"
"       var pointX = myPoint.x-centerPoint.x+centerOffsetX;\n"
"       var pointY = myPoint.y-centerPoint.y+centerOffsetY;\n"
"       return new google.maps.Point(pointX, pointY).toString();\n"
// "       return projectionHelper.getProjection().fromLatLngToDivPixel(latlng).toString();\n"
"   }\n"
"   function wmwPixelToLatLng(x, y) {\n"
//      There is an offset in fromDivPixelToLatLng once the map has been panned
"       var centerPoint = projectionHelper.getProjection().fromLatLngToDivPixel(map.getCenter());\n"
"       var centerOffsetX = mapDiv.offsetWidth / 2;\n"
"       var centerOffsetY = mapDiv.offsetHeight / 2;\n"
"       var pointX = x+centerPoint.x-centerOffsetX;\n"
"       var pointY = y+centerPoint.y-centerOffsetY;\n"
"       var point = new google.maps.Point(pointX, pointY); \n"
"       return projectionHelper.getProjection().fromDivPixelToLatLng(point).toUrlValue(12);\n"
"   }\n"
    // parameter: "SATELLITE"/"ROADMAP"/"HYBRID"/"TERRAIN"
"   function wmwSetMapType(newMapType) {\n"
"       if (newMapType == \"SATELLITE\") { map.setMapTypeId(google.maps.MapTypeId.SATELLITE); }\n"
"       if (newMapType == \"ROADMAP\")   { map.setMapTypeId(google.maps.MapTypeId.ROADMAP); }\n"
"       if (newMapType == \"HYBRID\")    { map.setMapTypeId(google.maps.MapTypeId.HYBRID); }\n"
"       if (newMapType == \"TERRAIN\")   { map.setMapTypeId(google.maps.MapTypeId.TERRAIN); }\n"
"   }\n"
"   function wmwGetMapType() {\n"
"       var myMapType = map.getMapTypeId();\n"
"       if (myMapType == google.maps.MapTypeId.SATELLITE) { return \"SATELLITE\"; }\n"
"       if (myMapType == google.maps.MapTypeId.ROADMAP )  { return \"ROADMAP\"; }\n"
"       if (myMapType == google.maps.MapTypeId.HYBRID )   { return \"HYBRID\"; }\n"
"       if (myMapType == google.maps.MapTypeId.TERRAIN )  { return \"TERRAIN\"; }\n"
"       return "";\n" // unexpected result
"   }\n"
"   function wmwSetShowMapTypeControl(state) {\n"
"       var myOptions = {\n"
"           mapTypeControl: state\n"
"       }\n"
"       map.setOptions(myOptions);\n"
"   }\n"
"   function wmwSetShowNavigationControl(state) {\n"
"       var myOptions = {\n"
"           navigationControl: state\n"
"       }\n"
"       map.setOptions(myOptions);\n"
"   }\n"
"   function wmwSetShowScaleControl(state) {\n"
"       var myOptions = {\n"
"           scaleControl: state\n"
"       }\n"
"       map.setOptions(myOptions);\n"
"   }\n"
"   function wmwClearMarkers() {\n"
"       for (var i in markerList) {\n"
"           markerList[i].setMap(null);\n"
"       }\n"
"       markerList = new Object();\n"
"   }\n"
"   function wmwAddMarker(id, lat, lon, setDraggable) {\n"
"       var latlng = new google.maps.LatLng(lat, lon);\n"
"       var marker = new google.maps.Marker({\n"
"           position: latlng,\n"
"           map: map,\n"
"           draggable: setDraggable\n"
"       });\n"
"       google.maps.event.addListener(marker, 'dragend', function() {\n"
"           wmwPostEventString('mm'+id.toString());\n"
"       });\n"
"       markerList[id] = marker;\n"
"   }\n"
"   function wmwGetMarkerPosition(id) {\n"
"       var latlngString;\n"
"       if (markerList[id.toString()]) {\n"
"           latlngString = markerList[id.toString()].getPosition().toUrlValue(12);\n"
"       }\n"
"       return latlngString;\n"
"   }\n"
"   function wmwClearClusters() {\n"
"       for (var i in clusterList) {\n"
"           clusterList[i].setMap(null);\n"
"       }\n"
"       clusterList = new Object();\n"
"   }\n"
"   function wmwAddCluster(id, lat, lon, setDraggable) {\n"
"       var latlng = new google.maps.LatLng(lat, lon);\n"
"       var marker = new google.maps.Marker({\n"
"           position: latlng,\n"
"           map: map,\n"
"           draggable: setDraggable\n"
"       });\n"
"       google.maps.event.addListener(marker, 'dragend', function() {\n"
"           wmwPostEventString('cm'+id.toString());\n"
"       });\n"
"       clusterList[id] = marker;\n"
"   }\n"
"   function wmwGetClusterPosition(id) {\n"
"       var latlngString;\n"
"       if (clusterList[id.toString()]) {\n"
"           latlngString = clusterList[id.toString()].getPosition().toUrlValue(12);\n"
"       }\n"
"       return latlngString;\n"
"   }\n"
"   function wmwWidgetResized(newWidth, newHeight) {\n"
"       document.getElementById('map_canvas').style.height=newHeight.toString()+'px';\n"
"   }\n"
"   function initialize() {\n"
"       var latlng = new google.maps.LatLng(%1, %2);\n"
"       var myOptions = {\n"
"           zoom: 8,\n"
"           center: latlng,\n"
"           mapTypeId: google.maps.MapTypeId.%3\n"
"       };\n"
"       mapDiv = document.getElementById(\"map_canvas\");\n"
// "       mapDiv.style.height=\"100%\"\n";
"       map = new google.maps.Map(mapDiv, myOptions);\n"
"       google.maps.event.addListener(map, 'maptypeid_changed', function() {\n"
"           wmwPostEventString('MT'+wmwGetMapType());\n"
"       });\n"
//  these are too heavy on the performance. monitor 'idle' event only for now:
// "       google.maps.event.addListener(map, 'bounds_changed', function() {\n"
// "           wmwPostEventString('MB');\n"
// "       });\n"
// "       google.maps.event.addListener(map, 'zoom_changed', function() {\n"
// "           wmwPostEventString('ZC');\n"
// "       });\n"
"       google.maps.event.addListener(map, 'idle', function() {\n"
"           wmwPostEventString('id');\n"
"       });\n"
// source: http://taapps-javalibs.blogspot.com/2009/10/google-map-v3how-to-use-overlayviews.html
"       projectionHelper = new ProjectionHelper(map);\n"
"   }\n"
"</script>\n"
"</head>\n"
"<body onload=\"initialize()\" style=\"padding: 0px; margin: 0px;\">\n"
// "   <div id=\"map_canvas\" style=\"width:400px; height:400px; border: 1px solid black;\"></div>\n"
"   <div id=\"map_canvas\" style=\"width:100%; height:400px;\"></div>\n"
// "<div style=\"display: block;\" id=\"mydiv\">aoeu</div>\n"
"</body>\n"
"</html>\n";

    const WMWGeoCoordinate initialCenter = WMWGeoCoordinate(52.0, 6.0);
    const QString initialMapType = "ROADMAP";
    mapHTMLCode = mapHTMLCode.arg(initialCenter.latString())
                             .arg(initialCenter.lonString())
                             .arg(initialMapType);

    d->htmlWidget->loadInitialHTML(mapHTMLCode);
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

bool BackendGoogleMaps::googleVariantToCoordinates(const QVariant& googleVariant, WMWGeoCoordinate* const coordinates) const
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

bool BackendGoogleMaps::googleVariantToPoint(const QVariant& googleVariant, QPoint* const point) const
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
        const QString mapBoundsString = d->htmlWidget->runScript("map.getBounds().toString();").toString();
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

        d->htmlWidget->runScript(QString("wmwAddCluster(%1, %2, %3, %4);")
                .arg(currentIndex)
                .arg(currentCluster.coordinates.latString())
                .arg(currentCluster.coordinates.lonString())
                .arg(true?"true":"false") // TODO: just for now, for testing
            );
    }
    kDebug()<<"end updateclusters";
}

bool BackendGoogleMaps::screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point)
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

bool BackendGoogleMaps::geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const
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

