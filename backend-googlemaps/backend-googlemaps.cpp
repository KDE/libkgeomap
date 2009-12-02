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

#include <QPointer>
#include <QTimer>

// KDE includes

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
      isReady(false)
    {
    }

    QPointer<BGMWidget> bgmWidget;
    QPointer<QWidget> bgmWidgetWrapper;
    bool isReady;
};

BackendGoogleMaps::BackendGoogleMaps(QObject* const parent)
: MapBackend(parent), d(new BackendGoogleMapsPrivate())
{
    d->bgmWidgetWrapper = new QWidget();
    d->bgmWidgetWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->bgmWidget = new BGMWidget(d->bgmWidgetWrapper);
    d->bgmWidgetWrapper->resize(400,400);

    connect(d->bgmWidget, SIGNAL(completed()),
            this, SLOT(slotHTMLInitialized()));

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

QWidget* BackendGoogleMaps::mapWidget() const
{
//     return new QWidget();
    return d->bgmWidgetWrapper.data();
}

WMWGeoCoordinate BackendGoogleMaps::getCenter() const
{
    QVariant theCenter = d->bgmWidget->executeScript("wmwGetCenter();");
    bool valid = ( theCenter.type()==QVariant::String );
    if (valid)
    {
        QStringList coordinateStrings = theCenter.toString().split(',');
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
                return WMWGeoCoordinate(ptLatitude, ptLongitude);
            }
        }
    }

    return WMWGeoCoordinate();
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
    emit(signalBackendReady(backendName()));
}


} /* WMW2 */

