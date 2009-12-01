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
      stackedLayout(0)
    {
    }
    
    QList<MapBackend*> loadedBackends;
    MapBackend* currentBackend;
    QStackedLayout* stackedLayout;

};

WorldMapWidget2::WorldMapWidget2(QWidget* const parent)
: QWidget(parent), d(new WorldMapWidget2Private)
{
    d->stackedLayout = new QStackedLayout(this);
    setLayout(d->stackedLayout);

    d->loadedBackends.append(new BackendMarble(this));
    d->loadedBackends.append(new BackendGoogleMaps(this));
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
    MapBackend* backend;
    foreach(backend, d->loadedBackends)
    {
        if (backend->backendName() == backendName)
        {
            kDebug()<<QString("setting backend %1").arg(backendName);
            d->currentBackend = backend;

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
            return true;
        }
    }

    return false;
}
    
} /* WMW2 */

