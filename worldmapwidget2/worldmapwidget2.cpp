/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : WorldMapWidget2
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

#include <QMenu>
#include <QPointer>
#include <QStackedLayout>
#include <QToolButton>

// KDE includes

#include <kaction.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <khbox.h>
#include <klocale.h>

// local includes

#include "worldmapwidget2.h"
#include "map-backend.h"
#include "backend-marble.h"
#include "backend-googlemaps.h"

namespace WMW2 {

class WorldMapWidget2Private
{
public:
    WorldMapWidget2Private()
    : loadedBackends(),
      currentBackend(0),
      currentBackendReady(false),
      currentBackendName(),
      stackedLayout(0),
      actionConfigurationMenu(0),
      actionZoomIn(0),
      actionZoomOut(0),
      controlWidget(0)
    {
    }
    
    QList<MapBackend*> loadedBackends;
    MapBackend* currentBackend;
    bool currentBackendReady;
    QString currentBackendName;
    QStackedLayout* stackedLayout;

    // these values are cached in case the backend is not ready:
    WMWGeoCoordinate cacheCenterCoordinate;

    // actions for controlling the widget
    QPointer<KAction> actionConfigurationMenu;
    QPointer<KAction> actionZoomIn;
    QPointer<KAction> actionZoomOut;
    QPointer<QWidget> controlWidget;
};

WorldMapWidget2::WorldMapWidget2(QWidget* const parent)
: QWidget(parent), s(new WMWSharedData), d(new WorldMapWidget2Private)
{
    d->stackedLayout = new QStackedLayout(this);
    setLayout(d->stackedLayout);

    d->loadedBackends.append(new BackendGoogleMaps(s, this));
    d->loadedBackends.append(new BackendMarble(s, this));
}

WorldMapWidget2::~WorldMapWidget2()
{
    // release all widgets:
    for (int i = 0; i<d->stackedLayout->count(); ++i)
    {
        d->stackedLayout->removeWidget(d->stackedLayout->widget(i));
    }
    
    qDeleteAll(d->loadedBackends);
    delete d;
}

QStringList WorldMapWidget2::availableBackends() const
{
    QStringList result;

    MapBackend* backend;
    foreach(backend, d->loadedBackends)
    {
        result.append(backend->backendName());
    }

    return result;
}

bool WorldMapWidget2::setBackend(const QString& backendName)
{
    if (backendName == d->currentBackendName)
        return true;

    saveBackendToCache();

    MapBackend* backend;
    foreach(backend, d->loadedBackends)
    {
        if (backend->backendName() == backendName)
        {
            kDebug()<<QString("setting backend %1").arg(backendName);
            d->currentBackend = backend;
            d->currentBackendName = backendName;
            d->currentBackendReady = false;

            connect(d->currentBackend, SIGNAL(signalBackendReady(const QString&)),
                    this, SLOT(slotBackendReady(const QString&)));

            // call this slot manually in case the backend was ready right away:
            if (d->currentBackend->isReady())
            {
                slotBackendReady(d->currentBackendName);
            }
            else
            {
                rebuildConfigurationMenu();
            }

            return true;
        }
    }

    return false;
}

void WorldMapWidget2::applyCacheToBackend()
{
    if (!d->currentBackendReady)
        return;
    
    setCenter(d->cacheCenterCoordinate);
}

void WorldMapWidget2::saveBackendToCache()
{
    if (!d->currentBackendReady)
        return;

    kDebug()<<1;
    d->cacheCenterCoordinate = getCenter();
}

WMWGeoCoordinate WorldMapWidget2::getCenter() const
{
    if (!d->currentBackendReady)
        return WMWGeoCoordinate();

    return d->currentBackend->getCenter();
}

void WorldMapWidget2::setCenter(const WMWGeoCoordinate& coordinate)
{
    d->cacheCenterCoordinate = coordinate;

    if (!d->currentBackendReady)
        return;

    d->currentBackend->setCenter(coordinate);
}

void WorldMapWidget2::slotBackendReady(const QString& backendName)
{
    kDebug()<<QString("backend %1 is ready!").arg(backendName);
    if (backendName != d->currentBackendName)
        return;

    d->currentBackendReady = true;

    QWidget* const currentMapWidget = d->currentBackend->mapWidget();
    bool foundWidget = false;
    for (int i = 0; i<d->stackedLayout->count(); ++i)
    {
        if (d->stackedLayout->widget(i) == currentMapWidget)
        {
            d->stackedLayout->setCurrentIndex(i);
            foundWidget = true;
        }
    }
    if (!foundWidget)
    {
        const int newIndex = d->stackedLayout->addWidget(currentMapWidget);
        d->stackedLayout->setCurrentIndex(newIndex);
    }

    applyCacheToBackend();

    updateMarkers();

    rebuildConfigurationMenu();
}

void WorldMapWidget2::saveSettingsToGroup(KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
    if (!group)
        return;

    if (!d->currentBackendName.isEmpty())
    {
        group->writeEntry("Backend", d->currentBackendName);
    }
    group->writeEntry("Center", getCenter().geoUrl());

    for (int i=0; i<d->loadedBackends.size(); ++i)
    {
        d->loadedBackends.at(i)->saveSettingsToGroup(group);
    }
}

void WorldMapWidget2::readSettingsFromGroup(const KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
    if (!group)
        return;

    const QString alternativeBackendName = d->loadedBackends.isEmpty() ? "" : d->loadedBackends.first()->backendName();
    setBackend(group->readEntry("Backend", alternativeBackendName));

    const WMWGeoCoordinate centerDefault = WMWGeoCoordinate(52.0, 6.0);
    const QString centerGeoUrl = group->readEntry("Center", centerDefault.geoUrl());
    bool centerGeoUrlValid = false;
    const WMWGeoCoordinate centerCoordinate = WMWGeoCoordinate::fromGeoUrl(centerGeoUrl, &centerGeoUrlValid);
    setCenter(centerGeoUrlValid ? centerCoordinate : centerDefault);

    for (int i=0; i<d->loadedBackends.size(); ++i)
    {
        d->loadedBackends.at(i)->readSettingsFromGroup(group);
    }
}

void WorldMapWidget2::rebuildConfigurationMenu()
{
    if (!d->actionConfigurationMenu)
    {
        d->actionConfigurationMenu = new KAction(this);
    }

    QMenu* configurationMenu = d->actionConfigurationMenu->menu();
    if (!configurationMenu)
    {
        // TODO: will the menu be deleted when the action is deleted?
        configurationMenu = new QMenu(this);
        d->actionConfigurationMenu->setMenu(configurationMenu);
    }
    else
    {
        configurationMenu->clear();
    }

    // create backend selection entries:
    QActionGroup* const backendActionGroup = new QActionGroup(configurationMenu);
    backendActionGroup->setExclusive(true);
    for (int i = 0; i<d->loadedBackends.size(); ++i)
    {
        const QString backendName = d->loadedBackends.at(i)->backendName();
        KAction* backendAction = new KAction(backendActionGroup);
        backendAction->setData(backendName);
        backendAction->setText(d->loadedBackends.at(i)->backendHumanName());
        backendAction->setCheckable(true);
        
        if (backendName==d->currentBackendName)
        {
            backendAction->setChecked(true);
        }

        configurationMenu->addAction(backendAction);
    }

    if (d->currentBackendReady)
    {
        d->currentBackend->addActionsToConfigurationMenu(configurationMenu);
    }

    connect(backendActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotChangeBackend(QAction*)));
}

KAction* WorldMapWidget2::getControlAction(const QString& actionName)
{
    if (actionName=="configuration")
    {
        bool needToRebuildMenu = !d->actionConfigurationMenu;
        if (!needToRebuildMenu)
        {
            needToRebuildMenu = !d->actionConfigurationMenu->menu();
        }
        if (needToRebuildMenu)
        {
            rebuildConfigurationMenu();
        }
        return d->actionConfigurationMenu;
    }
    else if (actionName=="zoomin")
    {
        if (!d->actionZoomIn)
        {
            d->actionZoomIn = new KAction(this);
            d->actionZoomIn->setIcon(SmallIcon("zoom-in"));
            d->actionZoomIn->setToolTip(i18n("Zoom in"));
            connect(d->actionZoomIn, SIGNAL(triggered()),
                    this, SLOT(slotZoomIn()));
        }
        return d->actionZoomIn;
    }
    else if (actionName=="zoomout")
    {
        if (!d->actionZoomOut)
        {
            d->actionZoomOut = new KAction(this);
            d->actionZoomOut->setIcon(SmallIcon("zoom-out"));
            d->actionZoomOut->setToolTip(i18n("Zoom out"));
            connect(d->actionZoomOut, SIGNAL(triggered()),
                    this, SLOT(slotZoomOut()));
        }
        return d->actionZoomOut;
    }

    return 0;
}

QWidget* WorldMapWidget2::getControlWidget()
{
    if (!d->controlWidget)
    {
        d->controlWidget = new KHBox(this);

        QToolButton* const configurationButton = new QToolButton(d->controlWidget);
        configurationButton->setToolTip(i18n("Map settings"));
        configurationButton->setIcon(SmallIcon("applications-internet"));
        configurationButton->setMenu(getControlAction("configuration")->menu());
        configurationButton->setPopupMode(QToolButton::InstantPopup);

        QToolButton* const zoomInButton = new QToolButton(d->controlWidget);
        zoomInButton->setDefaultAction(getControlAction("zoomin"));

        QToolButton* const zoomOutButton = new QToolButton(d->controlWidget);
        zoomOutButton->setDefaultAction(getControlAction("zoomout"));

        QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(d->controlWidget->layout());
        if (hBoxLayout)
        {
            hBoxLayout->addStretch();
        }
    }

    return d->controlWidget;
}

void WorldMapWidget2::slotZoomIn()
{
    if (!d->currentBackendReady)
        return;
    
    d->currentBackend->zoomIn();
}

void WorldMapWidget2::slotZoomOut()
{
    if (!d->currentBackendReady)
        return;

    d->currentBackend->zoomOut();
}

void WorldMapWidget2::slotUpdateActionsEnabled()
{
    
}

void WorldMapWidget2::slotChangeBackend(QAction* action)
{
    WMW2_ASSERT(action!=0);

    if (!action)
        return;

    const QString newBackendName = action->data().toString();
    setBackend(newBackendName);
}

void WorldMapWidget2::addMarkers(const WMWMarker::List& markerList)
{
    const int oldMarkerCount = s->markerList.count();
    s->markerList << markerList;

    // TODO: clustering
    for (int i=0; i<markerList.count(); ++i)
    {
        s->visibleMarkers << oldMarkerCount + i;
    }

    updateMarkers();
}

void WorldMapWidget2::updateMarkers()
{
    if (!d->currentBackendReady)
        return;

    // tell the backend to update the markers
    d->currentBackend->updateMarkers();
}

} /* WMW2 */

