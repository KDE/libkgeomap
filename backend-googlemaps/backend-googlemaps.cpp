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

namespace WMW2 {

class BackendGoogleMapsPrivate
{
public:
    BackendGoogleMapsPrivate()
    : bgmWidget(0),
      bgmWidgetWrapper(0),
      isReady(false),
      mapTypeActionGroup(0),
      cacheMapType("ROADMAP")
    {
    }

    QPointer<BGMWidget> bgmWidget;
    QPointer<QWidget> bgmWidgetWrapper;
    bool isReady;
    QPointer<QActionGroup> mapTypeActionGroup;

    QString cacheMapType;
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

WMWGeoCoordinate BackendGoogleMaps::getCenter() const
{
    WMWGeoCoordinate centerCoordinates;

    // there is nothing we can do if the coordinates are invalid
    /*const bool isValid = */googleVariantToCoordinates(d->bgmWidget->executeScript("wmwGetCenter();"), &centerCoordinates);

    return centerCoordinates;
}

void BackendGoogleMaps::setCenter(const WMWGeoCoordinate& coordinate)
{
    d->bgmWidget->executeScript(QString("wmwSetCenter(%1, %2);").arg(coordinate.latString()).arg(coordinate.lonString()));
}

bool BackendGoogleMaps::isReady() const
{
    return d->isReady;
}

void BackendGoogleMaps::slotHTMLInitialized()
{
    kDebug()<<1;
    d->isReady = true;
    d->bgmWidget->executeScript(QString("document.getElementById(\"map_canvas\").style.height=\"%1px\"").arg(d->bgmWidgetWrapper->height()));
    setMapType(d->cacheMapType);
    emit(signalBackendReady(backendName()));
}

void BackendGoogleMaps::zoomIn()
{
    if (!d->isReady)
        return;
    
    d->bgmWidget->executeScript(QString("wmwZoomIn();"));
}

void BackendGoogleMaps::zoomOut()
{
    if (!d->isReady)
        return;

    d->bgmWidget->executeScript(QString("wmwZoomOut();"));
}

QString BackendGoogleMaps::getMapType() const
{
    if (isReady())
    {
        d->cacheMapType = d->bgmWidget->executeScript(QString("wmwGetMapType();")).toString();
    }

    return d->cacheMapType;
}

void BackendGoogleMaps::setMapType(const QString& newMapType)
{
    d->cacheMapType = newMapType;
    kDebug()<<newMapType;

    if (isReady())
    {
        d->bgmWidget->executeScript(QString("wmwSetMapType(\"%1\");").arg(newMapType));
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
    Q_ASSERT(configurationMenu!=0);

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
    d->mapTypeActionGroup = new QActionGroup(this);
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
    
}

void BackendGoogleMaps::saveSettingsToGroup(KConfigGroup* const group)
{
    Q_ASSERT(group != 0);
    if (!group)
        return;

    group->writeEntry("GoogleMaps Map Type", getMapType());
}

void BackendGoogleMaps::readSettingsFromGroup(const KConfigGroup* const group)
{
    Q_ASSERT(group != 0);
    if (!group)
        return;

    const QString mapType = group->readEntry("GoogleMaps Map Type", "ROADMAP");
    setMapType(mapType);
}

void BackendGoogleMaps::slotMapTypeChanged(const QString& newMapType)
{
    d->cacheMapType = newMapType;

    updateActionsEnabled();
}

void BackendGoogleMaps::updateMarkers()
{
    Q_ASSERT(isReady());
    if (!isReady())
        return;
    
    // re-transfer all markers to the javascript-part:
    d->bgmWidget->executeScript(QString("wmwClearMarkers();"));
    for (QIntList::const_iterator it = s->visibleMarkers.constBegin(); it!=s->visibleMarkers.constEnd(); ++it)
    {
        const int currentIndex = *it;
        const WMWMarker& currentMarker = s->markerList.at(currentIndex);

        d->bgmWidget->executeScript(QString("wmwAddMarker(%1, %2, %3, %4);")
                .arg(currentIndex)
                .arg(currentMarker.coordinates.latString())
                .arg(currentMarker.coordinates.lonString())
                .arg(currentMarker.isDraggable()?"true":"false")
            );
    }
    
}

void BackendGoogleMaps::slotHTMLEvents(const QStringList& events)
{
    for (QStringList::const_iterator it = events.constBegin(); it!=events.constEnd(); ++it)
    {
        const QString eventCode = it->left(2);
        const QString eventParameter = it->mid(2);
        const QStringList eventParameters = eventParameter.split('/');

        if (eventCode=="MT")
        {
            // map type changed
            slotMapTypeChanged(eventParameter);
        }
        else if (eventCode=="mm")
        {
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
                d->bgmWidget->executeScript(QString("wmwGetMarkerPosition(%1);").arg(markerIndex)),
                                                            &markerCoordinates);

            if (!isValid)
                continue;

            // TODO: this discards the altitude!
            s->markerList[markerIndex].coordinates = markerCoordinates;

            emit(signalMarkerMoved(markerIndex));
        }
    }
}

} /* WMW2 */

