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

#include <QStackedLayout>

// KDE includes

#include <kdebug.h>

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
      currentlyActivatingBackendName(),
      currentlyActivatingBackend(0),
      stackedLayout(0)
    {
    }
    
    QList<MapBackend*> loadedBackends;
    MapBackend* currentBackend;
    bool currentBackendReady;
    QString currentBackendName;
    QString currentlyActivatingBackendName;
    MapBackend* currentlyActivatingBackend;
    QStackedLayout* stackedLayout;

    // these values are cached in case the backend is not ready:
    WMWGeoCoordinate cacheCenterCoordinate;

};

WorldMapWidget2::WorldMapWidget2(QWidget* const parent)
: QWidget(parent), d(new WorldMapWidget2Private)
{
    d->stackedLayout = new QStackedLayout(this);
    setLayout(d->stackedLayout);

    d->loadedBackends.append(new BackendGoogleMaps(this));
    d->loadedBackends.append(new BackendMarble(this));
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
            d->currentlyActivatingBackend = backend;
            d->currentlyActivatingBackendName = backendName;
            d->currentBackend = 0;
            d->currentBackendName.clear();
            d->currentBackendReady = false;

            connect(d->currentlyActivatingBackend, SIGNAL(signalBackendReady(const QString&)),
                    this, SLOT(slotBackendReady(const QString&)));

            // call this slot manually in case the backend was ready right away:
            if (d->currentlyActivatingBackend->isReady())
                slotBackendReady(d->currentlyActivatingBackendName);
            
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
    if (backendName != d->currentlyActivatingBackendName)
        return;
    
    d->currentBackendReady = true;
    d->currentBackend = d->currentlyActivatingBackend;
    d->currentBackendName = d->currentlyActivatingBackendName;
    d->currentlyActivatingBackendName.clear();
    d->currentlyActivatingBackend = 0;

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
}

} /* WMW2 */

