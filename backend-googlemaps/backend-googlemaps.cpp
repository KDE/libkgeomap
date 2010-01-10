/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Google-Maps-backend for WorldMapWidget2
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
#include "bgm_widget.h"
#include "worldmapwidget2.h"

namespace WMW2 {

class BackendGoogleMapsPrivate
{
public:
    BackendGoogleMapsPrivate()
    : bgmWidget(0),
      bgmWidgetWrapper(0),
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
      cacheZoom(1)
    {
    }

    QPointer<BGMWidget> bgmWidget;
    QPointer<QWidget> bgmWidgetWrapper;
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
};

BackendGoogleMaps::BackendGoogleMaps(WMWSharedData* const sharedData, QObject* const parent)
: MapBackend(sharedData, parent), d(new BackendGoogleMapsPrivate())
{
    d->bgmWidgetWrapper = new QWidget();
    d->bgmWidgetWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->bgmWidget = new BGMWidget(d->bgmWidgetWrapper);
    d->bgmWidgetWrapper->resize(400,400);

    connect(d->bgmWidget, SIGNAL(completed()),
            this, SLOT(slotHTMLInitialized()));

    connect(d->bgmWidget, SIGNAL(signalHTMLEvents(const QStringList&)),
            this, SLOT(slotHTMLEvents(const QStringList&)));

    d->bgmWidget->loadInitialHTML();
}

BackendGoogleMaps::~BackendGoogleMaps()
{
    if (d->bgmWidgetWrapper)
        delete d->bgmWidgetWrapper;
    
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
//     return new QWidget();
    return d->bgmWidgetWrapper.data();
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
    WMWGeoCoordinate centerCoordinates;

    // there is nothing we can do if the coordinates are invalid
    /*const bool isValid = */googleVariantToCoordinates(d->bgmWidget->runScript("wmwGetCenter();"), &centerCoordinates);

    return centerCoordinates;
}

void BackendGoogleMaps::setCenter(const WMWGeoCoordinate& coordinate)
{
    d->bgmWidget->runScript(QString("wmwSetCenter(%1, %2);").arg(coordinate.latString()).arg(coordinate.lonString()));
}

bool BackendGoogleMaps::isReady() const
{
    return d->isReady;
}

void BackendGoogleMaps::slotHTMLInitialized()
{
    kDebug()<<1;
    d->isReady = true;
    d->bgmWidget->runScript(QString("document.getElementById(\"map_canvas\").style.height=\"%1px\"").arg(d->bgmWidgetWrapper->height()));
    setMapType(d->cacheMapType);
    setShowMapTypeControl(d->cacheShowMapTypeControl);
    setShowNavigationControl(d->cacheShowNavigationControl);
    setShowScaleControl(d->cacheShowNavigationControl);
    d->bgmWidget->runScript(QString("wmwSetZoom(%1);").arg(d->cacheZoom));
    emit(signalBackendReady(backendName()));
}

void BackendGoogleMaps::zoomIn()
{
    if (!d->isReady)
        return;
    
    d->bgmWidget->runScript(QString("wmwZoomIn();"));
}

void BackendGoogleMaps::zoomOut()
{
    if (!d->isReady)
        return;

    d->bgmWidget->runScript(QString("wmwZoomOut();"));
}

QString BackendGoogleMaps::getMapType() const
{
    if (isReady())
    {
        d->cacheMapType = d->bgmWidget->runScript(QString("wmwGetMapType();")).toString();
    }

    return d->cacheMapType;
}

void BackendGoogleMaps::setMapType(const QString& newMapType)
{
    d->cacheMapType = newMapType;
    kDebug()<<newMapType;

    if (isReady())
    {
        d->bgmWidget->runScript(QString("wmwSetMapType(\"%1\");").arg(newMapType));
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

void BackendGoogleMaps::slotMapTypeChanged(const QString& newMapType)
{
    d->cacheMapType = newMapType;

    updateActionsEnabled();
}

void BackendGoogleMaps::slotMapBoundsChanged()
{
    s->worldMapWidget->updateClusters();

    updateActionsEnabled();
}

void BackendGoogleMaps::slotZoomChanged()
{
    d->cacheZoom = d->bgmWidget->runScript(QString("wmwGetZoom();")).toInt();
    emit(signalZoomChanged(QString("googlemaps:%1").arg(d->cacheZoom)));
}

void BackendGoogleMaps::updateMarkers()
{
    WMW2_ASSERT(isReady());
    if (!isReady())
        return;
    
    // re-transfer all markers to the javascript-part:
    d->bgmWidget->runScript(QString("wmwClearMarkers();"));
    for (QIntList::const_iterator it = s->visibleMarkers.constBegin(); it!=s->visibleMarkers.constEnd(); ++it)
    {
        const int currentIndex = *it;
        const WMWMarker& currentMarker = s->markerList.at(currentIndex);

        d->bgmWidget->runScript(QString("wmwAddMarker(%1, %2, %3, %4);")
                .arg(currentIndex)
                .arg(currentMarker.coordinates.latString())
                .arg(currentMarker.coordinates.lonString())
                .arg(currentMarker.isDraggable()?"true":"false")
            );
    }
    
}

void BackendGoogleMaps::slotHTMLEvents(const QStringList& events)
{
    // some events can be buffered:
    // TODO: verify that they are stored in the correct order
    //       or that the order does not matter!
    QMap<QString, QString> bufferedEvents;
    QStringList bufferableEvents;
    bufferableEvents<<"MT"<<"MB"<<"ZC";

    for (QStringList::const_iterator it = events.constBegin(); it!=events.constEnd(); ++it)
    {
        const QString eventCode = it->left(2);
        const QString eventParameter = it->mid(2);
        const QStringList eventParameters = eventParameter.split('/');

        if (bufferableEvents.contains(eventCode))
        {
            bufferedEvents[eventCode] = eventParameter;
        }
        else if (eventCode=="mm")
        {
            // TODO: buffer this event type!
            // marker moved
            bool okay = false;
            const int markerIndex = eventParameter.toInt(&okay);
            if (!okay)
                continue;

            if ((markerIndex<0)||(markerIndex>s->markerList.size()))
                continue;

            // re-read the marker position:
            WMWGeoCoordinate markerCoordinates;
            const bool isValid = googleVariantToCoordinates(
                d->bgmWidget->runScript(QString("wmwGetMarkerPosition(%1);").arg(markerIndex)),
                                                            &markerCoordinates);

            if (!isValid)
                continue;

            // TODO: this discards the altitude!
            s->markerList[markerIndex].coordinates = markerCoordinates;

            emit(signalMarkerMoved(markerIndex));
        }
        else if (eventCode=="do")
        {
            // debug output:
            kDebug()<<QString("javascript:%1").arg(eventParameter);
        }
    }

    // now process the buffered events:
    for (QMap<QString, QString>::const_iterator it = bufferedEvents.constBegin(); it!=bufferedEvents.constEnd(); ++it)
    {
        const QString eventCode = it.key();
        const QString eventParameter = it.value();

        if (eventCode=="MT")
        {
            // map type changed
            slotMapTypeChanged(eventParameter);
        }
        else if (eventCode == "MB")
        {
            // map bounds changed
            slotMapBoundsChanged();
        }
        else if (eventCode == "ZC")
        {
            // zoom changed
            slotZoomChanged();
        }
    }
}

void BackendGoogleMaps::updateClusters()
{
    // re-transfer the clusters to the map:
    WMW2_ASSERT(isReady());
    if (!isReady())
        return;

    // re-transfer all markers to the javascript-part:
    d->bgmWidget->runScript(QString("wmwClearClusters();"));
    for (int currentIndex = 0; currentIndex<s->clusterList.size(); ++currentIndex)
    {
        const WMWCluster& currentCluster = s->clusterList.at(currentIndex);

        d->bgmWidget->runScript(QString("wmwAddCluster(%1, %2, %3, %4);")
                .arg(currentIndex)
                .arg(currentCluster.coordinates.latString())
                .arg(currentCluster.coordinates.lonString())
                .arg(true?"true":"false") // TODO: just for now, for testing
            );
    }
    
}

bool BackendGoogleMaps::screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point)
{
    if (!d->isReady)
        return false;

    const bool isValid = googleVariantToPoint(d->bgmWidget->runScript(
            QString("wmwLatLngToPixel(%1, %2);")
                .arg(coordinates.latString())
                .arg(coordinates.lonString())
                ),
            point);

    // TODO: apparently, even points outside the visible area are returned as valid
    // check whether they are actually visible
    return isValid;
}

bool BackendGoogleMaps::geoCoordinates(const QPoint point, WMWGeoCoordinate* const coordinates) const
{
    if (!d->isReady)
        return false;

    const QVariant coordinatesVariant = d->bgmWidget->runScript(
            QString("wmwPixelToLatLng(%1, %2);")
                .arg(point.x())
                .arg(point.y()));
    const bool isValid = googleVariantToCoordinates(coordinatesVariant, coordinates);

    return isValid;
}

QSize BackendGoogleMaps::mapSize() const
{
    WMW2_ASSERT(d->bgmWidgetWrapper!=0);

    return d->bgmWidgetWrapper->size();
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

    d->bgmWidget->runScript(QString("wmwSetShowScaleControl(%1);").arg(state?"true":"false"));
}

void BackendGoogleMaps::setShowNavigationControl(const bool state)
{
    d->cacheShowNavigationControl = state;

    if (d->showNavigationControlAction)
        d->showNavigationControlAction->setChecked(state);

    if (!isReady())
        return;

    d->bgmWidget->runScript(QString("wmwSetShowNavigationControl(%1);").arg(state?"true":"false"));
}

void BackendGoogleMaps::setShowMapTypeControl(const bool state)
{
    d->cacheShowMapTypeControl = state;

    if (d->showMapTypeControlAction)
        d->showMapTypeControlAction->setChecked(state);

    if (!isReady())
        return;

    d->bgmWidget->runScript(QString("wmwSetShowMapTypeControl(%1);").arg(state?"true":"false"));
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
        d->bgmWidget->runScript(QString("wmwSetZoom(%1);").arg(d->cacheZoom));
    }
}

QString BackendGoogleMaps::getZoom() const
{
    if (isReady())
    {
        d->cacheZoom = d->bgmWidget->runScript(QString("wmwGetZoom();")).toInt();
    }

    return QString("googlemaps:%1").arg(d->cacheZoom);
}

} /* WMW2 */

