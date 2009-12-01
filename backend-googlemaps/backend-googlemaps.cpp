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
      bgmWidgetWrapper(0)
    {
    }

    QPointer<BGMWidget> bgmWidget;
    QPointer<QWidget> bgmWidgetWrapper;
};

BackendGoogleMaps::BackendGoogleMaps(QObject* const parent)
: MapBackend(parent), d(new BackendGoogleMapsPrivate())
{
    d->bgmWidgetWrapper = new QWidget();
    d->bgmWidgetWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->bgmWidgetWrapper->resize(256,256);
    d->bgmWidget = new BGMWidget(d->bgmWidgetWrapper);
    d->bgmWidgetWrapper->resize(256,256);
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

} /* WMW2 */

